# shopper_gamer

基于 **Unreal Engine 5.6.1（C++）** 的购物/喇叭类联网游戏工程。核心是一套**可复用的 TCP 长链接 Socket 框架**，配合 protobuf 进行服务器通信，并内置两套玩法变体（策略对战 / 双摇杆射击）。

> 当前仓库重点沉淀：通用 Socket 长链接子系统（`UBaseSocketSubsystem`）+ 喇叭业务实现（`UShopperSocketSubsystem`），以及从实战踩坑中沉淀的帧格式约定与心跳稳定性方案。

---

## 环境要求

| 项 | 版本 / 说明 |
| --- | --- |
| Unreal Engine | **5.6.1**（`.uproject` 绑定 5.6 补丁版） |
| IDE | Rider（或 Visual Studio 2022） |
| 构建工具 | UnrealBuildTool（随引擎） |
| protobuf 运行时 | 本机通过 Homebrew 安装 **protobuf 33.2**（`/opt/homebrew/opt/protobuf`），与 `protoc` 版本一致 |
| 平台 | macOS（编辑器运行）；打包流程见下文「打包注意事项」 |

---

## 模块结构

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

---

## 核心架构：Socket 长链接框架

### 1. `UBaseSocketSubsystem`（抽象基类，框架层）

继承自 `UGameInstanceSubsystem`，**跨关卡不失效、不依赖 UWorld**，非常适合常驻长链接。基类负责一切「通用」逻辑，子类只填「业务」钩子：

- **TCP 连接 / 断线重连**：`Connect(ip, port)` 显式建链；`ShouldAutoConnect()` 为真则启动即连；断开后按 `GetReconnectInterval()`（默认 3s）自动重连。
- **后台读线程**：`FSocketReader`（继承 `FRunnable`）阻塞读 socket，按帧格式拆包后抛回**游戏线程**广播。
- **帧编解码**：参数化的 `FGameSocketFrameConfig`（`ProtocolBytes` / `LengthBytes` / 字节序 / `bLengthIncludesHeader` / `bUseFrameChecksum` / `PacketMaxSize`），兼容不同服务器约定。
- **心跳 / 重连驱动器**：`FSocketTicker`（继承 `FTickableGameObject`）由引擎**每帧稳定调用**，内部用 `FPlatformTime` 真实时钟算经过时间，再驱动 `TickHeartbeat` / `TickReconnect`。

#### 子类只需覆盖的钩子

| 钩子 | 作用 |
| --- | --- |
| `InitFrameConfig` | 帧格式配置（默认小端 / length 含帧头 / md5 末字节） |
| `ShouldAutoConnect` / `GetDefaultEndpoint` | 自动建链开关与回退地址 |
| `GetHeartbeatInterval` / `GetReconnectInterval` | 心跳 / 重连间隔（秒，0 = 不发心跳） |
| `BuildHeartbeatPayload` | **构造心跳包（协议号 + 负载）**——「发什么」由子类决定 |
| `HandlePacket` | 收到完整包后按协议号分流解析 |
| `GetLogTag` | 日志前缀（区分不同服务器） |

基类不依赖 protobuf —— 序列化由子类经 `ShopperProto` 模块完成，再调 `SendMessage(Protocol, Data)` 组帧发送。

### 2. `UShopperSocketSubsystem`（业务实现）

继承基类，只负责喇叭（Speaker）协议：

- 帧配置 / 连接配置 / 日志前缀覆盖；
- `HandlePacket` 按协议号分流，解析后通过**强类型动态多播委托**广播（蓝图 `Bind Event` 直接拿业务字段，无需接触 protobuf）；
- 提供 `SendSpeakerLogin` / `SendSpeakerPing` / 各类 `ParseSpeaker*Notify` 的蓝图可调用函数。

---

## 协议一览（喇叭）

| 协议号 | 名称 | 方向 | 说明 |
| --- | --- | --- | --- |
| 100 | `login_speaker_req` | C→S | 喇叭登录（token） |
| 101 | `ping_speaker` | C→S | **心跳包**（见下） |
| 103 | `balance_notify` | S→C | 余额变更通知 |
| 107 | `level_exp_notify` | S→C | 等级经验变更通知 |
| 108 | `new_mail_notify` | S→C | 新邮件通知（空消息） |
| 109 | `new_event_notify` | S→C | 新事件通知（flag: 1-Bingo 变更 / 2-新活动） |

> ⚠️ 服务器持续下发的 **协议 105** 当前客户端未实现（`收到未知协议=105`），且这些包 `md5 recv=0`（与客户端计算的 `calc` 不一致）。代码对 md5 不符采取「仍继续解析、不主动断连」，不影响连接，但建议后续与服务器对齐帧格式 / 补齐 105 处理。

---

## 心跳机制（含踩坑记录）

- **机制在基类**：`FSocketTicker::Tick` 每帧用真实时钟算 `Elapsed` 喂给 `TickHeartbeat`；`bConnected` 时累计 `HeartbeatAccum`，达 `GetHeartbeatInterval()` 即调 `SendHeartbeat()` → 经子类 `BuildHeartbeatPayload` 取 (协议号 101 + 负载) → `SendMessage` 组帧发出。
- **子类实现具体心跳内容**：`UShopperSocketSubsystem::BuildHeartbeatPayload` 构造 `ping_speaker`（协议号 101）。
- **间隔可配**：`ShopperSettings.HeartbeatInterval`（默认 **3 秒**，可在「项目设置 → Shopper 环境配置 → Socket」调整；也可由构建 flavor 的 `DefaultGame.ini` 覆盖）。
- **为什么用 `FTickableGameObject` 而非 `FTSTicker`**：实测在编辑器 / PIE 环境下 `FTSTicker::GetCoreTicker()` 出现过「只触发首帧、之后不再回调」，`HeartbeatAccum` 永远积累不到间隔值 → 心跳一次都不发。改用 `FTickableGameObject` 后由引擎每帧稳定触发，并用真实时钟计算经过时间，彻底解决。

调试时建议 Output Log 按分类 **`LogShopperSocket`** 过滤，可看到：驱动器注册、心跳发送、连接断开 / 重连、md5 不一致告警等。

---

## 帧格式约定（重要，易踩坑）

默认帧（可被 `InitFrameConfig` 覆盖）：

```
[protocol : ProtocolBytes 字节, 小端][length : LengthBytes 字节, 小端]
[+ data 变长][+ 1 字节 md5（bUseFrameChecksum 时，作为 data 块末字节，length 已含该字节）]
```

踩坑要点（已写入 `BaseSocketSubsystem.h` 注释）：

1. **帧头长度必须与发送端严格一致**：多读 1 字节会把 `data[0]` 吞进帧头，致 protobuf 整体错位 1 字节。
2. **length 是整帧长度（含帧头）**：真实 `data = length - 帧头长度 - (md5?1:0)`。
3. **md5 是 data 块末字节（非独立字节）**：先整块读入，再 `SetNum(Len-1)` 剔除，交给解析器。
4. 校验和算法默认 **XOR 取低 8 位**，子类可覆盖 `ComputeChecksum` 换成服务端算法。

---

## 构建与运行

1. **生成工程文件**（修改过 `.Build.cs` / `.uproject` 后必须）：在引擎或命令行对 `.uproject` 右键「Generate Xcode/Rider Project」，或在项目目录运行对应生成脚本。
2. **打开 Rider / VS**，等待 UBT 完成索引与 UHT 代码生成。
3. `Build → Rebuild Solution`（首次会编译 `ShopperProto` + `shoppergame` 两个模块）。
4. PIE 或打包运行；如需联调，运行一个本地 TCP 服务器监听 `ShopperSettings.SocketHost:SocketPort`（默认 `127.0.0.1:9000`）。

### 构建注意事项

- **`PCHUsage = NoSharedPCHs`**：`shoppergame` 模块使用私有 PCH。原因：该模块依赖 UMG/Slate/StateTree 等，UBT 在新版构建设置下默认会选 UnrealEd 共享 PCH，其内含 `ToolMenus`，会把宏 `CURRENT_FILE_ID` 残留为 ToolMenus；一旦 `ShopperSocketSubsystem.h` 嵌套包含带反射的 `BaseSocketSubsystem.h`，其 `GENERATED_BODY()` 会展开成不存在的 `FID_..._ToolMenus_h_23_GENERATED_BODY` → 报 *"a type specifier is required"*。改用私有 PCH 后污染源被切断。**不要改回共享 PCH。**
- **`#include "X.generated.h"` 位置**：必须放在本文件「所有其它 `#include` 之后、第一个反射声明（`UCLASS`/`USTRUCT`/`DECLARE_DYNAMIC_*`）之前」，而非文件末尾。错误位置会导致前面的反射宏拿到错误的 `CURRENT_FILE_ID`。
- **`ShopperProto` 独立编译选项**：该模块 `bUseRTTI = true`、`bEnableExceptions = true`，独占 protobuf 类型与 libprotobuf 链接，避免污染游戏主模块。

### 打包注意事项（上架前必读）

`ShopperProto.Build.cs` 当前直接链接本机 brew 的动态库：

```
/opt/homebrew/opt/protobuf/lib/libprotobuf.dylib
```

- 其 `install_name` 为绝对路径，仅**本机编辑器**可用；
- **打包 / CI / 上架**需改为随包分发：把 dylib 拷进 `.app/Contents/Frameworks` 并用 `install_name_tool -id @rpath/libprotobuf.dylib` 改 id，再给模块加 rpath；或改用静态库 `libprotobuf.a`（见工作日志）。protobuf / abseil 头路径同理建议改为项目内 `ThirdParty` 相对路径以保证跨机器一致。

---

## 环境配置

在「项目设置 → Shopper 环境配置」中维护（`UDeveloperSettings`，自动写入 `DefaultGame.ini`）：

- **环境**：开发 / 测试 / 正式，各配一套基地址（`DevBaseUrl` / `TestBaseUrl` / `ProdBaseUrl`）；
- **Socket**：服务器地址 / 端口 / 自动建链 / 心跳间隔 / 重连间隔 / 单包上限 / 是否带帧校验；
- **版本**：`AppVersion` 整数，随构建 flavor 注入或读配置。

生效优先级：`SHOPPER_FORCE_ENV_*` 编译宏 > 命令行 `-ShopperEnv=xxx` > `ActiveEnvironment` 配置。

---

## 待办 / 已知问题

- [ ] 协议 **105** 客户端未实现（服务器持续下发，当前 `收到未知协议=105`）。
- [ ] `bUseFrameChecksum` 与服务器 md5 算法对齐（服务器部分包 `recv=0`，疑似未带校验或算法不同）。
- [ ] 心跳间隔按需求确认为 3s 还是 15s（代码默认 3s，可在设置调整）。
- [ ] protobuf 动态库改为随包分发（上架前）。
