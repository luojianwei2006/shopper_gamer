#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonObjectWrapper.h"   // FJsonObjectWrapper（UE5.6 路径无 Dom/ 前缀）
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "ShopperJsonLibrary.generated.h"

// ───────────────────────────────────────────────────────────
// 蓝图侧读取 FJsonObjectWrapper（任意 JSON 结构）字段的工具库
// 配合 FShopperJsonResponse 使用：拿到 data 后按 key 取任意字段，
// 不必为每个接口都建一套强类型结构。
// ───────────────────────────────────────────────────────────
UCLASS()
class SHOPPERGAME_API UShopperJsonLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 判断 data 里是否含某个 key（蓝图里先 Has 再 Get 更安全）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static bool HasJsonField(const FJsonObjectWrapper& Object, const FString& Key);

	// 取字符串字段（如 "id"、"nickname"）；不存在返回 false，OutValue 清空
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static bool GetJsonStringField(const FJsonObjectWrapper& Object, const FString& Key, FString& OutValue);

	// 取浮点字段（JSON 数字，含小数/整数都能拿）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static bool GetJsonNumberField(const FJsonObjectWrapper& Object, const FString& Key, float& OutValue);

	// 取整数字段（JSON 数字截断为整数）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static bool GetJsonIntField(const FJsonObjectWrapper& Object, const FString& Key, int32& OutValue);

	// 取布尔字段
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static bool GetJsonBoolField(const FJsonObjectWrapper& Object, const FString& Key, bool& OutValue);

	// 取子对象字段（嵌套 JSON 对象）；返回新的 FJsonObjectWrapper
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static bool GetJsonObjectField(const FJsonObjectWrapper& Object, const FString& Key, FJsonObjectWrapper& OutValue);

	// 取数组字段（每个元素须为 JSON 对象）；返回 FJsonObjectWrapper 数组
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static bool GetJsonArrayField(const FJsonObjectWrapper& Object, const FString& Key, TArray<FJsonObjectWrapper>& OutValue);

	// 把整个 JSON 结构序列化成字符串（调试/日志用）
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Http|Json")
	static FString JsonToString(const FJsonObjectWrapper& Object);
};
