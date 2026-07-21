#include "ShopperRewardLibrary.h"
#include "Internationalization/Regex.h"   // FRegexPattern / FRegexMatcher (UE5.6 路径，旧版为 HAL/Regex.h)

void UShopperRewardLibrary::ParseReward(const FString& Raw, FShopReward& OutReward, bool& bSuccess)
{
	OutReward = FShopReward();
	bSuccess = false;

	FString Trimmed = Raw.TrimStartAndEnd();
	// 兼容 "{1002,0}" 形式：去掉首尾花括号
	if (Trimmed.StartsWith(TEXT("{")) && Trimmed.EndsWith(TEXT("}")))
	{
		Trimmed = Trimmed.Mid(1, Trimmed.Len() - 2).TrimStartAndEnd();
	}

	TArray<FString> Parts;
	// 按逗号拆分，剔除空段（容错 "1002, 0" 这种带空格的写法）
	if (Trimmed.ParseIntoArray(Parts, TEXT(","), /*bCullEmpty=*/true) >= 2)
	{
		OutReward.ItemId = FCString::Atoi(*Parts[0]);
		OutReward.Count  = FCString::Atoi(*Parts[1]);
		bSuccess = true;
	}
}

void UShopperRewardLibrary::ParseRewardListRegex(const FString& Raw, TArray<FShopReward>& OutRewards, bool& bSuccess)
{
	OutRewards.Empty();
	bSuccess = false;

	// 匹配「可选花括号包裹的整数对」：{1002,0} 或 1002,0 都能抓到
	// 用 static 让正则只编译一次
	static const FRegexPattern Pattern(TEXT(R"(\{?\s*(\d+)\s*,\s*(\d+)\s*\}?)"));
	FRegexMatcher Matcher(Pattern, Raw);
	while (Matcher.FindNext())
	{
		FShopReward R;
		R.ItemId = FCString::Atoi(*Matcher.GetCaptureGroup(1));
		R.Count  = FCString::Atoi(*Matcher.GetCaptureGroup(2));
		OutRewards.Add(R);
	}
	bSuccess = OutRewards.Num() > 0;
}

void UShopperRewardLibrary::ParseBraceKeyValueList(const FString& Raw,
	TArray<FString>& OutKeys, TArray<FString>& OutValues, bool& bSuccess)
{
	OutKeys.Empty();
	OutValues.Empty();
	bSuccess = false;

	// 通用：匹配 {key,value}，key/value 可为任意非花括号字符
	static const FRegexPattern Pattern(TEXT(R"(\{\s*([^,{}]+?)\s*,\s*([^}]+?)\s*\})"));
	FRegexMatcher Matcher(Pattern, Raw);
	while (Matcher.FindNext())
	{
		OutKeys.Add(Matcher.GetCaptureGroup(1).TrimStartAndEnd());
		OutValues.Add(Matcher.GetCaptureGroup(2).TrimStartAndEnd());
	}
	bSuccess = OutKeys.Num() > 0;
}
