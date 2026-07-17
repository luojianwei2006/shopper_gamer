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
