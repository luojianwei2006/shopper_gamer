#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * ShopperProto 模块：封装 Google protobuf 运行时，供 shoppergame 使用。
 * - 提供 FProtoSerializer（protobuf 消息 <-> UE 字节数组互转）
 * - 所有生成的 .pb.h / .pb.cpp 放本模块的 Public/Proto 目录下
 * - 本模块单独开启 RTTI / 异常，隔离对游戏主模块的影响
 */
class FShopperProtoModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
