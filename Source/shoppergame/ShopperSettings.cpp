#include "ShopperSettings.h"
#include "Misc/CommandLine.h"

FString UShopperSettings::GetActiveBaseUrl() const
{
	// 1) 编译期强制（发正式包时由 Build.cs 注入宏，保证二进制只连正式服）
#if defined(SHOPPER_FORCE_ENV_PROD)
	return ProdBaseUrl;
#elif defined(SHOPPER_FORCE_ENV_TEST)
	return TestBaseUrl;
#elif defined(SHOPPER_FORCE_ENV_DEV)
	return DevBaseUrl;
#else
	// 2) 命令行覆盖（QA 不重新打包即可切环境）：-ShopperEnv=Test
	FString CmdEnv;
	if (FParse::Value(FCommandLine::Get(), TEXT("ShopperEnv="), CmdEnv))
	{
		if (CmdEnv == TEXT("Prod")) return ProdBaseUrl;
		if (CmdEnv == TEXT("Test")) return TestBaseUrl;
		if (CmdEnv == TEXT("Dev"))  return DevBaseUrl;
	}

	// 3) 默认走项目设置里的 ActiveEnvironment
	switch (ActiveEnvironment)
	{
		case EShopperEnvironment::Test: return TestBaseUrl;
		case EShopperEnvironment::Prod: return ProdBaseUrl;
		case EShopperEnvironment::Dev:
		default:                        return DevBaseUrl;
	}
#endif
}

FString UShopperSettings::ResolveBaseUrl()
{
	if (const UShopperSettings* Settings = GetDefault<UShopperSettings>())
	{
		return Settings->GetActiveBaseUrl();
	}
	UE_LOG(LogTemp, Warning, TEXT("[ShopperSettings] 未找到环境配置，返回空基地址"));
	return TEXT("");
}

int32 UShopperSettings::ResolveAppVersion()
{
	if (const UShopperSettings* Settings = GetDefault<UShopperSettings>())
	{
		return Settings->GetAppVersion();
	}
	UE_LOG(LogTemp, Warning, TEXT("[ShopperSettings] 未找到环境配置，返回版本号 0"));
	return 0;
}
