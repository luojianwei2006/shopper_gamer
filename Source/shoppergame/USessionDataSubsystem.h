// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpProtoStruct.h"       // FWallet, FLoginUser, FSpeaker, FLoginResponse
#include "USessionDataSubsystem.generated.h"

/**
 * 登录会话数据中枢：跨关卡 / Widget 共享 user、token、speaker
 * （注意：本类实例由 UGameInstance 持有，App 重启即清空；如需跨重启持久化 token 请配合 UDeviceSaveGame）
 */
UCLASS()
class SHOPPERGAME_API UUSessionDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
    // —— 新增：保存登录响应里的 user / token / speaker ——
    UFUNCTION(BlueprintCallable, Category = "Session",
              meta = (DisplayName = "保存会话数据"))
    void SaveSessionData(const FLoginUser& User, const FString& Token, const FSpeaker& Speaker);

    // —— 新增：读取 user / token / speaker ——
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session",
              meta = (DisplayName = "读取会话数据"))
    void LoadSessionData(FLoginUser& OutUser, FString& OutToken, FSpeaker& OutSpeaker) const;

    // —— 新增：保存钱包（获取钱包成功后调用）——
    UFUNCTION(BlueprintCallable, Category = "Session",
              meta = (DisplayName = "保存钱包"))
    void SaveWallet(const FWallet& Wallet);

    // —— 新增：读取钱包 ——
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session",
              meta = (DisplayName = "读取钱包"))
    void LoadWallet(FWallet& OutWallet) const;
	
	// -- 新增：保存商品列表
	UFUNCTION(BlueprintCallable, Category = "Session", meta = (DisplayName = "保存商品列表"))
	void SaveShopList(const TArray<FShopItem>& ShopList);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session", meta = (DisplayName = "读取商品列表"))
	void LoadShopList(TArray<FShopItem>& OutShopList) const;

    // 便捷：是否已登录（有 token 即视为已登录）
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session")
    bool HasSession() const { return !SessionToken.IsEmpty(); }

    // 退出登录：清空所有会话数据
    UFUNCTION(BlueprintCallable, Category = "Session")
    void ClearSession();

private:
    // 新增缓存字段
    UPROPERTY()
    FLoginUser CachedUser;

    UPROPERTY()
    FString SessionToken;

    UPROPERTY()
    FSpeaker CachedSpeaker;

    UPROPERTY()
    FWallet CachedWallet;

	UPROPERTY()
	TArray<FShopItem> CachedShopList;
};
