#include "ShopperWebSubsystem.h"
#include "Engine/GameInstance.h"

void UShopperWebSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveWidget = nullptr;
}

void UShopperWebSubsystem::ShowWebView(const FString& Url, const FString& ReturnUrlMarker)
{
    if (Url.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("ShopperWebSubsystem::ShowWebView: Url 为空，已忽略"));
        return;
    }

    UClass* Cls = WebViewWidgetClass.LoadSynchronous();
    if (!Cls)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopperWebSubsystem::ShowWebView: WebViewWidgetClass 未配置（请在 CDO 指到 WBP_WebView）"));
        return;
    }

    // 若已有活动页面，先关掉再开新的（避免叠层）
    if (ActiveWidget && ActiveWidget->IsInViewport())
    {
        ActiveWidget->CloseWebView();
    }

    UGameInstance* GI = GetGameInstance();
    ActiveWidget = CreateWidget<UShopperWebView>(GI, Cls);
    if (!ActiveWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopperWebSubsystem::ShowWebView: CreateWidget 失败"));
        return;
    }

    const FString Marker = ReturnUrlMarker.IsEmpty() ? DefaultReturnUrlMarker : ReturnUrlMarker;
    ActiveWidget->OpenUrl(Url, Marker);
    ActiveWidget->OnWebViewReturn.AddDynamic(this, &UShopperWebSubsystem::HandleWidgetReturn);
    ActiveWidget->AddToViewport(100);   // 高层级，压在其它 UI 之上
}

void UShopperWebSubsystem::CloseWebView()
{
    if (ActiveWidget && ActiveWidget->IsInViewport())
    {
        ActiveWidget->OnWebViewReturn.RemoveDynamic(this, &UShopperWebSubsystem::HandleWidgetReturn);
        ActiveWidget->CloseWebView();
    }
    ActiveWidget = nullptr;
}

void UShopperWebSubsystem::HandleWidgetReturn(FString FinalUrl)
{
    // WebView 自身已 CloseWebView，这里只负责广播给游戏逻辑
    OnWebViewCompleted.Broadcast(FinalUrl);
    ActiveWidget = nullptr;
}
