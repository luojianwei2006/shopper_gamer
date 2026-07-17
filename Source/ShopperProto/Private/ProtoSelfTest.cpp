// protobuf 管线自测：构造 SampleLogin -> 序列化 -> 反序列化 -> 字段比对。
// 用法（编辑器或打包后均可）：在控制台（Output Log 输入框 / 游戏内 ~）执行
//   protobuf_selftest
// 看到 [ProtobufTest] ... -> PASS 即证明 protobuf 在 UE 的 PCH/宏环境下可正常工作。
//
// 注意：Sample.proto 仅用于验证；正式协议请换成你自己的 .proto 生成的 .pb.h。

#include "CoreMinimal.h"
#include "ProtoSerializer.h"   // 仅前向声明 google::protobuf::Message，安全

// UE 的 PCH 把 verify 定义成宏，与 abseil btree.h 的 verify() 成员冲突，
// 必须在包含任何 protobuf/abseil 头之前取消。
#undef verify
#include "Proto/Sample.pb.h"   // UBT 以 Public 为根 -> Public/Proto/Sample.pb.h
#include "Misc/Char.h"
#include "HAL/IConsoleManager.h"

static void RunProtobufSelfTest()
{
	using namespace shopper::sample;

	// 1) 构造并填充消息
	SampleLogin Src;
	Src.set_uid(12345);
	Src.set_name("shopper_test");
	Src.set_timestamp(1784193958911LL);
	Src.set_is_guest(true);

	// 2) 序列化 -> 二进制字节
	TArray<uint8> Bytes;
	if (!FProtoSerializer::ProtoToBytes(Src, Bytes))
	{
		UE_LOG(LogTemp, Error, TEXT("[ProtobufTest] 序列化失败"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[ProtobufTest] 序列化得到 %d 字节"), Bytes.Num());

	// 3) 反序列化 -> 新消息
	SampleLogin Dst;
	if (!FProtoSerializer::BytesToProto(Bytes, Dst))
	{
		UE_LOG(LogTemp, Error, TEXT("[ProtobufTest] 反序列化失败"));
		return;
	}

	// 4) 字段比对
	const bool bOk =
		Dst.uid()       == Src.uid() &&
		Dst.name()      == Src.name() &&
		Dst.timestamp() == Src.timestamp() &&
		Dst.is_guest()  == Src.is_guest();

	UE_LOG(LogTemp, Log,
		TEXT("[ProtobufTest] uid=%d name=%s ts=%lld guest=%d -> %s"),
		Dst.uid(),
		UTF8_TO_TCHAR(Dst.name().c_str()),
		(int64)Dst.timestamp(),
		Dst.is_guest() ? 1 : 0,
		bOk ? TEXT("PASS") : TEXT("FAIL"));
}

static FAutoConsoleCommand GProtobufSelfTestCmd(
	TEXT("protobuf_selftest"),
	TEXT("Protobuf 序列化/反序列化往返自测（SampleLogin）"),
	FConsoleCommandDelegate::CreateStatic(&RunProtobufSelfTest)
);
