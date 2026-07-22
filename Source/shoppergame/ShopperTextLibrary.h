#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShopperTextLibrary.generated.h"

// ───────────────────────────────────────────────────────────
// 文本裁剪工具库（输出显示场景）
//   - ClampText        按"字符数"裁（中文汉字按 1 字计，UTF-16 安全）
//   - ClampTextByBytes 按"UTF-8 字节数"裁（适配后端 VARCHAR 字节限制）
// ───────────────────────────────────────────────────────────
UCLASS()
class SHOPPERGAME_API UShopperTextLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 按字符数裁剪：Text 超过 MaxChars 时取前 MaxChars 个字符
	//   bAddEllipsis=true 时预留 1 个字符位给省略号（实际保留 MaxChars-1 字 + …）
	//   中英文混排均按可视字符数计算（FString 为 UTF-16，Len() 即字符数）
	UFUNCTION(BlueprintCallable, Category = "Shop|Text",
		meta = (DisplayName = "Clamp Text"))
	static FString ClampText(const FString& Text, int32 MaxChars, bool bAddEllipsis = true);

	// 按 UTF-8 字节数裁剪：适配后端 VARCHAR(N) 这类"字节限制"字段
	//   中文每字占 3 字节，省略号占 3 字节；超限时逐字符回退直到字节数 <= MaxBytes
	//   例如后端限 50 字节，30 个汉字(90B)会被裁到约 15~16 字 + …
	UFUNCTION(BlueprintCallable, Category = "Shop|Text",
		meta = (DisplayName = "Clamp Text By Bytes"))
	static FString ClampTextByBytes(const FString& Text, int32 MaxBytes, bool bAddEllipsis = true);
};
