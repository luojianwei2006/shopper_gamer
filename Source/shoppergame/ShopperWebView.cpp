#include "ShopperWebView.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

void UShopperWebView::NativeConstruct()
{
    Super::NativeConstruct();

    // 运行时创建 WebBrowser 控件并挂到蓝图提供的 WebBrowserSlot（Overlay）里
    WebBrowser = NewObject<UWebBrowser>(this, UWebBrowser::StaticClass());
    if (WebBrowser)
    {
        UE_LOG(LogTemp, Display, TEXT("[ShopperWebView] NativeConstruct: WebBrowser 已创建, WebBrowserSlot 绑定=%s"),
            WebBrowserSlot ? TEXT("yes") : TEXT("no"));

        WebBrowser->SetVisibility(ESlateVisibility::Visible);
        WebBrowser->OnUrlChanged.AddDynamic(this, &UShopperWebView::HandleUrlChanged);

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
                    TEXT("ShopperWebView: 未找到 WebBrowserSlot 且根节点非 Panel，WebBrowser 无法显示"));
            }
        }

        // NativeConstruct 完成后才补加载待定 URL（OpenUrl 可能早于 Construct 被调用）
        if (!PendingUrl.IsEmpty())
        {
            ReturnMarker = PendingMarker;
            UE_LOG(LogTemp, Display, TEXT("[ShopperWebView] NativeConstruct 补加载 URL -> %s"), *PendingUrl);
            WebBrowser->LoadURL(PendingUrl);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[ShopperWebView] PendingUrl 为空，未加载任何网页（确认调用了 Open Url / Show Web View 并传入 URL）"));
        }
    }
}

void UShopperWebView::OpenUrl(const FString& Url, const FString& ReturnUrlMarker)
{
    PendingUrl = Url;
    PendingMarker = ReturnUrlMarker;
    // WebBrowser 已存在（Construct 之后）则立即加载；否则等 NativeConstruct 补加载
    if (WebBrowser && !Url.IsEmpty())
    {
        ReturnMarker = ReturnUrlMarker;
        UE_LOG(LogTemp, Display, TEXT("[ShopperWebView] OpenUrl 立即加载 -> %s"), *Url);
        WebBrowser->LoadURL(Url);
    }
    else if (Url.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShopperWebView] OpenUrl 收到空 URL，网页不会显示"));
    }
}

void UShopperWebView::CloseWebView()
{
    if (WebBrowser)
    {
        WebBrowser->OnUrlChanged.RemoveDynamic(this, &UShopperWebView::HandleUrlChanged);
    }
    RemoveFromParent();
}

void UShopperWebView::OnWebViewReturned_Implementation(const FString& FinalUrl)
{
    // 默认实现：命中回调即关闭。蓝图可覆写以做自定义收尾（如播放音效、刷新 UI）。
    CloseWebView();
}

void UShopperWebView::HandleUrlChanged(const FText& Text)
{
    const FString CurrentUrl = Text.ToString();
    if (!ReturnMarker.IsEmpty() && CurrentUrl.Contains(ReturnMarker))
    {
        OnWebViewReturn.Broadcast(CurrentUrl);
        OnWebViewReturned(CurrentUrl);
    }
}
