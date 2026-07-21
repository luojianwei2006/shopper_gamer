#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShopperHttpClient.h"       // 泛型 HTTP 客户端
#include "ShopperApiStructs.h"       // 全套请求/响应结构 + 委托
#include "ShopperApiBFL.generated.h"

/**
 * ShopperApiBFL —— 客户端可调用的全部后端 HTTP 接口封装
 *
 * 严格沿用 SendGetWallet 的写法：
 *   - 全部走 UShopperHttpClient 泛型客户端（GET/POST 自适应 + 自动反序列化）
 *   - 回调第二参数 = 类型化响应结构体（绝不返回 JSON 字符串）
 *       · data 为「对象」→ 复用 FShopperJsonResponse（委托 FOnShopperApiJson）
 *       · data 为「数组」→ 强类型 TArray 结构（FMailListResponse 等 6 个）
 *   - 需要登录的接口自动在 Authorization 头带 Token
 *   - Host 为空时自动取项目设置里的基地址
 *
 * 注意：3.4/3.5 文档标注为 form-urlencoded，本项目客户端统一以 JSON body 发送，
 *       若后端 Strictly 要求 form，需在 ShopperHttpClient 增加 form 编码分支（已在 README 记录）。
 */
UCLASS()
class SHOPPERGAME_API UShopperApiBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ───────────────────────── 3. 用户模块 User ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "发送验证码", WorldContext = "WorldContextObject"))
	static void SendUserSendAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserAuthCodeReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "校验验证码", WorldContext = "WorldContextObject"))
	static void SendUserCheckAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserAuthCodeReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "用户注册", WorldContext = "WorldContextObject"))
	static void SendUserRegister(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserRegisterReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "更新用户资料", WorldContext = "WorldContextObject"))
	static void SendUserUpdate(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserUpdateReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "更新地理位置", WorldContext = "WorldContextObject"))
	static void SendUserUpdateLocation(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserLocationReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "更新设备Token", WorldContext = "WorldContextObject"))
	static void SendUserUpdateDeviceToken(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserDeviceTokenReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "新手引导进度", WorldContext = "WorldContextObject"))
	static void SendUserBeginner(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserBeginnerReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "获取配置版本", WorldContext = "WorldContextObject"))
	static void SendUserGameConfigVersion(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserConfigVersionReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|User", meta = (DisplayName = "最新游戏记录", WorldContext = "WorldContextObject"))
	static void SendUserNewestRecord(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FUserNewestRecordReq& Proto, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 4. 商城 Shop ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Shop", meta = (DisplayName = "购买商品", WorldContext = "WorldContextObject"))
	static void SendShopBuy(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 ShopId, const FString& Method, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 5. 签到 Sign ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Sign", meta = (DisplayName = "每日签到", WorldContext = "WorldContextObject"))
	static void SendSignDaily(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 6. 提现 Withdraw ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Withdraw", meta = (DisplayName = "检查可否提现", WorldContext = "WorldContextObject"))
	static void SendWithdrawCanWithdraw(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 Amount, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Withdraw", meta = (DisplayName = "发送提现验证码", WorldContext = "WorldContextObject"))
	static void SendWithdrawSendAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Withdraw", meta = (DisplayName = "校验提现验证码", WorldContext = "WorldContextObject"))
	static void SendWithdrawCheckAuthCode(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FString& AuthCode, int32 Amount, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Withdraw", meta = (DisplayName = "提交提现申请", WorldContext = "WorldContextObject"))
	static void SendWithdrawSubmit(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FWithdrawReq& Proto, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Withdraw", meta = (DisplayName = "钱包变动日志", WorldContext = "WorldContextObject"))
	static void SendWithdrawWalletLogs(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 LastId, const FString& LastDate, int32 Size, const FOnWalletLogListDone& OnComplete);

	// ───────────────────────── 7. 邮件 Mail ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Mail", meta = (DisplayName = "邮件列表", WorldContext = "WorldContextObject"))
	static void SendMailList(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 LastId, int32 Size, const FOnMailListDone& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Mail", meta = (DisplayName = "标记邮件已读", WorldContext = "WorldContextObject"))
	static void SendMailRead(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 MailId, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Mail", meta = (DisplayName = "领取邮件奖励", WorldContext = "WorldContextObject"))
	static void SendMailReceive(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 MailId, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 8. 任务 Task ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Task", meta = (DisplayName = "获取任务列表", WorldContext = "WorldContextObject"))
	static void SendTaskList(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Task", meta = (DisplayName = "更新任务进度", WorldContext = "WorldContextObject"))
	static void SendTaskUpdateProgress(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 TaskId, int32 Progress, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Task", meta = (DisplayName = "领取任务奖励", WorldContext = "WorldContextObject"))
	static void SendTaskReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 TaskId, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 9. Banner/活动 ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Banner", meta = (DisplayName = "Banner列表", WorldContext = "WorldContextObject"))
	static void SendBannerList(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Banner", meta = (DisplayName = "领取免费礼包", WorldContext = "WorldContextObject"))
	static void SendBannerReceiveFreeGift(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 Entry, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Banner", meta = (DisplayName = "购买Banner商品", WorldContext = "WorldContextObject"))
	static void SendBannerBuy(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 BannerId, const FString& Method, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 10. 游戏记录 Game ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Game", meta = (DisplayName = "游戏模式列表", WorldContext = "WorldContextObject"))
	static void SendGameList(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnGameModeListDone& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Game", meta = (DisplayName = "获取排行榜", WorldContext = "WorldContextObject"))
	static void SendGameRank(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 GameId, const FOnGameRankDone& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Game", meta = (DisplayName = "我的对局记录", WorldContext = "WorldContextObject"))
	static void SendGameRecords(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 LastId, int32 Size, const FOnGameRecordListDone& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Game", meta = (DisplayName = "最近对局记录", WorldContext = "WorldContextObject"))
	static void SendGameNewestRecords(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 LastId, int32 Size, const FOnGameRecordListDone& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Game", meta = (DisplayName = "对局详情", WorldContext = "WorldContextObject"))
	static void SendGameRecordDetail(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 Id, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Game", meta = (DisplayName = "领取对局奖励", WorldContext = "WorldContextObject"))
	static void SendGameReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FString& Guid, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 11. 挖宝 Wabao ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Wabao", meta = (DisplayName = "挖宝记录", WorldContext = "WorldContextObject"))
	static void SendWabaoList(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnWabaoListDone& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Wabao", meta = (DisplayName = "开始新一关", WorldContext = "WorldContextObject"))
	static void SendWabaoStart(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 12. 远征 Expedition ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Expedition", meta = (DisplayName = "获取远征信息", WorldContext = "WorldContextObject"))
	static void SendExpeditionGetInfo(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Expedition", meta = (DisplayName = "攻打远征位置", WorldContext = "WorldContextObject"))
	static void SendExpeditionAttack(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 Pos, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 13. 宝箱 Chest ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Chest", meta = (DisplayName = "开始宝箱活动", WorldContext = "WorldContextObject"))
	static void SendChestStart(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 ActivityId, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Chest", meta = (DisplayName = "打开宝箱", WorldContext = "WorldContextObject"))
	static void SendChestOpen(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 ActivityId, int32 Pos, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Chest", meta = (DisplayName = "领取宝箱奖励", WorldContext = "WorldContextObject"))
	static void SendChestReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 ActivityId, const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 14. 广告任务 AdsTask ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|AdsTask", meta = (DisplayName = "广告任务列表", WorldContext = "WorldContextObject"))
	static void SendAdsTaskList(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|AdsTask", meta = (DisplayName = "领取广告任务奖励", WorldContext = "WorldContextObject"))
	static void SendAdsTaskReward(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 TaskId, const FOnShopperApiJson& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|AdsTask", meta = (DisplayName = "领取广告任务大奖", WorldContext = "WorldContextObject"))
	static void SendAdsTaskRewardAll(UObject* WorldContextObject, const FString& Host, const FString& Token,
		const FOnShopperApiJson& OnComplete);

	// ───────────────────────── 15. 海盗战斗 PirateBattle ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "ShopperAPI|Pirate", meta = (DisplayName = "报名海盗战斗", WorldContext = "WorldContextObject"))
	static void SendPirateMatch(UObject* WorldContextObject, const FString& Host, const FString& Token,
		int32 ActivityId, const FOnShopperApiJson& OnComplete);
};
