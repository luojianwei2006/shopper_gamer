#pragma once

#include "CoreMinimal.h"
#include "BaseSocketSubsystem.h"

// FSpeakerUser + 各 Speaker 结构 + protobuf 收发包装函数来自 ShopperProto 模块（该模块独占 protobuf 类型，
// 避免在 shoppergame 里直接引用 libprotobuf 符号导致链接期可见性失败）。
#include "ProtoSpeaker.h"

// 热重载管理器：提供运行时 MountPak / LoadAssetAsync 与 OnHotfixPakMounted 广播。
// （必须在本文件生成的 generated.h 之前 include，保证下面的反射宏用本文件 CURRENT_FILE_ID）
#include "HotReloadManager.h"

// 必须在所有 DECLARE_DYNAMIC_* / UCLASS 之前 include 本文件的 generated.h（见 BaseSocketSubsystem.h 注释）。
// 注意顺序：先 include 依赖头（BaseSocketSubsystem.h / ProtoSpeaker.h），再 include 本文件 generated.h，
// 这样下面这些委托宏展开时 CURRENT_FILE_ID 已重置为本文件，而不是被 ProtoSpeaker 的 id 污染。
#include "ShopperSocketSubsystem.generated.h"

// 随热更 pak 下发的 socket 参数覆盖层（UPrimaryDataAsset），不改动编译期 ShopperSettings。
class UShopperHotfixConfig;

// 收到 login_speaker_resp 的强类型广播：protobuf 负载反序列化为业务字段后抛出，
// 蓝图可直接 Bind Event 拿到 Code / Msg / MinLimit / BombLimit / User，无需接触 protobuf 类型。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
	FOnSpeakerLoginRespReceived, int32, Protocol, int32, Code, FString, Msg, int32, MinLimit, int32, BombLimit, FSpeakerUser, User);

// 余额变更通知（balance_notify 反序列化后的强类型事件）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnBalanceNotifyReceived, FSpeakerBalance, Balance);

// 角色等级经验变更通知（level_exp_notify 反序列化后的强类型事件）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnLevelExpNotifyReceived, FSpeakerLevelExp, LevelExp);

// 新邮件通知（new_mail_notify 为空消息，无负载）
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnNewMailNotifyReceived);

// 新事件通知（new_event_notify：flag=1-Bingo 状态变更，2-新活动开启）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnNewEventNotifyReceived, int32, Flag);

/**
 * 喇叭服务器长链接子系统。
 * 通讯底层（TCP/帧编解码/心跳/重连/读线程）全部由 UBaseSocketSubsystem 提供，
 * 本类只负责 Speaker 协议：帧配置（InitFrameConfig）、连接配置（ShouldAutoConnect 等）、
 * 按协议号分发（HandlePacket）、以及喇叭特有的发送/解析函数与委托。
 */
// BlueprintType 必须：否则蓝图里 Get Game Instance Subsystem 节点的类下拉框选不到本 Subsystem。
UCLASS(BlueprintType)
class SHOPPERGAME_API UShopperSocketSubsystem : public UBaseSocketSubsystem
{
	GENERATED_BODY()

public:
	// 喇叭登录：发送 login_speaker_req（token），协议号固定 SpeakerProto::LOGIN_SPEAKER(100)
	//   蓝图只需传入登录后拿到的 token，无需接触 protobuf
	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	void SendSpeakerLogin(const FString& Token);

	// 解析 login_speaker_resp：蓝图在 OnMessageReceived 回调里调用，
	//   或本子系统在 HandlePacket 中自动解析后通过 OnSpeakerLoginRespReceived 广播。
	//   返回 true 表示 Data 能解析成 login_speaker_resp，且已填充到输出参数
	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	bool ParseSpeakerLoginResp(const TArray<uint8>& Data,
		int32& OutCode, FString& OutMsg, int32& OutMinLimit, int32& OutBombLimit, FSpeakerUser& OutUser);

	// 心跳包：发送 ping_speaker（协议号固定 SpeakerProto::PING_SPEAKER=101）。
	//   Action : 当前停留的主界面名（如 "Lobby"）
	//   SubUi  : 子界面名
	//   Duration : 本界面停留时长（秒，最长 15）
	//   Ticket : 心跳序号/时间戳（uint64，蓝图用 int64 传入）
	// 连上后由基类心跳 ticker 自动周期调用（参数取默认空值）；业务层也可手动调以带上界面信息。
	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	void SendSpeakerPing(const FString& Action = TEXT(""), const FString& SubUi = TEXT(""), int32 Duration = 0, int64 Ticket = 0);

	// 各通知解析（蓝图在 OnMessageReceived 里也可手动调；本子系统在 HandlePacket 中按协议号自动解析并广播）。
	//   返回 true 表示 Data 能解析成对应结构并已填充输出参数。
	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	bool ParseSpeakerBalanceNotify(const TArray<uint8>& Data, FSpeakerBalance& OutBalance);

	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	bool ParseSpeakerLevelExpNotify(const TArray<uint8>& Data, FSpeakerLevelExp& OutLevelExp);

	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	bool ParseSpeakerNewMailNotify(const TArray<uint8>& Data);

	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	bool ParseSpeakerNewEventNotify(const TArray<uint8>& Data, int32& OutFlag);

	// 应用一个热更配置（来自热更 pak 的 UShopperHotfixConfig）。会热重载底层连接/帧格式参数，
	// 下次连接即生效。由 UHotReloadManager::OnHotfixPakMounted 在 pak 挂载后自动调用，也可手动调用。
	UFUNCTION(BlueprintCallable, Category = "Socket|Speaker")
	void ApplyHotfixConfig(UShopperHotfixConfig* Cfg);

	// 收到并已成功解析为 login_speaker_resp 的消息（强类型事件）
	UPROPERTY(BlueprintAssignable, Category = "Socket")
	FOnSpeakerLoginRespReceived OnSpeakerLoginRespReceived;

	// 余额变更通知（balance_notify）
	UPROPERTY(BlueprintAssignable, Category = "Socket")
	FOnBalanceNotifyReceived OnBalanceNotifyReceived;

	// 角色等级经验变更通知（level_exp_notify）
	UPROPERTY(BlueprintAssignable, Category = "Socket")
	FOnLevelExpNotifyReceived OnLevelExpNotifyReceived;

	// 新邮件通知（new_mail_notify，空消息）
	UPROPERTY(BlueprintAssignable, Category = "Socket")
	FOnNewMailNotifyReceived OnNewMailNotifyReceived;

	// 新事件通知（new_event_notify）
	UPROPERTY(BlueprintAssignable, Category = "Socket")
	FOnNewEventNotifyReceived OnNewEventNotifyReceived;

protected:
	// ── 基类钩子覆盖 ──
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void InitFrameConfig(FGameSocketFrameConfig& OutConfig) const override;
	virtual bool ShouldAutoConnect() const override;
	virtual void GetDefaultEndpoint(FString& OutHost, int32& OutPort) const override;
	virtual float GetHeartbeatInterval() const override;
	virtual float GetReconnectInterval() const override;
	virtual void BuildHeartbeatPayload(int32& OutProtocol, TArray<uint8>& OutData) override;
	virtual FString GetLogTag() const override { return TEXT("ShopperSocket"); }

	// 收到完整数据包后按协议号分流解析，并广播对应的强类型委托
	virtual void HandlePacket(int32 Protocol, TArray<uint8>&& Data) override;

private:
	// 随热更 pak 下发的配置覆盖层；非空时各钩子优先用它，否则回退 ShopperSettings。
	// UPROPERTY 持有弱引用，避免 hotfix 资产被 GC 误回收导致悬空。
	UPROPERTY()
	TWeakObjectPtr<UShopperHotfixConfig> HotfixConfig;

	// UHotReloadManager::OnHotfixPakMounted 回调：加载固定资产路径的 hotfix 配置并应用
	void OnHotfixPakMounted(const FString& PakPath);

private:
	// SpeakerProto 协议号（定义于 speaker_req.proto，无 package），用于收包按协议号分流。
	static constexpr int32 kSpeakerLoginProtocol   = 100;   // LOGIN_SPEAKER
	static constexpr int32 kPingProtocol            = 101;   // PING_SPEAKER
	static constexpr int32 kBalanceNotifyProtocol  = 103;   // BALANCE_NOTIFY
	static constexpr int32 kLevelExpNotifyProtocol = 107;   // LEVEL_EXP_NOTIFY
	static constexpr int32 kNewMailNotifyProtocol  = 108;   // NEW_MAIL_NOTIFY
	static constexpr int32 kNewEventNotifyProtocol = 109;   // NEW_EVENT_NOTIFY
};
