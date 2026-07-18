#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LoadingPlanAsset.generated.h"

class UShopperHotfixConfig;
class UWidgetRegistry;
class UTexture2D;

/**
 * UI 预载组。设计师按界面划分（Common / Login / Menu / Setting / Game），
 * 每组填一个显示名称 + 该界面用到的软引用贴图。LoadingSubsystem 会按数组顺序
 * 逐组加载，并在进度条上显示当前组名，避免“Login 图把 Menu 图一起提前拉”造成的内存浪费。
 */
USTRUCT(BlueprintType)
struct SHOPPERGAME_API FUILoadingGroup
{
	GENERATED_BODY()

	// 组标识，仅用于日志和默认状态文案（如 Login、Menu、Game）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Group")
	FName GroupName = FName("Common");

	// 该组加载时显示在进度条上的文案。留空则使用默认格式“预载 GroupName”。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Group")
	FText DisplayStatus;

	// 该界面/该组要预载的贴图。建议用软引用 + 运行时 SetBrush；
	// 若直接在 Widget 蓝图中硬引用，Widget 类加载时会被自动拉入（此处再列一次无害，且能让进度条走）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Group")
	TArray<TSoftObjectPtr<UTexture2D>> Textures;
};

/**
 * 启动 / 关卡加载计划（DataAsset）。设计师在编辑器里填入：
 *   - HotfixPakPath : 可选，热更 pak 的磁盘路径；为空则跳过挂载
 *   - HotfixConfig  : 可选，来自热更 pak 的配置覆盖层，加载后应用到喇叭 socket
 *   - WidgetRegistry: 可选，交给 WidgetManager 预载四个 Widget 软类（为空用默认路径）
 *   - UILoadGroups  : 各界面图片分组预载（Common / Login / Menu / Setting / Game 等）
 *   - PreloadAssets : 通用重资源（贴图 / 模型 / 材质 / Niagara 等）
 *   - LevelPreloadAssets: 目标关卡专用重资源（场景模型 / 大地形 / 光照贴图等）
 *   - TargetLevel   : 加载完成后 OpenLevel 的目标关卡名（Game / Menu …）
 *
 * 建议按“目标关卡”建多个 LoadingPlan 资产：
 *   LP_Login → TargetLevel=Login / UILoadGroups={Common, Login}
 *   LP_Menu  → TargetLevel=Menu  / UILoadGroups={Common, Menu, Setting}
 *   LP_Game  → TargetLevel=Game  / UILoadGroups={Common, Game} + LevelPreloadAssets=场景资源
 * LoadingSubsystem 按 Phase 顺序异步加载，并聚合进度广播给 Blueprint。
 */
UCLASS(BlueprintType)
class SHOPPERGAME_API ULoadingPlanAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 可选：热更 pak 磁盘路径（如 "../../../shoppergame/Content/Hotfix/hotfix_p.pak"）。空 = 跳过挂载。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	FString HotfixPakPath;

	// 可选：热更配置覆盖层（来自热更 pak）。空 = 回退 ShopperSettings。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	TSoftObjectPtr<UShopperHotfixConfig> HotfixConfig;

	// 可选：Widget 注册表（交给 WidgetManager 预载）。空 = 用默认路径 /Game/UI/WidgetRegistry。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	TSoftObjectPtr<UWidgetRegistry> WidgetRegistry;

	// 各界面图片分组预载。LoadingSubsystem 在 Widget 预载之后、重资源之前按数组顺序逐组加载，
	// 并在进度条上显示当前组名（DisplayStatus）。
	// 建议分组：Common（公共按钮/图标）、Login、Menu、Setting、Game。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading|UI")
	TArray<FUILoadingGroup> UILoadGroups;

#if WITH_EDITORONLY_DATA
	// ── 编辑器扫描工具（仅编辑器可用，不参与烘焙） ──
	// 用右侧“...”文件夹选择器挑 Content Browser 目录（如 /Game/UI/Common、/Game/UI/Login），
	// 多个文件夹可叠加。RelativeToGameContentDir 保证存的是 /Game/... 虚拟路径，无需手输。
	// 这些文件夹通常是各界面“共用图片”的归总目录，避免逐界面手动拖图。
	UPROPERTY(EditAnywhere, Category = "Loading|UI|Scan", meta = (RelativeToGameContentDir = "true"))
	TArray<FDirectoryPath> UIScanFolders;

	// 扫描到的图片归入此组名（默认 Common）。若 UILoadGroups 已有同名组则追加，否则新建。
	UPROPERTY(EditAnywhere, Category = "Loading|UI|Scan")
	FName ScannedGroupName = FName("Common");
#endif

#if WITH_EDITOR
	// 在 Details 面板点击执行：递归扫描 UIScanFolders 下所有 Texture2D，
	// 去重后填入 ScannedGroupName 对应的 UILoadGroups 组，并标记资产为脏（可保存）。
	UFUNCTION(CallInEditor, Category = "Loading|UI|Scan")
	void ScanFoldersIntoUILoadGroups();
#endif

	// 通用重资源（美术 / 模型 / 贴图 / Niagara 等），所有关卡都可能用到。
	// 这里是软引用，不会在加载计划本身时把资源拉进来；加载闸门 Phase 4 才真正 RequestAsyncLoad。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	TArray<TSoftObjectPtr<UObject>> PreloadAssets;

	// 目标关卡专用重资源（场景模型 / 大地形 / 光照贴图 / 关卡专属 Blueprint 等）。
	// 与 PreloadAssets 合并后在 Phase 4 异步加载，但分类存放方便按关卡维护。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading|Level")
	TArray<TSoftObjectPtr<UObject>> LevelPreloadAssets;

	// 加载完成后要打开的目标关卡（Game / Menu / Login 等）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	FName TargetLevel = NAME_None;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("LoadingPlan", GetFName());
	}
};
