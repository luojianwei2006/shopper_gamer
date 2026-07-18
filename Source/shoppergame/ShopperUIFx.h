#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "Sound/SoundBase.h"
#include "ShopperUIFx.generated.h"

// 动效播放完成回调（零参动态单播委托，可在蓝图里绑定自定义事件）。
DECLARE_DYNAMIC_DELEGATE(FOnUIFxFinished);

// 外挂式 UI 动效库：给任意已有 UButton / UWidget 挂接点击脉冲、面板回弹、点击音效，
// 无需改父类、无需用 WB_ButtonBase 替换控件。所有补间由引擎全局单 FTicker 统一驱动，
// 无活动动画时自动注销 ticker，零常驻开销。UE5.6 已移除 UUserWidget::SetTickEnabled，故用 FTicker。

// 内部用的绑定器：为每个挂接的按钮持有一个 UObject，负责在点击时触发脉冲+音效。
// 通过 AddToRoot 保活，按钮销毁时由 ticker 自动 RemoveFromRoot 并清理，避免悬空委托。
UCLASS()
class SHOPPERGAME_API UShopperButtonFxBinder : public UObject
{
	GENERATED_BODY()
public:
	TWeakObjectPtr<UButton> Button;
	TWeakObjectPtr<USoundBase> Sound;
	float PeakScale = 1.15f;
	float Duration = 0.18f;

	void Init(UButton* InButton, USoundBase* InSound, float InPeak, float InDuration);
	UFUNCTION()
	void HandleClicked();
};

UCLASS()
class SHOPPERGAME_API UShopperUIFx : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 播放「放大->缩回」脉冲；可选音效（SoundVolume 默认 1.0）；动画结束后触发 OnComplete（在回调里写业务逻辑响应）。
	// 音效在开始时播放（即时点击反馈），OnComplete 在动画结束时触发。
	UFUNCTION(BlueprintCallable, Category = "UI|FX")
	static void PlayButtonPulse(UWidget* Target, FOnUIFxFinished OnComplete, float PeakScale = 1.15f, float Duration = 0.18f,
		USoundBase* Sound = nullptr, float SoundVolume = 1.0f);

	// 播放「放大过冲->回弹」效果（默认 0.38s，从无到有，回弹强度 1.6）；动画结束后触发 OnComplete。
	UFUNCTION(BlueprintCallable, Category = "UI|FX")
	static void PlayPanelBounce(UWidget* Target, FOnUIFxFinished OnComplete, float Duration = 0.38f, float StartScale = 0.0f,
		float Overshoot = 1.6f);

	// 手动：播放一次点击音效（2D，无需世界音源）。
	UFUNCTION(BlueprintCallable, Category = "UI|FX", meta = (WorldContext = "WorldContextObject"))
	static void PlayClickSound(UObject* WorldContextObject, USoundBase* Sound, float Volume = 1.0f);

	// 外挂：一行给任意已有 UButton 挂上「点击脉冲 + 音效」，无需改父类、无需替换控件。
	// 在按钮所在 Widget 的 Construct / OnInitialized 里调用一次即可，之后该按钮每次点击都自带效果。
	UFUNCTION(BlueprintCallable, Category = "UI|FX")
	static void BindClickFx(UButton* Button, USoundBase* Sound = nullptr, float PeakScale = 1.15f, float Duration = 0.18f);
};
