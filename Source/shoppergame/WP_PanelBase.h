#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Blueprint/UserWidget.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundBase.h"
#include "WP_PanelBase.generated.h"

/**
 * 通用面板基类。
 * 构造时（或程序化 PlayBounceIn）自动播放「弹出放大 -> 过冲回弹」入场动画，并播放弹出音效。
 * 设计者基于此创建蓝图即可，无需每界面手搓动画关键帧。
 * 缩放由 C++ 通过引擎全局 FTicker 驱动（Back 缓动，自带过冲回弹），不依赖蓝图动画时间轴、不常驻 widget tick。
 * 注：UE5.6 已移除 UUserWidget::SetTickEnabled，故改用 FTSTicker 做帧驱动补间。
 */
UCLASS(Abstract, Blueprintable)
class SHOPPERGAME_API UWP_PanelBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UWP_PanelBase(const FObjectInitializer& ObjectInitializer);

	// 程序化重播入场动画（如界面再次显示时调用）。
	UFUNCTION(BlueprintCallable, Category = "UI|FX")
	void PlayBounceIn();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 弹出音效（可空）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	TObjectPtr<USoundBase> BounceSound;

	// 入场动画总时长（秒）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	float BounceDuration = 0.38f;

	// 起始缩放（0 = 从无到有；0.8 = 略缩后弹出）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	float BounceStartScale = 0.0f;

	// 过冲强度（Back 缓动参数，越大回弹越夸张）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	float BounceOvershoot = 1.70158f;

	// 是否在 Construct 时自动播放入场。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	bool bPlayOnConstruct = true;

	// 自定义缓动曲线（可选）；赋值后优先使用，可定义任意多段（1.1/0.95 等）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|FX")
	TObjectPtr<UCurveFloat> BounceCurve;

private:
	void ApplyScale(float Scale);
	void StartBounceTween();
	float ComputeBounceScale(float Alpha) const;
	static float BackEaseOut(float X, float Overshoot);

	// FTicker 回调：返回 true 续帧，false 停止。
	bool TickBounceTween(float DeltaTime);

	float BounceTweenTime = -1.f;                   // <0 = 未激活
	FTSTicker::FDelegateHandle BounceTickerHandle;  // 驱动弹入的全局 ticker 句柄
};
