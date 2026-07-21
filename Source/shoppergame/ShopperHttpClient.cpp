#include "ShopperHttpClient.h"
#include "Engine/GameInstance.h"

FString UShopperHttpClient::ResolveBaseUrl() const
{
	FString Base = GetDefault<UShopperHttpSettings>()->BaseUrl;
	Base.RemoveFromEnd(TEXT("/"));
	return Base;
}

float UShopperHttpClient::TimeoutSec() const
{
	return GetDefault<UShopperHttpSettings>()->TimeoutSec;
}

bool UShopperHttpClient::ShouldLog() const
{
	return GetDefault<UShopperHttpSettings>()->bLogRequests;
}

FString UShopperHttpClient::VerbToString(EShopperHttpVerb V)
{
	switch (V)
	{
		case EShopperHttpVerb::Post:   return TEXT("POST");
		case EShopperHttpVerb::Put:    return TEXT("PUT");
		case EShopperHttpVerb::Delete: return TEXT("DELETE");
		default:                       return TEXT("GET");
	}
}

void UShopperHttpClient::Deinitialize()
{
	for (const TSharedPtr<IHttpRequest>& Req : ActiveRequests)
	{
		if (Req.IsValid() && Req->GetStatus() == EHttpRequestStatus::Processing)
		{
			Req->CancelRequest();
		}
	}
	ActiveRequests.Empty();
	Super::Deinitialize();
}
