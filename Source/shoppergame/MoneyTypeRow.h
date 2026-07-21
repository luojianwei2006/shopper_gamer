#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"      // FTableRowBase
#include "Engine/Texture2D.h"      // UTexture2D（具体 UObject 类型必须显式 include）
#include "MoneyTypeRow.generated.h"

// 货币类型配置行：行名（RowName）即后端的 money_type 值（如 "0" / "1" / "2"）
USTRUCT(BlueprintType)
struct FMoneyTypeRow : public FTableRowBase
{
    GENERATED_BODY()

    // 货币显示名（如「金币」「钻石」）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Money")
    FText MoneyName;

    // 货币图标（软引用，避免常驻内存）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Money")
    TSoftObjectPtr<UTexture2D> Icon;

    // 图标染色（同一张贴图多货币复用时区分）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Money")
    FLinearColor Tint = FLinearColor::White;
};
