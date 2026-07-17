#include "ProtoSerializer.h"

// UE 把 verify 定义成宏，与 abseil btree.h 的 verify() 成员冲突，必须在包含 protobuf 头之前取消。
#ifdef verify
#undef verify
#endif

#include <google/protobuf/message.h>

namespace FProtoSerializer
{
	bool ProtoToBytes(const google::protobuf::Message& Msg, TArray<uint8>& OutBytes)
	{
		std::string Buffer;
		if (!Msg.SerializeToString(&Buffer))
		{
			return false;
		}
		OutBytes.Reset();
		OutBytes.Append(reinterpret_cast<const uint8*>(Buffer.data()), static_cast<int32>(Buffer.size()));
		return true;
	}

	bool BytesToProto(const TArray<uint8>& InBytes, google::protobuf::Message& OutMsg)
	{
		if (InBytes.Num() <= 0)
		{
			return false;
		}
		std::string Buffer(reinterpret_cast<const char*>(InBytes.GetData()), static_cast<size_t>(InBytes.Num()));
		bool ret = OutMsg.ParseFromString(Buffer);
		return ret;
	}
}
