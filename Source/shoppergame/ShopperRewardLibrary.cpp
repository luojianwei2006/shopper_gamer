#include "ShopperRewardLibrary.h"
#include "Internationalization/Regex.h"   // FRegexPattern / FRegexMatcher (UE5.6 路径，旧版为 HAL/Regex.h)
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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

void UShopperRewardLibrary::ParseMailAttachment(const FString& AttachmentJson, TArray<FShopReward>& OutRewards, bool& bSuccess)
{
	OutRewards.Empty();
	bSuccess = true;   // 无附件也视为成功（空列表）

	if (AttachmentJson.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(AttachmentJson);
	if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
	{
		bSuccess = false;   // 不是合法 JSON 数组 → 解析失败
		return;
	}

	for (const TSharedPtr<FJsonValue>& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
		if (!Val.IsValid() || !Val->TryGetObject(ObjPtr) || !ObjPtr->IsValid())
		{
			continue;
		}
		const TSharedPtr<FJsonObject>& Obj = *ObjPtr;

		FShopReward R;
		// type 可能是字符串("1003")或数字 → 统一转 int
		if (Obj->HasTypedField<EJson::String>(TEXT("type")))
		{
			R.ItemId = FCString::Atoi(*Obj->GetStringField(TEXT("type")));
		}
		else
		{
			R.ItemId = (int32)Obj->GetNumberField(TEXT("type"));
		}
		R.Count = (int32)Obj->GetNumberField(TEXT("value"));
		OutRewards.Add(R);
	}

	bSuccess = OutRewards.Num() > 0;
}

void UShopperRewardLibrary::ParseMailAttachmentKV(const FString& Raw, TArray<FShopReward>& OutRewards, bool& bSuccess)
{
	OutRewards.Empty();
	bSuccess = true;   // 无附件视为成功（空列表）

	if (Raw.IsEmpty())
	{
		return;
	}

	// 先按分号拆成若干 "key,value" 段（剔除空段，容错 "a,1; ;b,2" 这类脏数据）
	TArray<FString> Pairs;
	Raw.ParseIntoArray(Pairs, TEXT(";"), /*bCullEmpty=*/true);
	for (const FString& Pair : Pairs)
	{
		TArray<FString> Parts;
		// 再按逗号拆成 key / value（剔除空段，容错 "1002, 0" 这种带空格写法）
		if (Pair.ParseIntoArray(Parts, TEXT(","), /*bCullEmpty=*/true) >= 2)
		{
			FShopReward R;
			R.ItemId = FCString::Atoi(*Parts[0]);
			R.Count  = FCString::Atoi(*Parts[1]);
			OutRewards.Add(R);
		}
	}
	bSuccess = OutRewards.Num() > 0;
}
