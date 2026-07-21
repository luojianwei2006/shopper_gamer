#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "ShopperHttpTypes.h"
#include "ShopperHttpSettings.h"
#include "ShopperHttpClient.generated.h"

// 把 JSON 值转成可用于 query 字符串的文本（GET 参数展平用）。
// FJsonValue 没有 ToString()，必须按类型取值；该辅助为内部使用（static 内部链接）。
static FString ShopperJsonValueToQueryString(const TSharedPtr<FJsonValue>& Value)
{
	if (!Value.IsValid()) return FString();
	switch (Value->Type)
	{
	case EJson::String:
		return Value->AsString();
	case EJson::Number:
	{
		const double N = Value->AsNumber();
		// 整数值不输出 .000000，保持 query 干净
		if (FMath::IsNearlyEqual(N, FMath::RoundToDouble(N)))
			return FString::Printf(TEXT("%.0f"), N);
		return FString::SanitizeFloat(N);
	}
	case EJson::Boolean:
		return Value->AsBool() ? TEXT("true") : TEXT("false");
	case EJson::Null:
		return FString();
	default:
		return FString();
	}
}

/**
 * 泛型 HTTP 客户端（GameInstanceSubsystem，跨关卡存活，统一托管在途请求）。
 *
 * 核心方法 Request<TRequest, TResponse> 是模板，类型安全、零运行时开销：
 *   - TRequest  / TResponse 必须是带 GENERATED_BODY() 且字段标记 UPROPERTY 的 USTRUCT，
 *     FJsonObjectConverter 据此自动（反）序列化。
 *   - Verb 为 Post / Put：请求结构体序列化为 JSON body。
 *   - Verb 为 Get / Delete：请求结构体序列化为 URL query 参数（字段名=值，自动 UrlEncode）。
 *   - OnComplete: (是否成功, 解析后的响应结构体, HTTP 状态码)
 *
 * 蓝图侧不要直接调模板（蓝图无法用模板），改为在 ShopperApiLibrary 里给每个接口
 * 写一行薄封装 UFUNCTION（见 ShopperApiLibrary.h 示例）。
 */
UCLASS()
class SHOPPERGAME_API UShopperHttpClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	template <typename TRequest, typename TResponse>
	void Request(EShopperHttpVerb Verb, const FString& Endpoint, const TRequest& RequestStruct,
		TFunction<void(bool bSuccess, const TResponse& Response, int32 HttpCode)>&& OnComplete,
		const FString& BaseUrlOverride = FString(),
		const TMap<FString, FString>& AdditionalHeaders = TMap<FString, FString>())
	{
		const FString Base = BaseUrlOverride.IsEmpty() ? ResolveBaseUrl() : BaseUrlOverride;
		FString Url = Base;
		if (!Url.EndsWith(TEXT("/"))) Url += TEXT("/");
		Url += Endpoint.StartsWith(TEXT("/")) ? Endpoint.Mid(1) : Endpoint;

		const bool bHasBody = (Verb == EShopperHttpVerb::Post || Verb == EShopperHttpVerb::Put);
		if (!bHasBody)
		{
			// GET / DELETE：把请求结构体展平成 query（字段名=UrlEncode(值)）
			TSharedPtr<FJsonObject> Json = FJsonObjectConverter::UStructToJsonObject(RequestStruct);
			if (Json.IsValid())
			{
				FString Query;
				for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Json->Values)
				{
					const FString Val = ShopperJsonValueToQueryString(Pair.Value);
					Query += FString::Printf(TEXT("%s=%s&"), *Pair.Key, *FGenericPlatformHttp::UrlEncode(Val));
				}
				if (!Query.IsEmpty())
				{
					Query.RemoveFromEnd(TEXT("&"));
					Url += TEXT("?") + Query;
				}
			}
		}

		TSharedRef<IHttpRequest> ReqRef = FHttpModule::Get().CreateRequest();
		TSharedPtr<IHttpRequest> Req = ReqRef;
		Req->SetURL(Url);
		Req->SetVerb(VerbToString(Verb));
		Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Req->SetTimeout(TimeoutSec());

		if (bHasBody)
		{
			FString Body;
			FJsonObjectConverter::UStructToJsonObjectString(RequestStruct, Body);
			Req->SetContentAsString(Body);
		}

		// 应用调用方自定义请求头（如 Authorization: <token>）；在 Content-Type 之后设，避免被覆盖
		for (const TPair<FString, FString>& Header : AdditionalHeaders)
		{
			Req->SetHeader(Header.Key, Header.Value);
		}

		TWeakObjectPtr<UShopperHttpClient> Self = this;
		ActiveRequests.Add(Req);
		Req->OnProcessRequestComplete().BindLambda(
			[Self, Req, Verb, Endpoint, OnComplete = MoveTemp(OnComplete)]
			(FHttpRequestPtr, FHttpResponsePtr Resp, bool bConnected)
			{
				TResponse Out{};
				bool bSuccess = false;
				const int32 Code = Resp.IsValid() ? Resp->GetResponseCode() : 0;

				if (Self.IsValid() && Self->ShouldLog())
				{
					UE_LOG(LogTemp, Log, TEXT("[HTTP] %s %s -> connected=%d code=%d"),
						*VerbToString(Verb), *Endpoint, bConnected, Code);
				}

				if (bConnected && Resp.IsValid() && Code >= 200 && Code < 300)
				{
					const FString Body = Resp->GetContentAsString();
					bSuccess = FJsonObjectConverter::JsonObjectStringToUStruct<TResponse>(Body, &Out);
					if (!bSuccess && Self.IsValid() && Self->ShouldLog())
					{
						UE_LOG(LogTemp, Warning, TEXT("[HTTP] %s 响应解析失败: %s"), *Endpoint, *Body);
					}
				}
				else if (Self.IsValid() && Self->ShouldLog())
				{
					UE_LOG(LogTemp, Warning, TEXT("[HTTP] %s 请求失败: connected=%d code=%d body=%s"),
						*Endpoint, bConnected, Code, Resp.IsValid() ? *Resp->GetContentAsString() : TEXT(""));
				}

				if (Self.IsValid())
				{
					Self->ActiveRequests.Remove(Req);
				}
				OnComplete(bSuccess, Out, Code);
			});

		Req->ProcessRequest();
	}

	virtual void Deinitialize() override;

private:
	FString ResolveBaseUrl() const;
	float TimeoutSec() const;
	bool ShouldLog() const;
	static FString VerbToString(EShopperHttpVerb V);

	// 在途请求（非 UObject，不标记 UPROPERTY；子系统销毁时统一 Cancel）
	TArray<TSharedPtr<IHttpRequest>> ActiveRequests;
};
