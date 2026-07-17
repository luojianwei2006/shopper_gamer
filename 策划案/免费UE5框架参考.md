# 《合成小店长》免费 UE5 框架参考（轻量版）

> 说明：按需求，**工程结构不在此处搭建**，仅给免费可用的轻量底座与插件参考。
> 本次 v1.2 起**弃用 Lyra**（过重），改为「UE5 内置轻量模板 + 极简插件」。
> 配套：策划案_合成小店长.md（第 14 节 后端网络）、免费资源清单_Fab.md

---

## 一、首选底座：UE5 内置「Top Down」模板（极简）

- **来源**：新建项目 → 选 "Top Down" 模板（Starter Content 可选）。免费、官方维护、零冗余。
- **为什么是它，而不是 Lyra**
  - Lyra 自带 **GAS（技能系统）、模块化插件、多人联网范例**，对"单机休闲 + 服务器权威经济"是**过度设计**——理解和维护成本高，且要大量"做减法"。
  - Top Down 直接给：**俯视 3D 相机 + 点击交互 Pawn + 基础关卡**，正好匹配店铺俯视视角与"点供货 / 拖合成"交互。
  - 你真正需要的系统（UI / 输入 / 存档 / 网络）UE5 都已**内置**，按需取用，不背框架包袱。
- **等价备选**：想要更干净起点，选 **Blank（空模板）** 自行加相机/输入即可；Top Down 只是帮你省了相机和点击那点活。
- **需自行叠加的最小模块（全免费/内置）**
  - **Enhanced Input**（内置 UE5）：统一处理点击 / 拖拽 / 触控。
  - **UMG**（内置）：所有 UI（HUD / 弹窗 / 兑换商城），见《免费资源清单_Fab.md》的 UI 包。
  - **SaveGame**（内置）：本地存档 + 离线收益时间戳（策划案 §4.2）；服务器权威校正见 §14。
  - **DataAsset / DataTable**（内置）：承载经济数值，策划改表即调。

> **关键认知**：本作**不需要 GAS**。合成 / 增益 / 广告 buff 用简单的"状态枚举 + 计时器 + 属性变量"即可；强上 GAS 反而是负担。

---

## 二、TCP Socket 插件（直接服务"连接 Java 服务器"需求）

| 插件 | 许可 | UE5 | 特点 | 链接 |
|------|------|-----|------|------|
| **TCP-Unreal**（getnamo） | MIT | ✅ | ActorComponent 封装，风格接近 SocketIOClient，无依赖、跨平台好 | github.com/getnamo/tcp-ue4 |
| **UETcpSocketPlugin**（HyperminiDEv） | MIT | ✅(UE4/5) | 纯蓝图可用，每连接独立线程，事件 OnConnected/OnMessage/OnDisconnected | github.com/HyperminiDEv/UETcpSocketPlugin |
| **TCP Socket Plugin**（Fab） | 免费(GitHub) | ✅ | 蓝图为主，多线程，基础类型序列化 | Fab 搜 "TCP Socket Plugin" |
| **TCP-UDP Socket Helper**（uecandy） | FREE | 5.0–5.6 | TCP+UDP，客户端/服务端皆可，蓝图节点 | uecandy.com/product.php?id=7700 |

- **选型建议**：优先 **TCP-Unreal** 或 **UETcpSocketPlugin**（MIT、活跃、线程模型清晰）。它们正好对应 GDD §14.4 的"独立收线程 + 事件派发"模型，**省去手写 FSocket 线程**。
- **零插件依赖备选**：按 GDD §14.4 用 `FSocket` + `FRunnable` 自写约 200 行 C++ 模块（NetBridge），完全可控、包体最小。
- **重要**：插件通常只管"收发字节流"，**长度前缀分包 + Protobuf 解码仍需你在业务层做**（见 GDD §14.2 / §14.3）。插件 ≠ 完整协议栈。

---

## 三、其他可选内置模板（按需借鉴，非必需）

| 模板 | 可借鉴点 |
|------|----------|
| Third Person | Mixamo 人形 / NPC 动画接入（顾客、吉祥物） |
| Vehicle | 仅"配送 / 开车"玩法时参考物理 |

---

## 四、Lyra 何时才用（降级为备选）

仅当你后续要做**强联机 / 复杂技能 / 大量角色属性**时才考虑 Lyra 的 GAS 底座。当前版本**不采用**。

---

## 五、工程落地建议（给工程侧）

1. 用 **Top Down 模板**新建项目，删掉不需要的示例 mesh/材质（保持轻）。
2. 启用插件：选一个 TCP 插件（推荐 TCP-Unreal）+ Enhanced Input + 广告 SDK 插件。
3. 用 DataTable 承载经济数值，做到"策划改表、程序零改动"。
4. 移动端专项：关 Bloom/DOF、开纹理流送、模型 LOD、关光追、用 Mobile 渲染路径。
5. 数值走 `[PLACEHOLDER]` → 纸面模拟 → 原型试玩 → 投放调优。

> 以上为"轻量框架参考"，具体工程目录、类结构、蓝图片段由工程侧落地，本策划不越界。
