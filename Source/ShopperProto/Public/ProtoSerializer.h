#pragma once

#include "CoreMinimal.h"

// 仅前向声明 protobuf 消息类型，避免把 protobuf/abseil 头（含 verify 宏冲突）暴露给依赖本模块的
// shoppergame。真正需要完整类型时，在 .cpp 里先 #undef verify 再 #include <google/protobuf/message.h>。
namespace google { namespace protobuf { class Message; } }

// 跨模块 API 宏（UBT 会为 ShopperProto 模块自动定义 SHOPPERPROTO_API；
// 若某些环境未定义则回退为空，保证编译通过）
#ifndef SHOPPERPROTO_API
#define SHOPPERPROTO_API
#endif

/**
 * protobuf 消息 <-> UE 字节数组 互转工具。
 * 序列化结果可直接作为 ShopperSocketSubsystem::SendMessage 的 Data 负载；
 * 收包时在 OnMessageReceived 里用 BytesToProto 还原。
 *
 * 注意：protobuf 消息是纯 C++ 类型（google::protobuf::Message 子类），
 * 蓝图无法直接使用，需在 C++ 中构造 / 解析后再转成字节或 UStruct 给蓝图。
 */
namespace FProtoSerializer
{
	// 将 protobuf 消息序列化为二进制字节（成功返回 true，OutBytes 为结果）
	SHOPPERPROTO_API bool ProtoToBytes(const google::protobuf::Message& Msg, TArray<uint8>& OutBytes);

	// 将二进制字节反序列化为 protobuf 消息（成功返回 true）
	SHOPPERPROTO_API bool BytesToProto(const TArray<uint8>& InBytes, google::protobuf::Message& OutMsg);
}
