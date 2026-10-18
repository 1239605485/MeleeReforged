# 木剑三连斩 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** 发布可由 KernelLoader 加载的 Android ARM64 木剑动作与翡翠特效测试版。

实施记录：已完成本地主机两组测试、NDK r28 ARM64 编译及安装包清单检查；设备实测和远程仓库/CI 尚未完成。基于底层源码核对，挂钩改用保留 MethodInfo 的 Dobby 原生包装；生命周期固定避免当前加载器无卸载否决接口的问题。运行时资源未就绪时动作、命中和绘制均回退原版。

**Architecture:** 游戏适配层负责所有原生对象及挂钩；纯逻辑攻击状态机输出统一姿态。木剑配置、命中处理和渲染彼此独立，消费相同攻击轨迹。

**Tech Stack:** C++17、C ABI、CMake、Android NDK、TEFKernel/PatchLib、KernelLoader、GitHub Actions；主机 CTest 测试纯逻辑。

**Spec:** ../specs/2026-10-18-melee-reforged-design.md

## Global Constraints

- 首个构建目标 Android arm64-v8a。
- 第一版只接管原版木剑。
- 联机环境在验证前不启用自定义接管。
- 伤害数值、暴击和击退仍由原版计算。
- 禁止同时执行一套原版伤害和一套自定义伤害。
- 只在已接管木剑的持握绘制范围内替换。
- 不上传游戏导出源码、IL2CPP dumps 或整包原版素材。
- 构建成功与游戏实测通过分别报告，单机通过不代表联机可用。

## Review Focus

1. 切换武器后即使旧攻击尚未结束，也不能残留自定义命中和刀光；由 Task 2 测试。
2. 玩家槽位复用、死亡及退出世界不能继承上一角色的连段；由 Task 2 和 Task 3 测试。
3. 同一玩家被多次绘制、影子绘制或暂停不能推进攻击和重复生成粒子；由 Task 5 测试。
4. 高攻速与掉帧不能跳过整个伤害窗口或在多次采样中重复伤害；由 Task 4 测试。
5. 安装中途失败或重复初始化不能留下半套挂钩、释放其他模组回调；由 Task 1 和 Task 3 测试。

## 源码证据和依赖边界

本地只读参考位于 refs/kernel/TEFKernel-KernelLoader-main、refs/tef/TEFKernel-main、refs/game 和 refs/dumps，不复制进公开仓库。

已确认 WoodenSword = 24。dump 中候选动作入口为 Player.ItemCheck_ApplyUseStyle(float, Item, Rectangle)，命中入口为 Player.ItemCheck_GetMeleeHitbox(Item, Rectangle, out bool, out Rectangle)，持握绘制入口为 PlayerDrawLayers.DrawPlayer_27_HeldItem(PlayerDrawSet)。PlayerDrawSet 是引用类型 class，不得当作内联结构体解引用。候选入口必须先完成 Task 1 校验才允许安装。

两个 dump.cs 的 SHA-256 均为 6f98faced459ead3c5711d231c34f30650fc5b12bb8ec59705920138fe210d20，只计一份证据。PE 导出源码中的方法地址与此 dump 不完全一致，禁止混用 RVA。

KernelLoader 所带 patch_method_signature_t 与 TEFKernel 提供版本存在字段差异；所有 PatchLib 头文件、导入实现及实际运行时必须来自同一兼容版本。不得仅拼接一个头文件字段绕过编译。Task 1 若无法验证兼容版本，停止运行时接入，明确所需版本；纯逻辑部分可继续验证，但不当作可玩版本交付。

### Task 1: 接口与 ABI 接入验证

**Files:** Create src/interop/runtime_api.hpp、runtime_api.cpp、hook_set.hpp、hook_set.cpp；tests/interop_contract_test.cpp；CMakeLists.txt。

**Interfaces:** `RuntimeApi::resolve() -> ResolveReport`；`ResolveReport { bool compatible; FailureCode failure; }`；`HookSet::install(const HookRequest&) -> bool`；`HookSet::reset() -> bool`。RuntimeApi 持有经校验的方法/字段句柄，不向 combat 层暴露指针。HookRequest 只引用本项目已核对的候选入口及回调。

- [ ] 写失败测试：缺少方法、错误参数类型、错误静态/实例属性均返回 incompatible；第三个安装失败时撤回前两个；重复 reset 不影响外部 hook。
- [ ] 运行 `cmake -S . -B build-host -DMR_BUILD_TESTS=ON`、`cmake --build build-host`、`ctest --test-dir build-host --output-on-failure`，确认新增测试因缺失实现失败。
- [ ] 对照 TEFKernel Android method_hook.c、method.c、签名结构及类型转换源码核对参数表示、结构体按值/out 封送及卸载语义；按方法名、参数类型和参数数目解析，不写入 dump RVA。
- [ ] 实现 RuntimeApi 与 HookSet，使前置回调默认 false（继续原方法），只有经验证的局部替换才返回 true。捕获安装结果并仅撤回自身句柄。初始化状态为 Disabled/Ready/Failed，渲染资源和伤害接入未就绪时不启用动作替换。
- [ ] 运行上述 CTest 并检查主机测试通过；Android 专属 ABI 仍须交叉编译与设备探针验证，不以 mock 结果代替。
- [ ] 提交 `feat: validate runtime API and scoped hook lifecycle`。

### Task 2: 独立木剑攻击内核

**Files:** Create src/combat/attack_types.hpp、attack_controller.hpp/.cpp、trajectory.hpp/.cpp；src/weapons/weapon_registry.hpp/.cpp、wooden_sword.cpp；tests/attack_controller_test.cpp、trajectory_test.cpp。

**Interfaces:** `Vec2 {float x,y;}`；`AttackInput {uint64_t tick, worldGeneration, playerGeneration; int itemId, direction; float gravity, cycleSeconds, dtSeconds; Vec2 grip; bool attacking, alive, inWorld, singlePlayer;}`；`AttackPose {uint64_t attackId; int combo; Phase phase; float progress, angle; Vec2 grip, tip; bool damaging;}`；`AttackController::advance(const AttackInput&) -> AttackPose`；`AttackController::reset() -> void`；`sampleBlade(const AttackPose&, float length) -> BladeSegment`；`findWeapon(int itemId) -> const WeaponProfile*`。

- [ ] 写失败测试：24 以外 ID 不注册；连段为 0/1/2 循环；0.6 秒以内保留连段、超过后归零；切物品、死亡、worldGeneration/playerGeneration 改变、联机时立即失效；同 tick 二次 advance 不推进。
- [ ] 写轨迹测试：左右镜像及重力翻转对称；正常周期与半周期归一化姿态相同；无效或非有限 dt/cycle 不产生 NaN；角度连续、剑柄固定于 grip。
- [ ] 运行 CTest 确认失败，再实现状态机和纯轨迹函数。三段时间比例依次为 20/45/35、15/45/40、30/35/35；只在 Swing 输出 damaging。攻击方向在单次攻击内锁定；木剑长度由验证后的实际纹理握持距离提供，不把贴图宽度直接当剑长。
- [ ] 运行 CTest 确认通过，提交 `feat: add isolated wooden sword combo controller`。

### Task 3: 玩家状态与真实持剑动作

**Files:** Create src/bootstrap/mod_entry.cpp；src/interop/player_adapter.hpp/.cpp、action_hooks.cpp；src/combat/player_states.hpp/.cpp；tests/player_states_test.cpp；Manifest.json、Info.json。

**Interfaces:** `PlayerStates::get(PlayerKey) -> AttackController&`；`PlayerStates::clearWorld(uint64_t generation) -> void`；`PlayerAdapter::read(PlayerHandle, uint64_t tick) -> AttackInput`；`PlayerAdapter::applyPose(PlayerHandle,const AttackPose&) -> bool`。PlayerHandle 仅允许在当前有效回调内使用；PlayerKey 为槽位与玩家/世界代数，不是长期裸指针。

- [ ] 写失败测试：同一槽位不同代数得到新状态；clearWorld 清空全部状态；重复 init 不重复注册；动作应用失败时不抑制原版绘制和伤害。
- [ ] 运行 CTest 确认失败；通过 Task 1 已验证动作入口接入，读取原版攻速后的周期，统一处理 mountOffset、握持位置及重力，避免重复翻转。手臂调用也经 RuntimeApi 校验，未验证不直接调用枚举/值类型接口。
- [ ] 对已接管木剑替换持剑位置/角度及必要手臂姿态；其他武器立刻走原版。切武器与世界退出清空拖尾。只有 Task 4、5 均就绪时才将运行时状态置为 Ready。
- [ ] 跑 CTest；设备探针需观察左右、移动跳跃和切武器下动作位置，不得仅凭编译通过确认。
- [ ] 提交 `feat: bind per-player poses to wooden sword actions`。

### Task 4: 轨迹命中与伤害唯一性

**Files:** Create src/collision/blade_sweep.hpp/.cpp、damage_window.hpp/.cpp；src/interop/melee_hooks.cpp；tests/blade_sweep_test.cpp、damage_window_test.cpp。

**Interfaces:** `sweepIntersects(const BladeSegment& previous,const BladeSegment& current,const Rect& target) -> bool`；`DamageWindow::shouldProcess(uint64_t attackId,int npcId,uint64_t npcGeneration) -> bool`；`DamageWindow::markProcessed(...) -> void`。几何层无游戏调用，伤害层仅接入原版结算入口。

- [ ] 写失败测试：扫掠经过目标为 true，扇形外但在整体包围矩形内的目标为 false；蓄力与收势不伤害；单个 attackId 同目标最多一次，下一次攻击可再命中；NPC 槽位复用不继承去重结果。
- [ ] 写失败测试：高速攻击跨越 Swing 时仍得到一次有效扫掠；掉帧按阶段边界分段，不将整段跳跃连成错误大扇形。非连续世界状态不得使用上个世界的位置。
- [ ] 运行 CTest 确认失败，实现 blade sweep 和按攻击事件去重；只有原版实际确认命中时标记 processed，未命中不得抢占。
- [ ] 对照真实调用链验证原版免疫、暴击、击退与阻挡规则；窄相位筛选必须位于原版伤害之前。只改 GetMeleeHitbox 的矩形无法完成精确窄相位时，不宣称完成；停止该接入方案，提出最小边界变更，不能另写直扣血流程冒充。
- [ ] 运行 CTest；设备用单目标和多目标对比原版伤害，检查每挥次数及剑尖位置。提交 `feat: align wooden sword hit detection with swing trajectory`。

### Task 5: 分层翡翠刀光与命中反馈

**Files:** Create src/render/trail_buffer.hpp/.cpp、effect_events.hpp/.cpp、swing_renderer.hpp/.cpp；src/interop/draw_adapter.hpp/.cpp、held_item_hooks.cpp；src/config/quality.hpp；tests/render_budget_test.cpp、effect_events_test.cpp。

**Interfaces:** `TrailBuffer::push(uint64_t tick,const AttackPose&) -> void`；`EffectEvents::emitOnce(EffectKey) -> bool`；`SwingRenderer::build(const AttackPose&,const TrailBuffer&,Quality) -> DrawCommands`；`DrawAdapter::submit(const DrawCommands&,DrawContext) -> bool`。DrawCommands 为预分配绘制数据；渲染函数不改变 AttackController。

- [ ] 写失败测试：同 tick 多次绘制不增加轨迹点或粒子；影子绘制不生成事件；单次命中只发一次闪光；退出世界清空轨迹；过期点在 0.12 秒/第三段 0.18 秒后移除。
- [ ] 写预算测试：三档轨迹点 <=12/20/28，单玩家活动粒子 <=24/48/80，全局 <=320；满额仍保留武器实体绘制命令和攻击逻辑。
- [ ] 运行 CTest 确认失败。先检查木剑原版贴图及玩家绘制层，正确定位实体和握持原点，再实现背景柔光、轨迹残影、实体、前景火花顺序；第一段窄弧、第二段反向双残影、第三段宽月牙与叶片。
- [ ] 优先使用玩家绘制缓存/已有状态的提交路径，避免擅自全局 Begin/End。确需修改绘制状态时用作用域恢复；默认关闭屏幕震动。不能只隐藏原版武器然后留下一团特效。
- [ ] 运行 CTest；设备核对暗处实体、左右镜像、护甲/坐骑遮挡、缩放及帧率。提交 `feat: add layered emerald trails and hit feedback`。

### Task 6: 公开仓库、Android 构建与测试包

**Files:** Create .github/workflows/android-arm64.yml；cmake/android-arm64.cmake；tools/package.py；LICENSE；.gitignore。修改 CMakeLists.txt、Manifest.json、Info.json 仅补充构建与打包数据。

**Interfaces:** `python3 tools/package.py --build-dir build-android --output dist/MeleeReforged-0.1.0-Android-arm64.zip`。Manifest 与导出规范以提供的 KernelLoader 源码为准，不自行编造加载字段。

- [ ] 检查依赖授权和实际 KernelLoader 包装规则，只收录模组所需且可再分发的文件；明确 C++17 与固定 NDK 版本并锁定依赖提交。
- [ ] 在用户已授权的 GitHub 账户下创建公开 MeleeReforged 仓库；若名字被占用，选择 MeleeReforged-KernelLoader，记录实际 URL。不上传 refs/、dump、原版素材包或凭据。
- [ ] CI 顺序为主机 CTest → Android arm64-v8a Release 构建 → ELF/导出与 ZIP 清单检查 → 上传测试包。构建目录和 dist 不纳入 git。
- [ ] 用 `llvm-readelf -h` 检查 Machine 为 AArch64；用 `llvm-nm -D` 检查 create_kernel_mod；检查 ZIP 中无旧版二进制、无多余 ABI、路径与元数据符合加载器。
- [ ] 运行全部主机检查与 CI，审阅相关 diff；成功后给出仓库、Actions 运行和测试包链接。未有设备证据前只称“编译通过、待手机测试”。
- [ ] 用户设备按 spec 验收：加载/进图、三连段、命中、切物品、暗处实体、其他武器、重进世界和性能。任何失败保留日志并回到对应任务，先撤掉失败实验再修复。
- [ ] 提交 `build: package Android ARM64 test release`，发布 0.1.0 测试版，不标注正式稳定版。

## 执行及中断规则

推荐由当前会话直接逐任务执行，减少跨任务交接；任何子代理或独立审查只在用户选择该执行方式或明确授权后启动。Task 1 接口/ABI 验证失败不继续挂接游戏；Task 4/5 未完成不启用半套动作。挂钩只能在游戏设备上验证的部分明确列为待测，不将其写成已通过。
