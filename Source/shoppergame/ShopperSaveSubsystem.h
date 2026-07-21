#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "JsonObjectWrapper.h"       // FJsonObjectWrapper（UE5.6 路径无 Dom/ 前缀）
#include "ShopperSaveSubsystem.generated.h"

// 单个分类下的键值表：key -> 自定义 JSON（以原始 JSON 文本存储）
USTRUCT()
struct FCustomDataCategory
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, FString> Entries;   // key -> 自定义 JSON 文本
};

// 持久化数据体（USaveGame）：自定义 JSON 存储 + 音效/音乐偏好
UCLASS()
class SHOPPERGAME_API UShopperSaveData : public USaveGame
{
	GENERATED_BODY()

public:
	// 分类 -> 键值表（玩家自定义 JSON 存储）
	UPROPERTY()
	TMap<FString, FCustomDataCategory> CustomStore;

	// 音效：开关 + 音量（0~1）
	UPROPERTY()
	bool bSfxEnabled = true;

	UPROPERTY()
	float SfxVolume = 1.f;

	// 背景音乐：开关 + 音量（0~1）
	UPROPERTY()
	bool bBgmEnabled = true;

	UPROPERTY()
	float BgmVolume = 1.f;
};

/**
 * 玩家存档 + 音频偏好 子系统（GameInstanceSubsystem，跨关卡存活）。
 *
 * 职责：
 *  1) 自定义 JSON 存储：按「分类 → key → JSON」三级组织，持久化到本地存档。
 *  2) 音效偏好：开关 + 音量（PlaySfx 会据此自动跳过/调音量）。
 *  3) 背景音乐：开关 + 音量 + 播放/停止（内部托管一个常驻 UAudioComponent）。
 *
 * 蓝图用法：GetGameInstance(this)->GetSubsystem<UShopperSaveSubsystem>()，或直接用本类暴露的 BlueprintCallable 节点。
 */
UCLASS()
class SHOPPERGAME_API UShopperSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 1. 自定义 JSON 存储 ──────────────────────────────────
	// 保存：分类 + key + 自定义 JSON 文本（覆盖写），立即落盘
	UFUNCTION(BlueprintCallable, Category = "Save|CustomData")
	void SaveCustomData(const FString& Category, const FString& Key, const FString& JsonData);

	// 读取某分类下的全部 key 列表
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|CustomData")
	TArray<FString> GetCustomDataKeys(const FString& Category) const;

	// 按 分类 + key 读取自定义 JSON 文本
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|CustomData")
	bool LoadCustomData(const FString& Category, const FString& Key, FString& OutJsonData) const;

	// 按 分类 + key 读取为 FJsonObjectWrapper（配合 UShopperJsonLibrary 按 key 取字段）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|CustomData")
	FJsonObjectWrapper LoadCustomDataObject(const FString& Category, const FString& Key) const;

	// 是否存在 分类 + key
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|CustomData")
	bool HasCustomData(const FString& Category, const FString& Key) const;

	// 删除 分类 + key（分类空了会自动移除该分类），立即落盘
	UFUNCTION(BlueprintCallable, Category = "Save|CustomData")
	void RemoveCustomData(const FString& Category, const FString& Key);

	// ── 2. 音效偏好 ──────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Save|Audio")
	void SetSfxEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|Audio")
	bool IsSfxEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Save|Audio")
	void SetSfxVolume(float Volume);   // 自动 Clamp 到 0~1

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|Audio")
	float GetSfxVolume() const;

	// 播放一次音效，自动遵循音效开关与音量（关闭或无声源时静默跳过）
	UFUNCTION(BlueprintCallable, Category = "Save|Audio")
	void PlaySfx(UObject* WorldContextObject, USoundBase* Sound);

	// ── 3. 背景音乐 ─────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Save|Audio")
	void SetBgmEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|Audio")
	bool IsBgmEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Save|Audio")
	void SetBgmVolume(float Volume);   // 自动 Clamp 到 0~1，播放中实时生效

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save|Audio")
	float GetBgmVolume() const;

	// 播放背景音乐（默认循环）。受开关/音量约束，关闭时返回 nullptr。内部托管常驻组件，重复调用会先停旧的。
	UFUNCTION(BlueprintCallable, Category = "Save|Audio")
	UAudioComponent* PlayBackgroundMusic(USoundBase* Sound, bool bLoop = true);

	// 停止背景音乐
	UFUNCTION(BlueprintCallable, Category = "Save|Audio")
	void StopBackgroundMusic();

private:
	void LoadOrCreate();
	void Persist();

	UPROPERTY()
	UShopperSaveData* SaveData = nullptr;

	UPROPERTY()
	UAudioComponent* BgmComponent = nullptr;

	// 内存中记住上一首 BGM（不落盘），重新开启音乐时自动续播
	UPROPERTY()
	USoundBase* LastBgmSound = nullptr;

	// 当前 BGM 是否应循环（运行时控制，不依赖资产 bLooping）
	bool bBgmShouldLoop = false;

	// OnAudioFinished 回调：非循环资产播完时，若仍需循环则重新 Play
	UFUNCTION()
	void OnBgmFinished();

	static const FString SlotName;
	static constexpr int32 SlotUserIndex = 0;
};
