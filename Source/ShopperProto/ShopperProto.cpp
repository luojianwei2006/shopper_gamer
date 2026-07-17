#include "ShopperProto.h"

DEFINE_LOG_CATEGORY_STATIC(LogShopperProto, Log, All);

void FShopperProtoModule::StartupModule()
{
	UE_LOG(LogShopperProto, Log, TEXT("ShopperProto module started (protobuf linked from brew)"));
}

void FShopperProtoModule::ShutdownModule()
{
}
