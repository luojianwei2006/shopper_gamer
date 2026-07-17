// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ShopperProto : ModuleRules
{
	public ShopperProto(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// protobuf 生成的 .pb.cpp 及运行时需要 RTTI 与异常支持。
		// 单独放在这个模块里，避免污染游戏主模块（shoppergame）的编译选项。
		bUseRTTI = true;
		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });

		// ── protobuf 头文件（来自 brew 安装的 33.2，与 protoc 版本一致）──
		// 如要跨机器 / CI 一致，可改为项目内 ThirdParty 的相对路径。
		string ProtobufInclude = "/opt/homebrew/opt/protobuf/include";
		PublicSystemIncludePaths.Add(ProtobufInclude);

		// ── abseil 头文件（protobuf 33.2 的公开头依赖 absl，编译期必须可见）──
		string AbseilInclude = "/opt/homebrew/opt/abseil/include";
		PublicSystemIncludePaths.Add(AbseilInclude);

		// ── protobuf 运行时库（动态库；dev 本机可直接链接）──
		// 其 install_name 为绝对路径 /opt/homebrew/opt/protobuf/lib/libprotobuf.33.2.0.dylib，
		// 编辑器运行期 dyld 会去该路径加载，本机可用。
		// 打包上架需改为随包分发（把 dylib 拷进 .app/Contents/Frameworks 并
		// install_name_tool -id @rpath/libprotobuf.dylib，再给模块加 rpath），
		// 或改用静态库 libprotobuf.a（见工作日志说明）。
		string ProtobufLib = "/opt/homebrew/opt/protobuf/lib/libprotobuf.dylib";
		PublicAdditionalLibraries.Add(ProtobufLib);
	}
}
