#include "ShopperApiLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UShopperApiLibrary::Login(UObject* WorldContextObject, const FLoginReq& Req, const FOnLoginResp& OnDone)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	UShopperHttpClient* Client = GI ? GI->GetSubsystem<UShopperHttpClient>() : nullptr;
	if (!Client)
	{
		OnDone.ExecuteIfBound(false, FLoginResp{});
		return;
	}
	Client->Request<FLoginReq, FLoginResp>(EShopperHttpVerb::Post, TEXT("/api/login"), Req,
		[OnDone](bool bOk, const FLoginResp& Resp, int32)
		{
			OnDone.ExecuteIfBound(bOk, Resp);
		});
}

void UShopperApiLibrary::GetPlayerProfile(UObject* WorldContextObject, const FGetPlayerProfileReq& Req, const FOnGetPlayerProfileResp& OnDone)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	UShopperHttpClient* Client = GI ? GI->GetSubsystem<UShopperHttpClient>() : nullptr;
	if (!Client)
	{
		OnDone.ExecuteIfBound(false, FGetPlayerProfileResp{});
		return;
	}
	Client->Request<FGetPlayerProfileReq, FGetPlayerProfileResp>(EShopperHttpVerb::Get, TEXT("/api/player/profile"), Req,
		[OnDone](bool bOk, const FGetPlayerProfileResp& Resp, int32)
		{
			OnDone.ExecuteIfBound(bOk, Resp);
		});
}

