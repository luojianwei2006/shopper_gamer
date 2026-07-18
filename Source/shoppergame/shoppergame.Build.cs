// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class shoppergame : ModuleRules
{
	public shoppergame(ReadOnlyTargetRules Target) : base(Target)
	{
		// NoSharedPCHs：本模块使用自己的私有 PCH（基于自身 #include 图生成），
		// 不使用引擎的 UnrealEd 共享 PCH。
		// 原因：shoppergame 依赖 UMG/Slate/StateTree 等模块，UBT 在 IWYU/新版构建设置下
		// 默认会选 UnrealEd 共享 PCH，而该 PCH 内含 ToolMenus 模块，会把宏 CURRENT_FILE_ID
		// 残留为 ToolMenus。一旦 ShopperSocketSubsystem.h 嵌套包含带反射的
		// BaseSocketSubsystem.h，其 GENERATED_BODY() 就会展开成不存在的
		// FID_..._ToolMenus_h_23_GENERATED_BODY → 报 "a type specifier is required"。
		// 改用私有 PCH 后，CURRENT_FILE_ID 在各文件自己的 X.generated.h（文件末尾）处被正确设置，
		// 污染源头被切断。
		PCHUsage = PCHUsageMode.NoSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate",
			"HTTP",
			"Json",          // JSON 构造
			"JsonUtilities", // FJsonObjectConverter
			"DeveloperSettings", // UDeveloperSettings：项目设置里的环境配置
			"AssetRegistry",  // 热更 pak 挂载后刷新资源注册表（ScanFilesSynchronous）
			"Sockets",       // 持久化 TCP 长链接
			"Networking",    // FIPv4Endpoint / FInternetAddr
			"PakFile",       // FPakPlatformFile::Mount/Unmount —— 运行时热更 pak 挂载（链接期需要此模块）
			"ShopperProto"   // protobuf 运行时：消息 <-> 字节互转（依赖该模块才能 #include 其头与链接 libprotobuf）
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"shoppergame",
			"shoppergame/Variant_Strategy",
			"shoppergame/Variant_Strategy/UI",
			"shoppergame/Variant_TwinStick",
			"shoppergame/Variant_TwinStick/AI",
			"shoppergame/Variant_TwinStick/Gameplay",
			"shoppergame/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

		// ── 环境强绑定（发正式包时取消对应注释，保证二进制只连正式服）──
		// ShopperSettings 的 GetActiveBaseUrl() 会优先读取这些宏。
		// 开发 / 测试包保持注释，地址走 DefaultGame.ini 的 ActiveEnvironment。
		// PublicDefinitions.Add("SHOPPER_FORCE_ENV_DEV");   // 强制开发环境
		// PublicDefinitions.Add("SHOPPER_FORCE_ENV_TEST");  // 强制测试环境
		// PublicDefinitions.Add("SHOPPER_FORCE_ENV_PROD");   // 强制正式环境（上架包用这个）
	}
}
