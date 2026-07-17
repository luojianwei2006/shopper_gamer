#include "BaseSocketSubsystem.h"

#include "Tickable.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Async/Async.h"
#include "Misc/Timespan.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"

DEFINE_LOG_CATEGORY(LogShopperSocket);

// ───────────────────────────────────────────────────────────
// 生命周期
// ───────────────────────────────────────────────────────────
void UBaseSocketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitFrameConfig(FrameConfig);
	CachedLogTag = GetLogTag();   // 缓存日志前缀：后续后台读线程断连日志可安全引用，避免跨线程虚调用

	// 启动后台读线程（常驻，切换关卡不会销毁）
	Reader = MakeUnique<FSocketReader>(this);
	ReaderThread = FRunnableThread::Create(
		Reader.Get(), *FString::Printf(TEXT("%sReader"), *GetLogTag()), 0, TPri_Normal);

	// 心跳 / 重连驱动器：用 FTickableGameObject 自驱动（每帧游戏线程 Tick，不依赖 UWorld/关卡，
	// 且保证稳定触发）。内部用 FPlatformTime 真实时钟累计，避免 FTSTicker 在某些环境不重触发 / DeltaTime=0。
	SocketTicker = MakeUnique<FSocketTicker>();
	SocketTicker->Owner = this;
	UE_LOG(LogShopperSocket, Log,
		TEXT("[%s] ✅ 已注册心跳/重连驱动器（FTickableGameObject，每帧驱动）；子类心跳间隔=%.1fs%s"),
		*GetLogTag(), GetHeartbeatInterval(),
		GetHeartbeatInterval() > 0.f ? TEXT("") : TEXT("（0=不发心跳）"));

	// 自动建链（子类 ShouldAutoConnect 控制）
	if (ShouldAutoConnect())
	{
		ConnectInternal();
	}
}

void UBaseSocketSubsystem::Deinitialize()
{
	bShuttingDown = true;   // 阻止仍在排队的消息下发

	// 停驱动器（销毁 FTickableGameObject 即自动从引擎可 Tick 列表注销）
	SocketTicker.Reset();

	// 先通知线程退出、关 socket 解除 Recv 阻塞，再等待线程结束
	if (ReaderThread)
	{
		Reader->Stop();            // bRun = false
		CloseSocketAndNotify();    // 销毁 socket，让阻塞的 Recv 立即返回
		ReaderThread->Kill(true);  // 等待读线程退出
		ReaderThread = nullptr;
	}
	Reader.Reset();

	Super::Deinitialize();
}

// ───────────────────────────────────────────────────────────
// 对外 API
// ───────────────────────────────────────────────────────────
void UBaseSocketSubsystem::Connect(const FString& IpAddress, int32 Port)
{
	bManualClosed = false;
	ConnectInternal(&IpAddress, &Port);
}

void UBaseSocketSubsystem::Disconnect()
{
	bManualClosed = true;       // 主动断开后停止自动重连，直到再次 Connect()
	CloseSocketAndNotify();
}

bool UBaseSocketSubsystem::SendMessage(int32 Protocol, const TArray<uint8>& Data)
{
	if (Protocol < 0 || Protocol > 65535)
	{
		UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 协议号越界 %d（需 0~65535）"), *GetLogTag(), Protocol);
		return false;
	}

	// 帧长度计算（与收包端 ReadField + length 语义完全对称）：
	//   HeaderSize       = ProtocolBytes + LengthBytes
	//   PayloadAndMd5    = data 长度 + (md5?1:0)
	//   Len = 整帧长度（bLengthIncludesHeader） 或 data 长度（不含帧头）
	const int32 HeaderSize = FrameConfig.ProtocolBytes + FrameConfig.LengthBytes;
	const int32 PayloadAndMd5 = Data.Num() + (FrameConfig.bUseFrameChecksum ? 1 : 0);
	const int32 Len = FrameConfig.bLengthIncludesHeader ? (HeaderSize + PayloadAndMd5) : PayloadAndMd5;

	// 组帧：protocol + length（按字段宽度/字节序写入）+ data [+ md5(单字节)]
	TArray<uint8> Frame;
	Frame.Reserve(HeaderSize + PayloadAndMd5);
	AppendField(Frame, Protocol, FrameConfig.ProtocolBytes, FrameConfig.bLittleEndian);
	AppendField(Frame, Len, FrameConfig.LengthBytes, FrameConfig.bLittleEndian);
	Frame.Append(Data);
	if (FrameConfig.bUseFrameChecksum)
	{
		// 校验和只对 protobuf 负载本身计算（Data.Num() 字节）。
		// 注意：绝不能传整帧长度，否则会在 Data 缓冲区之后越界读堆内存；
		// 与接收侧“对 Remaining-1 字节 proto 负载算校验”对称。
		Frame.Add(ComputeChecksum(Data.Num() > 0 ? Data.GetData() : nullptr, Data.Num()));
	}

	// 调试：dump 发出的负载字节，便于和服务端逐字节比对
	UE_LOG(LogShopperSocket, Log, TEXT("[%s] SendMessage 协议号=%d，长度=%d，md5=%s"),
		*GetLogTag(), Protocol, Len, FrameConfig.bUseFrameChecksum ? TEXT("on") : TEXT("off"));
	const int32 DumpLen = Data.Num();
	if (DumpLen > 0 && DumpLen <= 512)
	{
		FString Hex;
		for (int32 i = 0; i < DumpLen; ++i)
		{
			Hex += FString::Printf(TEXT("%02X "), Data[i]);
		}
		UE_LOG(LogShopperSocket, Log, TEXT("[%s] 发出负载字节: %s"), *GetLogTag(), *Hex);
	}

	return SendRaw(Frame);
}

// ───────────────────────────────────────────────────────────
// 内部实现
// ───────────────────────────────────────────────────────────
bool UBaseSocketSubsystem::ConnectInternal(const FString* IpOverride, int32* PortOverride)
{
	if (bConnected || bConnecting)
	{
		return false;
	}
	bConnecting = true;

	// 解析目标地址端口：显式参数优先 → 复用上一次连接地址 → 回退到 GetDefaultEndpoint
	FString Host;
	int32 Port = 0;
	if (IpOverride && PortOverride)
	{
		Host = *IpOverride;
		Port = *PortOverride;
		CachedHost = Host;
		CachedPort = Port;
		bHasCachedEndpoint = true;
	}
	else if (bHasCachedEndpoint)
	{
		Host = CachedHost;
		Port = CachedPort;
	}
	else
	{
		GetDefaultEndpoint(Host, Port);
		if (!Host.IsEmpty() && Port > 0)
		{
			CachedHost = Host;
			CachedPort = Port;
			bHasCachedEndpoint = true;
		}
	}

	if (Port <= 0 || Port > 65535)
	{
		UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 端口 %d 非法（需 1~65535）"), *GetLogTag(), Port);
		bConnecting = false;
		return false;
	}

	ISocketSubsystem* SSS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SSS)
	{
		bConnecting = false;
		return false;
	}

	FIPv4Address Addr;
	if (!FIPv4Address::Parse(Host, Addr))
	{
		UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 无法解析主机 %s，请填 IP"), *GetLogTag(), *Host);
		bConnecting = false;
		return false;
	}
	const FIPv4Endpoint Endpoint(Addr, static_cast<uint16>(Port));

	FSocket* NewSock = SSS->CreateSocket(NAME_Stream, *FString::Printf(TEXT("%sSock"), *GetLogTag()), false);
	if (!NewSock)
	{
		UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 创建 socket 失败"), *GetLogTag());
		bConnecting = false;
		return false;
	}

	// 非阻塞 connect + WaitForWrite 限时，避免无限阻塞
	NewSock->SetNonBlocking(true);
	NewSock->Connect(*Endpoint.ToInternetAddr());
	const bool bWritable = NewSock->Wait(ESocketWaitConditions::WaitForWrite, FTimespan::FromSeconds(5.0));
	if (!bWritable)
	{
		UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 连接 %s 超时"), *GetLogTag(), *Endpoint.ToString());
		SSS->DestroySocket(NewSock);
		bConnecting = false;
		return false;
	}
	// 连上后恢复阻塞模式，Recv 用 WaitAll 收满整包
	NewSock->SetNonBlocking(false);

	{
		FScopeLock Lock(&SocketMutex);
		Socket = NewSock;
	}
	bConnected  = true;
	bConnecting = false;
	bManualClosed = false;

	UE_LOG(LogShopperSocket, Log, TEXT("[%s] 已连接 %s"), *GetLogTag(), *Endpoint.ToString());
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		OnConnectionChanged.Broadcast(true);
	});
	return true;
}

void UBaseSocketSubsystem::CloseSocketAndNotify()
{
	FSocket* SockToClose = nullptr;
	{
		FScopeLock Lock(&SocketMutex);
		SockToClose = Socket;
		Socket = nullptr;
	}
	if (SockToClose)
	{
		if (ISocketSubsystem* SSS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
		{
			SSS->DestroySocket(SockToClose);
		}
	}

	const bool bWas = bConnected;
	bConnected = false;
	UE_LOG(LogShopperSocket, Log, TEXT("[%s] 连接断开（CloseSocketAndNotify），原 bConnected=%s"),
		*CachedLogTag, bWas ? TEXT("true") : TEXT("false"));
	if (bWas)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnConnectionChanged.Broadcast(false);
		});
	}
}

bool UBaseSocketSubsystem::SendRaw(const TArray<uint8>& Frame)
{
	// 全程持锁，避免与 CloseSocketAndNotify 的 Destroy 产生竞态；
	// 这里不再调用 Close（否则会重入同一锁造成死锁），发送失败直接返回 false，
	// 由读线程的 Recv 失败或重连逻辑接管。
	FScopeLock Lock(&SocketMutex);
	if (!Socket)
	{
		return false;
	}
	int32 Total  = Frame.Num();
	int32 Offset = 0;
	while (Offset < Total)
	{
		int32 Out = 0;
		if (!Socket->Send(Frame.GetData() + Offset, Total - Offset, Out) || Out <= 0)
		{
			return false;
		}
		Offset += Out;
	}
	return true;
}

bool UBaseSocketSubsystem::RecvExact(FSocket* Sock, uint8* Buf, int32 Total)
{
	int32 Got = 0;
	while (Got < Total)
	{
		if (!Sock)
		{
			return false;
		}
		int32 Read = 0;
		// WaitAll：阻塞直到读满或出错/断开
		if (!Sock->Recv(Buf + Got, Total - Got, Read, ESocketReceiveFlags::WaitAll))
		{
			return false;
		}
		if (Read <= 0)
		{
			return false;
		}
		Got += Read;
	}
	return true;
}

uint32 UBaseSocketSubsystem::FSocketReader::Run()
{
	while (bRun)
	{
		FSocket* Sock = nullptr;
		{
			FScopeLock Lock(&Owner->SocketMutex);
			Sock = Owner->Socket;
		}
		if (!Sock)
		{
			FPlatformProcess::Sleep(0.2f);
			continue;
		}

		// 1) 读帧头：protocol(ProtocolBytes) + length(LengthBytes)
		//   必须与发送端一致为 HeaderSize 字节；多读 1 字节会把 data[0] 吞进帧头，
		//   导致后续 protobuf 整体错位 1 字节 → parseFrom 失败。
		const int32 HeaderSize = Owner->FrameConfig.ProtocolBytes + Owner->FrameConfig.LengthBytes;
		TArray<uint8> Header;
		Header.SetNumUninitialized(HeaderSize);
		if (!Owner->RecvExact(Sock, Header.GetData(), HeaderSize))
		{
			Owner->CloseSocketAndNotify();
			continue;
		}

		// 2) 按字节序解析 protocol / length
		const int32 Protocol = static_cast<int32>(
			Owner->ReadField(Header.GetData(), Owner->FrameConfig.ProtocolBytes, Owner->FrameConfig.bLittleEndian));
		const int32 Len = static_cast<int32>(
			Owner->ReadField(Header.GetData() + Owner->FrameConfig.ProtocolBytes,
				Owner->FrameConfig.LengthBytes, Owner->FrameConfig.bLittleEndian));

		if (Len < 0 || Len > Owner->FrameConfig.PacketMaxSize)
		{
			UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 非法包长度 %d，断开重连"), *Owner->GetLogTag(), Len);
			Owner->CloseSocketAndNotify();
			continue;
		}

		// 3) 读 data + md5
		//    约定：length 字段 = 整帧长度（含帧头），与发送端 SendMessage 写入的 Len 完全对称。
		//    帧头已在第 1 步单独读出，故剩余待读字节 = Len - HeaderSize = data + md5。
		const int32 Remaining = Owner->FrameConfig.bLengthIncludesHeader ? (Len - HeaderSize) : Len;
		if (Remaining < 0)
		{
			UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 协议 %d 长度 %d 非法（小于帧头 %d 字节），断开"),
				*Owner->GetLogTag(), Protocol, Len, HeaderSize);
			Owner->CloseSocketAndNotify();
			continue;
		}
		TArray<uint8> Data;
		if (Remaining > 0)
		{
			Data.SetNumUninitialized(Remaining);
			if (!Owner->RecvExact(Sock, Data.GetData(), Remaining))
			{
				Owner->CloseSocketAndNotify();
				continue;
			}
		}

		// 4) md5 校验与剔除
		//    约定：md5 作为 data 块的【最后一个字节】一并下发，length 计数已含该字节。
		//    即 剩余 = [protobuf 负载 (Remaining-1 字节)][md5 (1 字节)]。
		//    剔除末字节后，Data 长度 = Remaining - 1 = 真实 data 字节数。
		if (Owner->FrameConfig.bUseFrameChecksum)
		{
			if (Remaining < 1)
			{
				UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 协议 %d 长度 %d 不足承载 md5，断开"),
					*Owner->GetLogTag(), Protocol, Len);
				Owner->CloseSocketAndNotify();
				continue;
			}
			const uint8 RecvMd5 = Data.Last();
			const uint8 CalcMd5 = Owner->ComputeChecksum(Data.GetData(), Remaining - 1);
			if (CalcMd5 != RecvMd5)
			{
				//UE_LOG(LogShopperSocket, Warning,
				//	TEXT("[%s] 协议 %d md5 不一致 (calc=%d recv=%d)，仍继续解析"), *Owner->GetLogTag(), Protocol, CalcMd5, RecvMd5);
			}
			// 剔除末字节 md5，仅保留纯 protobuf 负载交给解析器
			Data.SetNum(Remaining - 1);
		}

		// 5) 调试日志（Data.Num() 已是剔除 md5 后的纯负载长度）
		//UE_LOG(LogShopperSocket, Log, TEXT("[%s] 收到协议=%d，负载长度=%d，md5=%s"),
		//	*Owner->GetLogTag(), Protocol, Data.Num(),
		//	Owner->FrameConfig.bUseFrameChecksum ? TEXT("on") : TEXT("off"));

		// 6) 抛回游戏线程（bShuttingDown 为真时跳过）
		const int32 P = Protocol;
		AsyncTask(ENamedThreads::GameThread, [this, P, LocalData = MoveTemp(Data)]() mutable
		{
			if (!Owner->bShuttingDown)
			{
				Owner->DispatchPacket(P, MoveTemp(LocalData));
			}
		});
	}
	return 0;
}

void UBaseSocketSubsystem::DispatchPacket(int32 Protocol, TArray<uint8>&& Data)
{
	// 原始字节先广播（便于调用方自建分发），再交给子类按协议号强类型分发
	OnMessageReceived.Broadcast(Protocol, Data);
	HandlePacket(Protocol, MoveTemp(Data));
}

void UBaseSocketSubsystem::SendHeartbeat()
{
	// 心跳内容由子类 BuildHeartbeatPayload 提供（协议号 + 负载），基类负责组帧与发送
	int32 Protocol = 0;
	TArray<uint8> Data;
	BuildHeartbeatPayload(Protocol, Data);
	if (Protocol != 0 || Data.Num() > 0)
	{
		if (!SendMessage(Protocol, Data))
		{
			UE_LOG(LogShopperSocket, Warning, TEXT("[%s] 心跳发送失败（socket 不可用，可能刚断线）"), *CachedLogTag);
		}
	}
	else
	{
		UE_LOG(LogShopperSocket, Warning,
			TEXT("[%s] 心跳钩子未返回有效负载（协议=%d），跳过本次心跳"),
			*GetLogTag(), Protocol);
	}
}

bool UBaseSocketSubsystem::TickHeartbeat(float DeltaTime)
{
	if (!bConnected)
	{
		// 未连接：不累计、不发送；连上后从 0 重新计时。
		// 只在「连→断」跳变时记一次日志（避免每 tick 刷屏），且用 LogShopperSocket 便于过滤。
		if (bHeartbeatHadConnection)
		{
			bHeartbeatHadConnection = false;
			UE_LOG(LogShopperSocket, Log, TEXT("[%s] 心跳暂停：连接已断开，等待重连"), *CachedLogTag);
		}
		HeartbeatAccum = 0.f;
		return true;
	}

	// 断→连恢复后第一次进入：记一次恢复日志
	if (!bHeartbeatHadConnection)
	{
		bHeartbeatHadConnection = true;
		UE_LOG(LogShopperSocket, Log, TEXT("[%s] 心跳恢复：连接已建立，重新开始 %.1f s 计时"),
			*CachedLogTag, GetHeartbeatInterval());
	}

	HeartbeatAccum += DeltaTime;
	const float Interval = GetHeartbeatInterval();
	if (Interval <= 0.f)
	{
		return true;   // 子类返回 0 表示不发送心跳
	}
	if (HeartbeatAccum >= Interval)
	{
		HeartbeatAccum = 0.f;
		SendHeartbeat();   // 定时器到点 → 真正发出心跳
	}
	return true;   // 持续 ticker
}

bool UBaseSocketSubsystem::TickReconnect(float DeltaTime)
{
	if (bManualClosed || bConnected || bConnecting)
	{
		ReconnectAccum = 0.f;
		return true;
	}
	ReconnectAccum += DeltaTime;
	const float Interval = GetReconnectInterval();
	if (ReconnectAccum >= Interval)
	{
		ReconnectAccum = 0.f;
		UE_LOG(LogShopperSocket, Log, TEXT("[%s] 尝试断线重连…"), *CachedLogTag);
		ConnectInternal();
	}
	return true;   // 持续 ticker
}

uint8 UBaseSocketSubsystem::ComputeChecksum(const uint8* Data, int32 Len) const
{
	uint8 C = 0;
	if (Data && Len > 0)
	{
		for (int32 i = 0; i < Len; ++i)
		{
			C ^= Data[i];
		}
	}
	return C;
}

void UBaseSocketSubsystem::AppendField(TArray<uint8>& Out, int64 Value, int32 NumBytes, bool bLittleEndian) const
{
	for (int32 i = 0; i < NumBytes; ++i)
	{
		const int32 Shift = bLittleEndian ? i : (NumBytes - 1 - i);
		Out.Add(static_cast<uint8>((Value >> (Shift * 8)) & 0xFF));
	}
}

int64 UBaseSocketSubsystem::ReadField(const uint8* Buf, int32 NumBytes, bool bLittleEndian) const
{
	int64 V = 0;
	for (int32 i = 0; i < NumBytes; ++i)
	{
		const int32 Shift = bLittleEndian ? i : (NumBytes - 1 - i);
		V |= static_cast<int64>(Buf[i]) << (Shift * 8);
	}
	return V;
}
