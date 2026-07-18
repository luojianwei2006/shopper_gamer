#include "WidgetManager.h"
#include "WidgetRegistry.h"
#include "HotReloadManager.h"
#include "Engine/GameInstance.h"    // UGameInstance::GetSubsystem<> 需要完整类型
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UWidgetManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 复用已建好的 HotReloadManager：pak 一挂上就重新解析 widget 软引用
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UHotReloadManager* HR = GI->GetSubsystem<UHotReloadManager>())
		{
			HR->OnHotfixPakMounted.AddUObject(this, &UWidgetManager::ReloadWidgetsFromHotfix);
		}
	}

	LoadAllWidgetsAsync();
}

void UWidgetManager::LoadAllWidgetsAsync()
{
	Registry = LoadObject<UWidgetRegistry>(nullptr, RegistryPath);
	if (!Registry)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Widget] 未找到 WidgetRegistry：%s"), RegistryPath);
		NotifyWidgetsLoaded();   // 无 registry 也视为"加载完成"，避免加载闸门卡死
		return;
	}

	WidgetClasses.Empty();
	// 注册表用 TMap 按名字登记任意数量 widget：直接把 key/value 拷进缓存，
	// 这样 CreateWidgetOfType(任意逻辑名) 都能命中，不再受 4 个固定字段限制。
	for (const TPair<FName, TSoftClassPtr<UUserWidget>>& Pair : Registry->Widgets)
	{
		if (!Pair.Value.IsNull())
			WidgetClasses.Add(Pair.Key, Pair.Value);
	}

	if (WidgetClasses.Num() == 0)
	{
		NotifyWidgetsLoaded();
		return;
	}

	TArray<FSoftObjectPath> Paths;
	for (const auto& Pair : WidgetClasses)
		Paths.Add(Pair.Value.ToSoftObjectPath());

	FStreamableManager& Mgr = UAssetManager::Get().GetStreamableManager();
	LoadHandle = Mgr.RequestAsyncLoad(Paths,
		FStreamableDelegate::CreateLambda([this]()
		{
			UE_LOG(LogTemp, Log, TEXT("[Widget] 所有 UI widget 已异步加载并驻留，可立即使用"));
			NotifyWidgetsLoaded();
		}));
}

void UWidgetManager::NotifyWidgetsLoaded()
{
	bWidgetsLoaded = true;
	OnWidgetsPreloaded.Broadcast();
}

TSubclassOf<UUserWidget> UWidgetManager::GetWidgetClass(FName Key) const
{
	if (const TSoftClassPtr<UUserWidget>* Ptr = WidgetClasses.Find(Key))
		return Ptr->IsNull() ? nullptr : Ptr->LoadSynchronous();
	return nullptr;
}

UUserWidget* UWidgetManager::CreateWidgetOfType(FName Key, APlayerController* PC)
{
	if (TSubclassOf<UUserWidget> Cls = GetWidgetClass(Key))
		return CreateWidget<UUserWidget>(PC, Cls);
	UE_LOG(LogTemp, Warning, TEXT("[Widget] 创建失败，未找到 widget 类：%s"), *Key.ToString());
	return nullptr;
}

void UWidgetManager::ReloadWidgetsFromHotfix(const FString& /*PakPath*/)
{
	// 释放旧引用，使其可被 GC；随后重新异步加载 -> 从新挂载的 pak 取覆盖资产
	LoadHandle.Reset();
	WidgetClasses.Empty();
	LoadAllWidgetsAsync();
	UE_LOG(LogTemp, Log, TEXT("[Widget] 热更包挂载后已重新解析 widget 软引用"));
}
