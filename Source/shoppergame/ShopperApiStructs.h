#pragma once

#include "CoreMinimal.h"
#include "HttpProtoStruct.h"   // 复用 FShopperJsonResponse（通用兜底响应）
#include "ShopperApiStructs.generated.h"

// ════════════════════════════════════════════════════════════════════════
// ShopperApi 全套接口的 请求/响应 结构 + 委托
//
// 设计约定（与 SendGetWallet 完全一致）：
//   1) 回调第二参数必须是「类型化响应结构体」，绝不返回 JSON 字符串。
//   2) data 为「对象」的接口 → 复用 FShopperJsonResponse（data 是 FJsonObjectWrapper，
//      蓝图用 UShopperJsonLibrary 按 key 取字段）。见下方 FOnShopperApiJson 共享委托。
//   3) data 为「数组」的接口 → 必须建强类型 TArray 结构（FJsonObjectWrapper 装不下数组），
//      见下方 6 个 List 响应结构。
//   4) 空 body 的 POST → 用 FShopperEmptyReq 占位（满足模板参数要求）。
//   5) POST 带 ?key=value → 由 ShopperApiBFL 内部构造 QueryParams 传入 Request。
// ════════════════════════════════════════════════════════════════════════

// ── 空请求体占位（POST 无 body 的接口统一用它）──
USTRUCT(BlueprintType)
struct FShopperEmptyReq
{
	GENERATED_BODY()
};

// ── 通用对象响应共享委托：所有 data 为对象的接口共用这一个 ──
DECLARE_DYNAMIC_DELEGATE_TwoParams(
	FOnShopperApiJson, bool, bSuccess, FShopperJsonResponse, Response);

// ════════════════════════════════════════════════════════════════════════
// 请求体结构（仅 JSON-body 的 POST 需要；query 参数接口无需结构体）
// 字段名严格对齐后端 JSON key（区分大小写）
// ════════════════════════════════════════════════════════════════════════

// 3.4 发送验证码 / 3.5 校验验证码
USTRUCT(BlueprintType)
struct FUserAuthCodeReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString mobile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString area_code;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32  type = 0;       // 用途：1注册 2登录 3提现 4改密
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString device;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString authcode;       // 3.5 用
};

// 3.6 注册
USTRUCT(BlueprintType)
struct FUserRegisterReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString mobile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString area_code;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString email;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString authcode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString firstname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString lastname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString birthday;
};

// 3.7 更新用户资料
USTRUCT(BlueprintType)
struct FUserUpdateReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString nickname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString headimg;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString headframe;
};

// 3.8 更新地理位置
USTRUCT(BlueprintType)
struct FUserLocationReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString lat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString lng;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString country;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString state;
};

// 3.9 更新设备 Token
USTRUCT(BlueprintType)
struct FUserDeviceTokenReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString device_token;
};

// 3.10 新手引导进度
USTRUCT(BlueprintType)
struct FUserBeginnerReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 step = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString remark;
};

// 3.11 获取游戏配置版本
USTRUCT(BlueprintType)
struct FUserConfigVersionReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 item = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 lv = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 minelevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 minetype = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 expeditionlevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 expeditiontype = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 systemopen = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 goldleague = 0;
};

// 3.12 获取最新游戏记录
USTRUCT(BlueprintType)
struct FUserNewestRecordReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") FString last_guid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User") int32 size = 20;
};

// 6.4 提交提现申请
USTRUCT(BlueprintType)
struct FWithdrawReq
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Withdraw") FString authcode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Withdraw") FString email;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Withdraw") int32 amount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Withdraw") FString orderids;   // "[1001,1002]" 字符串
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Withdraw") FString firstname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Withdraw") FString lastname;
};

// ════════════════════════════════════════════════════════════════════════
// 强类型「数组」响应（data 为数组，FShopperJsonResponse 装不下，必须强类型）
// ════════════════════════════════════════════════════════════════════════

// 7.1 邮件列表
USTRUCT(BlueprintType)
struct FMailItem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") int32 id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") int32 mailId = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") FString title;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") FString content;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") int32 type = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") int32 isRead = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") int32 isReceive = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") FString attachment;   // JSON 数组字符串
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") FString createTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") FString outdateTime;
};

USTRUCT(BlueprintType)
struct FMailListResponse
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") int32 code = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") FString msg;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") TArray<FMailItem> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail") FString token;
};
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMailListDone, bool, bSuccess, FMailListResponse, Response);

// 10.1 游戏模式列表
USTRUCT(BlueprintType)
struct FGameModeInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 maxPlayers = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 minLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 entryFee = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString icon;
};

USTRUCT(BlueprintType)
struct FGameModeListResponse
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 code = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString msg;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") TArray<FGameModeInfo> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString token;
};
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGameModeListDone, bool, bSuccess, FGameModeListResponse, Response);

// 10.3 / 10.4 我的历史对局 / 最近对局
USTRUCT(BlueprintType)
struct FGameRecord
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString guid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString userId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 rank = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 totalScore = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 result = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 hasReward = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 reward = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 rewardType = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString recordTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString match_name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 max_players = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 status = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString create_time;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString finish_time;
};

USTRUCT(BlueprintType)
struct FGameRecordListResponse
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") int32 code = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString msg;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") TArray<FGameRecord> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game") FString token;
};
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGameRecordListDone, bool, bSuccess, FGameRecordListResponse, Response);

// 10.2 排行榜
USTRUCT(BlueprintType)
struct FRankUser
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") int32 id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") FString nickname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") int32 level = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") FString headimg;
};

USTRUCT(BlueprintType)
struct FRankEntry
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") int32 rank = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") FRankUser user;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") int32 score = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") int32 winCount = 0;
};

USTRUCT(BlueprintType)
struct FGameRankResponse
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") int32 code = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") FString msg;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") TArray<FRankEntry> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Rank") FString token;
};
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGameRankDone, bool, bSuccess, FGameRankResponse, Response);

// 6.5 钱包变动日志
USTRUCT(BlueprintType)
struct FFundLog
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") FString userId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 type = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 direction = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 balance = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 bonus = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 diamond = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 afterBalance = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 afterBonus = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 afterDiamond = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") FString toGuid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") FString createTime;
};

USTRUCT(BlueprintType)
struct FWalletLogListResponse
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") int32 code = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") FString msg;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") TArray<FFundLog> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet|Log") FString token;
};
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnWalletLogListDone, bool, bSuccess, FWalletLogListResponse, Response);

// 11.1 挖宝记录
USTRUCT(BlueprintType)
struct FWabaoRecord
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") int32 id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") FString userId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") int32 chapter = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") int32 result = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") int32 score = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") FString createTime;
};

USTRUCT(BlueprintType)
struct FWabaoListResponse
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") int32 code = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") FString msg;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") TArray<FWabaoRecord> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wabao") FString token;
};
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnWabaoListDone, bool, bSuccess, FWabaoListResponse, Response);
