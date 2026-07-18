#include "WP_PanelBase.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Ticker.h"

UWP_PanelBase::UWP_PanelBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWP_PanelBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (bPlayOnConstruct)
	{
		// 先落到起始缩放，避免首帧闪现全尺寸。
		ApplyScale(BounceStartScale);
		PlayBounceIn();
	}
}

void UWP_PanelBase::NativeDestruct()
{
	// 移除可能仍在跑的 ticker，避免悬空回调。
	if (BounceTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BounceTickerHandle);
		BounceTickerHandle.Reset();
	}
	BounceTweenTime = -1.f;
	Super::NativeDestruct();
}

void UWP_PanelBase::PlayBounceIn()
{
	if (BounceSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(this, BounceSound);
	}
	StartBounceTween();
}

void UWP_PanelBase::ApplyScale(float Scale)
{
	SetRenderScale(FVector2D(Scale));
}

void UWP_PanelBase::StartBounceTween()
{
	BounceTweenTime = 0.f;

	// 用引擎全局 FTicker 驱动补间（UE5.6 已移除 UUserWidget::SetTickEnabled）。
	if (BounceTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BounceTickerHandle);
	}
	BounceTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UWP_PanelBase::TickBounceTween));
}

bool UWP_PanelBase::TickBounceTween(float DeltaTime)
{
	BounceTweenTime += DeltaTime;
	const float Alpha = BounceDuration > KINDA_SMALL_NUMBER
		? FMath::Clamp(BounceTweenTime / BounceDuration, 0.f, 1.f)
		: 1.f;

	ApplyScale(ComputeBounceScale(Alpha));

	if (Alpha >= 1.f)
	{
		ApplyScale(1.f);
		BounceTweenTime = -1.f;
		BounceTickerHandle.Reset(); // 释放句柄；返回 false 让 ticker 自动注销
		return false;
	}
	return true;
}

float UWP_PanelBase::ComputeBounceScale(float Alpha) const
{
	if (BounceCurve)
	{
		// 设计者在曲线里自由定义 0..1 -> 缩放（可含 1.1/0.95 等多段过冲）。
		return BounceCurve->GetFloatValue(Alpha);
	}

	// 内置：从 BounceStartScale 经 Back 缓动到 1.0，自带过冲回弹。
	const float Eased = BackEaseOut(Alpha, BounceOvershoot); // 0..1（含过冲）
	return FMath::Lerp(BounceStartScale, 1.f, Eased);
}

float UWP_PanelBase::BackEaseOut(float X, float Overshoot)
{
	const float C1 = Overshoot;
	const float C3 = C1 + 1.f;
	const float Xm = X - 1.f;
	return 1.f + C3 * FMath::Pow(Xm, 3.f) + C1 * FMath::Pow(Xm, 2.f);
}
