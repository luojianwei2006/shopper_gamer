# shopper_gamer

> 3D 低多边形休闲游戏《合成小店长》的项目仓库：合成 / 放置经营 · IAA（激励广告变现）· 边玩边赚。
> A project repo for **Synthesize Shop Manager (合成小店长)** — a 3D low-poly casual game: merge / idle-management · IAA (rewarded-ad monetization) · play-and-earn.

---

## 目录结构 / Repository Layout

| 路径 | 说明 / Description |
|------|-------------------|
| `策划案/` | 全部策划文档（GDD、经济表、界面原型、资源与框架参考） / All design docs (GDD, economy sheet, UI prototype, asset & framework references) |
| `Source/` | Unreal Engine 5 C++ 工程源码（游戏模块 `shoppergame` + protobuf 模块 `ShopperProto`） / UE5 C++ source (game module + protobuf module) |

进入 `策划案/` 查看完整策划案：`策划案/README.md`。
See `策划案/README.md` for the full design index.

---

## 技术栈 / Tech Stack

- **引擎 Engine**：Unreal Engine 5.6.1（移动端优先 / mobile-first）
- **平台 Platform**：iOS / Android
- **风格 Style**：Low-poly 卡通 / low-poly cartoon
- **后端 Backend**：HTTP REST（账号 / 取数 / 存档） + TCP 长连接（实时权威事件，Protobuf + 长度头），Java 端 Spring Boot + Netty

---

## 文档状态 / Doc Status

- 策划文档为**设计稿（draft）**，经济数值均为假设值，标 `[PLACEHOLDER]` 者需经纸面模拟与投放调优。
- Design docs are **design drafts**; economy numbers are hypotheses, values marked `[PLACEHOLDER]` need paper-sim and live tuning.
- **工程实现已启动**：`Source/` 下的 UE5 C++ 工程已入库，核心为一套可复用的 TCP 长链接 Socket 框架（见下「工程实现」），对齐技术栈中「TCP 长连接 + Protobuf + 长度头」的设计。
- **Engineering has started**: the UE5 C++ project under `Source/` is committed; the core is a reusable TCP long-connection Socket framework (see *Engineering* below), matching the "TCP long connection + Protobuf + length header" design.

---

## 本地预览原型 / Preview the Prototype Locally

界面原型是单个 HTML 文件，浏览器直接打开即可点：

The UI prototype is a single HTML file — open it directly in a browser:

```bash
# 直接打开
open 策划案/界面原型_合成小店长.html        # macOS
# 或起个本地服务器
python3 -m http.server 8080               # 然后访问 http://localhost:8080/策划案/界面原型_合成小店长.html
```

---

## 仓库规范 / Repo Conventions

- 策划文档统一放在 `策划案/`，便于版本管理与审阅。
- Design docs live under `策划案/` for version control and review.
- 提交信息用中文 `+` 英文摘要（如本仓库历史）。
- Commit messages use Chinese with an English summary.
- 建议开启 **main 分支保护**（禁止直推 / require PR review）——需在 GitHub 仓库 Settings 手动配置（本仓库当前未设置）。
- Recommended: enable **main branch protection** (no direct push / require PR review) in GitHub repo Settings (not yet configured here).

---

## License

设计文档版权归项目方所有，仅供内部评审与开发使用。
Design documents © project owner. For internal review and development only.

---
---

# 工程实现 / Engineering

> 以下为 `Source/` 中 UE5 C++ 工程的架构与协议说明。
> The following describes the architecture and protocols of the UE5 C++ project under `Source/`.

## 模块结构 / Module Layout

```
Source/
├── ShopperProto/            # protobuf 运行时模块（独立编译选项，独占 libprotobuf 符号）
│   ├── Public/Proto/        # *.proto + 生成的 *.pb.h/*.pb.cpp（gen_proto.sh 重新生成）
│   ├── ProtoSpeaker.h/.cpp  # 喇叭协议收发包装（FSpeakerUser 等结构 + 序列化函数）
│   └── ShopperProto.Build.cs
└── shoppergame/             # 游戏主模块
    ├── BaseSocketSubsystem.*     # 【框架】通用 TCP 长链接基类（抽象）
    ├── ShopperSocketSubsystem.*  # 【业务】喇叭协议实现（继承基类）
    ├── ShopperSettings.*         # 项目设置：环境地址 / Socket 配置 / 心跳间隔
    ├── HttpProtoBFL.*            # HTTP + protobuf 蓝图函数库
    ├── DeviceSaveGame.*          # 设备存档
    ├── USessionDataSubsystem.*   # 会话数据子系统
    ├── Variant_Strategy/         # 玩法变体 1：策略对战
    ├── Variant_TwinStick/        # 玩法变体 2：双摇杆射击
    └── shoppergame.Build.cs
```

> **架构边界（C++ / 蓝图）**：Socket 底层、帧编解码、心跳、重连、读线程等**全部在 C++**；蓝图仅通过 `UFUNCTION(BlueprintCallable)` / `BlueprintAssignable` 委托消费（登录、收包回调、心跳包构造参数等），不触碰 protobuf 类型。

## 核心架构：Socket 长链接框架 / Socket Framework

### `UBaseSocketSubsystem`（抽象基类，框架层）

继承自 `UGameInstanceSubsystem`，**跨关卡不失效、不依赖 UWorld**，适合常驻长链接。基类负责通用逻辑，子类只填业务钩子：

- **TCP 连接 / 断线重连**：`Connect(ip, port)` 显式建链；`ShouldAutoConnect()` 为真则启动即连；断开后按 `GetReconnectInterval()`（默认 3s）自动重连。
- **后台读线程**：`FSocketReader`（`FRunnable`）阻塞读 socket，按帧格式拆包后抛回**游戏线程**广播。
- **帧编解码**：参数化 `FGameSocketFrameConfig`（`ProtocolBytes` / `LengthBytes` / 字节序 / `bLengthIncludesHeader` / `bUseFrameChecksum` / `PacketMaxSize`），兼容不同服务器约定。
- **心跳 / 重连驱动器**：`FSocketTicker`（`FTickableGameObject`）由引擎**每帧稳定调用**，内部用 `FPlatformTime` 真实时钟算经过时间，再驱动 `TickHeartbeat` / `TickReconnect`。

子类需覆盖的钩子：`InitFrameConfig` / `ShouldAutoConnect`+`GetDefaultEndpoint` / `GetHeartbeatInterval`+`GetReconnectInterval` / `BuildHeartbeatPayload`（构造心跳包）/ `HandlePacket`（按协议号分流）/ `GetLogTag`。

基类不依赖 protobuf —— 序列化由子类经 `ShopperProto` 模块完成，再调 `SendMessage(Protocol, Data)` 组帧发送。

### `UShopperSocketSubsystem`（业务实现）

继承基类，负责喇叭（Speaker）协议：帧配置 / 连接配置 / 日志前缀覆盖；`HandlePacket` 按协议号分流，解析后通过**强类型动态多播委托**广播（蓝图 `Bind Event` 直接拿业务字段，无需接触 protobuf）；提供 `SendSpeakerLogin` / `SendSpeakerPing` / 各类 `ParseSpeaker*Notify` 蓝图可调用函数。

## 协议一览（喇叭）/ Protocol Table (Speaker)

| 协议号 | 名称 | 方向 | 说明 |
| --- | --- | --- | --- |
| 100 | `login_speaker_req` | C→S | 喇叭登录（token） |
| 101 | `ping_speaker` | C→S | **心跳包**（见下） |
| 103 | `balance_notify` | S→C | 余额变更通知 |
| 107 | `level_exp_notify` | S→C | 等级经验变更通知 |
| 108 | `new_mail_notify` | S→C | 新邮件通知（空消息） |
| 109 | `new_event_notify` | S→C | 新事件通知（flag: 1-Bingo 变更 / 2-新活动） |

> ⚠️ 服务器持续下发的 **协议 105** 当前客户端未实现（`收到未知协议=105`），且这些包 `md5 recv=0`（与客户端计算的 `calc` 不一致）。代码对 md5 不符采取「仍继续解析、不主动断连」，不影响连接，但建议后续与服务器对齐帧格式 / 补齐 105 处理。

## 心跳机制 / Heartbeat

- **机制在基类**：`FSocketTicker::Tick` 每帧用真实时钟算 `Elapsed` 喂给 `TickHeartbeat`；`bConnected` 时累计 `HeartbeatAccum`，达 `GetHeartbeatInterval()` 即调 `SendHeartbeat()` → 经子类 `BuildHeartbeatPayload` 取 (协议号 101 + 负载) → `SendMessage` 组帧发出。
- **子类实现具体心跳内容**：`UShopperSocketSubsystem::BuildHeartbeatPayload` 构造 `ping_speaker`（协议号 101）。
- **间隔可配**：`ShopperSettings.HeartbeatInterval`（默认 **3 秒**，可在「项目设置 → Shopper 环境配置 → Socket」调整；也可由构建 flavor 的 `DefaultGame.ini` 覆盖）。
- **为何用 `FTickableGameObject` 而非 `FTSTicker`**：实测编辑器 / PIE 下 `FTSTicker::GetCoreTicker()` 出现过「只触发首帧、之后不再回调」，`HeartbeatAccum` 永远积累不到间隔值 → 心跳一次都不发。改用 `FTickableGameObject` 后由引擎每帧稳定触发，并用真实时钟计算经过时间，彻底解决。

调试时 Output Log 按分类 **`LogShopperSocket`** 过滤，可见：驱动器注册、心跳发送、连接断开 / 重连、md5 不一致告警等。

## 帧格式约定 / Frame Format（易踩坑）

默认帧（可被 `InitFrameConfig` 覆盖）：

```
[protocol : ProtocolBytes 字节, 小端][length : LengthBytes 字节, 小端]
[+ data 变长][+ 1 字节 md5（bUseFrameChecksum 时，作为 data 块末字节，length 已含该字节）]
```

1. **帧头长度必须与发送端严格一致**：多读 1 字节会把 `data[0]` 吞进帧头，致 protobuf 整体错位 1 字节。
2. **length 是整帧长度（含帧头）**：真实 `data = length - 帧头长度 - (md5?1:0)`。
3. **md5 是 data 块末字节（非独立字节）**：先整块读入，再 `SetNum(Len-1)` 剔除，交给解析器。
4. 校验和算法默认 **XOR 取低 8 位**，子类可覆盖 `ComputeChecksum` 换成服务端算法。

## 构建与运行 / Build & Run

1. 修改过 `.Build.cs` / `.uproject` 后必须**重新生成工程文件**（对 `.uproject` 右键 Generate，或运行对应生成脚本）。
2. 用 Rider / VS 打开，等待 UBT 索引与 UHT 代码生成。
3. `Build → Rebuild Solution`（首次编译 `ShopperProto` + `shoppergame` 两模块）。
4. PIE 或打包运行；联调时运行本地 TCP 服务器监听 `ShopperSettings.SocketHost:SocketPort`（默认 `127.0.0.1:9000`）。

### 构建注意事项 / Build Notes

- **`PCHUsage = NoSharedPCHs`**：`shoppergame` 用私有 PCH。原因：该模块依赖 UMG/Slate/StateTree，UBT 新版默认选 UnrealEd 共享 PCH（内含 `ToolMenus`），会把宏 `CURRENT_FILE_ID` 残留为 ToolMenus；一旦 `ShopperSocketSubsystem.h` 嵌套包含带反射的 `BaseSocketSubsystem.h`，其 `GENERATED_BODY()` 展开成不存在的 `FID_..._ToolMenus_h_23_GENERATED_BODY` → 报 *"a type specifier is required"*。改私有 PCH 后污染源切断。**不要改回共享 PCH。**
- **`#include "X.generated.h"` 位置**：必须放在本文件「所有其它 `#include` 之后、第一个反射声明之前」，而非文件末尾。错误位置会导致前面的反射宏拿到错误的 `CURRENT_FILE_ID`。
- **`ShopperProto` 独立编译选项**：`bUseRTTI = true`、`bEnableExceptions = true`，独占 protobuf 类型与 libprotobuf 链接，避免污染游戏主模块。

### 打包注意事项（上架前）/ Packaging

`ShopperProto.Build.cs` 当前直接链接本机 brew 动态库 `/opt/homebrew/opt/protobuf/lib/libprotobuf.dylib`（其 `install_name` 为绝对路径，仅本机编辑器可用）。打包 / CI / 上架需改为随包分发：把 dylib 拷进 `.app/Contents/Frameworks` 并用 `install_name_tool -id @rpath/libprotobuf.dylib` 改 id，再给模块加 rpath；或改用静态库 `libprotobuf.a`。protobuf / abseil 头路径同理建议改为项目内 `ThirdParty` 相对路径以保证跨机器一致。

## 环境配置 / Environment

在「项目设置 → Shopper 环境配置」（`UDeveloperSettings`，写入 `DefaultGame.ini`）维护：环境（开发 / 测试 / 正式各一套基地址）、Socket（地址 / 端口 / 自动建链 / 心跳间隔 / 重连间隔 / 单包上限 / 是否带帧校验）、版本号。生效优先级：`SHOPPER_FORCE_ENV_*` 宏 > 命令行 `-ShopperEnv=xxx` > `ActiveEnvironment` 配置。

## 启动加载闸门与运行时热更 / Boot Loading Gate & Runtime Hotfix

> 启动期资源加载闸门：先异步预载热更包、UI Widget、界面图片与重资源，进度聚合后由蓝图 `OpenLevel` 进入目标关；可选挂载热更 pak 并应用 socket 配置覆盖层（运行时替换，不改 `ShopperSettings`）。
> Boot-time loading gate: async-preload hotfix pak, UI widgets, UI textures and heavy assets, then Blueprint `OpenLevel` into the target level; optionally mount a hotfix pak and apply a socket-config override layer (runtime-only, `ShopperSettings` untouched).

### 关键类 / Key Classes

| 类 | 类型 | 职责 |
| --- | --- | --- |
| `UHotReloadManager` | `UGameInstanceSubsystem` | 挂载 / 卸载热更 pak（`MountPak` / `UnmountPak`），挂载后广播 `OnHotfixPakMounted` |
| `UShopperHotfixConfig` | `UPrimaryDataAsset` | 热更配置覆盖层（host / port / 心跳 / 帧格式）；随 pak 下发，运行时覆盖默认 `ShopperSettings` |
| `ULoadingPlanAsset` | `UPrimaryDataAsset` | 加载计划：热更路径 / 配置 / WidgetRegistry / UI 分组 / 重资源 / 目标关 |
| `ULoadingSubsystem` | `UGameInstanceSubsystem` | 加载闸门：按 Phase 异步加载 + 聚合进度广播 |
| `UWidgetManager` | `UGameInstanceSubsystem` | 按 `WidgetRegistry` 异步预载 / 重解析 Widget 软类 |
| `UWidgetRegistry` | `UPrimaryDataAsset` | Widget 登记表：`TMap<FName, TSoftClassPtr<UUserWidget>>`，key = 逻辑名（数量不限） |

### 加载相位 / Loading Phases

`ULoadingSubsystem` 按权重顺序推进（权重之和 = 1.0），全部走 `FStreamableManager::RequestAsyncLoad` / pak 挂载异步路径，**不阻塞游戏线程**：

| Phase | 权重 | 内容 |
| --- | --- | --- |
| 0 Hotfix | 0.15 | 挂载热更 pak（路径空则跳过）+ 应用 `HotfixConfig`（空则回退默认 `ShopperSettings`） |
| 1 Widgets | 0.20 | 预载 `WidgetRegistry` 登记的 Widget 软类（已加载则跳过） |
| 2 UI | 0.25 | 按 `UILoadGroups` 顺序逐组预载界面图片（每组可显示独立状态文案） |
| 3 Assets | 0.40 | 预载通用重资源 + 关卡专用资源（`LevelPreloadAssets`） |

进度由 C++ 聚合后通过 `OnLoadingProgress(float Percent, FText Status)` 广播；全部完成后通过 `OnLoadingComplete(FName TargetLevel)` 广播目标关名。

### C++ / 蓝图边界 / Boundary

- **C++（引擎层）**：异步加载、pak 挂载、进度计算与聚合、相位调度。
- **蓝图（设计师层）**：`OnLoadingProgress` → 刷新进度条 + 状态文案；`OnLoadingComplete` → `Open Level`；`WBP_Loading` 仅含 ProgressBar + Text，**不得硬引用预载的 Widget 类**（否则无法瞬时显示）。

### 接线步骤 / Wiring

1. 建轻量 **Loading 关卡**（如 `/Game/Loading/Loading`），设为 `Project Settings → Maps & Modes → Game Default Map`。
2. 建 `WBP_Loading`（ProgressBar + Text），暴露 `SetProgress(float)` / `SetStatus(FText)`。
3. Loading 关卡 `BeginPlay`：
   - `Get Game Instance → GetSubsystem(Loading Subsystem)`；
   - **先** `Bind` `OnLoadingProgress` / `OnLoadingComplete`，`CreateWidget(WBP_Loading)` 并 `AddToViewport`；
   - **最后** 调 `StartLoading(你的 LoadingPlan)`。
4. `OnLoadingComplete` 里 `Open Level(TargetLevel)`。

> ⚠️ **Bind 必须在 `StartLoading` 之前**：`StartLoading` 内部同步发出首波进度广播，且若 LoadingPlan 的 `UILoadGroups` / `PreloadAssets` 皆空，Phase 会同步级联到 `OnLoadingComplete`——顺序反了会导致首波（甚至整轮）广播丢失、进度条不动或跳过加载直接跳关。

### 资产配置 / Asset Setup

- **WidgetRegistry**：用 *Miscellaneous → Data Asset* 建 `UWidgetRegistry` **实例**（不是蓝图类），在 `Widgets` 映射按逻辑名登记任意数量 Widget 软类（key 与 `PlayerController.StartupWidgetKey`、`WidgetManager::CreateWidgetOfType(Key)` 对应）。
- **LoadingPlan**：同法建 `ULoadingPlanAsset` 实例，填 Hotfix 路径 / 配置 / WidgetRegistry 引用 / UI 分组 / 重资源 / 目标关；可按目标关建多份（`LP_Login` / `LP_Menu` / `LP_Game`）。
- 路径类字段（`HotfixPakPath` 等）留空 = 跳过对应步骤，纯本地开发模式下完全合法。

> ⚠️ `TSoftObjectPtr<UWidgetRegistry>` / `TSoftObjectPtr<UShopperHotfixConfig>` 字段要的是**数据资产实例**，不是蓝图类资产——在资产选择器里建 Data Asset 而非 Blueprint Class，否则 picker 过滤不到。

## UI 动效体系 / UI FX System

> 按钮点击脉冲（放大→缩回）、面板弹出回弹（过冲→回落）、点击/弹出音效，以及动画完成回调。两种接入方式：**继承基类**（新控件）或**外挂函数库**（改造已有控件），二者表现一致、底层都用引擎全局单 `FTicker` 驱动补间（UE5.6 已移除 `UUserWidget::SetTickEnabled`，故不用 widget Tick）。
> Button click pulse (scale up/down), panel popup bounce (overshoot/recoil), click/popup SFX, and an animation-complete callback. Two integration styles: **inherit a base class** (new widgets) or **attach via a function library** (retrofit existing widgets). Both share the same look and are driven by one module-level `FTicker`.

### 关键类 / Key Classes

| 类 | 类型 | 职责 |
| --- | --- | --- |
| `UWB_ButtonBase` | `UUserWidget`（`Abstract`） | 通用按钮基类：内置点击脉冲 + 音效 + `OnClicked` 广播；蓝图派生一次 `WBP_Button` 复用 |
| `UWP_PanelBase` | `UUserWidget`（`Abstract`） | 通用面板基类：构造时播放弹出回弹 + 音效；`Play On Construct` 可关 |
| `UShopperUIFx` | `UBlueprintFunctionLibrary` | 外挂式动效库：`BindClickFx` 一行挂接、`PlayButtonPulse` / `PlayPanelBounce` / `PlayClickSound` 手动调用 |

### 效果与驱动 / Effects & Driver

- **脉冲（Pulse）**：`Scale = 1 + (Peak-1)·sin(α·π)`，从 1.0 → 峰值（默认 1.15）→ 1.0 平滑一圈；默认时长 0.18s。
- **回弹（Bounce）**：Back 缓动 `1 + C3·(x-1)³ + C1·(x-1)²`，从 `StartScale`（默认 0.0，即「从无到有」）弹到 1.0，自带过冲回落；默认时长 0.38s，`Overshoot` 控制回弹强度（默认 1.6）。
- **音效**：`UGameplayStatics::PlaySound2D`（无需世界音源）；脉冲/点击音效在动画**开始时**播放，`PlayButtonPulse` 支持 `SoundVolume` 音量倍率。
- **完成回调**：`PlayButtonPulse` / `PlayPanelBounce` 的 `OnComplete` 在动画**结束时**触发，业务逻辑（开店 / 关界面 / 跳转）写在此回调里。
- **驱动**：所有补间由模块级单 `FTicker`（`FTSTicker::GetCoreTicker().AddTicker(...)`）统一推进；无活动动画且无绑定器时自动 `Reset()` 注销 ticker，**零常驻开销**。`BindClickFx` 为每个按钮建 `UShopperButtonFxBinder` 并 `AddToRoot` 保活，按钮销毁时由 ticker 自动 `RemoveFromRoot` 清理，无悬空委托。

### 接入方式一：继承基类（新控件）/ Approach A — Inherit Base Class

1. Content Browser 右键 → Widget Blueprint → Parent Class 选 **`WB_ButtonBase`**，命名 `WBP_Button`。
2. 画布内放一个 **Button**（必须命名 `Button`，`BindWidget` 按名绑定）与一个可选 **TextBlock**（命名 `Label` 显示标题）。
3. Details → `UI|FX` 设 `Click Sound`（留空不播）；可调 `Click Peak Scale` / `Click Duration` / `Scale Self`。
4. 父级菜单拖入 `WBP_Button` 实例，像普通 Button 一样绑 **`OnClicked`**（基类内部已 `Broadcast`）写业务逻辑。
5. 面板同理：建 `WBP_PanelBase` 派生控件，设 `Bounce Sound`，构造即播回弹；`Play Bounce In` 节点可在再次显示时重播。

> 注：`WB_ButtonBase` / `WP_PanelBase` 为纯 `UUserWidget` **无可视化树**，必须派生蓝图才能摆进 UMG；仅建**一个** `WBP_Button` 复用即可，无需每个按钮一个蓝图。

### 接入方式二：外挂函数库（改造已有控件）/ Approach B — Attach via Library

不改父类、不替换控件，在按钮所在 Widget 的 `Construct` / `OnInitialized` 里调一次即可：

- **`BindClickFx(Button, Sound)`** —— 一行给任意已有 `UButton` 挂上「点击脉冲 + 音效」，之后该按钮每次点击都自带效果。
- **`PlayButtonPulse(Target, OnComplete, PeakScale=1.15, Duration=0.18, Sound=nullptr, SoundVolume=1.0)`** —— 手动在已有按钮的 `OnClicked` 末尾调，传 `OnComplete` 回调写响应逻辑。
- **`PlayPanelBounce(Target, OnComplete, Duration=0.38, StartScale=0.0, Overshoot=1.6)`** —— 手动在面板构造时调。
- **`PlayClickSound(WorldContextObject, Sound, Volume=1.0)`** —— 单独播一次点击音效。

### ⚠️ 已知约束 / Notes

- **缩放锚点**：脉冲 / 回弹走 `SetRenderScale`，务必将控件 / 按钮的 **Render Transform Pivot 设为 0.5/0.5**，否则从角落弹出而非居中放大。
- **UE5.6 API**：`UUserWidget::SetTickEnabled` / `IsTickEnabled` 已被移除（tick 开关改为私有 `TickFrequency`），本系统改用 `FTicker`，跨版本零依赖。
- **委托参数顺序**：`PlayButtonPulse` / `PlayPanelBounce` 的 `OnComplete` 排在 `Target` 之后、所有带默认值参数之前（C++ 默认参数后置规则 + UHT 不支持委托构造式默认值）。
- **`OnClicked` 签名**：UE5.6 的 `UButton::OnClicked` 为零参委托（`FOnButtonClickedEvent`），绑定回调不得带参。

