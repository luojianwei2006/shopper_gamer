#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "Tickable.h"

// 必须在所有 USTRUCT / UCLASS / DECLARE_DYNAMIC_* 之前 include 本文件的 generated.h：
// X.generated.h 会在其末尾 #define CURRENT_FILE_ID = X，而 GENERATED_BODY / 委托宏依赖这个 id。
// 若放到文件末尾（类声明之后），前面的反射宏会用「上一个被包含头的 CURRENT_FILE_ID」展开 →
// 报 "a type specifier is required" / "unknown type name FID_…"。（本工程不逐文件 force-include generated.h）
#include "BaseSocketSubsystem.generated.h"

class FSocket;

// 通用 Socket 长链接日志分类（替代 LogTemp，便于在 Output Log 中按分类过滤 / 分级）
SHOPPERGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogShopperSocket, Log, All);

/**
 * 通用 Socket 长链接帧格式配置。
 * 不同服务器可能略有差异，子类通过 InitFrameConfig 覆盖（默认即 shopper 服务器约定）。
 * 这些值在 Initialize 时由 InitFrameConfig 读取一次，运行期视为常量。
 */
USTRUCT(BlueprintType)
struct SHOPPERGAME_API FGameSocketFrameConfig
{
	GENERATED_BODY()

	// 协议号字段宽度（字节）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 ProtocolBytes = 2;

	// 长度字段宽度（字节）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 LengthBytes = 4;

	// 小端（Java ByteOrder.LITTLE_ENDIAN）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bLittleEndian = true;

	// length 是否含整个帧头（含协议号 + 长度字段本身）。
	//   true  : length = 整帧长度 → 真实 data = length - (ProtocolBytes+LengthBytes) - (md5?1:0)
	//   false : length = data 长度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bLengthIncludesHeader = true;

	// 帧尾是否带 1 字节校验（md5）。开启时 md5 作为 data 块的【最后一个字节】，length 已含该字节。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bUseFrameChecksum = true;

	// 单包上限（防御畸形包导致 OOM）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 PacketMaxSize = 10 * 1024 * 1024;
};

// 收到服务器下推数据包时广播（游戏线程执行）
//   Protocol : 协议号
//   Data     : 已去掉帧头/校验的纯负载（需由调用方反序列化）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSocketMessage, int32, Protocol, const TArray<uint8>&, Data);

// 连接状态变化广播（连上=true / 断开=false），游戏线程执行
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSocketConnectionChanged, bool, bConnected);

/**
 * 通用 Socket 长链接子系统基类（抽象）。
 * 负责：TCP 连接 / 断线重连 / 心跳 / 后台读线程 / 帧编解码（字节序、length、md5）/ 原始消息广播。
 * 子类只负责：按协议号分发（HandlePacket）、构造各自的发送/解析函数、提供连接与帧配置。
 *
 * 帧格式（默认，可被 InitFrameConfig 覆盖）：
 *   [protocol (ProtocolBytes 字节, 小端)][length (LengthBytes 字节, 小端)]
 *   [+ data 变长][+ 1 字节 md5（bUseFrameChecksum 时，作为 data 块末字节，length 已含该字节）]
 *
 * 设计要点（从 shopper 实战踩坑沉淀，参数化后通用）：
 *   - 帧头长度必须与发送端严格一致：多读 1 字节会把 data[0] 吞进帧头，致 protobuf 整体错位 1 字节。
 *   - length 是整帧长度（含帧头）：真实 data = length - 帧头长度 - (md5?1:0)。
 *   - md5 是 data 块末字节（非独立字节）：先整块读入，再 SetNum(Len-1) 剔除，交给解析器。
 *   - 校验和算法默认 XOR，子类可覆盖 ComputeChecksum 换成服务端算法。
 *
 * 注意：本基类不依赖 protobuf —— 序列化由子类通过 ShopperProto 模块完成并调用 SendMessage。
 */
UCLASS(Abstract, BlueprintType)
class SHOPPERGAME_API UBaseSocketSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ── 生命周期 ──
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 蓝图 / C++ 对外 API（通用）──
	UFUNCTION(BlueprintCallable, Category = "Socket")
	void Connect(const FString& IpAddress, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "Socket")
	void Disconnect();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	bool IsConnected() const { return bConnected; }

	// 发送一个数据包：Protocol 协议号（0~65535），Data 为已序列化（protobuf）的二进制负载
	UFUNCTION(BlueprintCallable, Category = "Socket")
	bool SendMessage(int32 Protocol, const TArray<uint8>& Data);

	// 连接状态变化（连上 / 断开）
	UPROPERTY(BlueprintAssignable, Category = "Socket")
	FOnSocketConnectionChanged OnConnectionChanged;

	// 收到服务器消息（游戏线程，Data 为纯负载，需反序列化）
	UPROPERTY(BlueprintAssignable, Category = "Socket")
	FOnSocketMessage OnMessageReceived;

protected:
	// ── 子类钩子（按需覆盖）──

	// 帧格式配置：默认 FGameSocketFrameConfig{}（小端 / length 含帧头 / md5 末字节）。
	// 子类如游戏服务器帧不同，则在此覆盖（可从各自 Settings 读取）。
	virtual void InitFrameConfig(FGameSocketFrameConfig& OutConfig) const
	{
		OutConfig = FGameSocketFrameConfig{};
	}

	// 启动后是否自动建链（默认 false）
	virtual bool ShouldAutoConnect() const { return false; }

	// 自动建链 / 重连回退地址（显式 Connect(ip,port) 时不用此值）
	virtual void GetDefaultEndpoint(FString& OutHost, int32& OutPort) const {}

	// 心跳间隔（秒）。返回 0 表示不发心跳
	virtual float GetHeartbeatInterval() const { return 0.f; }

	// 断线重连间隔（秒）
	virtual float GetReconnectInterval() const { return 3.f; }

	// 构造心跳包（协议号 + 负载）。默认无操作（不发心跳）
	virtual void BuildHeartbeatPayload(int32& OutProtocol, TArray<uint8>& OutData) {}

	// 日志前缀（区分不同服务器的日志），默认 "Socket"
	virtual FString GetLogTag() const { return TEXT("Socket"); }

	// 收到完整（已去帧头/校验）的数据包时调用，由子类按协议号分流。
	// 注意：OnMessageReceived 已在基类 DispatchPacket 中广播，此处只做协议分发。
	virtual void HandlePacket(int32 Protocol, TArray<uint8>&& Data) {}

	// 计算校验和（可覆盖；默认 XOR 取低 8 位）
	virtual uint8 ComputeChecksum(const uint8* Data, int32 Len) const;

	// 帧格式配置（Initialize 时由 InitFrameConfig 填充）
	FGameSocketFrameConfig FrameConfig;

private:
	// 后台读线程：阻塞读 socket，按帧格式拆分数据包后抛回游戏线程
	class FSocketReader : public FRunnable
	{
	public:
		explicit FSocketReader(UBaseSocketSubsystem* InOwner) : Owner(InOwner) {}
		virtual uint32 Run() override;
		virtual void Stop() override { bRun = false; }
		FThreadSafeBool bRun = true;
		UBaseSocketSubsystem* Owner = nullptr;
	};

	// 自驱动心跳/重连驱动器：继承 FTickableGameObject，由引擎每帧在游戏线程调用 Tick。
	// 相比 FTSTicker：保证每帧稳定触发（FTSTicker 在部分编辑器/PIE 环境下不保证稳定重触发，
	// 实测出现过「只触发首帧、之后不再回调」导致 HeartbeatAccum 永远积累不到间隔值、心跳一次都不发）；
	// 内部用 FPlatformTime 真实时钟算经过时间，避免某些环境下传入 DeltaTime 为 0 致使累计失效。
	class FSocketTicker : public FTickableGameObject
	{
	public:
		FSocketTicker() = default;
		virtual ~FSocketTicker() = default;

		virtual void Tick(float DeltaTime) override
		{
			if (!Owner.IsValid())
			{
				return;
			}
			// 用真实时钟算本帧经过时间（限幅 5s，防止卡顿后一次性补发一堆心跳）
			const double Now = FPlatformTime::Seconds();
			if (LastTime <= 0.0)
			{
				LastTime = Now;
			}
			const float Elapsed = static_cast<float>(FMath::Min(Now - LastTime, 5.0));
			LastTime = Now;

			Owner->TickHeartbeat(Elapsed);
			Owner->TickReconnect(Elapsed);
		}

		virtual TStatId GetStatId() const override
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FBaseSocketTicker, STATGROUP_Tickables);
		}

		virtual bool IsTickable() const override { return Owner.IsValid(); }

		TWeakObjectPtr<UBaseSocketSubsystem> Owner;
		double LastTime = 0.0;
	};

	// 底层 TCP 连接（游戏线程）。IpOverride/PortOverride 非空→连接指定地址（并缓存供重连复用）；
	// 为空→复用上一次连接地址；若从未连过则回退到 GetDefaultEndpoint。
	bool ConnectInternal(const FString* IpOverride = nullptr, int32* PortOverride = nullptr);

	void CloseSocketAndNotify();
	bool SendRaw(const TArray<uint8>& Frame);
	bool RecvExact(FSocket* Sock, uint8* Buf, int32 Total);
	bool TickHeartbeat(float DeltaTime);
	bool TickReconnect(float DeltaTime);

	// 定时器（TickHeartbeat）到点后真正发出心跳：调用子类 BuildHeartbeatPayload 取得
	// （协议号 + 负载），再经 SendMessage 组帧发送。基类不关心心跳内容，只负责发送动作。
	void SendHeartbeat();

	// 游戏线程收到完整数据包：广播 OnMessageReceived，再调用虚钩子 HandlePacket
	void DispatchPacket(int32 Protocol, TArray<uint8>&& Data);

	// 按字节序把整数写进帧 / 从帧读出（支持任意字段宽度，兼容不同服务器）
	void AppendField(TArray<uint8>& Out, int64 Value, int32 NumBytes, bool bLittleEndian) const;
	int64 ReadField(const uint8* Buf, int32 NumBytes, bool bLittleEndian) const;

	FSocket*           Socket       = nullptr;   // 受 SocketMutex 保护
	FRunnableThread*   ReaderThread = nullptr;
	TUniquePtr<FSocketReader> Reader;
	FCriticalSection   SocketMutex;

	FThreadSafeBool bConnected  = false;
	FThreadSafeBool bConnecting = false;
	bool           bManualClosed = false;       // 主动 Disconnect 后停止自动重连
	FThreadSafeBool bShuttingDown = false;       // Deinitialize 时置位，阻止已排队的消息下发

	// 最近一次连接地址（显式 Connect 或自动连接都会写入），重连必须复用同一地址。
	FString CachedHost;
	int32  CachedPort = 0;
	bool   bHasCachedEndpoint = false;

	FString CachedLogTag;   // 缓存日志前缀，供后台读线程安全使用（避免跨线程调用 UObject 虚函数）

	float HeartbeatAccum = 0.f;
	float ReconnectAccum = 0.f;
	bool   bHeartbeatHadConnection = false;   // 心跳「连→断」跳变记忆，用于只在跳变时打日志

	TUniquePtr<FSocketTicker> SocketTicker;    // 自驱动心跳/重连（FTickableGameObject），销毁即自动注销
};
