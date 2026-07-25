#include "ShopperPaymentWebView.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

void UShopperPaymentWebView::NativeConstruct()
{
    Super::NativeConstruct();

    // 运行时创建 WebBrowser 控件并挂到蓝图提供的 WebBrowserSlot（Overlay）里
    WebBrowser = NewObject<UWebBrowser>(this, UWebBrowser::StaticClass());
    if (WebBrowser)
    {
        WebBrowser->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        WebBrowser->OnUrlChanged.AddDynamic(this, &UShopperPaymentWebView::HandleUrlChanged);

        if (WebBrowserSlot)
        {
            UOverlaySlot* Slot = WebBrowserSlot->AddChildToOverlay(WebBrowser);
            if (Slot)
            {
                Slot->SetHorizontalAlignment(HAlign_Fill);
                Slot->SetVerticalAlignment(VAlign_Fill);
            }
        }
        else
        {
            // 兜底：蓝图没放 WebBrowserSlot 时，挂到根节点（若为 Panel）
            if (UPanelWidget* Root = Cast<UPanelWidget>(GetRootWidget()))
            {
                Root->AddChild(WebBrowser);
            }
            else
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("ShopperPaymentWebView: 未找到 WebBrowserSlot 且根节点非 Panel，WebBrowser 无法显示"));
            }
        }
    }
}

void UShopperPaymentWebView::SetupPayment(const FString& JumpUrl, const FString& ReturnUrlMarker)
{
    ReturnMarker = ReturnUrlMarker;
    if (WebBrowser && !JumpUrl.IsEmpty())
    {
        WebBrowser->LoadURL(JumpUrl);
    }
}

void UShopperPaymentWebView::ClosePayment()
{
    if (WebBrowser)
    {
        WebBrowser->OnUrlChanged.RemoveDynamic(this, &UShopperPaymentWebView::HandleUrlChanged);
    }
    RemoveFromParent();
}

void UShopperPaymentWebView::OnPaymentReturned_Implementation(const FString& FinalUrl)
{
    // 默认实现：命中回跳即关闭。蓝图可覆写以做自定义收尾（如播放音效、刷新 UI）。
    ClosePayment();
}

void UShopperPaymentWebView::HandleUrlChanged(const FWebBrowserUrlChangedParams& Params)
{
    if (!ReturnMarker.IsEmpty() && Params.CurrentUrl.Contains(ReturnMarker))
    {
        OnPaymentReturn.Broadcast(Params.CurrentUrl);
        OnPaymentReturned(Params.CurrentUrl);
    }
}
