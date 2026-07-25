#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"         // UOverlay：蓝图子类里的 WebBrowserSlot 容器
#include "WebBrowser.h"                 // UWebBrowser / FOnUrlChanged(const FText&) — WebBrowserWidget 模块
#include "ShopperWebView.generated.h"

// 通用 WebView 基类（C++ 提供引擎能力，蓝图子类 WBP_WebView 负责视觉布局）
//
// 设计边界：
//   - C++ 负责：运行时创建 UWebBrowser、OpenUrl 加载网页、绑定 OnUrlChanged 检测回调地址、
//     命中后广播 OnWebViewReturn。
//   - 蓝图负责：在子类里放一个命名为 "WebBrowserSlot" 的 Overlay（作为 WebBrowser 的容器），
//     以及背景遮罩、关闭按钮等视觉；可选覆写 OnWebViewReturned 做自定义收尾。
//
// 回调特征串（ReturnUrlMarker）可选：传入后，当页面跳转到包含该特征串的 URL 时触发回调；
// 不传则纯展示，不自动回调（适用于公告、活动页等只展示的场景）。

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebViewReturn, FString, FinalUrl);

UCLASS()
class SHOPPERGAME_API UShopperWebView : public UUserWidget
{
    GENERATED_BODY()

public:
    // 由 UShopperWebSubsystem::ShowWebView 调用：加载网页并（可选）设定回调检测特征串
    UFUNCTION(BlueprintCallable, Category = "Shopper|WebView")
    void OpenUrl(const FString& Url, const FString& ReturnUrlMarker = TEXT(""));

    // 关闭并移出视口（命中回调或用户点 X 时调用）
    UFUNCTION(BlueprintCallable, Category = "Shopper|WebView")
    void CloseWebView();

    // 命中回调地址时广播（蓝图可绑，也可不绑——Subsystem 已默认绑定）
    UPROPERTY(BlueprintAssignable, Category = "Shopper|WebView")
    FOnWebViewReturn OnWebViewReturn;

    // 蓝图可覆写：命中回调地址后的自定义收尾（关闭按钮逻辑也建议放这里）
    UFUNCTION(BlueprintNativeEvent, Category = "Shopper|WebView")
    void OnWebViewReturned(const FString& FinalUrl);

protected:
    virtual void NativeConstruct() override;

    // 蓝图子类必须放置一个命名为 "WebBrowserSlot" 的 Overlay 作为 WebBrowser 容器
    // （BindWidget 属性按惯例放 protected，避免部分引擎版本下绑定解析失败）
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOverlay> WebBrowserSlot = nullptr;

    UPROPERTY()
    TObjectPtr<UWebBrowser> WebBrowser = nullptr;

    // 待加载 URL（OpenUrl 可能在 NativeConstruct 之前被调用，那时 WebBrowser 尚未创建）
    UPROPERTY()
    FString PendingUrl;

    UPROPERTY()
    FString PendingMarker;

    UPROPERTY()
    FString ReturnMarker;

    UFUNCTION()
    void HandleUrlChanged(const FText& Text);
};
