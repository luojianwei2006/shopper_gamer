#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoadingSubsystem.generated.h"

class ULoadingPlanAsset;

// 进度广播（0..1 百分比 + 状态文案）。DYNAMIC 多播委托，Blueprint 可用 Bind Event 绑定刷新进度条。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadingProgress, float, Percent, FText, Status);
// 加载完成广播（传出目标关卡名）。DYNAMIC 多播，Blueprint 绑定后执行 Open Level。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingComplete, FName, TargetLevel);

/**
 * 启动 / 关卡加载闸门（GameInstanceSubsystem，随游戏实例全程存在）。
 * 职责：
 *   - 按 Phase 顺序异步加载：热更包挂载+配置 → Widget 预载 → UI 分组预载 → 重资源预载；
 *   - 聚合各 Phase 进度，通过 OnLoadingProgress 广播（C++ 算，Blueprint 显示）；
 *   - 全部完成后通过 OnLoadingComplete 广播目标关卡名，由 Blueprint 调用 Open Level。
 *
 * 进度计算与异步回调全部在 C++；进度条 / 提示语 / Open Level 在 Blueprint。
 * 不阻塞游戏线程：所有加载走 StreamableManager / pak 挂载的异步路径。
 */
UCLASS()
class SHOPPERGAME_API ULoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 启动加载流程。Plan 为空则加载默认路径 /Game/Loading/LoadingPlan.LoadingPlan。
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void StartLoading(ULoadingPlanAsset* Plan = nullptr);

	// Blueprint 绑定：进度 / 完成
	UPROPERTY(BlueprintAssignable, Category = "Loading")
	FOnLoadingProgress OnLoadingProgress;

	UPROPERTY(BlueprintAssignable, Category = "Loading")
	FOnLoadingComplete OnLoadingComplete;

private:
	void RunCurrentPhase();
	void BeginPhase(float Weight, const FText& Status);
	void AdvancePhase(float Fraction);   // 0..1，当前 Phase 内部进度
	void EndPhase();
	void ReportProgress();               // 计算总进度并广播
	void FinishLoading();

	// ── Phase 实现 ──
	void PhaseHotfix();                  // 挂载热更 pak + 应用配置覆盖层
	void PhaseWidgets();                 // 预载四个 UI Widget 软类
	void PhaseUI();                      // 按 UILoadGroups 顺序逐组预载界面图片
	void PhaseAssets();                  // 预载通用重资源 + 关卡专用资源

	// UI 分组加载：在 PhaseUI 内驱动逐组异步加载
	void LoadNextUIGroup();

	// 各 Phase 完成回调
	void OnWidgetsPreloaded();
	void OnOneUITextureLoaded();
	void OnOneAssetLoaded();

	UPROPERTY()
	ULoadingPlanAsset* ActivePlan = nullptr;

	int32 PhaseIndex = 0;
	float CompletedWeight = 0.f;        // 已完成 Phase 权重累计
	float CurrentPhaseWeight = 0.f;     // 当前 Phase 权重
	float CurrentPhaseFraction = 0.f;   // 当前 Phase 内部完成度 0..1
	FText CurrentStatus;

	int32 AssetsTotal = 0;
	int32 AssetsDone = 0;

	int32 UITotal = 0;                 // UI 图片预载总数（已过滤空/无效路径）
	int32 UIDone = 0;                  // UI 图片已完成数
	int32 CurrentUIGroupIndex = 0;     // 当前正在加载的 UI 组索引
	int32 CurrentUIGroupDone = 0;      // 当前组已完成数
	int32 CurrentUIGroupRequested = 0; // 当前组实际发起的异步请求数（过滤空路径后）

	bool bLoading = false;              // 防止重复 StartLoading

	static constexpr const TCHAR* DefaultPlanPath = TEXT("/Game/Loading/LoadingPlan.LoadingPlan");
};
