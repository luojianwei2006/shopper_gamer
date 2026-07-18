#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Blueprint/UserWidget.h"   // UUserWidget（TSoftClassPtr 模板实参需要完整类型）
#include "WidgetRegistry.generated.h"

/**
 * UI Widget 登记表（随热更 pak 可下发，运行时覆盖）。
 * 集中登记所有 UMG 蓝图 widget 的软引用（TSoftClassPtr），按逻辑名（FName）索引，
 * 编译期不硬加载，Cooked 后可被打进热更 pak 实现热更。
 *
 * 使用方式：
 *   1. 在 /Game/UI 下建一个本资产实例（WidgetRegistry）；
 *   2. 在 Widgets 映射里按名字登记任意数量的 widget（不止 Login/Menu/Setting/Game，
 *      例如 Shop / Profile / Rank / Mail …），key 即逻辑名；
 *   3. 把它们打进热更 pak 下发；
 *   4. UWidgetManager 在启动和热更时按软引用加载 / 重解析；
 *   5. 运行时用 UWidgetManager::CreateWidgetOfType(逻辑名) 按 key 取类并创建。
 */
UCLASS(BlueprintType)
class SHOPPERGAME_API UWidgetRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 全部用软引用：编译期不加载，运行时由 WidgetManager 异步加载 / 可被 pak 覆盖。
	// key = widget 逻辑名（如 "Login" / "Menu" / "Shop" / "Profile"），数量不限；
	// 该 key 与 PlayerController 的 StartupWidgetKey、WidgetManager::CreateWidgetOfType(Key) 对应。
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TMap<FName, TSoftClassPtr<UUserWidget>> Widgets;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("WidgetRegistry", GetFName());
	}
};
