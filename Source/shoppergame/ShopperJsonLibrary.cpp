#include "ShopperJsonLibrary.h"
#include "Serialization/JsonReader.h"

// FJsonObjectWrapper 内部靠 JsonString + PostSerialize 解析出 JsonObject。
// 极少数路径下 JsonObject 可能为 null 而 JsonString 已填，这里做一层兜底：
// JsonObject 无效时尝试把 JsonString 解析进 JsonObject，避免取字段无故失败。
static void EnsureJsonObject(FJsonObjectWrapper& Object)
{
	if (Object.JsonObject.IsValid() || Object.JsonString.IsEmpty())
	{
		return;
	}
	TSharedPtr<FJsonObject> Parsed;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Object.JsonString);
	if (FJsonSerializer::Deserialize(Reader, Parsed) && Parsed.IsValid())
	{
		Object.JsonObject = Parsed;
	}
}

bool UShopperJsonLibrary::HasJsonField(const FJsonObjectWrapper& Object, const FString& Key)
{
	FJsonObjectWrapper Local = Object;   // 廉价拷贝（内部 TSharedPtr 引用计数）
	EnsureJsonObject(Local);
	return Local.JsonObject.IsValid() && Local.JsonObject->HasField(Key);
}

bool UShopperJsonLibrary::GetJsonStringField(const FJsonObjectWrapper& Object, const FString& Key, FString& OutValue)
{
	OutValue = FString();
	FJsonObjectWrapper Local = Object;
	EnsureJsonObject(Local);
	if (!Local.JsonObject.IsValid() || !Local.JsonObject->HasField(Key))
	{
		return false;
	}
	const TSharedPtr<FJsonValue> Value = Local.JsonObject->TryGetField(Key);
	if (!Value.IsValid() || Value->Type != EJson::String)
	{
		return false;
	}
	OutValue = Value->AsString();
	return true;
}

bool UShopperJsonLibrary::GetJsonNumberField(const FJsonObjectWrapper& Object, const FString& Key, float& OutValue)
{
	OutValue = 0.f;
	FJsonObjectWrapper Local = Object;
	EnsureJsonObject(Local);
	if (!Local.JsonObject.IsValid() || !Local.JsonObject->HasField(Key))
	{
		return false;
	}
	const TSharedPtr<FJsonValue> Value = Local.JsonObject->TryGetField(Key);
	if (!Value.IsValid() || (Value->Type != EJson::Number))
	{
		return false;
	}
	OutValue = static_cast<float>(Value->AsNumber());
	return true;
}

bool UShopperJsonLibrary::GetJsonIntField(const FJsonObjectWrapper& Object, const FString& Key, int32& OutValue)
{
	OutValue = 0;
	float Number = 0.f;
	if (!GetJsonNumberField(Object, Key, Number))
	{
		return false;
	}
	OutValue = FMath::FloorToInt(Number);
	return true;
}

bool UShopperJsonLibrary::GetJsonBoolField(const FJsonObjectWrapper& Object, const FString& Key, bool& OutValue)
{
	OutValue = false;
	FJsonObjectWrapper Local = Object;
	EnsureJsonObject(Local);
	if (!Local.JsonObject.IsValid() || !Local.JsonObject->HasField(Key))
	{
		return false;
	}
	const TSharedPtr<FJsonValue> Value = Local.JsonObject->TryGetField(Key);
	if (!Value.IsValid() || Value->Type != EJson::Boolean)
	{
		return false;
	}
	OutValue = Value->AsBool();
	return true;
}

bool UShopperJsonLibrary::GetJsonObjectField(const FJsonObjectWrapper& Object, const FString& Key, FJsonObjectWrapper& OutValue)
{
	OutValue = FJsonObjectWrapper();
	FJsonObjectWrapper Local = Object;
	EnsureJsonObject(Local);
	if (!Local.JsonObject.IsValid() || !Local.JsonObject->HasField(Key))
	{
		return false;
	}
	const TSharedPtr<FJsonValue> Value = Local.JsonObject->TryGetField(Key);
	if (!Value.IsValid() || Value->Type != EJson::Object)
	{
		return false;
	}
	OutValue.JsonObject = Value->AsObject();
	return true;
}

bool UShopperJsonLibrary::GetJsonArrayField(const FJsonObjectWrapper& Object, const FString& Key, TArray<FJsonObjectWrapper>& OutValue)
{
	OutValue.Empty();
	FJsonObjectWrapper Local = Object;
	EnsureJsonObject(Local);
	if (!Local.JsonObject.IsValid() || !Local.JsonObject->HasField(Key))
	{
		return false;
	}
	const TSharedPtr<FJsonValue> Value = Local.JsonObject->TryGetField(Key);
	if (!Value.IsValid() || Value->Type != EJson::Array)
	{
		return false;
	}
	const TArray<TSharedPtr<FJsonValue>>& Arr = Value->AsArray();
	for (const TSharedPtr<FJsonValue>& Elem : Arr)
	{
		if (Elem.IsValid() && Elem->Type == EJson::Object)
		{
			FJsonObjectWrapper ElemWrapper;
			ElemWrapper.JsonObject = Elem->AsObject();
			OutValue.Add(ElemWrapper);
		}
	}
	return true;
}

FString UShopperJsonLibrary::JsonToString(const FJsonObjectWrapper& Object)
{
	FString Out;
	// 优先用内置方法序列化 JsonObject；兜底直接返回 JsonString
	if (Object.JsonObjectToString(Out))
	{
		return Out;
	}
	return Object.JsonString;
}
