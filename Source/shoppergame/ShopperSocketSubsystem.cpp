#include "ShopperSocketSubsystem.h"

#include "ShopperSettings.h"

// 本文件【不再】直接 include 任何 protobuf 头。所有 protobuf 消息的构造/解析都已封装进
// ShopperProto 模块（ProtoSpeaker.h 的 Proto_Build*/Proto_Parse*），因为 UE 模块默认隐藏
// 非 SHOPPERPROTO_API 符号，shoppergame 直接引用 protobuf 生成的类会链接失败。
// 通讯底层（TCP/帧编解码/心跳/重连）由基类 UBaseSocketSubsystem 提供。
#include "ProtoSpeaker.h"

// SpeakerProto 协议号（定义于 speaker_req.proto，无 package），用于收包按协议号分流。
// 与服务器约定，改动需两端同步。
static constexpr int32 kSpeakerLoginProtocol   = 100;   // LOGIN_SPEAKER
static constexpr int32 kBalanceNotifyProtocol  = 103;   // BALANCE_NOTIFY
static constexpr int32 kLevelExpNotifyProtocol = 107;   // LEVEL_EXP_NOTIFY
static constexpr int32 kNewMailNotifyProtocol  = 108;   // NEW_MAIL_NOTIFY
static constexpr int32 kNewEventNotifyProtocol = 109;   // NEW_EVENT_NOTIFY

// ───────────────────────────────────────────────────────────
// 基类钩子覆盖：连接与帧配置
// ───────────────────────────────────────────────────────────
void UShopperSocketSubsystem::InitFrameConfig(FGameSocketFrameConfig& OutConfig) const
{
	OutConfig = FGameSocketFrameConfig{};
	// shopper 约定：小端 / length 含帧头 / md5 末字节。仅以下两项取自配置。
	if (const UShopperSettings* Cfg = GetDefault<UShopperSettings>())
	{
		OutConfig.bUseFrameChecksum = Cfg->bUseFrameChecksum;
		OutConfig.PacketMaxSize     = Cfg->PacketMaxSize;
	}
}

bool UShopperSocketSubsystem::ShouldAutoConnect() const
{
	const UShopperSettings* Cfg = GetDefault<UShopperSettings>();
	return Cfg && Cfg->bAutoConnect;
}

void UShopperSocketSubsystem::GetDefaultEndpoint(FString& OutHost, int32& OutPort) const
{
	const UShopperSettings* Cfg = GetDefault<UShopperSettings>();
	if (Cfg)
	{
		OutHost = Cfg->SocketHost;
		OutPort = Cfg->SocketPort;
	}
}

float UShopperSocketSubsystem::GetHeartbeatInterval() const
{
	const UShopperSettings* Cfg = GetDefault<UShopperSettings>();
	return Cfg ? Cfg->HeartbeatInterval : 15.f;
}

float UShopperSocketSubsystem::GetReconnectInterval() const
{
	const UShopperSettings* Cfg = GetDefault<UShopperSettings>();
	return Cfg ? Cfg->ReconnectInterval : 3.f;
}

void UShopperSocketSubsystem::BuildHeartbeatPayload(int32& OutProtocol, TArray<uint8>& OutData)
{
	// 心跳包：ping_speaker（协议号固定 SpeakerProto::PING_SPEAKER = 101），默认空参数。
	// 此处用默认空值；若需上报当前界面/停留时长，业务层可手动调 SendSpeakerPing(...) 覆写。
	Proto_BuildSpeakerPing(TEXT(""), TEXT(""), 0, 0, OutProtocol, OutData);
}

// ───────────────────────────────────────────────────────────
// 收包分发：按协议号解析为强类型后广播对应事件；未识别的协议号仅暴露原始字节
// （仿照 Lyra 的 HandleGameplayMessage 思路：新增协议在下面 switch 加 case 即可）
// ───────────────────────────────────────────────────────────
void UShopperSocketSubsystem::HandlePacket(int32 Protocol, TArray<uint8>&& Data)
{
	switch (Protocol)
	{
	case kSpeakerLoginProtocol:
	{
		int32 Code = 0;
		FString Msg;
		int32 MinLimit = 0;
		int32 BombLimit = 0;
		FSpeakerUser User;
		if (ParseSpeakerLoginResp(Data, Code, Msg, MinLimit, BombLimit, User))
		{
			OnSpeakerLoginRespReceived.Broadcast(Protocol, Code, Msg, MinLimit, BombLimit, User);
		}
		break;
	}
	case kBalanceNotifyProtocol:
	{
		FSpeakerBalance Balance;
		if (ParseSpeakerBalanceNotify(Data, Balance))
		{
			OnBalanceNotifyReceived.Broadcast(Balance);
		}
		break;
	}
	case kLevelExpNotifyProtocol:
	{
		FSpeakerLevelExp LevelExp;
		if (ParseSpeakerLevelExpNotify(Data, LevelExp))
		{
			OnLevelExpNotifyReceived.Broadcast(LevelExp);
		}
		break;
	}
	case kNewMailNotifyProtocol:
	{
		if (ParseSpeakerNewMailNotify(Data))
		{
			OnNewMailNotifyReceived.Broadcast();
		}
		break;
	}
	case kNewEventNotifyProtocol:
	{
		int32 Flag = 0;
		if (ParseSpeakerNewEventNotify(Data, Flag))
		{
			OnNewEventNotifyReceived.Broadcast(Flag);
		}
		break;
	}
	case kPingProtocol:
	{
		break;		
	}
	default:
		// 未识别协议号：不强行解析，留给 OnMessageReceived 的订阅者自行处理。
		UE_LOG(LogShopperSocket, Log, TEXT("收到未知协议=%d，负载长度=%d"),
			Protocol, Data.Num());
		break;
	}
}

// ───────────────────────────────────────────────────────────
// Speaker 协议发送 / 解析（protobuf 构造与反序列化均在 ShopperProto 模块完成）
// ───────────────────────────────────────────────────────────
void UShopperSocketSubsystem::SendSpeakerLogin(const FString& Token)
{
	// 构造 + 序列化委托给 ShopperProto 模块（独占 protobuf 类型），本模块只拿到字节流与协议号。
	int32 Protocol = 0;
	TArray<uint8> Bytes;
	if (!Proto_BuildSpeakerLoginRequest(Token, Protocol, Bytes))
	{
		UE_LOG(LogShopperSocket, Warning, TEXT("[ShopperSocket] SendSpeakerLogin 序列化失败"));
		return;
	}
	// 组帧并发送（协议号由 Proto_BuildSpeakerLoginRequest 填充：SpeakerProto::LOGIN_SPEAKER = 100）
	SendMessage(Protocol, Bytes);
}

bool UShopperSocketSubsystem::ParseSpeakerLoginResp(const TArray<uint8>& Data,
	int32& OutCode, FString& OutMsg, int32& OutMinLimit, int32& OutBombLimit, FSpeakerUser& OutUser)
{
	// 反序列化委托给 ShopperProto 模块（独占 protobuf 类型），本模块只暴露强类型结果给蓝图。
	return Proto_ParseSpeakerLoginResponse(Data, OutCode, OutMsg, OutMinLimit, OutBombLimit, OutUser);
}

void UShopperSocketSubsystem::SendSpeakerPing(const FString& Action, const FString& SubUi, int32 Duration, int64 Ticket)
{
	// 构造 + 序列化委托给 ShopperProto 模块（独占 protobuf 类型），本模块只拿到字节流与协议号。
	int32 Protocol = 0;
	TArray<uint8> Bytes;
	if (!Proto_BuildSpeakerPing(Action, SubUi, Duration, Ticket, Protocol, Bytes))
	{
		UE_LOG(LogShopperSocket, Warning, TEXT("[ShopperSocket] SendSpeakerPing 序列化失败"));
		return;
	}
	// 组帧并发送（协议号由 Proto_BuildSpeakerPing 填充：SpeakerProto::PING_SPEAKER = 101）
	SendMessage(Protocol, Bytes);
}

bool UShopperSocketSubsystem::ParseSpeakerBalanceNotify(const TArray<uint8>& Data, FSpeakerBalance& OutBalance)
{
	return Proto_ParseSpeakerBalanceNotify(Data, OutBalance);
}

bool UShopperSocketSubsystem::ParseSpeakerLevelExpNotify(const TArray<uint8>& Data, FSpeakerLevelExp& OutLevelExp)
{
	return Proto_ParseSpeakerLevelExpNotify(Data, OutLevelExp);
}

bool UShopperSocketSubsystem::ParseSpeakerNewMailNotify(const TArray<uint8>& Data)
{
	return Proto_ParseSpeakerNewMailNotify(Data);
}

bool UShopperSocketSubsystem::ParseSpeakerNewEventNotify(const TArray<uint8>& Data, int32& OutFlag)
{
	return Proto_ParseSpeakerNewEventNotify(Data, OutFlag);
}
