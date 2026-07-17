#include "ProtoSpeaker.h"

#include "ProtoSerializer.h"   // ProtoToBytes / BytesToProto（仅前向声明 Message，安全）

// 必须在包含任何 protobuf/abseil 头之前 #undef verify：UE 的 PCH 把 verify 定义成宏，
// 会与 abseil btree::verify() 成员方法冲突。UE 5.6 启用 C++ 模块，故 #include 必须处于文件作用域。
#undef verify
#include "Proto/speaker_req.pb.h"

bool Proto_BuildSpeakerLoginRequest(
	const FString& Token, int32& OutProtocol, TArray<uint8>& OutBytes)
{
	login_speaker_req Msg;
	Msg.set_token(TCHAR_TO_UTF8(*Token));

	if (!FProtoSerializer::ProtoToBytes(Msg, OutBytes))
	{
		return false;
	}
	OutProtocol = static_cast<int32>(SpeakerProto::LOGIN_SPEAKER);
	return true;
}

bool Proto_ParseSpeakerLoginResponse(
	const TArray<uint8>& Bytes,
	int32& OutCode, FString& OutMsg, int32& OutMinLimit, int32& OutBombLimit, FSpeakerUser& OutUser)
{
	login_speaker_resp Msg;
	if (!FProtoSerializer::BytesToProto(Bytes, Msg))
	{
		// 被动解析失败（收到的可能是其它协议，或 framing 错位导致 data 头尾错乱）
		UE_LOG(LogTemp, Warning,
			TEXT("[ProtoSpeaker] Proto_ParseSpeakerLoginResponse 解析失败：data 长度=%d（若长度异常，多为收包帧头多读/少读字节）"),
			Bytes.Num());
		return false;
	}

	OutCode = Msg.code();
	OutMsg = FString(UTF8_TO_TCHAR(Msg.msg().c_str()));
	OutMinLimit = Msg.min_limit();
	OutBombLimit = Msg.bomb_limit();

	const ::User& U = Msg.user();
	OutUser.Account = FString(UTF8_TO_TCHAR(U.account().c_str()));
	OutUser.Nickname = FString(UTF8_TO_TCHAR(U.nickname().c_str()));
	OutUser.Headimg = FString(UTF8_TO_TCHAR(U.headimg().c_str()));
	OutUser.Exp = U.exp();
	OutUser.Level = U.level();
	OutUser.Id = U.id();
	OutUser.Headframe = FString(UTF8_TO_TCHAR(U.headframe().c_str()));
	OutUser.Mobile = FString(UTF8_TO_TCHAR(U.mobile().c_str()));
	return true;
}

bool Proto_BuildSpeakerPing(
	const FString& Action, const FString& SubUi, int32 Duration, int64 Ticket,
	int32& OutProtocol, TArray<uint8>& OutBytes)
{
	ping_speaker Msg;
	Msg.set_ticket(static_cast<uint64>(Ticket));
	Msg.set_action(TCHAR_TO_UTF8(*Action));
	Msg.set_subui(TCHAR_TO_UTF8(*SubUi));
	Msg.set_duration(Duration);

	if (!FProtoSerializer::ProtoToBytes(Msg, OutBytes))
	{
		return false;
	}
	OutProtocol = static_cast<int32>(SpeakerProto::PING_SPEAKER);
	return true;
}

bool Proto_ParseSpeakerBalanceNotify(const TArray<uint8>& Bytes, FSpeakerBalance& Out)
{
	balance_notify Msg;
	if (!FProtoSerializer::BytesToProto(Bytes, Msg))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ProtoSpeaker] Proto_ParseSpeakerBalanceNotify 解析失败：data 长度=%d"), Bytes.Num());
		return false;
	}
	Out.Balance       = static_cast<int64>(Msg.balance());
	Out.FreezeBalance = static_cast<int64>(Msg.freeze_balance());
	Out.Diamond       = static_cast<int64>(Msg.diamond());
	Out.Bonus         = static_cast<int64>(Msg.bonus());
	Out.Star          = Msg.star();
	Out.Flag          = Msg.flag();
	return true;
}

bool Proto_ParseSpeakerLevelExpNotify(const TArray<uint8>& Bytes, FSpeakerLevelExp& Out)
{
	level_exp_notify Msg;
	if (!FProtoSerializer::BytesToProto(Bytes, Msg))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ProtoSpeaker] Proto_ParseSpeakerLevelExpNotify 解析失败：data 长度=%d"), Bytes.Num());
		return false;
	}
	Out.Level     = Msg.level();
	Out.Exp       = Msg.exp();
	Out.MinLimit  = Msg.min_limit();
	Out.BombLimit = Msg.bomb_limit();

	const int32 N = Msg.rewards_size();
	Out.Rewards.Reset(N);
	for (int32 i = 0; i < N; ++i)
	{
		const ::item& It = Msg.rewards(i);
		FSpeakerItem& Slot = Out.Rewards.AddDefaulted_GetRef();
		Slot.Id    = It.id();
		Slot.Entry = It.entry();
		Slot.Num   = It.num();
	}
	return true;
}

bool Proto_ParseSpeakerNewMailNotify(const TArray<uint8>& Bytes)
{
	new_mail_notify Msg;
	if (!FProtoSerializer::BytesToProto(Bytes, Msg))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ProtoSpeaker] Proto_ParseSpeakerNewMailNotify 解析失败：data 长度=%d"), Bytes.Num());
		return false;
	}
	return true;
}

bool Proto_ParseSpeakerNewEventNotify(const TArray<uint8>& Bytes, int32& OutFlag)
{
	new_event_notify Msg;
	if (!FProtoSerializer::BytesToProto(Bytes, Msg))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ProtoSpeaker] Proto_ParseSpeakerNewEventNotify 解析失败：data 长度=%d"), Bytes.Num());
		return false;
	}
	OutFlag = Msg.flag();
	return true;
}
