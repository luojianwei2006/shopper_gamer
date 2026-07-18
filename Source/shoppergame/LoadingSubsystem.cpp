#include "LoadingSubsystem.h"
#include "LoadingPlanAsset.h"
#include "HotReloadManager.h"
#include "WidgetManager.h"
#include "ShopperSocketSubsystem.h"
#include "ShopperHotfixConfig.h"   // USShopperHotfixConfig 完整类型：TSoftObjectPtr::IsValid/LoadSynchronous 需 dynamic_cast，前向声明不够
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UObject/SoftObjectPtr.h"
#include "Engine/Texture2D.h"          // TSoftObjectPtr<UTexture2D> 完整类型（ToSoftObjectPath 用）

void ULoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULoadingSubsystem::StartLoading(ULoadingPlanAsset* Plan)
{
	if (bLoading) return;   // 防止 Loading 关卡重复 BeginPlay 触发重入
	bLoading = true;

	ActivePlan = Plan ? Plan : LoadObject<ULoadingPlanAsset>(nullptr, DefaultPlanPath);
	if (!ActivePlan)
	{
		UE_LOG(LogTemp, Error, TEXT("[Loading] 未找到 LoadingPlan，直接完成加载"));
		bLoading = false;
		OnLoadingComplete.Broadcast(NAME_None);
		return;
	}

	PhaseIndex = 0;
	CompletedWeight = 0.f;
	RunCurrentPhase();
}

void ULoadingSubsystem::RunCurrentPhase()
{
	switch (PhaseIndex)
	{
	case 0: BeginPhase(0.15f, NSLOCTEXT("Loading", "Hotfix",   "挂载热更包"));   PhaseHotfix();  break;
	case 1: BeginPhase(0.20f, NSLOCTEXT("Loading", "Widgets",  "预载界面资源")); PhaseWidgets(); break;
	case 2: BeginPhase(0.25f, NSLOCTEXT("Loading", "UI",       "预载界面图片")); PhaseUI();      break;
	case 3: BeginPhase(0.40f, NSLOCTEXT("Loading", "Assets",   "预载美术资源")); PhaseAssets();  break;
	default: FinishLoading(); break;
	}
}

void ULoadingSubsystem::BeginPhase(float Weight, const FText& Status)
{
	CurrentPhaseWeight = Weight;
	CurrentPhaseFraction = 0.f;
	CurrentStatus = Status;
	ReportProgress();
}

void ULoadingSubsystem::AdvancePhase(float Fraction)
{
	CurrentPhaseFraction = FMath::Clamp(Fraction, 0.f, 1.f);
	ReportProgress();
}

void ULoadingSubsystem::EndPhase()
{
	CompletedWeight += CurrentPhaseWeight;
	CurrentPhaseFraction = 1.f;
	ReportProgress();
	++PhaseIndex;
	RunCurrentPhase();
}

void ULoadingSubsystem::ReportProgress()
{
	// Phase 权重之和 = 1.0（0.15 热更 + 0.20 Widget + 0.25 登录图片 + 0.40 重资源）
	const float Total = 0.15f + 0.20f + 0.25f + 0.40f;
	const float Overall = (CompletedWeight + CurrentPhaseWeight * CurrentPhaseFraction) / Total;
	OnLoadingProgress.Broadcast(Overall, CurrentStatus);
}

void ULoadingSubsystem::FinishLoading()
{
	const FName Target = ActivePlan ? ActivePlan->TargetLevel : NAME_None;
	bLoading = false;
	OnLoadingComplete.Broadcast(Target);
}

void ULoadingSubsystem::PhaseHotfix()
{
	// 1) 挂载热更 pak（可选）
	if (ActivePlan && !ActivePlan->HotfixPakPath.IsEmpty())
	{
		if (UHotReloadManager* HR = GetGameInstance()->GetSubsystem<UHotReloadManager>())
		{
			HR->MountPak(ActivePlan->HotfixPakPath, 100);
		}
	}
	// 2) 加载并应用配置覆盖层（可能来自刚挂载的 pak）
	if (ActivePlan && ActivePlan->HotfixConfig.IsValid())
	{
		if (UShopperHotfixConfig* Cfg = ActivePlan->HotfixConfig.LoadSynchronous())
		{
			if (UShopperSocketSubsystem* Sock = GetGameInstance()->GetSubsystem<UShopperSocketSubsystem>())
			{
				Sock->ApplyHotfixConfig(Cfg);
			}
		}
	}
	AdvancePhase(1.f);
	EndPhase();
}

void ULoadingSubsystem::PhaseWidgets()
{
	if (UWidgetManager* WM = GetGameInstance()->GetSubsystem<UWidgetManager>())
	{
		if (WM->AreWidgetsLoaded())
		{
			// 已在启动时预载完成（GameInstanceSubsystem 先于关卡初始化），直接过
			AdvancePhase(1.f);
			EndPhase();
		}
		else
		{
			// 仅当尚未加载时才绑定 + 触发；避免重复绑定导致闸门多推进一次
			WM->OnWidgetsPreloaded.AddUObject(this, &ULoadingSubsystem::OnWidgetsPreloaded);
			WM->LoadAllWidgetsAsync();
		}
	}
	else
	{
		AdvancePhase(1.f);
		EndPhase();
	}
}

void ULoadingSubsystem::OnWidgetsPreloaded()
{
	AdvancePhase(1.f);
	EndPhase();
}

void ULoadingSubsystem::PhaseUI()
{
	if (!ActivePlan || ActivePlan->UILoadGroups.Num() == 0)
	{
		AdvancePhase(1.f);
		EndPhase();
		return;
	}

	// 先统计所有有效 UI 贴图总数，作为 Phase 内进度分母
	UITotal = 0;
	for (const FUILoadingGroup& Group : ActivePlan->UILoadGroups)
	{
		for (const TSoftObjectPtr<UTexture2D>& Ptr : Group.Textures)
		{
			if (!Ptr.IsNull()) ++UITotal;
		}
	}

	UIDone = 0;
	CurrentUIGroupIndex = 0;
	CurrentUIGroupDone = 0;
	CurrentUIGroupRequested = 0;

	if (UITotal == 0)
	{
		// 所有 UI 组都是空的，直接跳过
		AdvancePhase(1.f);
		EndPhase();
		return;
	}

	AdvancePhase(0.f);
	LoadNextUIGroup();
}

void ULoadingSubsystem::LoadNextUIGroup()
{
	if (!ActivePlan)
	{
		EndPhase();
		return;
	}

	while (CurrentUIGroupIndex < ActivePlan->UILoadGroups.Num())
	{
		const FUILoadingGroup& Group = ActivePlan->UILoadGroups[CurrentUIGroupIndex];

		// 过滤无效路径，只加载真正配置了的贴图
		TArray<FSoftObjectPath> Paths;
		Paths.Reserve(Group.Textures.Num());
		for (const TSoftObjectPtr<UTexture2D>& Ptr : Group.Textures)
		{
			if (!Ptr.IsNull()) Paths.Add(Ptr.ToSoftObjectPath());
		}

		if (Paths.Num() > 0)
		{
			CurrentUIGroupDone = 0;
			CurrentUIGroupRequested = Paths.Num();
			CurrentStatus = Group.DisplayStatus.IsEmpty()
				? FText::Format(NSLOCTEXT("Loading", "UIGroupDefault", "预载 {0}"), FText::FromName(Group.GroupName))
				: Group.DisplayStatus;
			ReportProgress();

			FStreamableManager& Mgr = UAssetManager::Get().GetStreamableManager();
			for (const FSoftObjectPath& P : Paths)
			{
				Mgr.RequestAsyncLoad(P, FStreamableDelegate::CreateUObject(this, &ULoadingSubsystem::OnOneUITextureLoaded));
			}
			return;
		}

		// 当前组为空，直接跳过
		++CurrentUIGroupIndex;
	}

	// 所有组处理完毕
	EndPhase();
}

void ULoadingSubsystem::OnOneUITextureLoaded()
{
	// EndPhase 会把 PhaseIndex 推到 3，后续回调直接忽略，防止双发 EndPhase
	if (PhaseIndex != 2 || !ActivePlan) return;

	++UIDone;
	++CurrentUIGroupDone;
	AdvancePhase(static_cast<float>(UIDone) / static_cast<float>(FMath::Max(UITotal, 1)));

	if (CurrentUIGroupDone >= CurrentUIGroupRequested)
	{
		++CurrentUIGroupIndex;
		LoadNextUIGroup();
		return;
	}
}

void ULoadingSubsystem::PhaseAssets()
{
	TArray<FSoftObjectPath> Paths;
	if (ActivePlan)
	{
		for (const TSoftObjectPtr<UObject>& Ptr : ActivePlan->PreloadAssets)
		{
			if (!Ptr.IsNull()) Paths.Add(Ptr.ToSoftObjectPath());
		}
		for (const TSoftObjectPtr<UObject>& Ptr : ActivePlan->LevelPreloadAssets)
		{
			if (!Ptr.IsNull()) Paths.Add(Ptr.ToSoftObjectPath());
		}
	}

	if (Paths.Num() == 0)
	{
		AdvancePhase(1.f);
		EndPhase();
		return;
	}

	AssetsTotal = Paths.Num();
	AssetsDone = 0;
	FStreamableManager& Mgr = UAssetManager::Get().GetStreamableManager();
	for (const FSoftObjectPath& P : Paths)
	{
		// 每个资产单独 RequestAsyncLoad，用完成计数驱动进度
		Mgr.RequestAsyncLoad(P, FStreamableDelegate::CreateUObject(this, &ULoadingSubsystem::OnOneAssetLoaded));
	}
	AdvancePhase(0.f);
}

void ULoadingSubsystem::OnOneAssetLoaded()
{
	++AssetsDone;
	AdvancePhase(static_cast<float>(AssetsDone) / static_cast<float>(FMath::Max(AssetsTotal, 1)));
	if (AssetsDone >= AssetsTotal)
	{
		EndPhase();
	}
}
