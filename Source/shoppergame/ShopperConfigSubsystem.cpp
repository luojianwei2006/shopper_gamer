#include "ShopperConfigSubsystem.h"
#include "UObject/UObjectGlobals.h"

// 约定路径：若 MoneyTypeTable 软引用未配置，则回退到此路径的 DataTable 资产
// 在编辑器中把货币类型表建到 /Game/Data/DT_MoneyType 即可，无需改 C++
static constexpr const TCHAR* DefaultMoneyTablePath =
    TEXT("/Game/Data/DT_MoneyType.DT_MoneyType");

void UShopperConfigSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // 启动时解析软引用，确保后续查询无需每次重新加载
    ResolveTable();
}

UDataTable* UShopperConfigSubsystem::ResolveTable() const
{
    if (UDataTable* Table = MoneyTypeTable.LoadSynchronous())
    {
        return Table;
    }
    // 软引用未配时，回退到约定路径的资产
    return LoadObject<UDataTable>(nullptr, DefaultMoneyTablePath);
}

void UShopperConfigSubsystem::FindMoneyTypeByCode(
    int32 MoneyCode, FMoneyTypeRow& OutRow) const
{
    OutRow = FMoneyTypeRow();

    if (UDataTable* Table = ResolveTable())
    {
        // 行名即 money_type 数字转成的 FName（如 "0" / "1" / "2"）
        if (const FMoneyTypeRow* Row =
            Table->FindRow<FMoneyTypeRow>(FName(*FString::FromInt(MoneyCode)), TEXT("MoneyType")))
        {
            OutRow = *Row;
        }
    }
}

TArray<FMoneyTypeRow> UShopperConfigSubsystem::GetAllMoneyTypes() const
{
    TArray<FMoneyTypeRow> Result;
    if (UDataTable* Table = ResolveTable())
    {
        Table->ForeachRow<FMoneyTypeRow>(TEXT("MoneyType"),
            [&Result](const FName&, const FMoneyTypeRow& Row)
            {
                Result.Add(Row);
            });
    }
    return Result;
}
