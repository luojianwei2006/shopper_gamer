#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HttpProtoStruct.h"
#include "ShopperSettings.h"
#include "HttpProtoBFL.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(
	FOnGuestLoginDone, bool, bSuccess, FString, ResponseJson);

DECLARE_DYNAMIC_DELEGATE_TwoParams(
	FOnWalletDone, bool, bSuccess, FString, ResponseJson);

DECLARE_DYNAMIC_DELEGATE_TwoParams(
	FOnLoginByTokenDone, bool, bSuccess, FString, ResponseJson);

UCLASS()
class UHttpProtoBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 发送游客登录请求（Host 由蓝图传入，支持多环境切换）
	// 成功后内部自动调用 SaveSessionToken 持久化会话 token
	UFUNCTION(BlueprintCallable, Category = "Http协议",
			  meta = (DisplayName = "发送游客登录"))
	static void SendGuestLogin(
		const FString& Host,
		const FGuestLoginProto& Proto,
		const FOnGuestLoginDone& OnComplete);

	// 用游客 token 续期登录：POST /user/login_by_token
	// 成功后内部自动调用 SaveSessionToken 刷新本地 token
	// Host   : 接口域名，如 "http://192.168.10.8:8081"
	// Proto  : device（取 GetOrCreateDeviceId）+ token（取 LoadSessionToken）
	// 响应结构与游客登录完全一致，蓝图可直接复用「解析游客登录响应」
	UFUNCTION(BlueprintCallable, Category = "Http协议",
			  meta = (DisplayName = "发送Token续期登录"))
	static void SendLoginByToken(
		const FString& Host,
		const FLoginByTokenProto& Proto,
		const FOnLoginByTokenDone& OnComplete);

	// 获取或创建设备码：优先读本地存档，无则生成并持久化
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http协议",
			  meta = (DisplayName = "获取或创建设备码"))
	static FString GetOrCreateDeviceId();

	// 保存会话 token 到本地存档（登录成功后调用）
	UFUNCTION(BlueprintCallable, Category = "Http协议",
			  meta = (DisplayName = "保存会话Token"))
	static void SaveSessionToken(const FString& Token);

	// 读取本地保存的会话 token（启动判登录态用）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http协议",
			  meta = (DisplayName = "读取会话Token"))
	static FString LoadSessionToken();

	// 解析游客登录响应 JSON → 结构体（一层到位，蓝图直接拿 user / speaker / token）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http协议",
			  meta = (DisplayName = "解析游客登录响应"))
	static bool ParseGuestLoginResponse(const FString& JsonString, FLoginResponse& OutResponse);

	// 发送获取钱包请求（GET，自动在 header 带上 Authorization = Token）
	// Host   : 接口域名，如 "http://192.168.10.8:8081"
	// Token  : 登录时拿到的 token（来自登录响应的 token 字段 / 本地存档）
	UFUNCTION(BlueprintCallable, Category = "Http协议",
			  meta = (DisplayName = "发送获取钱包"))
	static void SendGetWallet(
		const FString& Host,
		const FString& Token,
		const FOnWalletDone& OnComplete);

	// 解析钱包响应 JSON → 结构体（蓝图直接拿 data.balance / star / bonus / diamond）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http协议",
			  meta = (DisplayName = "解析钱包响应"))
	static bool ParseWalletResponse(const FString& JsonString, FWalletResponse& OutResponse);

	// 获取当前环境配置里的接口基地址（项目设置 → Shopper 环境配置）
	// 蓝图可直接拿这个地址传给下面的发送函数；传空 Host 时发送函数也会自动用它
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http协议",
			  meta = (DisplayName = "获取接口基地址"))
	static FString GetApiBaseUrl();

	// 读取配置的客户端版本号（整数，项目设置 → Shopper 环境配置 → 版本）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http协议",
			  meta = (DisplayName = "获取客户端版本号"))
	static int32 GetAppVersion();
};
