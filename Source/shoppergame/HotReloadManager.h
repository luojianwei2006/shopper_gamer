#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HotReloadManager.generated.h"

// 任意热更 pak 挂载完成后广播（参数为 pak 路径）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotfixPakMounted, const FString&, PakPath);

// 异步热加载完成回调（LoadedAsset 为 nullptr 表示加载失败）
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHotAssetLoaded, UObject*, LoadedAsset);

/**
 * 运行时热重载管理器（GameInstanceSubsystem，随游戏实例自动创建）。
 * 负责：
 *   - 挂载 / 卸载热更 .pak（资源热更的核心：下载到本地的 pak 在这里挂入虚拟文件系统）
 *   - 挂载后自动刷新 AssetRegistry 并广播 OnHotfixPakMounted，供各系统热应用配置 / 资源
 *   - 通过 StreamableManager 异步热加载单个资源（资源热加载）
 *
 * 典型链路：HTTP 下载 .pak → MountPak → (各系统 OnHotfixPakMounted 回调里) 加载覆盖配置 / 资源。
 */
UCLASS()
class SHOPPERGAME_API UHotReloadManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 运行时挂载一个 .pak（通常来自 HTTP 下载到 Saved/Paks 的热更包）。
	// 成功挂载后自动刷新 AssetRegistry 并广播 OnHotfixPakMounted。
	// PakOrder 越大优先级越高（后挂载的覆盖先挂载的同名资源）。
	UFUNCTION(BlueprintCallable, Category = "HotReload")
	bool MountPak(const FString& PakPath, int32 PakOrder = 100);

	// 卸载一个已挂载的 .pak（一般热更不需要卸载，预留接口）。
	UFUNCTION(BlueprintCallable, Category = "HotReload")
	bool UnmountPak(const FString& PakPath);

	// 异步热加载一个资源（TSoftObjectPath），加载完成后回调 OnLoaded（失败为 nullptr）。
	// 配合 TSoftObjectPtr / PrimaryAssetLabel 可实现大资源按需加载、闲置释放。
	UFUNCTION(BlueprintCallable, Category = "HotReload")
	void LoadAssetAsync(const TSoftObjectPath& AssetPath, FOnHotAssetLoaded OnLoaded);

	// 任意 pak 挂载完成后广播（参数为 pak 路径），各系统在此重读配置 / 资源
	UPROPERTY(BlueprintAssignable, Category = "HotReload")
	FOnHotfixPakMounted OnHotfixPakMounted;

private:
	// 向下遍历平台文件栈，找到 Pak 包装器（编辑器/开发构建中 pak 在更低层；shipping 中即顶层）
	static class FPakPlatformFile* GetPakPlatformFile();
};
