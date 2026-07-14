# shopper_gamer

> 3D 低多边形休闲游戏《合成小店长》的项目仓库：合成 / 放置经营 · IAA（激励广告变现）· 边玩边赚。
> A project repo for **Synthesize Shop Manager (合成小店长)** — a 3D low-poly casual game: merge / idle-management · IAA (rewarded-ad monetization) · play-and-earn.

---

## 目录结构 / Repository Layout

| 路径 | 说明 / Description |
|------|-------------------|
| `策划案/` | 全部策划文档（GDD、经济表、界面原型、资源与框架参考） / All design docs (GDD, economy sheet, UI prototype, asset & framework references) |

进入 `策划案/` 查看完整策划案：`策划案/README.md`。
See `策划案/README.md` for the full design index.

---

## 技术栈 / Tech Stack

- **引擎 Engine**：Unreal Engine 5（移动端优先 / mobile-first）
- **平台 Platform**：iOS / Android
- **风格 Style**：Low-poly 卡通 / low-poly cartoon
- **后端 Backend**：HTTP REST（账号 / 取数 / 存档） + TCP 长连接（实时权威事件，Protobuf + 长度头），Java 端 Spring Boot + Netty

---

## 文档状态 / Doc Status

- 全部为**设计稿（draft）**，尚未进入工程实现。
- All files are **design drafts**; no engineering implementation yet.
- 经济数值均为假设值，标 `[PLACEHOLDER]` 者需经纸面模拟与投放调优。
- Economy numbers are hypotheses; values marked `[PLACEHOLDER]` need paper-sim and live tuning.

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
