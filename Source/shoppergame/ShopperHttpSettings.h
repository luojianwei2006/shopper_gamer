#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ShopperHttpSettings.generated.h"

// HTTP 客户端全局配置：在「项目设置 → Shopper HTTP 配置」里维护，自动写入 DefaultGame.ini。
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Shopper HTTP 配置"))
class SHOPPERGAME_API UShopperHttpSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 接口基地址（结尾不要带斜杠）。例如 http://192.168.10.8:8080
	UPROPERTY(Config, EditAnywhere, Category = "HTTP")
	FString BaseUrl = TEXT("http://127.0.0.1:8080");

	// 单次请求超时（秒）
	UPROPERTY(Config, EditAnywhere, Category = "HTTP")
	float TimeoutSec = 30.f;

	// 是否在 Output Log 打印请求/响应（调试用）
	UPROPERTY(Config, EditAnywhere, Category = "HTTP")
	bool bLogRequests = true;
};
