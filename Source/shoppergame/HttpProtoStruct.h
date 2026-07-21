#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"   // FJsonObjectWrapper：可承载任意 JSON 结构（UE5.6 路径无 Dom/ 前缀）
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "HttpProtoStruct.generated.h"

USTRUCT(BlueprintType)
struct FGuestLoginProto
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString nickname;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString headimg;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString device;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString lat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString lng;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString country;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString state;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    int32 ver = 0;          // 截图里是绿色 int → int32，JSON 里输出数字而非字符串

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString device_token;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString adid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString adjust_id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString os;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
    FString channel;
};

// ───────────────────────────────────────────────────────────
// login_by_token 请求体：用已保存的游客 token 直接续期登录
// 字段直连后端，与 GuestLogin 共用 device；token 取本地存档的游客 token
// ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FLoginByTokenProto
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString account;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString token;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString device;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString lat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString lng;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString country;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString state;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	int32 ver = 0;          // 截图里是绿色 int → int32，JSON 里输出数字而非字符串

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString device_token;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString adid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString adjust_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString os;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString channel;
};

// ───────────────────────────────────────────────────────────
// Wallet：对应 /user/get_wallet 返回的 data
// 字段类型说明（按常见玩法划分，若后端返回小数/整数不同请调整）：
//   balance / bonus 为金额类，可能含小数 → float
//   star   / diamond 为计数类，通常为整数 → int32
// 若后端实际全是整数，把 balance/bonus 改成 int32 即可；反之若全是小数，把 star/diamond 改成 float
// ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWallet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	int32 balance = 0.f;        // 余额

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	int32 star = 0;             // 星星

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	int32 bonus = 0.f;          // 奖励金

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	int32 diamond = 0;          // 钻石
};

// ───────────────────────────────────────────────────────────
// 钱包接口最外层响应（与登录响应同构：code / msg / data / token）
// ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWalletResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	int32 code = 0;             // 业务码，200 成功

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	FString msg;                // "success"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	FWallet data;               // 钱包数据

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	FString token;              // JWT（通常与登录同源，可能刷新）
};

// ───────────────────────────────────────────────────────────
// 钱包请求体：/user/get_wallet 仅用 Authorization 头鉴权，body 留空
// 这里定义占位空结构，仅为了满足泛型客户端 Request<TRequest,TResponse>
// 的模板参数要求（POST 时序列化出来是 "{}"，对后端无影响）
// ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FGetWalletProto
{
	GENERATED_BODY()
};

// ───────────────────────────────────────────────────────────
// 以下登录响应结构原位于 LoginResponseStruct.h，已并入本文件统一管理
// ───────────────────────────────────────────────────────────

// speaker：语音/客服长连接节点（后续接语音服务用）
USTRUCT(BlueprintType)
struct FSpeaker
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	int32 port = 0;          // 端口，如 8701

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	FString ip;              // 服务 IP，如 "192.168.10.8"
};

// user：登录返回的用户信息主体
USTRUCT(BlueprintType)
struct FLoginUser
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString avatar;          // null → 空串

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int64 create_date = 0;   // 毫秒时间戳，> int32 范围，必须 int64

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int64 last_login_date = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int64 register_date = 0; // null → 0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int64 referred_date = 0; // null → 0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	bool delflag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int32 id = 0;            // 用户 ID，如 100671

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString account;         // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString mobile;          // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString area_code;       // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString email;           // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString nickname;        // "ue101"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString headimg;         // "head_101"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString headframe;       // "kuang_101"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString firstname;       // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString lastname;        // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString birthday;        // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int32 level = 0;         // 等级

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int32 exp = 0;           // 经验

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int32 status = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int32 test = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString session_id;      // JWT，会话标识

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString device;          // 设备码，与请求里的 device 对应（示例里正是我们生成的码）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int32 beginner = 0;      // 新手奖励等

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	int32 ver = 0;           // 协议版本号

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString lat;             // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString lng;             // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString country;         // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString state;           // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString device_token;    // ""

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString adid;            // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString adjust_id;       // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString os;              // ""

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString channel;         // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString referrer;        // null

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|User")
	FString refer_code;      // "2sjtg8"，邀请码
};

// data：response 的业务数据包裹层（user / speaker 都在这里）
USTRUCT(BlueprintType)
struct FLoginData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	FString survey_url;      // 问卷链接（含 token 查询参数）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	int32 production = 0;    // 是否生产环境

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	int32 timezone = 0;      // 时区，如 8

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	int32 canWithdraw = 0;   // 能否提现

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	FSpeaker speaker;        // 语音/客服节点

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	TArray<FString> pay_method; // 支付方式数组，如 ["paypal","stripe"]

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	FString chat_link;       // 客服聊天链接

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	FLoginUser user;         // 用户主体

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	int32 isTester = 0;      // 是否测试员

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login|Data")
	int32 canRecharge = 0;   // 能否充值
};

// 最外层响应
USTRUCT(BlueprintType)
struct FLoginResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	int32 code = 0;          // 业务码，200 成功

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString msg;             // "success"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FLoginData data;         // 业务数据

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString token;           // JWT，全局鉴权用（与 user.session_id 同源）
};

// 单个商店条目（字段名严格对齐后端 JSON key，区分大小写）
USTRUCT(BlueprintType)
struct FShopItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 type = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 money_type = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 deposit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 sale_type = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString reward1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString reward2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 order_num = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 status = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString targetType;   // 后端 Java 遗留字段，原样存字符串
};

// 单条奖励（reward1/reward2 这类 "1002,0" 串解析后的强类型结构）
// ItemId = 物品/道具 id，Count = 数量
USTRUCT(BlueprintType)
struct FShopReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Reward")
	int32 ItemId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Reward")
	int32 Count = 0;
};

// shop/list
USTRUCT(BlueprintType)
struct FGetShopListProto
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FShopListResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 code = 0;          // 业务码，200 成功

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString msg;             // "success"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TArray<FShopItem> data;  // 业务数据：商店条目数组（后端 data 为数组）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString token;           // JWT，全局鉴权用（与 user.session_id 同源）
};

// ───────────────────────────────────────────────────────────
// 通用默认响应（兜底用）：与登录/钱包响应同构 code/msg/token，
// 但 data 是 FJsonObjectWrapper —— 可原样承载后端返回的任何 JSON 结构，
// 不需要为每个接口都建一套强类型结构。
// 用法：新接口不想建 FxxxResponse 时，TResponse 直接填 FShopperJsonResponse，
// 拿到的 data 用 UShopperJsonLibrary 的工具函数按 key 取字段。
// ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FShopperJsonResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Http|Default")
	int32 code = 0;             // 业务码，200 成功

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Http|Default")
	FString msg;                // "success"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Http|Default")
	FJsonObjectWrapper data;    // 任意 JSON 结构（原样保留后端 data 里的全部字段）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Http|Default")
	FString token;              // JWT（如有）
};