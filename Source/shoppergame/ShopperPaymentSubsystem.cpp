#include "ShopperPaymentSubsystem.h"
#include "Engine/GameInstance.h"

void UShopperPaymentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveWidget = nullptr;
}

void UShopperPaymentSubsystem::ShowPayment(const FString& JumpUrl, const FString& ReturnUrlMarker)
{
    if (JumpUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("ShopperPaymentSubsystem::ShowPayment: JumpUrl 为空，已忽略"));
        return;
    }

    UClass* Cls = PaymentWidgetClass.LoadSynchronous();
    if (!Cls)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopperPaymentSubsystem::ShowPayment: PaymentWidgetClass 未配置（请在 CDO 指到 WBP_PaymentWebView）"));
        return;
    }

    // 若已有活动页面，先关掉再开新的（避免叠层）
    if (ActiveWidget && ActiveWidget->IsInViewport())
    {
        ActiveWidget->ClosePayment();
    }

    UGameInstance* GI = GetGameInstance();
    ActiveWidget = CreateWidget<UShopperPaymentWebView>(GI, Cls);
    if (!ActiveWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopperPaymentSubsystem::ShowPayment: CreateWidget 失败"));
        return;
    }

    const FString Marker = ReturnUrlMarker.IsEmpty() ? DefaultReturnUrlMarker : ReturnUrlMarker;
    ActiveWidget->SetupPayment(JumpUrl, Marker);
    ActiveWidget->OnPaymentReturn.AddDynamic(this, &UShopperPaymentSubsystem::HandleWidgetReturn);
    ActiveWidget->AddToViewport(100);   // 高层级，压在其它 UI 之上
}

void UShopperPaymentSubsystem::ClosePayment()
{
    if (ActiveWidget && ActiveWidget->IsInViewport())
    {
        ActiveWidget->OnPaymentReturn.RemoveDynamic(this, &UShopperPaymentSubsystem::HandleWidgetReturn);
        ActiveWidget->ClosePayment();
    }
    ActiveWidget = nullptr;
}

void UShopperPaymentSubsystem::HandleWidgetReturn(const FString& FinalUrl)
{
    // WebView 自身已 ClosePayment，这里只负责广播给游戏逻辑
    OnPaymentCompleted.Broadcast(FinalUrl);
    ActiveWidget = nullptr;
}
