#pragma once

#include "CoreMinimal.h"
#include "ShopperHttpTypes.generated.h"

// HTTP 方法。蓝图可按需扩展 Put/Delete（核心已原生支持）。
UENUM(BlueprintType)
enum class EShopperHttpVerb : uint8
{
	Get    UMETA(DisplayName = "GET"),
	Post   UMETA(DisplayName = "POST"),
	Put    UMETA(DisplayName = "PUT"),
	Delete UMETA(DisplayName = "DELETE"),
};
