#include "HttpProtoBFL.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "DeviceSaveGame.h"
#include "Kismet/GameplayStatics.h"   // 存档读写
#include "Misc/Guid.h"                // FGuid
#include "HAL/PlatformMisc.h"         // FPlatformMisc

// 自建日志分类，无需额外模块依赖
DEFINE_LOG_CATEGORY_STATIC(LogHttpProto, Log, All);

// 本地存档槽位（设备码 / 游客 token 持久化）
static const FString DeviceSaveSlot    = TEXT("DeviceSaveSlot");
static const int32   DeviceSaveUserIdx = 0;

FString UHttpProtoBFL::GetOrCreateDeviceId()
{
	// 1) 先读存档——已生成过就直接返回，保证设备码稳定
	if (USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(DeviceSaveSlot, DeviceSaveUserIdx))
	{
		if (UDeviceSaveGame* Save = Cast<UDeviceSaveGame>(Raw))
		{
			if (!Save->DeviceId.IsEmpty())
			{
				return Save->DeviceId;
			}
		}
	}

	// 2) 没存档 → 生成新设备码：平台设备码作种子 + 一层 GUID 保证唯一
	const FString Seed  = FPlatformMisc::GetDeviceId();
	const FString NewId = FGuid::NewGuid().ToString() + TEXT("_") + Seed;

	// 3) 存回本地（忽略创建失败，仅打日志）
	UDeviceSaveGame* Save = Cast<UDeviceSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UDeviceSaveGame::StaticClass()));
	if (Save)
	{
		Save->DeviceId = NewId;
		const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, DeviceSaveSlot, DeviceSaveUserIdx);
		UE_LOG(LogHttpProto, Warning, TEXT("[GetOrCreateDeviceId] 生成并保存设备码: %s (保存%s)"),
			*NewId, bSaved ? TEXT("成功") : TEXT("失败"));
	}
	else
	{
		UE_LOG(LogHttpProto, Warning, TEXT("[GetOrCreateDeviceId] 存档对象创建失败，仅返回临时设备码"));
	}

	return NewId;
}

void UHttpProtoBFL::SaveSessionToken(const FString& Token)
{
	// 复用既有存档（保留 DeviceId），不存在则新建
	UDeviceSaveGame* Save = nullptr;
	if (USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(DeviceSaveSlot, DeviceSaveUserIdx))
	{
		Save = Cast<UDeviceSaveGame>(Raw);
	}
	if (!Save)
	{
		Save = Cast<UDeviceSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UDeviceSaveGame::StaticClass()));
	}
	if (Save)
	{
		Save->SessionToken = Token;
		const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, DeviceSaveSlot, DeviceSaveUserIdx);
		UE_LOG(LogHttpProto, Warning, TEXT("[SaveSessionToken] 保存 token (保存%s)"),
			bSaved ? TEXT("成功") : TEXT("失败"));
	}
}

FString UHttpProtoBFL::LoadSessionToken()
{
	if (USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(DeviceSaveSlot, DeviceSaveUserIdx))
	{
		if (UDeviceSaveGame* Save = Cast<UDeviceSaveGame>(Raw))
		{
			return Save->SessionToken;
		}
	}
	return TEXT("");
}

bool UHttpProtoBFL::ParseGuestLoginResponse(const FString& JsonString, FLoginResponse& OutResponse)
{
	const bool bOk = FJsonObjectConverter::JsonObjectStringToUStruct<FLoginResponse>(
		JsonString, &OutResponse, 0, 0);
	UE_LOG(LogHttpProto, Warning,
		TEXT("[ParseGuestLoginResponse] 解析%s | code=%d msg=%s tokenLen=%d userId=%d nickname=%s speakerIp=%s"),
		bOk ? TEXT("成功") : TEXT("失败"),
		OutResponse.code, *OutResponse.msg,
		OutResponse.token.Len(), OutResponse.data.user.id,
		*OutResponse.data.user.nickname, *OutResponse.data.speaker.ip);
	return bOk;
}

FString UHttpProtoBFL::GetApiBaseUrl()
{
	return UShopperSettings::ResolveBaseUrl();
}

int32 UHttpProtoBFL::GetAppVersion()
{
	return UShopperSettings::ResolveAppVersion();
}

void UHttpProtoBFL::SendGuestLogin(
    const FString& Host,
    const FGuestLoginProto& Proto,
    const FOnGuestLoginDone& OnComplete)
{
    // Host 为空时自动用项目设置里的接口基地址（环境配置）
    const FString ResolvedHost = Host.IsEmpty() ? GetApiBaseUrl() : Host;

    // 拼完整 URL：自动处理 Host 末尾有无斜杠
    FString Url = ResolvedHost;
    if (!Url.EndsWith(TEXT("/")))
    {
        Url += TEXT("/");
    }
    Url += TEXT("user/guest_login");

    // 结构体 → JSON Body
    FString Body;
    FJsonObjectConverter::UStructToJsonObjectString(
        FGuestLoginProto::StaticStruct(), &Proto, Body);

    // ① 函数被调用 + 发出前确认 URL/Body
    UE_LOG(LogHttpProto, Warning,
        TEXT("[SendGuestLogin] 准备发送请求\n  URL : %s\n  Body: %s"),
        *Url, *Body);

    // 发请求
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req =
        FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetContentAsString(Body);

    // ② 回调里确认是否收到响应
    Req->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr, FHttpResponsePtr Resp, bool bConnected)
        {
            const int32 Code = Resp.IsValid() ? Resp->GetResponseCode() : -1;
            const bool  bOk  = bConnected && Resp.IsValid() && Code == 200;
            const FString Content = Resp.IsValid() ? Resp->GetContentAsString() : TEXT("");

            UE_LOG(LogHttpProto, Warning,
                TEXT("[SendGuestLogin] 收到响应\n  bConnected=%d  bOk=%d  Code=%d\n  Content: %s"),
                bConnected, bOk, Code, *Content);

            // 登录成功 → 自动持久化游客 token，下次启动可直接 login_by_token 续期
            if (bOk && !Content.IsEmpty())
            {
                FLoginResponse RespStruct;
                if (FJsonObjectConverter::JsonObjectStringToUStruct<FLoginResponse>(
                        Content, &RespStruct, 0, 0)
                    && !RespStruct.token.IsEmpty())
                {
                    UHttpProtoBFL::SaveSessionToken(RespStruct.token);
                }
            }

            OnComplete.ExecuteIfBound(bOk, Content);
        });

    // ③ 确认 ProcessRequest 是否真正启动
    const bool bStarted = Req->ProcessRequest();
    UE_LOG(LogHttpProto, Warning,
        TEXT("[SendGuestLogin] ProcessRequest 返回 = %s"),
        bStarted ? TEXT("成功") : TEXT("失败"));
}

// ───────────────────────────────────────────────────────────
// Token 续期登录：POST /user/login_by_token，结构与游客登录一致
// ───────────────────────────────────────────────────────────
void UHttpProtoBFL::SendLoginByToken(
    const FString& Host,
    const FLoginByTokenProto& Proto,
    const FOnLoginByTokenDone& OnComplete)
{
    // Host 为空时自动用项目设置里的接口基地址（环境配置）
    const FString ResolvedHost = Host.IsEmpty() ? GetApiBaseUrl() : Host;

    // 拼完整 URL：自动处理 Host 末尾有无斜杠
    FString Url = ResolvedHost;
    if (!Url.EndsWith(TEXT("/")))
    {
        Url += TEXT("/");
    }
    Url += TEXT("user/login_by_token");

    // 结构体 → JSON Body
    FString Body;
    FJsonObjectConverter::UStructToJsonObjectString(
        FLoginByTokenProto::StaticStruct(), &Proto, Body);

    UE_LOG(LogHttpProto, Warning,
        TEXT("[SendLoginByToken] 准备发送请求\n  URL : %s\n  Body: %s"),
        *Url, *Body);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req =
        FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetContentAsString(Body);

    Req->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr, FHttpResponsePtr Resp, bool bConnected)
        {
            const int32 Code = Resp.IsValid() ? Resp->GetResponseCode() : -1;
            const bool  bOk  = bConnected && Resp.IsValid() && Code == 200;
            const FString Content = Resp.IsValid() ? Resp->GetContentAsString() : TEXT("");

            UE_LOG(LogHttpProto, Warning,
                TEXT("[SendLoginByToken] 收到响应\n  bConnected=%d  bOk=%d  Code=%d\n  Content: %s"),
                bConnected, bOk, Code, *Content);

            // 续期成功 → 刷新本地 token（后端可能返回新 token）
            if (bOk && !Content.IsEmpty())
            {
                FLoginResponse RespStruct;
                if (FJsonObjectConverter::JsonObjectStringToUStruct<FLoginResponse>(
                        Content, &RespStruct, 0, 0)
                    && !RespStruct.token.IsEmpty())
                {
                    UHttpProtoBFL::SaveSessionToken(RespStruct.token);
                }
            }

            OnComplete.ExecuteIfBound(bOk, Content);
        });

    const bool bStarted = Req->ProcessRequest();
    UE_LOG(LogHttpProto, Warning,
        TEXT("[SendLoginByToken] ProcessRequest 返回 = %s"),
        bStarted ? TEXT("成功") : TEXT("失败"));
}

// ───────────────────────────────────────────────────────────
// 钱包接口：GET /user/get_wallet，带 Authorization 鉴权头
// ───────────────────────────────────────────────────────────
void UHttpProtoBFL::SendGetWallet(
    const FString& Host,
    const FString& Token,
    const FOnWalletDone& OnComplete)
{
    // Host 为空时自动用项目设置里的接口基地址（环境配置）
    const FString ResolvedHost = Host.IsEmpty() ? GetApiBaseUrl() : Host;

    // 拼完整 URL
    FString Url = ResolvedHost;
    if (!Url.EndsWith(TEXT("/")))
    {
        Url += TEXT("/");
    }
    Url += TEXT("user/get_wallet");

    UE_LOG(LogHttpProto, Warning,
        TEXT("[SendGetWallet] 准备发送 POST 请求\n  URL : %s\n  TokenLen: %d"),
        *Url, Token.Len());

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req =
        FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    // 鉴权头：值直接为登录拿到的 token
    // 若后端要求 "Bearer <token>" 形式，改成：
    //   Req->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Token);
    Req->SetHeader(TEXT("Authorization"), Token);

    Req->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr, FHttpResponsePtr Resp, bool bConnected)
        {
            const int32 Code = Resp.IsValid() ? Resp->GetResponseCode() : -1;
            const bool  bOk  = bConnected && Resp.IsValid() && Code == 200;
            const FString Content = Resp.IsValid() ? Resp->GetContentAsString() : TEXT("");

            UE_LOG(LogHttpProto, Warning,
                TEXT("[SendGetWallet] 收到响应\n  bConnected=%d  bOk=%d  Code=%d\n  Content: %s"),
                bConnected, bOk, Code, *Content);

            OnComplete.ExecuteIfBound(bOk, Content);
        });

    const bool bStarted = Req->ProcessRequest();
    UE_LOG(LogHttpProto, Warning,
        TEXT("[SendGetWallet] ProcessRequest 返回 = %s"),
        bStarted ? TEXT("成功") : TEXT("失败"));
}

bool UHttpProtoBFL::ParseWalletResponse(const FString& JsonString, FWalletResponse& OutResponse)
{
    const bool bOk = FJsonObjectConverter::JsonObjectStringToUStruct<FWalletResponse>(
        JsonString, &OutResponse, 0, 0);
    UE_LOG(LogHttpProto, Warning,
        TEXT("[ParseWalletResponse] 解析%s | code=%d msg=%s balance=%d star=%d bonus=%d diamond=%d"),
        bOk ? TEXT("成功") : TEXT("失败"),
        OutResponse.code, *OutResponse.msg,
        OutResponse.data.balance, OutResponse.data.star,
        OutResponse.data.bonus, OutResponse.data.diamond);
    return bOk;
}
