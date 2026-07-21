#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "MoneyTypeRow.h"
#include "ShopperConfigSubsystem.generated.h"

// 全局配置子系统（GameInstance 级，跨关卡存活）
// 当前职责：持有货币类型 DataTable，并提供按 money_type 查询的接口
UCLASS()
class SHOPPERGAME_API UShopperConfigSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 按后端 money_type 值查配置；找不到时 bFound=false，OutRow 为默认值
    // 用法：FindMoneyTypeByCode(ShopItem.money_type, OutRow, bFound)
    //       -> bFound 为 true 时再用 OutRow.Icon / OutRow.MoneyName
    UFUNCTION(BlueprintCallable, Category = "Config|Money")
    void FindMoneyTypeByCode(int32 MoneyCode, FMoneyTypeRow& OutRow) const;

    // 列出全部货币类型（喂给 WB_ItemList 做展示）
    UFUNCTION(BlueprintCallable, Category = "Config|Money")
    TArray<FMoneyTypeRow> GetAllMoneyTypes() const;

protected:
    // 在 Default 蓝图或 C++ 默认值里指到你的货币类型 DataTable 资产
    UPROPERTY(EditDefaultsOnly, Category = "Config",
        meta = (AllowedClasses = "/Script/Engine.DataTable"))
    TSoftObjectPtr<UDataTable> MoneyTypeTable;

    // 解析软引用（已加载则直接返回，未加载同步加载一次并缓存）
    UDataTable* ResolveTable() const;
};
