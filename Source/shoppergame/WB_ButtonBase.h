#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Sound/SoundBase.h"
#include "WB_ButtonBase.generated.h"

/**
 * 通用按钮基类。
 * 点击（或程序化 TriggerClickFeedback）时自动播放「放大 -> 缩回」脉冲，并播放点击音效。
 * 设计者基于此创建蓝图，放一个命名为 "Button" 的 UButton 即可，效果全内置，无需每按钮手搓动画。
 * 缩放由 C++ 通过引擎全局 FTicker 驱动（半个正弦），不依赖蓝图动画时间轴、不常驻 widget tick。
 * 注：UE5.6 已移除 UUserWidget::SetTickEnabled，故改用 FTSTicker 做帧驱动补间。
 */
UCLASS(Abstract, Blueprintable)
class SHOPPERGAME_API UWB_ButtonBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UWB_ButtonBase(const FObjectInitializer& ObjectInitializer);

	// 程序化触发一次点击反馈（脉冲 + 音效）。供键盘/手柄“确认”时调用，与鼠标点击一致。
	UFUNCTION(BlueprintCallable, Category = "UI|FX")
	void TriggerClickFeedback();

	// 点击完成（脉冲+音效已触发）后广播，父级菜单绑定此事件写业务逻辑（开店/关界面等）。
	// 它在 HandleButtonClicked 内部于反馈播放之后 Broadcast，因此父级逻辑与点击动效同步触发。
	UPROPERTY(BlueprintAssignable, Category = "UI|FX")
	FOnButtonClickedEvent OnClicked;

	// 设置可选标题文字（需蓝图内有一个命名为 "Label" 的 TextBlock）。
	UFUNCTION(BlueprintCallable, Category = "UI|FX")
	void SetLabelText(const FText& InText);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 蓝图里必须有一个命名为 "Button" 的 UButton（BindWidget 自动绑定）。
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button;

	// 可选标题文字（BindWidgetOptional：蓝图里放一个名为 "Label" 的 TextBlock 即生效，不放也不报错）。
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Label;

	// 点击音效（可空，留空不播）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	TObjectPtr<USoundBase> ClickSound;

	// 峰值缩放倍率（1.15 = 放大到 115%）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	float ClickPeakScale = 1.15f;

	// 脉冲总时长（秒）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	float ClickDuration = 0.18f;

	// true = 缩放整个控件根（推荐，视觉最一致）；false = 只缩放内部 Button。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	bool bScaleSelf = true;

private:
	UFUNCTION()
	void HandleButtonClicked();

	void PlayClickSound();
	void ApplyScale(float Scale);
	void StartClickTween();
	float ComputeClickScale(float Alpha) const;

	// FTicker 回调：返回 true 续帧，false 停止。
	bool TickClickTween(float DeltaTime);

	float ClickTweenTime = -1.f;                  // <0 = 未激活
	FTSTicker::FDelegateHandle ClickTickerHandle; // 驱动脉冲的全局 ticker 句柄
};
