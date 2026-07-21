#include "ShopperSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

const FString UShopperSaveSubsystem::SlotName = TEXT("ShopperPlayerSave");

void UShopperSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreate();
}

void UShopperSaveSubsystem::Deinitialize()
{
	StopBackgroundMusic();
	Persist();
	Super::Deinitialize();
}

void UShopperSaveSubsystem::LoadOrCreate()
{
	if (USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(SlotName, SlotUserIndex))
	{
		SaveData = Cast<UShopperSaveData>(Raw);
	}
	if (!SaveData)
	{
		SaveData = Cast<UShopperSaveData>(
			UGameplayStatics::CreateSaveGameObject(UShopperSaveData::StaticClass()));
		Persist();
	}
}

void UShopperSaveSubsystem::Persist()
{
	if (SaveData)
	{
		UGameplayStatics::SaveGameToSlot(SaveData, SlotName, SlotUserIndex);
	}
}

// ── 1. 自定义 JSON 存储 ──────────────────────────────────
void UShopperSaveSubsystem::SaveCustomData(const FString& Category, const FString& Key, const FString& JsonData)
{
	if (!SaveData) return;
	FCustomDataCategory& Cat = SaveData->CustomStore.FindOrAdd(Category);
	Cat.Entries.FindOrAdd(Key) = JsonData;
	Persist();
}

TArray<FString> UShopperSaveSubsystem::GetCustomDataKeys(const FString& Category) const
{
	TArray<FString> Keys;
	if (SaveData)
	{
		if (const FCustomDataCategory* Cat = SaveData->CustomStore.Find(Category))
		{
			Cat->Entries.GetKeys(Keys);
		}
	}
	return Keys;
}

bool UShopperSaveSubsystem::LoadCustomData(const FString& Category, const FString& Key, FString& OutJsonData) const
{
	OutJsonData.Empty();
	if (!SaveData) return false;
	const FCustomDataCategory* Cat = SaveData->CustomStore.Find(Category);
	if (!Cat) return false;
	const FString* Found = Cat->Entries.Find(Key);
	if (!Found) return false;
	OutJsonData = *Found;
	return true;
}

FJsonObjectWrapper UShopperSaveSubsystem::LoadCustomDataObject(const FString& Category, const FString& Key) const
{
	FJsonObjectWrapper Out;
	FString Json;
	if (LoadCustomData(Category, Key, Json) && !Json.IsEmpty())
	{
		TSharedPtr<FJsonObject> Parsed;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		if (FJsonSerializer::Deserialize(Reader, Parsed) && Parsed.IsValid())
		{
			Out.JsonObject = Parsed;
		}
	}
	return Out;
}

bool UShopperSaveSubsystem::HasCustomData(const FString& Category, const FString& Key) const
{
	if (!SaveData) return false;
	const FCustomDataCategory* Cat = SaveData->CustomStore.Find(Category);
	return Cat && Cat->Entries.Contains(Key);
}

void UShopperSaveSubsystem::RemoveCustomData(const FString& Category, const FString& Key)
{
	if (!SaveData) return;
	if (FCustomDataCategory* Cat = SaveData->CustomStore.Find(Category))
	{
		Cat->Entries.Remove(Key);
		if (Cat->Entries.Num() == 0)
		{
			SaveData->CustomStore.Remove(Category);
		}
		Persist();
	}
}

// ── 2. 音效偏好 ──────────────────────────────────────────
void UShopperSaveSubsystem::SetSfxEnabled(bool bEnabled)
{
	if (SaveData && SaveData->bSfxEnabled != bEnabled)
	{
		SaveData->bSfxEnabled = bEnabled;
		Persist();
	}
}

bool UShopperSaveSubsystem::IsSfxEnabled() const
{
	return SaveData ? SaveData->bSfxEnabled : true;
}

void UShopperSaveSubsystem::SetSfxVolume(float Volume)
{
	Volume = FMath::Clamp(Volume, 0.f, 1.f);
	if (SaveData && !FMath::IsNearlyEqual(SaveData->SfxVolume, Volume))
	{
		SaveData->SfxVolume = Volume;
		Persist();
	}
}

float UShopperSaveSubsystem::GetSfxVolume() const
{
	return SaveData ? SaveData->SfxVolume : 1.f;
}

void UShopperSaveSubsystem::PlaySfx(UObject* WorldContextObject, USoundBase* Sound)
{
	if (!Sound || !IsSfxEnabled() || !WorldContextObject)
	{
		return;
	}
	UGameplayStatics::PlaySound2D(WorldContextObject, Sound, GetSfxVolume());
}

// ── 3. 背景音乐 ─────────────────────────────────────────
void UShopperSaveSubsystem::SetBgmEnabled(bool bEnabled)
{
	if (SaveData)
	{
		SaveData->bBgmEnabled = bEnabled;
		Persist();
	}
	if (!bEnabled)
	{
		StopBackgroundMusic();
	}
	else if (LastBgmSound && !BgmComponent)
	{
		// 重新开启时自动续播上一首
		PlayBackgroundMusic(LastBgmSound);
	}
}

bool UShopperSaveSubsystem::IsBgmEnabled() const
{
	return SaveData ? SaveData->bBgmEnabled : true;
}

void UShopperSaveSubsystem::SetBgmVolume(float Volume)
{
	Volume = FMath::Clamp(Volume, 0.f, 1.f);
	if (SaveData)
	{
		SaveData->BgmVolume = Volume;
		Persist();
	}
	if (BgmComponent)
	{
		BgmComponent->SetVolumeMultiplier(Volume);
	}
}

float UShopperSaveSubsystem::GetBgmVolume() const
{
	return SaveData ? SaveData->BgmVolume : 1.f;
}

UAudioComponent* UShopperSaveSubsystem::PlayBackgroundMusic(USoundBase* Sound, bool bLoop)
{
	if (!Sound || !IsBgmEnabled())
	{
		return nullptr;
	}
	StopBackgroundMusic();

	// bAutoDestroy=false：自己托管组件生命周期，便于后续调音量/停止
	BgmComponent = UGameplayStatics::SpawnSound2D(
		this, Sound, GetBgmVolume(), 1.f, 0.f, nullptr, /*bPersistAcrossLevelTransition=*/false, /*bAutoDestroy=*/false);
	if (BgmComponent)
	{
		// UE5 的 UAudioComponent 无运行时 bLooping 开关，循环完全由声音资产的 bLooping 决定。
		// 这里用 OnAudioFinished 重新 Play 实现运行时循环控制：资产本身不循环时也能按参数循环。
		bBgmShouldLoop = bLoop;
		BgmComponent->OnAudioFinished.AddDynamic(this, &UShopperSaveSubsystem::OnBgmFinished);
		LastBgmSound = Sound;
	}
	return BgmComponent;
}

void UShopperSaveSubsystem::OnBgmFinished()
{
	if (bBgmShouldLoop && IsBgmEnabled() && IsValid(BgmComponent))
	{
		BgmComponent->Play();
	}
}

void UShopperSaveSubsystem::StopBackgroundMusic()
{
	if (BgmComponent)
	{
		BgmComponent->Stop();
		BgmComponent->DestroyComponent();
		BgmComponent = nullptr;
	}
}
