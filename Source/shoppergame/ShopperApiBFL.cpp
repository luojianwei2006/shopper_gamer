#include "ShopperApiBFL.h"
#include "ShopperSettings.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogShopperApi, Log, All);

// ── 文件内辅助：取客户端子系统 + 解析基地址 + 构造鉴权头 ──
// 与 SendGetWallet 保持一致，集中在此避免每个函数重复。
namespace ShopperApiBFLImpl
{
	static UShopperHttpClient* ResolveClient(UObject* WorldContextObject, const FString& Host, FString& OutEffectiveHost)
	{
		OutEffectiveHost = Host.IsEmpty() ? UShopperSettings::ResolveBaseUrl() : Host;
		UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
		return GI ? GI->GetSubsystem<UShopperHttpClient>() : nullptr;
	}

	static TMap<FString, FString> AuthHeaders(const FString& Token)
	{
		TMap<FString, FString> Headers;
		if (!Token.IsEmpty())
		{
			Headers.Add(TEXT("Authorization"), Token);
		}
		return Headers;
	}
}

// ═══════════════════════════════ 3. 用户模块 ═══════════════════════════════
void UShopperApiBFL::SendUserSendAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserAuthCodeReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserAuthCodeReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/send_authcode"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserCheckAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserAuthCodeReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserAuthCodeReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/check_authcode"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserRegister(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserRegisterReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserRegisterReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/register"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserUpdate(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserUpdateReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserUpdateReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/update"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserUpdateLocation(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserLocationReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserLocationReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/update_location"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserUpdateDeviceToken(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserDeviceTokenReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserDeviceTokenReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/update_device_token"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserBeginner(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserBeginnerReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserBeginnerReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/beginner"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserGameConfigVersion(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserConfigVersionReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserConfigVersionReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/game_config_version"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendUserNewestRecord(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FUserNewestRecordReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FUserNewestRecordReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("user/newest_record"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

// ═══════════════════════════════ 4. 商城 ═══════════════════════════════
void UShopperApiBFL::SendShopBuy(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 ShopId, const FString& Method, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("shopId"), FString::FromInt(ShopId));
	Query.Add(TEXT("method"), Method);
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("shop/buy"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 5. 签到 ═══════════════════════════════
void UShopperApiBFL::SendSignDaily(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("sign/sign"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

// ═══════════════════════════════ 6. 提现 ═══════════════════════════════
void UShopperApiBFL::SendWithdrawCanWithdraw(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 Amount, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("amount"), FString::FromInt(Amount));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("withdraw/can_withdraw"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendWithdrawSendAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("withdraw/send_authcode"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendWithdrawCheckAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FString& AuthCode, int32 Amount, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("authcode"), AuthCode);
	Query.Add(TEXT("amount"), FString::FromInt(Amount));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("withdraw/check_authcode"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendWithdrawSubmit(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FWithdrawReq& Proto, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	Client->Request<FWithdrawReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("withdraw/withdraw"),
		Proto, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendWithdrawWalletLogs(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 LastId, const FString& LastDate, int32 Size, const FOnWalletLogListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FWalletLogListResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("last_id"), FString::FromInt(LastId));
	Query.Add(TEXT("last_date"), LastDate);
	Query.Add(TEXT("size"), FString::FromInt(Size));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FWalletLogListResponse>(EShopperHttpVerb::Post, TEXT("withdraw/wallet_logs"),
		Req, [OnComplete](bool b, const FWalletLogListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 7. 邮件 ═══════════════════════════════
void UShopperApiBFL::SendMailList(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 LastId, int32 Size, const FOnMailListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FMailListResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("last_id"), FString::FromInt(LastId));
	Query.Add(TEXT("size"), FString::FromInt(Size));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FMailListResponse>(EShopperHttpVerb::Post, TEXT("mail/list"),
		Req, [OnComplete](bool b, const FMailListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendMailRead(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 MailId, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("id"), FString::FromInt(MailId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("mail/read"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendMailReceive(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 MailId, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("id"), FString::FromInt(MailId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("mail/receive"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 8. 任务 ═══════════════════════════════
void UShopperApiBFL::SendTaskList(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnTaskListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FTaskListResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FTaskListResponse>(EShopperHttpVerb::Post, TEXT("task/tasks"),
		Req, [OnComplete](bool b, const FTaskListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendTaskUpdateProgress(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 TaskId, int32 Progress, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("taskId"), FString::FromInt(TaskId));
	Query.Add(TEXT("progress"), FString::FromInt(Progress));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("task/update_progress"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendTaskReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 TaskId, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("taskId"), FString::FromInt(TaskId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("task/reward"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 9. Banner ═══════════════════════════════
void UShopperApiBFL::SendBannerList(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("banner/list"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendBannerReceiveFreeGift(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 Entry, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("entry"), FString::FromInt(Entry));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("banner/receive_free_gift"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendBannerBuy(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 BannerId, const FString& Method, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("bannerId"), FString::FromInt(BannerId));
	Query.Add(TEXT("method"), Method);
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("banner/buy"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 10. 游戏记录 ═══════════════════════════════
void UShopperApiBFL::SendGameList(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnGameModeListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FGameModeListResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FGameModeListResponse>(EShopperHttpVerb::Post, TEXT("game/list"),
		Req, [OnComplete](bool b, const FGameModeListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendGameRank(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 GameId, const FOnGameRankDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FGameRankResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("id"), FString::FromInt(GameId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FGameRankResponse>(EShopperHttpVerb::Post, TEXT("game/get_rank"),
		Req, [OnComplete](bool b, const FGameRankResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendGameRecords(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 LastId, int32 Size, const FOnGameRecordListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FGameRecordListResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("last_id"), FString::FromInt(LastId));
	Query.Add(TEXT("size"), FString::FromInt(Size));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FGameRecordListResponse>(EShopperHttpVerb::Post, TEXT("game/records"),
		Req, [OnComplete](bool b, const FGameRecordListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendGameNewestRecords(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 LastId, int32 Size, const FOnGameRecordListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FGameRecordListResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("last_id"), FString::FromInt(LastId));
	Query.Add(TEXT("size"), FString::FromInt(Size));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FGameRecordListResponse>(EShopperHttpVerb::Post, TEXT("game/newst_records"),
		Req, [OnComplete](bool b, const FGameRecordListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendGameRecordDetail(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 Id, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("id"), FString::FromInt(Id));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("game/record_detail"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendGameReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FString& Guid, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("guid"), Guid);
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("game/reward"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 11. 挖宝 ═══════════════════════════════
void UShopperApiBFL::SendWabaoList(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnWabaoListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FWabaoListResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FWabaoListResponse>(EShopperHttpVerb::Get, TEXT("wabao/list"),
		Req, [OnComplete](bool b, const FWabaoListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendWabaoStart(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("wabao/start"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

// ═══════════════════════════════ 12. 远征 ═══════════════════════════════
void UShopperApiBFL::SendExpeditionGetInfo(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Get, TEXT("expedition/get_info"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendExpeditionAttack(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 Pos, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("pos"), FString::FromInt(Pos));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("expedition/attack"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 13. 宝箱 ═══════════════════════════════
void UShopperApiBFL::SendChestStart(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 ActivityId, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("activityId"), FString::FromInt(ActivityId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("chest/start"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendChestOpen(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 ActivityId, int32 Pos, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("activityId"), FString::FromInt(ActivityId));
	Query.Add(TEXT("pos"), FString::FromInt(Pos));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("chest/open"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendChestReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 ActivityId, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("activityId"), FString::FromInt(ActivityId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("chest/reward"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

// ═══════════════════════════════ 14. 广告任务 ═══════════════════════════════
void UShopperApiBFL::SendAdsTaskList(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnAdsTaskListDone& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FAdsTaskListResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FAdsTaskListResponse>(EShopperHttpVerb::Post, TEXT("ads_task/tasks"),
		Req, [OnComplete](bool b, const FAdsTaskListResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

void UShopperApiBFL::SendAdsTaskReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 TaskId, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("taskId"), FString::FromInt(TaskId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("ads_task/reward"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}

void UShopperApiBFL::SendAdsTaskRewardAll(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("ads_task/reward_all"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token));
}

// ═══════════════════════════════ 15. 海盗战斗 ═══════════════════════════════
void UShopperApiBFL::SendPirateMatch(UObject* WorldContextObject, const FString& Host, const FString& Token,
	int32 ActivityId, const FOnShopperApiJson& OnComplete)
{
	FString EffectiveHost;
	UShopperHttpClient* Client = ShopperApiBFLImpl::ResolveClient(WorldContextObject, Host, EffectiveHost);
	if (!Client) { OnComplete.ExecuteIfBound(false, FShopperJsonResponse()); return; }
	TMap<FString, FString> Query;
	Query.Add(TEXT("activityId"), FString::FromInt(ActivityId));
	FShopperEmptyReq Req;
	Client->Request<FShopperEmptyReq, FShopperJsonResponse>(EShopperHttpVerb::Post, TEXT("pirate_battle/match"),
		Req, [OnComplete](bool b, const FShopperJsonResponse& R, int32) { OnComplete.ExecuteIfBound(b, R); },
		EffectiveHost, ShopperApiBFLImpl::AuthHeaders(Token), Query);
}
