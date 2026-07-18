#include "LoadingPlanAsset.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"   // 与本项目 HotReloadManager.cpp 一致的入口（UE5.6 实测可用）：声明 IAssetRegistry / FARFilter 及静态 Get()
#include "AssetRegistry/AssetData.h"              // FAssetData 完整定义（GetSoftObjectPath() 需要，模块头只前向声明）
#include "Engine/Texture2D.h"

void ULoadingPlanAsset::ScanFoldersIntoUILoadGroups()
{
	// 递归扫描各虚拟路径下的 Texture2D，收集到 Collected（去重）
	TArray<TSoftObjectPtr<UTexture2D>> Collected;
	IAssetRegistry* AR = IAssetRegistry::Get();
	if (!AR)
	{
		UE_LOG(LogTemp, Error, TEXT("[LoadingPlan] AssetRegistry 不可用，无法扫描文件夹。"));
		return;
	}

	for (const FDirectoryPath& Folder : UIScanFolders)
	{
		FString RawPath = Folder.Path;
		if (RawPath.IsEmpty()) continue;

		// ── 路径归一化（兜底手输错误；用文件夹选择器时通常已是 /Game/...） ──
		RawPath.TrimStartAndEndInline();
		while (RawPath.EndsWith(TEXT("/"))) { RawPath.LeftChopInline(1); }
		// 若填的是磁盘绝对路径（含 Content/），转成 /Game/... 虚拟路径
		// FString 没有 FindLast：用 Find(..., FromEnd)，返回 int32 索引（未命中=INDEX_NONE）
		int32 ContentIdx = RawPath.Find(TEXT("Content/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (ContentIdx != INDEX_NONE)
		{
			RawPath = TEXT("/Game/") + RawPath.Mid(ContentIdx + FCString::Strlen(TEXT("Content/")));
		}
		else if (RawPath.StartsWith(TEXT("Game/")))
		{
			RawPath = TEXT("/") + RawPath;          // Game/UI -> /Game/UI
		}
		else if (!RawPath.StartsWith(TEXT("/Game")))
		{
			RawPath = TEXT("/Game/") + RawPath;     // UI -> /Game/UI
		}

		// 用 FARFilter 递归扫描指定虚拟路径下的 Texture2D，跨 UE5 版本签名稳定
		FARFilter Filter;
		Filter.PackagePaths.Add(FName(*RawPath));
		Filter.bRecursivePaths = true;
		Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
		Filter.bIncludeOnlyOnDiskAssets = false;

		TArray<FAssetData> Assets;
		AR->GetAssets(Filter, Assets);

		UE_LOG(LogTemp, Log, TEXT("[LoadingPlan] 扫描目录 %s : 命中 %d 个 Texture2D"),
			*RawPath, Assets.Num());

		for (const FAssetData& AD : Assets)
		{
			Collected.AddUnique(TSoftObjectPtr<UTexture2D>(AD.GetSoftObjectPath()));
		}
	}

	if (Collected.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[LoadingPlan] 扫描到 0 张图片。请确认 UIScanFolders 填的是虚拟路径（如 /Game/UI/Common），"
				"且资产已保存到磁盘（未保存的资产不会被 AssetRegistry 索引）。"));
		return;
	}

	// 找或建目标组
	FUILoadingGroup* Target = UILoadGroups.FindByPredicate(
		[this](const FUILoadingGroup& G) { return G.GroupName == ScannedGroupName; });
	if (!Target)
	{
		FUILoadingGroup NewGroup;
		NewGroup.GroupName = ScannedGroupName;
		NewGroup.DisplayStatus = FText::Format(
			NSLOCTEXT("LoadingPlan", "ScanStatus", "预载 {0}"), FText::FromName(ScannedGroupName));
		UILoadGroups.Add(NewGroup);
		Target = &UILoadGroups.Last();
	}

	int32 Added = 0;
	for (const TSoftObjectPtr<UTexture2D>& Ptr : Collected)
	{
		if (!Target->Textures.Contains(Ptr))
		{
			Target->Textures.Add(Ptr);
			++Added;
		}
	}

	// 让改动可保存
	Modify();
	MarkPackageDirty();

	UE_LOG(LogTemp, Log,
		TEXT("[LoadingPlan] 扫描到 %d 张图片，新增 %d 张到组 '%s'（该组现共 %d 张）。记得 Ctrl+S 保存资产。"),
		Collected.Num(), Added, *ScannedGroupName.ToString(), Target->Textures.Num());
}
#endif // WITH_EDITOR
