#include "HttpProtoBFL.h"
#include "DeviceSaveGame.h"
#include "Engine/GameInstance.h"      // UGameInstance 完整定义（GetSubsystem 成员访问需要）
#include "Kismet/GameplayStatics.h"   // 存档读写 / GetGameInstance
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

FString UHttpProtoBFL::GetApiBaseUrl()
{
	return UShopperSettings::ResolveBaseUrl();
}

int32 UHttpProtoBFL::GetAppVersion()
{
	return UShopperSettings::ResolveAppVersion();
}

void UHttpProtoBFL::SendGuestLogin(
    UObject* WorldContextObject,
    const FString& Host,
    const FGuestLoginProto& Proto,
    const FOnGuestLoginDone& OnComplete)
{
    // Host 为空时自动用项目设置里的接口基地址（环境配置）；否则用传入 Host 覆盖
    const FString EffectiveHost = Host.IsEmpty() ? GetApiBaseUrl() : Host;

    // 取 GameInstance 上的泛型 HTTP 客户端子系统（与 ShopperApiLibrary 写法一致）
    UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
    UShopperHttpClient* Client = GI ? GI->GetSubsystem<UShopperHttpClient>() : nullptr;
    if (!Client)
    {
        UE_LOG(LogHttpProto, Warning,
            TEXT("[SendGuestLogin] 无法获取 UShopperHttpClient 子系统，请求未发出"));
        OnComplete.ExecuteIfBound(false, FLoginResponse());
        return;
    }

    // 委托给泛型客户端：POST /user/guest_login，响应直接反序列化为 FLoginResponse
    Client->Request<FGuestLoginProto, FLoginResponse>(
        EShopperHttpVerb::Post,
        TEXT("user/guest_login"),
        Proto,
        [OnComplete](bool bSuccess, const FLoginResponse& Resp, int32 HttpCode)
        {
            // 登录成功 → 自动持久化游客 token，下次启动可直接 login_by_token 续期
            if (HttpCode == 200 && !Resp.token.IsEmpty())
            {
                UHttpProtoBFL::SaveSessionToken(Resp.token);
            }
            // 直接把类型化响应结构体交给蓝图（不再序列化为 JSON 字符串）
            OnComplete.ExecuteIfBound(bSuccess, Resp);
        },
        EffectiveHost);
}

// ───────────────────────────────────────────────────────────
// Token 续期登录：POST /user/login_by_token，结构与游客登录一致
// ───────────────────────────────────────────────────────────
void UHttpProtoBFL::SendLoginByToken(
    UObject* WorldContextObject,
    const FString& Host,
    const FLoginByTokenProto& Proto,
    const FOnLoginByTokenDone& OnComplete)
{
    // Host 为空时自动用项目设置里的接口基地址（环境配置）；否则用传入 Host 覆盖
    const FString EffectiveHost = Host.IsEmpty() ? GetApiBaseUrl() : Host;

    UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
    UShopperHttpClient* Client = GI ? GI->GetSubsystem<UShopperHttpClient>() : nullptr;
    if (!Client)
    {
        UE_LOG(LogHttpProto, Warning,
            TEXT("[SendLoginByToken] 无法获取 UShopperHttpClient 子系统，请求未发出"));
        OnComplete.ExecuteIfBound(false, FLoginResponse());
        return;
    }

    Client->Request<FLoginByTokenProto, FLoginResponse>(
        EShopperHttpVerb::Post,
        TEXT("user/login_by_token"),
        Proto,
        [OnComplete](bool bSuccess, const FLoginResponse& Resp, int32 HttpCode)
        {
            // 续期成功 → 刷新本地 token（后端可能返回新 token）
            if (HttpCode == 200 && !Resp.token.IsEmpty())
            {
                UHttpProtoBFL::SaveSessionToken(Resp.token);
            }
            // 直接把类型化响应结构体交给蓝图
            OnComplete.ExecuteIfBound(bSuccess, Resp);
        },
        EffectiveHost);
}

// ───────────────────────────────────────────────────────────
// 钱包接口：POST /user/get_wallet，Authorization 头带 token，body 留空
// 统一走 UShopperHttpClient 泛型客户端（与登录函数一致）
// ───────────────────────────────────────────────────────────
void UHttpProtoBFL::SendGetWallet(
    UObject* WorldContextObject,
    const FString& Host,
    const FString& Token,
    const FOnWalletDone& OnComplete)
{
    // Host 为空时自动用项目设置里的接口基地址（环境配置）；否则用传入 Host 覆盖
    const FString EffectiveHost = Host.IsEmpty() ? GetApiBaseUrl() : Host;

    // 取 GameInstance 上的泛型 HTTP 客户端子系统（与登录函数写法一致）
    UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
    UShopperHttpClient* Client = GI ? GI->GetSubsystem<UShopperHttpClient>() : nullptr;
    if (!Client)
    {
        UE_LOG(LogHttpProto, Warning,
            TEXT("[SendGetWallet] 无法获取 UShopperHttpClient 子系统，请求未发出"));
        OnComplete.ExecuteIfBound(false, FWalletResponse());
        return;
    }

    // 鉴权头：值直接为登录拿到的 token
    // 若后端要求 "Bearer <token>" 形式，改成：TEXT("Bearer ") + Token
    TMap<FString, FString> Headers;
    if (!Token.IsEmpty())
    {
        Headers.Add(TEXT("Authorization"), Token);
    }

    // 请求体留空（FGetWalletProto 无字段）；响应直接反序列化为 FWalletResponse
    FGetWalletProto ReqProto;
    Client->Request<FGetWalletProto, FWalletResponse>(
        EShopperHttpVerb::Post,
        TEXT("user/get_wallet"),
        ReqProto,
        [OnComplete](bool bSuccess, const FWalletResponse& Resp, int32 HttpCode)
        {
            // 直接把类型化响应结构体交给蓝图
            OnComplete.ExecuteIfBound(bSuccess, Resp);
        },
        EffectiveHost,
        Headers);
}

void UHttpProtoBFL::SendGetShopList(UObject* WorldContextObject, const FString& Host, const FString& Token,
	const FOnShopListDone& OnComplete)
{
	// Host 为空时自动用项目设置里的接口基地址（环境配置）；否则用传入 Host 覆盖
	const FString EffectiveHost = Host.IsEmpty() ? GetApiBaseUrl() : Host;

	// 取 GameInstance 上的泛型 HTTP 客户端子系统（与登录函数写法一致）
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	UShopperHttpClient* Client = GI ? GI->GetSubsystem<UShopperHttpClient>() : nullptr;
	if (!Client)
	{
		UE_LOG(LogHttpProto, Warning,
			TEXT("[SendGetShopList] 无法获取 UShopperHttpClient 子系统，请求未发出"));
		OnComplete.ExecuteIfBound(false, FShopListResponse());
		return;
	}

	// 鉴权头：值直接为登录拿到的 token
	// 若后端要求 "Bearer <token>" 形式，改成：TEXT("Bearer ") + Token
	TMap<FString, FString> Headers;
	if (!Token.IsEmpty())
	{
		Headers.Add(TEXT("Authorization"), Token);
	}

	// 请求体留空（FGetShopListProto 无字段）；响应直接反序列化为 FShopListResponse
	FGetShopListProto ReqProto;
	Client->Request<FGetShopListProto, FShopListResponse>(
		EShopperHttpVerb::Post,
		TEXT("shop/list"),
		ReqProto,
		[OnComplete](bool bSuccess, const FShopListResponse& Resp, int32 HttpCode)
		{
			// 直接把类型化响应结构体交给蓝图
			OnComplete.ExecuteIfBound(bSuccess, Resp);
		},
		EffectiveHost,
		Headers);
}