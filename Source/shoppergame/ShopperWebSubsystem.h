#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShopperWebView.h"
#include "ShopperWebSubsystem.generated.h"

// 通用 WebView 编排子系统（GameInstance 级，跨关卡存活）
//
// 职责：
//   1. 持有 WebView 的蓝图子类引用（WebViewWidgetClass），由项目设置 CDO 指定 WBP_WebView。
//   2. ShowWebView(Url, ReturnUrlMarker?) 创建并加载网页。
//   3. 页面跳转到回调特征串地址后，通过 OnWebViewCompleted 广播最终 URL，
//      游戏逻辑（如刷新钱包 SendGetWallet）绑这个事件即可；纯展示场景不传特征串则不触发。
//
// 回调特征串优先用调用方传入的 ReturnUrlMarker；为空时回退到 DefaultReturnUrlMarker（CDO 配一次）。

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebViewCompleted, FString, FinalUrl);

UCLASS()
class SHOPPERGAME_API UShopperWebSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 弹出通用 WebView。Url 为要打开的网页地址（任何 http/https 链接）。
    // ReturnUrlMarker 为空时纯展示不回调；传入特征串则在跳转命中时广播 OnWebViewCompleted。
    UFUNCTION(BlueprintCallable, Category = "Shopper|WebView")
    void ShowWebView(const FString& Url, const FString& ReturnUrlMarker = TEXT(""));

    // 主动关闭 WebView（如用户取消）
    UFUNCTION(BlueprintCallable, Category = "Shopper|WebView")
    void CloseWebView();

    // 游戏逻辑绑这个：拿到最终回调 URL 后做后续处理
    UPROPERTY(BlueprintAssignable, Category = "Shopper|WebView")
    FOnWebViewCompleted OnWebViewCompleted;

protected:
    // 蓝图子类（WBP_WebView）赋值到这里（项目设置 CDO 指定一次）
    UPROPERTY(EditDefaultsOnly, Category = "Shopper|WebView",
        meta = (AllowedClasses = "/Script/shoppergame.ShopperWebView"))
    TSoftClassPtr<UShopperWebView> WebViewWidgetClass;

    // 回调地址特征串默认值（按需配置，如 "status=success"），CDO 配一次
    UPROPERTY(EditDefaultsOnly, Category = "Shopper|WebView")
    FString DefaultReturnUrlMarker;

private:
    UPROPERTY()
    TObjectPtr<UShopperWebView> ActiveWidget = nullptr;

    UFUNCTION()
    void HandleWidgetReturn(FString FinalUrl);
};
