#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DeviceSaveGame.generated.h"

// 本地持久化存档：保存设备码与会话 token，避免重开 App 丢失登录态
UCLASS()
class UDeviceSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 持久化的设备码（游客账号锚点）
	UPROPERTY(VisibleAnywhere, Category = "Device")
	FString DeviceId;

	// 会话 token（登录成功后写入，启动时可免登录直接续期）
	UPROPERTY(VisibleAnywhere, Category = "Device")
	FString SessionToken;
};
