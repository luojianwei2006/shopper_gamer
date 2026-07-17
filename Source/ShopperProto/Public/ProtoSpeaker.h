#pragma once

#include "CoreMinimal.h"
#include "ProtoSpeaker.generated.h"

// 喇叭用户信息（对应 common.proto 的 User）。
// 定义在本模块（而非 shoppergame），因为本模块是唯一直接触碰 protobuf 类型的地方；
// 这样 shoppergame 只需引用这个纯数据 USTRUCT，不沾 protobuf / abseil 头。
USTRUCT(BlueprintType)
struct SHOPPERPROTO_API FSpeakerUser
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	FString Account;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	FString Nickname;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	FString Headimg;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Exp = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Level = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Id = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	FString Headframe;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	FString Mobile;
};

// 道具（对应 speaker_req.proto 的 item；level_exp_notify 的 rewards 用到）
USTRUCT(BlueprintType)
struct SHOPPERPROTO_API FSpeakerItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Id = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Entry = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Num = 0;
};

// 余额变更（balance_notify）：balance / freeze_balance / diamond / bonus 为金额，用 int64
USTRUCT(BlueprintType)
struct SHOPPERPROTO_API FSpeakerBalance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int64 Balance = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int64 FreezeBalance = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int64 Diamond = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int64 Bonus = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Star = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Flag = 0;
};

// 角色等级经验变更（level_exp_notify），rewards 为升级奖励列表
USTRUCT(BlueprintType)
struct SHOPPERPROTO_API FSpeakerLevelExp
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Level = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 Exp = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 MinLimit = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	int32 BombLimit = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	TArray<FSpeakerItem> Rewards;
};

// 构造 login_speaker_req（token）并序列化为字节。协议号固定 SpeakerProto::LOGIN_SPEAKER = 100。
// 所有 protobuf 类型的使用都封装在本模块内，避免被外部模块因符号可见性限制而链接失败。
SHOPPERPROTO_API bool Proto_BuildSpeakerLoginRequest(
	const FString& Token, int32& OutProtocol, TArray<uint8>& OutBytes);

// 把 login_speaker_resp 的二进制反序列化为业务字段（含嵌套的 User -> FSpeakerUser）。
SHOPPERPROTO_API bool Proto_ParseSpeakerLoginResponse(
	const TArray<uint8>& Bytes,
	int32& OutCode, FString& OutMsg, int32& OutMinLimit, int32& OutBombLimit, FSpeakerUser& OutUser);

// 构造 ping_speaker 心跳包（ticket / action / subui / duration），协议号固定 SpeakerProto::PING_SPEAKER = 101。
SHOPPERPROTO_API bool Proto_BuildSpeakerPing(
	const FString& Action, const FString& SubUi, int32 Duration, int64 Ticket,
	int32& OutProtocol, TArray<uint8>& OutBytes);

// balance_notify 反序列化（余额变更）
SHOPPERPROTO_API bool Proto_ParseSpeakerBalanceNotify(const TArray<uint8>& Bytes, FSpeakerBalance& Out);

// level_exp_notify 反序列化（角色等级经验变更，含 repeated item rewards）
SHOPPERPROTO_API bool Proto_ParseSpeakerLevelExpNotify(const TArray<uint8>& Bytes, FSpeakerLevelExp& Out);

// new_mail_notify 反序列化（空消息，仅校验可解析）
SHOPPERPROTO_API bool Proto_ParseSpeakerNewMailNotify(const TArray<uint8>& Bytes);

// new_event_notify 反序列化（flag：1-Bingo 状态变更，2-新活动开启）
SHOPPERPROTO_API bool Proto_ParseSpeakerNewEventNotify(const TArray<uint8>& Bytes, int32& OutFlag);
