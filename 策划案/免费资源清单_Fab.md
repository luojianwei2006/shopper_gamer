# 《合成小店长》免费资源清单（Fab / 公开可商用）

> 目的：本项目 UI、3D 模型、场景、特效均优先采用**免费且可商用**的资源，降低启动成本。
> 说明：以下资源为 2026-07-14 联网核实到的真实在架免费包；Fab 内容会变动，使用前请再次确认授权范围（多数 Fab 免费包为"Engine + 个人/商业使用"，但**禁止转售原始资源**）。
> 配套：策划案_合成小店长.md、免费UE5框架参考.md

---

## 一、UI 资源（移动端休闲风，2D 纹理/PSD/PNG）

| 资源名 | 类型 | 适配度 | 链接 | 备注 |
|--------|------|--------|------|------|
| **Orange Casual GUI** | 休闲移动端 UI | ⭐⭐⭐ | fab.com/zh-cn/listings/15d29bc5-e5c5-486d-baf2-bba69ec58fe5 | 含关卡完成/失败/设置/社交弹窗 PSD+PNG，含 coin/heart/star 图标，1920×1080。**首推** |
| **Flat Cartoon GUI** | 卡通扁平 UI | ⭐⭐⭐ | fab.com/listings/8f152103-c8a1-4d56-a659-dec6c00d5070 | 3000+ 纹理，7 色系按钮/图标/进度条/弹窗，PSD+PNG，UE4.14–5.7 全兼容 |
| **Basic Casual GUI** | 基础休闲 UI | ⭐⭐ | fab.com/listings/6bc0f24f-c47e-49b6-9c75-a9ef71c3f67c | 轻量，含标题创建图层，适合快速原型 |
| **GUI6600（7600 纹理）** | 巨型 UI 图素库 | ⭐⭐ | fab.com/es-es/listings/ab385aa8-f93c-42d8-9f42-09c75576b790 | 2448 按钮/100 图标/coin/diamond/gem/star/ trophy 等，适合做图标兜底 |
| **Game UI Pack (Medieval)** | 像素风 UI | ⭐ | fab.com/listings/f356c5ae-880a-447b-a8e6-69da04e51159 | 仅作风格参考，与本作卡通调性不符 |

**UI 落地建议**
- 主界面 HUD、订单弹窗、兑换商城、广告激励按钮，优先用 **Orange Casual GUI** 改色（橙→品牌主色）。
- 图标不足时，用 **GUI6600** 补足 coin/diamond/gem/star 等通用图标。
- 所有 UI 在移动端需做 9-slice 与多分辨率适配（UE UMG 的 DPI 缩放）。

---

## 二、3D 模型资源（低多边形 / 卡通，移动端友好）

| 资源名 | 内容 | 适配度 | 链接 | 备注 |
|--------|------|--------|------|------|
| **Low Poly Ultimate Pack** | 3000+ 模型，含 Food×55、Drinks×38、Dishes×32、Electronics×67、Furniture×194、People×220、Buildings×236、Vehicles×70 等 | ⭐⭐⭐ | fab.com/listings/6e216f65-3f4a-40b8-96da-3285ac32ed01 | **核心包**：合成物件/货架/店铺/顾客/NPC 全覆盖；UE5.3–5.6；免费月更；Mixamo 人形支持 |
| **Low Poly Scenario Pack** | 约 90 模型：Forest / Wild West / Snowy / Old Town 四套场景件 | ⭐⭐⭐ | fab.com/zh-cn/listings/a064f927-d8c8-42d5-b12a-479eb8c0503c | 店铺 exterior / 街道 / 装饰，部分带 LOD，移动端友好 |
| **Free Cute Casual Dog** | 卡通狗（静态网格，含 PBR 贴图） | ⭐⭐ | fab.com/listings/993b0c6a-37de-41d8-b6c8-4bbe5022669f | 店铺吉祥物/宠物玩法候选；可自绑骨 |
| **Low Poly Car Pack** | 7 辆卡通车（SUV/巴士/冰淇淋车/皮卡/跑车） | ⭐⭐ | fab.com/listings/dd21ee87-2704-4693-adc6-a25bb2c13aaf | 送货/配送玩法、街道点缀 |
| **Free Pack - Lowpoly People** | 8 个超低面人形（CC 授权） | ⭐⭐ | sketchfab.com/3d-models/free-pack-lowpoly-people-6c13a4d1d04943e487313f377b4f3548 | 顾客群；CC Attribution 需署名 |

**模型落地建议**
- 合成物件（果蔬→烘焙→饮品链）从 **Low Poly Ultimate Pack** 的 Food/Drinks/Dishes 取用，按阶数替换模型+缩放+颜色区分。
- 店铺 interior 用 Furniture + Buildings 子集；exterior/街道用 Scenario Pack 的 Old Town。
- 顾客用 People 包 + Mixamo 简单走动动画；吉祥物用 Casual Dog。
- 移动端必须开 **LOD + 纹理流送 + 静态网格可移动性（Mobility=Static）**，目标中端机 30fps。

### 二-B、45° 室内卡通场景 / 角色（本轮新增 · 2026-07-14 核实）

> 配套 GDD §15.2 / §17.2（真 3D · 45° 室内 · 卡通 + 黑色阴影光照揭示）。下列为针对"室内 45° 俯视店铺"新核实的免费/可商用包。

**室内场景（Cozy Cartoon 风，模块化墙地，适配 §17.2 灯光扩张）**

| 资源名 | 内容 | 适配度 | 链接 | 备注 |
|--------|------|--------|------|------|
| **Interiors FREE – Cozy Cartoon Pack** | 40 个卡通室内模型（杯/壶/盆栽/床/沙发/柜等），单材质一致风 | ⭐⭐⭐ | fab.com/es-es/listings/ef194997-eb8c-4fc0-9d11-dad28d00eaa2 | 免费含 UE 格式；暖卡通调性，与本项目吻合 |
| **Low Poly Cartoon House Interiors** | 447 个低多边形室内模型（墙 79/地 28/厨卫/家具/灯/窗帘），单材质 | ⭐⭐⭐ | fab.com/ja/listings/d8b7c23d-760c-4ef5-b29d-5b327cb48a6a | 447 mesh，模块化墙地，UE4.24–5.7；做 45° 店铺 interior 主包 |
| **Low Poly Cozy Interior** | 611 个低多边形室内模型（墙/门/家具/厨房/卫浴/**食物**），含 Demo 场景 | ⭐⭐⭐ | fab.com/listings/e29b584f-b27a-49ec-ad92-c5f0d81e49cc | 611 mesh，自带食物模型可当合成物件 |
| **Low Poly Cartoon Profession Rooms** | 20 种主题房间（**Bakery/酒馆/Pizzeria** 等）585 资产 | ⭐⭐⭐ | fab.com/zh-cn/listings/3be0cc6f-df42-4646-9745-024ca945e474 | 含面包房/披萨店主题，正好做店铺 interior |

**卡通角色（店长 / 店员 / 厨师 / 顾客，需可绑定动画）**

| 资源名 | 内容 | 适配度 | 链接 | 备注 |
|--------|------|--------|------|------|
| **Anime Low Poly Characters – Starter Pack Vol.3** | 5 个低多边形卡通人物（成人男女/男孩女孩），绑 UE5 标准骨架，20 动画 | ⭐⭐⭐ | unrealengine.com/marketplace/en-US/product/adult-cartoon-style-character-3d-lowpoly | 绑 Epic 骨架，兼容 UE5 动画重定向；像素风贴图卡通感 |
| **Free Stylized Cartoon Male (Rigged)** | 1 个绑骨卡通男性，拓扑干净 | ⭐⭐ | fab.com/it/listings/ad8b2d2a-616b-4d5b-a3d8-599dda32137e | 免费，需自补贴图/服装；适合店长原型 |
| **17 Toon Characters** | 17 个多样卡通角色，自带简易人形骨架 | ⭐⭐ | fab.com/listings/fce7d314-5f9e-491b-bd87-d1ca181cd088 | 多样性格，适合顾客群 |
| **Free Pack – Lowpoly People** | 8 个超低面人形（CC 署名） | ⭐⭐ | sketchfab.com/3d-models/free-pack-lowpoly-people-6c13a4d1d04943e487313f377b4f3548 | 顾客群兜底（见 §二） |
| **Mixamo（Adobe，免费）** | 海量绑定动画（走/站/挥手/跑） | ⭐⭐⭐ | mixamo.com | 免费账号；静态角色套 Mixamo 动画即可走动（对照 §15.2/§16 顾客移动） |

**45° 室内落地建议**
- 店铺 interior 主包用 **Low Poly Cartoon House Interiors** 或 **Low Poly Cozy Interior**（模块化墙地，便于按 §17.2"灯光扩张"逐步实例化新区域）。
- 角色：用 **Anime Low Poly Characters** / **17 Toon Characters** 做店长/店员/厨师/顾客，套 **Mixamo** 动画实现"入口→座位→出口"往前移动；顾客怒气用表情贴图 / 冒火粒子（对照 §16 怒气）。
- 合成物件：优先 **Low Poly Cozy Interior** 自带食物模型，或 §二 的 **Low Poly Ultimate Pack** Food/Drinks。
- **黑色阴影 / 光照揭示（§17.2）**：用 UE 点光源 / 聚光灯界定可玩区，外围压暗到近黑；Expansion 时移动光源 + 实例化新区域资产，省首包与首帧。

---

## 三、补充免费来源（非 Fab，但强烈推荐）

| 来源 | 用途 | 链接 |
|------|------|------|
| **Kenney.nl** | 海量免费低多边形/像素资源（CC0，无署名要求），含 food、city、ui | kenney.nl |
| **Poly Haven** | 免费 HDRi / 纹理 / 模型（CC0），做环境光照与材质 | polyhaven.com |
| **Quixel Bridge（Megascans）** | UE 内免费接入，写实/PBR 材质与扫描模型，做地面/墙面细节（注意包体） | 通过 UE 编辑器 Fab/Quixel 插件 |

---

## 四、资源采购与授权提醒
1. **禁止转售原始资产**：Fab 免费包大多允许"在你的游戏中使用"，但不得原样转卖。
2. **署名要求**：Sketchfab CC 资源需按 license 署名；Kenney CC0 无需署名但建议致谢。
3. **风格统一**：Low Poly Ultimate Pack 为单材质多实例风格， safest 是全程用它保持视觉一致；混用 Kenney 时注意色板统一。
4. **包体控制**：移动端 IAA 游戏包体建议 < 200MB（APK/AAB），导入后删未用资产、开 Pak 分包。
