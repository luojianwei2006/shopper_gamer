#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HttpProtoStruct.h"   // FShopReward
#include "ShopperRewardLibrary.generated.h"

// ───────────────────────────────────────────────────────────
// 奖励串解析工具库
//   reward1 / reward2 后端返回形如 "1002,0"（itemId,count）
//   另有 {key,value};{key,value};... 这类列表的通用正则解析
// ───────────────────────────────────────────────────────────
UCLASS()
class SHOPPERGAME_API UShopperRewardLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ① 解析单条奖励串 "1002,0"（兼容花括号 "{1002,0}" 与前后空格）
	//    OutReward.ItemId / OutReward.Count 为解析结果
	UFUNCTION(BlueprintCallable, Category = "Shop|Reward",
		meta = (ExpandBoolAsExecs = "bSuccess"))
	static void ParseReward(const FString& Raw, FShopReward& OutReward, bool& bSuccess);

	// ② 正则解析奖励列表：
	//      支持 "{1002,0};{1003,50}" 也支持 "1002,0;1003,50"
	//      逐个提取为整数对，返回 FShopReward 数组
	UFUNCTION(BlueprintCallable, Category = "Shop|Reward",
		meta = (ExpandBoolAsExecs = "bSuccess"))
	static void ParseRewardListRegex(const FString& Raw, TArray<FShopReward>& OutRewards, bool& bSuccess);

	// ②b 通用版：正则解析任意 "{key,value};{key,value};..." 列表
	//       key/value 保留为字符串（不强制整数），并行输出到两个数组
	UFUNCTION(BlueprintCallable, Category = "Shop|Reward",
		meta = (ExpandBoolAsExecs = "bSuccess"))
	static void ParseBraceKeyValueList(const FString& Raw,
		TArray<FString>& OutKeys, TArray<FString>& OutValues, bool& bSuccess);
};
