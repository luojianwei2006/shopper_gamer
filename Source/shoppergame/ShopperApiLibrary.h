#pragma once

#include "CoreMinimal.h"
#include "HttpRetrySystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShopperHttpClient.h"
#include "ShopperApiLibrary.generated.h"

// ════════════════════════════════════════════════════════════════════════
// 以下为「示例接口」，请按真实后端替换 FXXXReq / FXXXResp 字段。
// 新增任意接口只需三步：
//   1) 定义 FReq / FResp 两个 USTRUCT（字段加 UPROPERTY，结构即 JSON 契约）
//   2) DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnXxxResp, bool, bSuccess, FXxxResp, Resp)
//   3) 写一个 UFUNCTION 调 Client->Request<FReq, FResp>(Verb, Endpoint, Req, ...)
// 蓝图里就能直接拖出该节点，无需再碰 C++。
// ════════════════════════════════════════════════════════════════════════

// ── 示例 1：登录（POST，请求体为 JSON）──
USTRUCT(BlueprintType)
struct FLoginReq
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Login") FString Account;
	UPROPERTY(BlueprintReadWrite, Category = "Login") FString Password;
};

USTRUCT(BlueprintType)
struct FLoginResp
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Login") int32 Code = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Login") FString Token;
	UPROPERTY(BlueprintReadWrite, Category = "Login") FString Nickname;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLoginResp, bool, bSuccess, FLoginResp, Resp);

// ── 示例 2：查询玩家档案（GET，请求结构体自动变成 ?playerId=xxx）──
USTRUCT(BlueprintType)
struct FGetPlayerProfileReq
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Profile") FString PlayerId;
};

USTRUCT(BlueprintType)
struct FGetPlayerProfileResp
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Profile") FString PlayerId;
	UPROPERTY(BlueprintReadWrite, Category = "Profile") FString Name;
	UPROPERTY(BlueprintReadWrite, Category = "Profile") int32 Level = 0;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetPlayerProfileResp, bool, bSuccess, FGetPlayerProfileResp, Resp);


UCLASS()
class SHOPPERGAME_API UShopperApiLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 示例：POST /api/login
	UFUNCTION(BlueprintCallable, Category = "Shopper|API", meta = (WorldContext = "WorldContextObject"))
	static void Login(UObject* WorldContextObject, const FLoginReq& Req, const FOnLoginResp& OnDone);

	// 示例：GET /api/player/profile?playerId=xxx （演示 query 自动拼接）
	UFUNCTION(BlueprintCallable, Category = "Shopper|API", meta = (WorldContext = "WorldContextObject"))
	static void GetPlayerProfile(UObject* WorldContextObject, const FGetPlayerProfileReq& Req, const FOnGetPlayerProfileResp& OnDone);
}
;
