#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShopperPaymentWebView.h"
#include "ShopperPaymentSubsystem.generated.h"

// 支付编排子系统（GameInstance 级，跨关卡存活）
//
// 职责：
//   1. 持有支付 WebView 的蓝图子类引用（PaymentWidgetClass），由项目设置 CDO 指定 WBP_PaymentWebView。
//   2. ShowPayment(JumpUrl, ReturnUrlMarker?) 创建并加载支付页。
//   3. 支付页命中回跳地址后，通过 OnPaymentCompleted 广播最终回跳 URL，
//      游戏逻辑（如刷新钱包 SendGetWallet）绑这个事件即可。
//
// 回跳特征串优先用调用方传入的 ReturnUrlMarker；为空时回退到 DefaultReturnUrlMarker（CDO 配一次）。

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentCompleted, FString, FinalUrl);

UCLASS()
class SHOPPERGAME_API UShopperPaymentSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 弹出支付 WebView。JumpUrl 来自 SendShopBuy 的 Response.data.param.jumpUrl。
    // ReturnUrlMarker 为空时回退到 DefaultReturnUrlMarker。
    UFUNCTION(BlueprintCallable, Category = "Shopper|Payment")
    void ShowPayment(const FString& JumpUrl, const FString& ReturnUrlMarker = FString());

    // 主动关闭支付页（如用户取消）
    UFUNCTION(BlueprintCallable, Category = "Shopper|Payment")
    void ClosePayment();

    // 游戏逻辑绑这个：拿到最终回跳 URL 后刷新钱包 / 订单状态
    UPROPERTY(BlueprintAssignable, Category = "Shopper|Payment")
    FOnPaymentCompleted OnPaymentCompleted;

protected:
    // 蓝图子类（WBP_PaymentWebView）赋值到这里（项目设置 CDO 指定一次）
    UPROPERTY(EditDefaultsOnly, Category = "Shopper|Payment",
        meta = (AllowedClasses = "/Script/shoppergame.ShopperPaymentWebView"))
    TSoftClassPtr<UShopperPaymentWebView> PaymentWidgetClass;

    // 回跳地址特征串默认值（如 "web/pay/return" 或 "status=success"），CDO 配一次
    UPROPERTY(EditDefaultsOnly, Category = "Shopper|Payment")
    FString DefaultReturnUrlMarker;

private:
    UPROPERTY()
    TObjectPtr<UShopperPaymentWebView> ActiveWidget = nullptr;

    UFUNCTION()
    void HandleWidgetReturn(const FString& FinalUrl);
};
