#include "WB_ButtonBase.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Ticker.h"

UWB_ButtonBase::UWB_ButtonBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 默认不挂 ticker；点击时才临时注册，完成即移除，零常驻开销。
}

void UWB_ButtonBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
	{
		// 防止 NativeConstruct 被多次调用时重复绑定。
		Button->OnClicked.RemoveDynamic(this, &UWB_ButtonBase::HandleButtonClicked);
		Button->OnClicked.AddDynamic(this, &UWB_ButtonBase::HandleButtonClicked);
	}
}

void UWB_ButtonBase::NativeDestruct()
{
	// 移除可能仍在跑的 ticker，避免悬空回调。
	if (ClickTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ClickTickerHandle);
		ClickTickerHandle.Reset();
	}
	if (Button)
	{
		Button->OnClicked.RemoveDynamic(this, &UWB_ButtonBase::HandleButtonClicked);
	}
	Super::NativeDestruct();
}

void UWB_ButtonBase::TriggerClickFeedback()
{
	HandleButtonClicked();
}

void UWB_ButtonBase::HandleButtonClicked()
{
	PlayClickSound();
	StartClickTween();
	OnClicked.Broadcast(); // 父级界面绑定的业务逻辑在此触发
}

void UWB_ButtonBase::SetLabelText(const FText& InText)
{
	if (Label)
	{
		Label->SetText(InText);
	}
}

void UWB_ButtonBase::PlayClickSound()
{
	if (ClickSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
}

void UWB_ButtonBase::ApplyScale(float Scale)
{
	if (bScaleSelf)
	{
		SetRenderScale(FVector2D(Scale));
	}
	else if (Button)
	{
		Button->SetRenderScale(FVector2D(Scale));
	}
}

void UWB_ButtonBase::StartClickTween()
{
	ClickTweenTime = 0.f;

	// 用引擎全局 FTicker 驱动补间（UE5.6 已移除 UUserWidget::SetTickEnabled）。
	if (ClickTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ClickTickerHandle);
	}
	ClickTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UWB_ButtonBase::TickClickTween));
}

bool UWB_ButtonBase::TickClickTween(float DeltaTime)
{
	ClickTweenTime += DeltaTime;
	const float Alpha = ClickDuration > KINDA_SMALL_NUMBER
		? FMath::Clamp(ClickTweenTime / ClickDuration, 0.f, 1.f)
		: 1.f;

	ApplyScale(ComputeClickScale(Alpha));

	if (Alpha >= 1.f)
	{
		ApplyScale(1.f);
		ClickTweenTime = -1.f;
		ClickTickerHandle.Reset(); // 释放句柄；返回 false 让 ticker 自动注销
		return false;
	}
	return true;
}

float UWB_ButtonBase::ComputeClickScale(float Alpha) const
{
	// 1.0 -> 峰值 -> 1.0，用半个正弦得到平滑脉冲。
	return 1.f + (ClickPeakScale - 1.f) * FMath::Sin(Alpha * PI);
}
