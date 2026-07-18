#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UObject/SoftObjectPtr.h"   // TSoftObjectPtr / TSoftClassPtr
#include "WidgetManager.generated.h"

// widget 预载完成广播（C++ 内部用。非 Dynamic 多播委托才能 AddUObject 绑定）
DECLARE_MULTICAST_DELEGATE(FOnWidgetsPreloaded);

class UWidgetRegistry;
struct FStreamableHandle;

/**
 * UI Widget 热加载 / 热更管理器（GameInstanceSubsystem，随游戏实例自动创建）。
 * 职责：
 *   - 启动时异步加载 WidgetRegistry 及其登记的 widget 软类（按名字索引，数量不限，不阻塞主线程），
 *     用 StreamableManager 持有 handle 使其驻留内存；
 *   - 绑定 HotReloadManager::OnHotfixPakMounted：热更 pak 挂载后重新解析软引用，
 *     下次创建的 widget 即使用 pak 覆盖后的新资产。
 *
 * 典型用法（PlayerController 里）：
 *   if (UWidgetManager* WM = GetGameInstance()->GetSubsystem<UWidgetManager>())
 *       WM->CreateWidgetOfType(FName("Login"), this)->AddToViewport();
 */
UCLASS()
class SHOPPERGAME_API UWidgetManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 启动调用：异步加载所有 widget 软类（不阻塞），全部就绪后打印日志
	UFUNCTION(BlueprintCallable, Category = "UI")
	void LoadAllWidgetsAsync();

	// 取已加载的 widget 类；若尚未加载则同步兜底加载
	UFUNCTION(BlueprintCallable, Category = "UI")
	TSubclassOf<UUserWidget> GetWidgetClass(FName Key) const;

	// 按 key 创建 widget 实例（如 "Login" / "Menu" / "Setting" / "Game"）
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* CreateWidgetOfType(FName Key, APlayerController* PC);

	// 热更：收到 pak 挂载后调用，释放旧 handle 并重新异步加载（从新 pak 取覆盖资产）。
	// 参数 PakPath 与 FOnHotfixPakMounted 委托签名保持一致（AddUObject 要求签名严格匹配），本函数不使用该参数。
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ReloadWidgetsFromHotfix(const FString& PakPath);

	// widget 预载是否已完成（加载闸门据此判断是否跳过等待）。
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool AreWidgetsLoaded() const { return bWidgetsLoaded; }

	// widget 预载完成广播（C++ 绑定，非 Dynamic 多播委托）。
	FOnWidgetsPreloaded OnWidgetsPreloaded;

private:
	// 标记 widget 预载完成：置位 bWidgetsLoaded 并广播 OnWidgetsPreloaded。
	// 成功/失败路径都会调用，确保加载闸门不会卡在 Widget Phase。
	void NotifyWidgetsLoaded();

private:
	UPROPERTY()
	TSoftObjectPtr<UWidgetRegistry> Registry;

	// key -> 软类引用缓存（Initialize / 热更时填充）
	TMap<FName, TSoftClassPtr<UUserWidget>> WidgetClasses;

	// 持有所有 widget 的 streamable handle：handle 存活 = 资源驻留，GC 不卸载
	TSharedPtr<FStreamableHandle> LoadHandle;

	// widget 是否已成功预载（无论成功或失败都置 true，避免加载闸门卡死）
	bool bWidgetsLoaded = false;

	static constexpr const TCHAR* RegistryPath =
		TEXT("/Game/UI/WidgetRegistry.WidgetRegistry");
};
