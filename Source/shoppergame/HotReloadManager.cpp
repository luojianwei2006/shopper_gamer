#include "HotReloadManager.h"

#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformFile.h"
#include "IPlatformFilePak.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "UObject/UObjectGlobals.h"

void UHotReloadManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

FPakPlatformFile* UHotReloadManager::GetPakPlatformFile()
{
	// FindPlatformFile 会沿平台文件栈（含下层）按类型名查找，
	// 比手动 GetLowerLevel 遍历更稳：编辑器 / shipping 都能正确取到 PakPlatformFile。
	IPlatformFile* PF = FPlatformFileManager::Get().FindPlatformFile(FPakPlatformFile::GetTypeName());
	return static_cast<FPakPlatformFile*>(PF);
}

bool UHotReloadManager::MountPak(const FString& PakPath, int32 PakOrder)
{
	FString Normalized = PakPath;
	FPaths::NormalizeFilename(Normalized);
	if (!FPaths::FileExists(Normalized))
	{
		UE_LOG(LogTemp, Warning, TEXT("[HotReload] pak 不存在：%s"), *Normalized);
		return false;
	}

	FPakPlatformFile* PakPF = GetPakPlatformFile();
	if (!PakPF)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HotReload] 未找到 PakPlatformFile，无法挂载：%s"), *Normalized);
		return false;
	}

	// MountPoint 决定 pak 内资源映射到哪个虚拟路径；默认挂到 /Game 下（与项目 Content 同根）
	FString MountPoint = FPaths::ProjectContentDir();
	if (!MountPoint.EndsWith(TEXT("/"))) MountPoint += TEXT("/");

	if (!PakPF->Mount(*Normalized, PakOrder, *MountPoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[HotReload] 挂载失败：%s"), *Normalized);
		return false;
	}

	// 关键：挂完必须刷新 AssetRegistry，否则 LoadObject / 软引用仍按旧缓存找不到新资源
	if (IAssetRegistry* AR = IAssetRegistry::Get())
	{
		TArray<FString> Files{ Normalized };
		AR->ScanFilesSynchronous(Files);
	}

	UE_LOG(LogTemp, Log, TEXT("[HotReload] 已挂载热更包：%s"), *Normalized);
	OnHotfixPakMounted.Broadcast(Normalized);
	return true;
}

bool UHotReloadManager::UnmountPak(const FString& PakPath)
{
	FString Normalized = PakPath;
	FPaths::NormalizeFilename(Normalized);
	FPakPlatformFile* PakPF = GetPakPlatformFile();
	if (!PakPF) return false;

	const bool bOk = PakPF->Unmount(*Normalized);
	UE_LOG(LogTemp, Log, TEXT("[HotReload] 卸载热更包：%s (%s)"),
		*Normalized, bOk ? TEXT("ok") : TEXT("fail"));
	return bOk;
}

void UHotReloadManager::LoadAssetAsync(const FSoftObjectPath& AssetPath, FOnHotAssetLoaded OnLoaded)
{
	// 保存路径副本供回调里 TryLoad；OnLoaded 为动态委托，按值捕获即可
	FSoftObjectPath Path = AssetPath;
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		Path,
		FStreamableDelegate::CreateLambda([Path, OnLoaded]()
		{
			UObject* Obj = Path.TryLoad();
			OnLoaded.ExecuteIfBound(Obj);
		}));
}
