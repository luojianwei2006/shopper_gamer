#include "ShopperTextLibrary.h"
#include "Misc/StringConv.h"   // FTCHARToUTF8

// 省略号 U+2026；用码点构造，避免源码编码问题
static const TCHAR EllipsisChar = static_cast<TCHAR>(0x2026);

FString UShopperTextLibrary::ClampText(const FString& Text, int32 MaxChars, bool bAddEllipsis)
{
	if (MaxChars <= 0)
	{
		return FString();
	}
	if (Text.Len() <= MaxChars)
	{
		return Text;
	}

	// 预留省略号位置（bAddEllipsis 时实际保留 MaxChars-1 字）
	const int32 Keep = bAddEllipsis ? FMath::Max(0, MaxChars - 1) : MaxChars;
	FString Result = Text.Left(Keep);
	if (bAddEllipsis)
	{
		Result.AppendChar(EllipsisChar);
	}
	return Result;
}

FString UShopperTextLibrary::ClampTextByBytes(const FString& Text, int32 MaxBytes, bool bAddEllipsis)
{
	if (MaxBytes <= 0)
	{
		return FString();
	}

	// 省略号本身占 3 字节（UTF-8），预留出来
	const int32 Reserve = bAddEllipsis ? FMath::Max(0, MaxBytes - 3) : MaxBytes;
	FString Result = Text;
	// 逐字符回退，直到实际字节数 <= Reserve
	while (Result.Len() > 0 && FTCHARToUTF8(*Result).Length() > Reserve)
	{
		Result = Result.Left(Result.Len() - 1);
	}

	// 还能塞得下省略号才追加
	if (bAddEllipsis && (FTCHARToUTF8(*Result).Length() + 3) <= MaxBytes)
	{
		Result.AppendChar(EllipsisChar);
	}
	return Result;
}
