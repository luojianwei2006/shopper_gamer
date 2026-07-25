#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WebBrowser.h"                 // UWebBrowser / FWebBrowserUrlChangedParams (WebBrowserWidget)
#include "ShopperPaymentWebView.generated.h"

// 应用内支付 WebView 基类（C++ 提供引擎能力，蓝图子类 WBP_PaymentWebView 负责视觉布局）
//
// 设计边界：
//   - C++ 负责：运行时创建 UWebBrowser、LoadURL、绑定 OnUrlChanged 检测回跳地址、
//     命中后广播 OnPaymentReturn 并自动关闭。
//   - 蓝图负责：在子类里放一个命名为 "WebBrowserSlot" 的 Overlay（作为 WebBrowser 的容器），
//     以及背景遮罩、关闭按钮等视觉；可选覆写 OnPaymentReturn 做自定义收尾。
//
// 回跳特征串（ReturnUrlMarker，如 "web/pay/return" 或 "status=success"）由调用方在
// SetupPayment 时传入，无需在蓝图里硬编码。

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentWebViewReturn, FString, FinalUrl);

UCLASS()
class SHOPPERGAME_API UShopperPaymentWebView : public UUserWidget
{
    GENERATED_BODY()

public:
    // 由 UShopperPaymentSubsystem::ShowPayment 调用：加载支付页并设定回跳检测特征串
    UFUNCTION(BlueprintCallable, Category = "Shopper|Payment")
    void SetupPayment(const FString& JumpUrl, const FString& ReturnUrlMarker);

    // 关闭并移出视口（命中回跳或用户点 X 时调用）
    UFUNCTION(BlueprintCallable, Category = "Shopper|Payment")
    void ClosePayment();

    // 命中回跳地址时广播（蓝图可绑，也可不绑——Subsystem 已默认绑定）
    UPROPERTY(BlueprintAssignable, Category = "Shopper|Payment")
    FOnPaymentWebViewReturn OnPaymentReturn;

    // 蓝图可覆写：命中回跳地址后的自定义收尾（关闭按钮逻辑也建议放这里）
    UFUNCTION(BlueprintNativeEvent, Category = "Shopper|Payment")
    void OnPaymentReturned(const FString& FinalUrl);

protected:
    virtual void NativeConstruct() override;

private:
    // 蓝图子类必须放置一个命名为 "WebBrowserSlot" 的 Overlay 作为 WebBrowser 容器
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOverlay> WebBrowserSlot = nullptr;

    UPROPERTY()
    TObjectPtr<UWebBrowser> WebBrowser = nullptr;

    UPROPERTY()
    FString ReturnMarker;

    UFUNCTION()
    void HandleUrlChanged(const FWebBrowserUrlChangedParams& Params);
};
