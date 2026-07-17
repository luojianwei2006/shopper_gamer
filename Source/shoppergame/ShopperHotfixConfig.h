#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShopperHotfixConfig.generated.h"

/**
 * 喇叭服热更配置（随热更 pak 下发，运行时覆盖 socket 连接 / 帧格式参数）。
 * 不改动编译期的 UShopperSettings（项目设置），仅作为「可被 pak 覆盖的运行时层」。
 *
 * 使用方式：
 *   1. 在 /Game/Hotfix 下建一个本资产实例（如 ShopperHotfixConfig），
 *      填入要热更的 host/port/心跳/帧格式；
 *   2. 把它打进热更 pak 下发；
 *   3. UHotReloadManager::MountPak 挂载后，UShopperSocketSubsystem 自动加载并应用，
 *      下次连接即生效（已连接则不强行断链，仅心跳间隔/帧格式于下次重连生效）。
 */
UCLASS(BlueprintType)
class SHOPPERGAME_API UShopperHotfixConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FString SocketHost = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 SocketPort = 9000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bAutoConnect = true;

	// 心跳间隔（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket", meta = (ClampMin = "1"))
	float HeartbeatInterval = 15.f;

	// 断线重连间隔（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket", meta = (ClampMin = "1"))
	float ReconnectInterval = 3.f;

	// 帧尾是否带 1 字节校验（与服务器约定一致，否则解析全部错位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bUseFrameChecksum = true;

	// 单包字节上限（防御畸形包 OOM）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 PacketMaxSize = 10 * 1024 * 1024;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ShopperHotfixConfig", GetFName());
	}
};
