// Fill out your copyright notice in the Description page of Project Settings.

#include "USessionDataSubsystem.h"
#include "HttpProtoStruct.h"

void UUSessionDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UUSessionDataSubsystem::SaveSessionData(const FLoginUser& User, const FString& Token, const FSpeaker& Speaker)
{
    CachedUser    = User;
    SessionToken  = Token;
    CachedSpeaker = Speaker;
    
    UE_LOG(LogTemp, Log, TEXT("[SessionData] 保存会话: user=%s id=%d tokenLen=%d speaker=%s:%d"),
        *User.nickname, User.id, Token.Len(), *Speaker.ip, Speaker.port);
}

void UUSessionDataSubsystem::LoadSessionData(FLoginUser& OutUser, FString& OutToken, FSpeaker& OutSpeaker) const
{
    OutUser    = CachedUser;
    OutToken   = SessionToken;
    OutSpeaker = CachedSpeaker;
}

void UUSessionDataSubsystem::ClearSession()
{
    CachedUser    = FLoginUser();
    SessionToken.Reset();
    CachedSpeaker = FSpeaker();
    CachedWallet  = FWallet();
}

void UUSessionDataSubsystem::SaveWallet(const FWallet& Wallet)
{
    CachedWallet = Wallet;
    UE_LOG(LogTemp, Log, TEXT("[SessionData] 保存钱包: balance=%d star=%d bonus=%d diamond=%d"),
        Wallet.balance, Wallet.star, Wallet.bonus, Wallet.diamond);
}

void UUSessionDataSubsystem::LoadWallet(FWallet& OutWallet) const
{
    OutWallet = CachedWallet;
}

void UUSessionDataSubsystem::SaveShopList(const TArray<FShopItem>& ShopList)
{
    CachedShopList = ShopList;
}

void UUSessionDataSubsystem::LoadShopList(TArray<FShopItem>& OutShopList) const
{
    OutShopList = CachedShopList;
}
