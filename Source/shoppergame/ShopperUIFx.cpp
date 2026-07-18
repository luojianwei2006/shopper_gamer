#include "ShopperUIFx.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Ticker.h"
#include "UObject/Package.h" // 使 UPackage 完整，NewObject( GetTransientPackage() ) 才能将 UPackage* 上转 UObject*

namespace
{
	// 脉冲补间状态
	struct FWidgetPulse
	{
		TWeakObjectPtr<UWidget> Widget;
		float Time = 0.f;
		float Duration = 0.18f;
		float PeakScale = 1.15f;
		FOnUIFxFinished OnComplete;
	};

	// 回弹补间状态
	struct FWidgetBounce
	{
		TWeakObjectPtr<UWidget> Widget;
		float Time = 0.f;
		float Duration = 0.38f;
		float StartScale = 0.f;
		float Overshoot = 1.6f;
		FOnUIFxFinished OnComplete;
	};

	static TMap<TWeakObjectPtr<UWidget>, FWidgetPulse> GActivePulses;
	static TMap<TWeakObjectPtr<UWidget>, FWidgetBounce> GActiveBounces;
	static TSet<UShopperButtonFxBinder*> GBinders; // 裸指针 + AddToRoot 管理生命周期
	static FTSTicker::FDelegateHandle GTickerHandle;

	// 1.0 -> 峰值 -> 1.0，半个正弦得到平滑脉冲
	static float PulseScale(float Alpha, float Peak)
	{
		return 1.f + (Peak - 1.f) * FMath::Sin(Alpha * PI);
	}

	// Back 缓动（带过冲回弹）：从 StartScale 弹到 1.0
	static float BackOut(float X, float S)
	{
		const float C1 = S;
		const float C3 = C1 + 1.f;
		const float T = X - 1.f;
		return 1.f + C3 * T * T * T + C1 * T * T;
	}

	static bool TickFx(float Dt)
	{
		// 脉冲
		TArray<TWeakObjectPtr<UWidget>> Done;
		for (auto& Pair : GActivePulses)
		{
			UWidget* W = Pair.Key.Get();
			if (!W)
			{
				Done.Add(Pair.Key);
				continue;
			}
			FWidgetPulse& P = Pair.Value;
			P.Time += Dt;
			const float Alpha = P.Duration > KINDA_SMALL_NUMBER
				? FMath::Clamp(P.Time / P.Duration, 0.f, 1.f)
				: 1.f;
			W->SetRenderScale(FVector2D(PulseScale(Alpha, P.PeakScale)));
			if (Alpha >= 1.f)
			{
				W->SetRenderScale(FVector2D(1.f));
				// 先取出回调并标记完成，再执行，避免回调内重入同一控件时的重入问题
				FOnUIFxFinished Cb = P.OnComplete;
				P.OnComplete.Unbind();
				Done.Add(Pair.Key);
				Cb.ExecuteIfBound();
			}
		}
		for (const auto& K : Done) GActivePulses.Remove(K);

		// 回弹
		Done.Reset();
		for (auto& Pair : GActiveBounces)
		{
			UWidget* W = Pair.Key.Get();
			if (!W)
			{
				Done.Add(Pair.Key);
				continue;
			}
			FWidgetBounce& B = Pair.Value;
			B.Time += Dt;
			const float Alpha = B.Duration > KINDA_SMALL_NUMBER
				? FMath::Clamp(B.Time / B.Duration, 0.f, 1.f)
				: 1.f;
			const float E = BackOut(Alpha, B.Overshoot);
			const float Scale = B.StartScale + (1.f - B.StartScale) * E;
			W->SetRenderScale(FVector2D(Scale));
			if (Alpha >= 1.f)
			{
				W->SetRenderScale(FVector2D(1.f));
				FOnUIFxFinished Cb = B.OnComplete;
				B.OnComplete.Unbind();
				Done.Add(Pair.Key);
				Cb.ExecuteIfBound();
			}
		}
		for (const auto& K : Done) GActiveBounces.Remove(K);

		// 清理按钮已销毁的绑定器（RemoveFromRoot 释放，避免悬空委托）
		for (auto It = GBinders.CreateIterator(); It; ++It)
		{
			UShopperButtonFxBinder* B = *It;
			if (!B)
			{
				It.RemoveCurrent();
				continue;
			}
			if (!B->Button.IsValid())
			{
				B->RemoveFromRoot();
				It.RemoveCurrent();
			}
		}

		// 无活动动画且无绑定器时，注销 ticker
		if (GActivePulses.Num() == 0 && GActiveBounces.Num() == 0 && GBinders.Num() == 0)
		{
			GTickerHandle.Reset();
			return false;
		}
		return true;
	}

	static void EnsureTicker()
	{
		if (!GTickerHandle.IsValid())
		{
			GTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateStatic(&TickFx));
		}
	}
} // namespace

// --- UShopperButtonFxBinder ---
void UShopperButtonFxBinder::Init(UButton* InButton, USoundBase* InSound, float InPeak, float InDuration)
{
	Button = InButton;
	Sound = InSound;
	PeakScale = InPeak;
	Duration = InDuration;
	if (InButton)
	{
		InButton->OnClicked.AddDynamic(this, &UShopperButtonFxBinder::HandleClicked);
	}
	AddToRoot(); // 保活，直到按钮销毁由 ticker 清理
}

void UShopperButtonFxBinder::HandleClicked()
{
	UButton* B = Button.Get();
	if (!B) return;
	// 脉冲 + 音效一体播放（不再单独播声音，避免双重音效）；此处无需完成回调
	UShopperUIFx::PlayButtonPulse(B, FOnUIFxFinished(), PeakScale, Duration, Sound.Get(), 1.0f);
}

// --- UShopperUIFx ---
void UShopperUIFx::PlayButtonPulse(UWidget* Target, FOnUIFxFinished OnComplete, float PeakScale, float Duration, USoundBase* Sound, float SoundVolume)
{
	if (!Target) return;

	// 音效在开始时播放，提供即时点击反馈
	if (Sound && Target->GetWorld())
	{
		UGameplayStatics::PlaySound2D(Target->GetWorld(), Sound, SoundVolume);
	}

	EnsureTicker();
	FWidgetPulse& P = GActivePulses.FindOrAdd(Target);
	P.Widget = Target;
	P.Time = 0.f;
	P.Duration = Duration;
	P.PeakScale = PeakScale;
	P.OnComplete = MoveTemp(OnComplete);
}

void UShopperUIFx::PlayPanelBounce(UWidget* Target, FOnUIFxFinished OnComplete, float Duration, float StartScale, float Overshoot)
{
	if (!Target) return;
	EnsureTicker();
	FWidgetBounce& B = GActiveBounces.FindOrAdd(Target);
	B.Widget = Target;
	B.Time = 0.f;
	B.Duration = Duration;
	B.StartScale = StartScale;
	B.Overshoot = Overshoot;
	B.OnComplete = MoveTemp(OnComplete);
}

void UShopperUIFx::PlayClickSound(UObject* WorldContextObject, USoundBase* Sound, float Volume)
{
	if (Sound && WorldContextObject)
	{
		UGameplayStatics::PlaySound2D(WorldContextObject, Sound, Volume);
	}
}

void UShopperUIFx::BindClickFx(UButton* Button, USoundBase* Sound, float PeakScale, float Duration)
{
	if (!Button) return;
	UShopperButtonFxBinder* Binder = NewObject<UShopperButtonFxBinder>(GetTransientPackage());
	Binder->Init(Button, Sound, PeakScale, Duration);
	GBinders.Add(Binder);
}
