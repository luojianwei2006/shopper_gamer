#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ShopperSettings.generated.h"

// 环境枚举：开发 / 测试 / 正式，对应三套接口地址
UENUM(BlueprintType)
enum class EShopperEnvironment : uint8
{
	Dev  UMETA(DisplayName = "开发环境"),
	Test UMETA(DisplayName = "测试环境"),
	Prod UMETA(DisplayName = "正式环境"),
};

// 全局环境配置：在「项目设置 → Shopper 环境配置」里维护，自动写入 DefaultGame.ini
// 三套版本（开发 / 测试 / 正式）各配一个基地址，运行时按 ActiveEnvironment 选择
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Shopper 环境配置"))
class UShopperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 当前生效的环境（开发 / 测试 / 正式）
	UPROPERTY(Config, EditAnywhere, Category = "环境")
	EShopperEnvironment ActiveEnvironment = EShopperEnvironment::Dev;

	// 各环境接口基地址（结尾不要带斜杠）
	UPROPERTY(Config, EditAnywhere, Category = "环境")
	FString DevBaseUrl  = TEXT("http://192.168.10.8:8081");

	UPROPERTY(Config, EditAnywhere, Category = "环境")
	FString TestBaseUrl = TEXT("https://test.example.com");

	UPROPERTY(Config, EditAnywhere, Category = "环境")
	FString ProdBaseUrl = TEXT("https://api.example.com");

	// 客户端版本号（整数）。随构建 flavor 在 DefaultGame.ini 里配，或出包时由 Build.cs 注入宏覆盖。
	// 可填进登录协议的 ver 字段，或作为请求头随版本上报。
	UPROPERTY(Config, EditAnywhere, Category = "版本")
	int32 AppVersion = 1;

	// 返回当前环境的接口基地址
	// 优先级：编译期 SHOPPER_FORCE_ENV_* 宏 > 命令行 -ShopperEnv=xxx > ActiveEnvironment 配置
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "环境")
	FString GetActiveBaseUrl() const;

	// 静态便捷入口：给 BFL / 任意模块直接拿地址，无需持有实例
	static FString ResolveBaseUrl();

	// 读取当前配置的客户端版本号（整数）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "版本")
	int32 GetAppVersion() const { return AppVersion; }

	// 静态便捷入口：给 BFL / 任意模块直接拿版本号，无需持有实例
	static int32 ResolveAppVersion();

	// ───────────────────────────────────────────────────────────
	// Socket 长链接配置
	// ───────────────────────────────────────────────────────────
	// 服务器地址（当前为全局单值；若开发/测试/正式各不同，按构建 flavor 用不同 DefaultGame.ini 覆盖）
	UPROPERTY(Config, EditAnywhere, Category = "Socket")
	FString SocketHost = TEXT("127.0.0.1");

	UPROPERTY(Config, EditAnywhere, Category = "Socket")
	int32 SocketPort = 9000;

	// 启动后是否自动建链（进入应用即连；若需登录后再连，设 false 并手动调 Connect）
	UPROPERTY(Config, EditAnywhere, Category = "Socket")
	bool bAutoConnect = true;

	// 心跳间隔（秒）
	UPROPERTY(Config, EditAnywhere, Category = "Socket", meta = (ClampMin = "1"))
	float HeartbeatInterval = 3.f;

	// 断线重连间隔（秒）
	UPROPERTY(Config, EditAnywhere, Category = "Socket", meta = (ClampMin = "1"))
	float ReconnectInterval = 3.f;

	// 单包 data 字节上限（防御畸形包导致 OOM），默认 10MB
	UPROPERTY(Config, EditAnywhere, Category = "Socket")
	int32 PacketMaxSize = 10 * 1024 * 1024;

	// 帧尾是否带 1 字节校验（UE 侧自定义 XOR 取低 8 位）。
	//  true  : 帧 = [protocol(小端 short)][length(小端 int)][data][md5(1)]
	//  false : 帧 = [protocol(小端 short)][length(小端 int)][data]
	//           —— data.data 即纯净 protobuf，Java 端 parseFrom 不会因尾部多 1 字节而失败。
	// 注意：本端校验算法是自定义 XOR，Java 端若用不同算法验证会不匹配；
	//       若服务器 parseFrom(data.data) 把 md5 也算进 data 导致解析失败，保持 false（默认）。
	UPROPERTY(Config, EditAnywhere, Category = "Socket")
	bool bUseFrameChecksum = true;
};
