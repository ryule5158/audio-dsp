# AUDIO_DSP 项目工作记录

> 状态：持续维护（append-first）  
> 建立时间：2026-08-30T14:26:32+08:00  
> 仓库：`C:\Users\LENOVO\Desktop\AUDIO_DSP`  
> 远端：`git@github.com:ryule5158/audio-dsp.git`  
> 当前主分支：`main`

## 0. 强制执行规则

这是本项目的权威过程记录。后续主代理、子代理或人工接手者必须执行以下规则：

1. **任何实质动作前，先完整审阅本文件的最新版本。** 实质动作包括但不限于：运行终端命令、访问或修改虚拟机、下载/克隆依赖、编辑/删除文件、生成工程、构建、仿真、部署、烧录、测试、Git 暂存/提交/推送。
2. 审阅后，先确认本次动作与“用户要求”“当前状态”“未决项”和“安全/验证边界”一致；若动作改变既定范围或可能破坏数据，必须先向用户取得相应授权。
3. 每次动作完成后，在“动作时间线”追加一条结果，至少记录：时间、动作目的、命令或工具、修改文件、结果/错误、验证证据、未完成边界。不能用后来的成功覆盖或删除早期失败。
4. 对尚未亲自核验的历史信息标记为“历史记录”或“待核实”；禁止把编译、仿真、静态检查或生成文件写成真实硬件通过。
5. Git 工作按平台或阶段形成小提交，并在记录中写入完整 SHA、提交信息、是否已推送及远端核对结果。禁止以文件名或未提交工作树替代提交证据。
6. 本文件采用追加优先：可修正“当前状态”，但时间线只追加勘误，不删除原始条目。涉及同时工作的代理时，由专职记录代理汇总；其他代理必须把阶段结果、命令、输出和边界发送给记录代理。
7. 首次建立本文件前无法执行“先读本文件”；本文件的首条时间线记录明确保留这一次性例外。自文件建立后不再有例外。

建议的动作前审阅命令（Windows）：

```powershell
Get-Content -LiteralPath .\docs\PROJECT_WORKLOG.md
```

建议的动作前审阅命令（Linux/VM）：

```sh
sed -n '1,$p' docs/PROJECT_WORKLOG.md
```

## 1. 用户要求总表

### 1.1 项目目标

- 本对话和仓库用于今后 DIY 模块合成器与效果器开发。
- 审阅当前 AUDIO_DSP 库，说明它能完成的合成/效果/DSP 能力，以及 MCU、MPU、FPGA 要增加哪些硬件和板级软件才能真正使用这些算法。
- 先在 GitHub 等开源社区调研相近的合成器、效果器和 Audio DSP 库，择优借鉴、融合或替换本地实现；必须保留来源、固定提交和许可证边界。
- 在根目录按平台提供 `mcu/`、`mpu/`、`fpga/` 三套**可直接开发的完备原生工程**，而不是只有算法接口、零散 RTL 或生成脚本。
- 删除重复、过时、与最终开发入口冲突的文件和文档；保留有独立价值的参考资料、来源说明和许可证文档。
- 阶段性成果要形成独立 Git 提交并推送到对应远端；每次必须核对本地 SHA 与远端状态。

### 1.2 固定平台与开发工具

| 平台 | 固定硬件/SoC | 固定开发环境 | 最低交付形态 |
|---|---|---|---|
| MCU | STM32H743IIT6 | STM32CubeMX + Keil MDK/ArmClang | 可重新生成的 `.ioc`、可直接打开的 `.uvprojx`、完整 HAL/CMSIS/启动文件、板级 Codec/SAI/DMA 应用、至少两个可复现构建目标 |
| MPU | i.MX6ULL，资料位于桌面同名入口所指目录 | 原始环境为 VMware Ubuntu 18.04；用户已迁移到 Ubuntu 24.04，当前以 Ubuntu 24.04 为开发主机 | 可在 VM 内构建/测试的 CMake/ALSA 工程、与开发板 BSP/rootfs ABI 匹配的 ARMHF 交叉环境、打包/部署/服务脚本、主机与目标验证路径 |
| FPGA | BX71，Zynq-7020（XC7Z020）；资料位于桌面 BX71 入口所指目录 | Vivado 2018.x，当前本机实际工具为 Vivado/SDK 2018.3 | **仓库内可直接双击/打开的原生 `.xpr`**、BD/IP/RTL/XDC/仿真源、综合/实现/bitstream/export hardware、可构建的软件工程或 ELF；只有 Tcl 生成器不算完成 |

### 1.3 完成度纠正

- 用户明确指出早期三平台工程“太笼统”，要求达到可直接开发、已配置、已编译验证的模板程度；MCU 的标准是完整 CubeMX + Keil 工程，MPU/FPGA 同理。
- 用户再次明确指出 FPGA 目录中看不到 Vivado 工程，因此 FPGA 验收必须以**已提交且可直接打开的 `.xpr`**为硬门槛，不能把脚本、RTL、临时 `build/` 下的工程或口头说明当作交付完成。
- 构建、XSim、综合、实现、bitstream、ELF 只说明工具链/工程级状态；真实 WM8960、I2S、模拟输入输出、板上时钟、电平、长时间稳定性等必须单独上板验证。

### 1.4 2026-08-30 新增硬要求

- 必须保留本工作记录；每次执行动作之前先审阅，动作后追加结果；已委派专职子代理负责补齐和维护。
- Ubuntu 24.04 VM 当前由用户报告为已开启，Guest 为 `hry@ubuntu24`，地址 `192.168.79.149/24`，网卡 `ens33`，MAC `00:0c:29:29:8c:b9`。该信息来自用户粘贴的 `ip addr`；SSH、OS 版本和系统状态仍需代理实际核验。
- Ubuntu 24.04 是从原 Ubuntu 18.04 i.MX6ULL 开发环境迁移而来。必须先按桌面开发板资料检查工具链、BSP、sysroot、ALSA/交叉依赖和 ABI；移植不完整时先修复环境，再继续 MPU DSP 工程兼容与测试。
- MCU 需要融合用户既有仓库 [`ryule5158/NUEDC_2026`](https://github.com/ryule5158/NUEDC_2026) 中的 `dsp/`（实际路径大小写和结构待克隆后核实）。
- FPGA 需要融合用户既有仓库 [`ryule5158/BX71-DSP-ProMax`](https://github.com/ryule5158/BX71-DSP-ProMax)。
- 最终 AUDIO_DSP 的目录组织、命名、接口、注释和错误处理风格参考 `NUEDC_2026` 统一；具体风格规则必须先对该仓库做代码审计后记录，不能凭印象杜撰。

## 2. 已确定的架构与边界

### 2.1 根库责任

- 根目录 `aud_*.c/h`：可移植 C DSP 基元和效果/合成模块。
- `eurorack_algorithms/`：Mutable Instruments 相关高复杂度算法移植，带独立许可证和固定采样率/内存要求。
- 平台目录负责音频 I/O、时钟、DMA/缓存、控制面、启动、服务、部署和板级合同；根算法库本身不提供 Codec、SAI/I2S、ADC/DAC、MIDI/Gate/CV、UI 或完整产品固件。

### 2.2 许可证决策

- 默认 MCU/MPU 构建使用 49 个 permissive DSP 源文件。
- 17 个 DaisySP LGPL 模块保持显式 opt-in，不混入默认静态固件/库。
- CMSIS-DSP 作为可选后端通过已有 Pack/CubeMX 使用；不无选择地复制整库。
- GPL/LGPL 或许可证不清晰的外部项目只能隔离、参考或在履行义务后另建目标；融合 `NUEDC_2026` 和 `BX71-DSP-ProMax` 前仍须逐文件核验其许可证和来源。
- 开源调研快照与采用矩阵见 `docs/OPEN_SOURCE_SURVEY.md`。该调研不能证明覆盖互联网上“所有”项目，后续新增来源需追加固定提交和许可证结论。

### 2.3 硬件验证边界

- MCU：CubeMX 生成和 Keil 0 错误/0 警告不等于烧录或 WM8960 真实音频通过。
- MPU：x86_64 主机测试、ARMHF 链接和归档校验不等于 i.MX6ULL 板上 ALSA/Codec/模拟链通过；Ubuntu 主机 ABI 不能代替目标 rootfs ABI。
- FPGA：XSim/综合/实现/时序/DRC/bitstream/ELF 不等于 BX71 编程、I/O 电压、Codec 时钟或模拟音频通过。
- 最终物理验收至少记录：供电与 I/O 电压、MCLK/BCLK/LRCLK、I2C ACK、输入/输出电平、噪声/削顶、端到端延迟、CPU/FPGA 裕量、xrun、温度和长时间稳定性。

## 3. 当前状态快照（2026-08-30T14:26:32+08:00）

### 3.1 Git

- `main` 与 `origin/main` 当前都指向 `3475508f860335ae2ae6278dd9f7758d843091a5`。
- 已确认远端：`git@github.com:ryule5158/audio-dsp.git`。
- MCU 完备工程已提交并推送：`78dfbb005ca8732229aac1933ed3d71f2d9b6a1c`。
- MPU 加固工程已提交并推送：`3475508f860335ae2ae6278dd9f7758d843091a5`。
- FPGA 原生工程重做、根 README/许可证、冗余文档删除当前仍是**未提交工作树**；不得把它们当作远端已交付。
- 当前工作树包含与 FPGA 重做和文档清理有关的修改/删除/新增；在分平台暂存前必须重新运行 `git status --short --branch` 并审阅 diff，避免混入并行代理的未完成改动。

### 3.2 MCU

- 工程入口存在：`mcu/project/STM32H743_Audio/STM32H743_Audio.ioc`。
- Keil 入口存在：`mcu/project/STM32H743_Audio/MDK-ARM/STM32H743_Audio.uvprojx`。
- 已有 SelfTest 与 WM8960 Stream 两个 target、SAI1 双向 DMA、WM8960、D2 SRAM 非缓存 DMA 区、49 个 permissive DSP 源文件和再生成/构建脚本。
- 2026-08-24 历史验证：ArmClang 6.24，两 target 均 `0 Error(s), 0 Warning(s)`；SelfTest `Code=25408, ZI=41012`，WM8960 Stream `Code=32608, ZI=41540`；DMA map 为 `s_rx=0x30000000`、`s_tx=0x30000400`、区域 `0x800`。这些是提交 `78dfbb0` 对应的构建证据，不是实物板测。
- 尚未完成本轮 `NUEDC_2026/dsp` 审计、融合和统一风格回归。

### 3.3 MPU

- 已有 CMake/ALSA 全双工应用、严格配置解析、主机测试、ARMHF 打包、事务安装/回滚、systemd/SysV 入口、100ASK BSP/rootfs profile。
- 历史主机测试：`MPU_PCM_TEST_OK`、`MPU_FX_TEST_OK`、`MPU_CONFIG_TEST_OK`、`MPU_IO_STATE_TEST_OK`；BSP/rootfs profile：`MPU_100ASK_BSP_PROFILE_OK=1`、`MPU_100ASK_ROOTFS_PROFILE_OK=1`。
- 目标 rootfs 历史事实：镜像 SHA-256 `a0a393b4e7bdd6f58314156e1ac4e4a533f8346c18aa82f883189d2bfc4a5921`，Buildroot glibc 2.30，ALSA 1.2.1.2，ARMHF 解释器 `/lib/ld-linux-armhf.so.3`。这些需在 Ubuntu 24.04 迁移环境中重新核验并用于 sysroot/链接门禁。
- 旧的 Ubuntu 18.04 VM 没有完成本轮 ARMHF 全链接、板端部署和 WM8960 测试。
- 用户现在报告 Ubuntu 24.04 已开启并给出 `192.168.79.149`；尚未在本轮通过 SSH 核验 `whoami`、`hostname`、`/etc/os-release`、工具链、磁盘/内存负载或开发环境完整性。
- 用户曾报告 Ubuntu 24.04 GUI 卡住且无法打开终端；历史主机诊断发现 VMnet8 曾落到 link-local 地址，修复未验证。当前应优先使用 SSH 和只读健康检查，不能假定 GUI 恢复等于开发环境可用。

### 3.4 FPGA

- 旧提交 `09664830f188551a6a7dea4cbd0e2f119d0a40c9` 只提供初版 RTL/Tcl/说明，未达到用户要求的原生 Vivado 工程验收。
- 当前未提交工作树已出现 `fpga/projects/audio_dsp_bx71_selftest/audio_dsp_bx71_selftest.xpr`（13,576 字节，历史文件时间 2026-08-24 13:47:15），但仅“文件存在”不足以证明可打开、路径可移植或与当前 RTL 同步。
- 截至本快照，`fpga/projects/` 中未确认存在已纳入版本管理的 `soc_i2s` `.xpr`；`fpga/build/` 中的临时/忽略工程及输出不能替代仓库交付。
- `fpga/build/output/` 历史上出现 selftest/soc_i2s `.bit` 和 soc_i2s `.hdf`，但它们对应当前未提交修改的可追溯性、最终时序/DRC、软件 ELF 地址范围仍需重新核验。
- `fpga/README.md` 和 `fpga/VERIFICATION.md` 仍描述旧的 Tcl-only/synthesis-only 状态，与当前重做方向冲突，必须在最终提交前更新或删除冗余说明。
- 本轮还未审计/融合 `BX71-DSP-ProMax`；其可复用的数据面、控制面、主机 API、测试和 Vivado 工程结构需要先基于固定提交与许可证评估。

## 4. 历史时间线与提交账本

### 4.1 已有 Git 提交

| 时间（+08:00） | 提交 | 内容 | 远端状态 | 备注 |
|---|---|---|---|---|
| 2026-07-13 09:59 | `be3d2b2be636fd331e63957f6d4afeba7a6bfae7` | Import existing AUDIO_DSP baseline | 已在 `origin/main` 历史 | 基线导入 |
| 2026-07-13 11:20 | `962c53c926dddae37f16befda242a8eb187fbe94` | Add STM32H7 Eurorack algorithm library | 已在 `origin/main` 历史 | Mutable/Eurorack 移植 |
| 2026-08-09 18:02 | `b2b539cbb9e39d2b08c5060902a5ce2e3422a36b` | Document multi-platform audio DSP architecture | 已在 `origin/main` 历史 | 后续平台架构文档已进入清理范围 |
| 2026-08-09 18:24 | `c51990d9849f1518e647c43fc68d848b16286334` | Add STM32H743 audio platform library | 已在 `origin/main` 历史 | 早期骨架，后由 `78dfbb0` 替换 |
| 2026-08-09 20:13 | `e5e97000c63df13153350fefbe65488bdf0df639` | Add i.MX6ULL ALSA audio platform library | 已在 `origin/main` 历史 | 早期平台库，后由 `3475508` 加固 |
| 2026-08-09 20:15 | `09664830f188551a6a7dea4cbd0e2f119d0a40c9` | Add BX71 Zynq audio DSP RTL library | 已在 `origin/main` 历史 | 未达到 `.xpr` 验收标准 |
| 2026-08-09 20:16 | `b6af1346cad44cd62848ed49ac1fbba628e81ea1` | Correct DaisySP license boundaries | 已在 `origin/main` 历史 | permissive/LGPL 边界 |
| 2026-08-24 13:19 | `78dfbb005ca8732229aac1933ed3d71f2d9b6a1c` | Replace MCU skeleton with complete CubeMX Keil project | 已推送 | MCU 当前基线 |
| 2026-08-24 13:27 | `3475508f860335ae2ae6278dd9f7758d843091a5` | Harden i.MX6ULL ALSA deployment project | 已推送 | MPU 当前基线、Ubuntu 24 迁移仍待验证 |

### 4.2 需求/执行历史补录

| 阶段 | 用户要求 | 已做事项 | 结果与边界 |
|---|---|---|---|
| 初始审阅 | 说明 AUDIO_DSP 能做什么，以及 MCU/MPU/FPGA 要配什么硬件 | 审阅 DaisySP C 端口、Eurorack manifest、模块采样率/内存/许可证 | 形成能力与硬件责任边界；不是数学等价性或板测证明 |
| 开源调研 | 搜索 GitHub 等社区的合成器/效果器 Audio DSP 库，择优融合 | 形成 `docs/OPEN_SOURCE_SURVEY.md`，包含 CMSIS-DSP、DaisySP、Mutable、STK、Airwindows、Faust、FPGA 候选等 | 2026-08-09 快照；后续外部库仍需固定提交/许可证回归 |
| 三平台初版 | 建 `mcu/mpu/fpga`，分别面向 H743IIT6、i.MX6ULL、BX71 Zynq7020 | 形成初版平台目录并分阶段提交 | 用户判定完成度过低，不能视为最终交付 |
| 工程化纠正 | MCU 必须是已配置、已编译的 CubeMX + Keil 工程，MPU/FPGA 同理；清冗余 | MCU 完整工程和 MPU 加固工程完成并推送；根文档/FPGA 重做进行中 | MCU/MPU 软件工程级基线存在；FPGA 未完成，硬件未验证 |
| FPGA 纠正 | 必须看到并直接打开 Vivado 工程 | 未提交工作树开始生成 native `.xpr`、BD、profile、bit/HDF/软件脚本 | selftest `.xpr` 文件存在但验收未闭环；soc_i2s native `.xpr` 未确认 |
| Ubuntu 24.04 | VM 已迁移，先补环境再做 MPU；GUI 曾卡住 | 历史仅做主机网络/VMX 诊断 | 本轮尚未 SSH 实测；不得声称环境已修好 |
| 既有库融合 | 合并 `NUEDC_2026/dsp` 与 `BX71-DSP-ProMax`，统一到 NUEDC_2026 代码风格 | 新增要求，尚未执行 | 必须先审计来源、许可证、重复功能、测试与风格，再择优融合 |
| 工作记录 | 建立持续工作记录，每次动作前审阅，专职代理维护并补齐历史 | 本文件建立 | 以后所有代理均受第 0 节约束 |

## 5. 验证证据账本

| 对象 | 证据 | 结论 | 明确不代表 |
|---|---|---|---|
| MCU `78dfbb0` | CubeMX 再生成；Keil/ArmClang 6.24 两 target 0 错误/0 警告；49 DSP 对象；DMA map | 工程生成、编译、链接布局通过（历史记录，2026-08-24） | 烧录、WM8960 ACK、时钟、模拟音频、稳定性 |
| MPU `3475508` | 4 个主机单测 PASS；BSP/rootfs profile PASS；包验证 | 主机逻辑、板级资料/profile 和部署包设计通过（历史记录） | Ubuntu 24 环境完整、ARMHF 当前全链接、目标板运行、音质 |
| FPGA `0966483` 旧基线 | 旧 `fpga/VERIFICATION.md` 记录 XSim 与综合、BRAM 推断 | 旧 RTL 的历史仿真/综合证据 | 当前未提交 RTL、实现/bitstream、原生 `.xpr`、板测 |
| FPGA 当前工作树 | native selftest `.xpr` 文件和 `build/output` 文件存在 | 说明曾生成过相关文件，需继续验收 | 工程可移植、结果与当前源码一致、DRC/时序/ELF 合格、已提交 |
| Ubuntu 24.04 | 用户提供 `ip addr`：`192.168.79.149/24` | 用户侧证明 Guest 网卡当前有地址 | SSH 可达、OS 身份、工具链完整、系统不卡、开发环境可用 |

## 6. 当前决策记录

| ID | 决策 | 理由 | 状态 |
|---|---|---|---|
| D-001 | 根算法库与平台 I/O/硬件层分离 | 便于相同 DSP 在 MCU/MPU 复用并清晰界定板级责任 | 生效 |
| D-002 | 默认只编入 49 个 permissive DaisySP C 源；17 LGPL 模块显式 opt-in | 降低默认固件分发许可证风险 | 生效 |
| D-003 | MCU 使用 H743IIT6 + WM8960 的固定参考合同，并保留 SelfTest/Stream 双 target | 未接 Codec 时仍可安全验证，接线后再启用真实流 | 生效，待融入 NUEDC DSP |
| D-004 | i.MX6ULL 目标 ABI 由开发板 BSP/rootfs/SDK决定，不由 Ubuntu 版本决定 | Ubuntu 24 只是构建主机，错误 sysroot 会产生不可运行 ELF | 生效 |
| D-005 | FPGA 原生 `.xpr` 是提交物；`build/` 下临时工程和 Tcl 生成器只是辅助 | 直接满足用户可打开、可开发要求 | 生效，未闭环 |
| D-006 | `NUEDC_2026/dsp` 与 `BX71-DSP-ProMax` 采用“审计、去重、适配、测试、再融合”流程 | 避免重复实现、许可证污染和未经验证的替换 | 新增，待执行 |
| D-007 | 最终代码风格以实际审计后的 `NUEDC_2026` 规则为准 | 用户明确指定；禁止凭主观印象定义风格 | 新增，待审计 |
| D-008 | 工作记录是动作前门禁和动作后证据载体 | 用户硬要求；保证长周期项目可追溯 | 生效 |

## 7. 未决项与下一步门禁

### P0：记录与协作门禁

- [x] 建立并补齐首版 `docs/PROJECT_WORKLOG.md`。
- [x] 主代理已确认 D-008/P0 生效；所有活跃代理在每个实质动作前审阅本记录。
- [ ] 各代理把阶段命令、输出、文件、SHA 和验证边界发送给专职记录代理追加。

### P1：Ubuntu 24.04 / i.MX6ULL 环境

- [ ] 通过 SSH 核验 `hry@192.168.79.149` 的身份、hostname、Ubuntu 版本、内核、负载、内存、磁盘、卡顿原因；不得复用 Ubuntu 18.04 host-key 身份而不核验。
- [ ] 只读盘点 `git`、`cmake`、`make/ninja`、native GCC、`arm-linux-gnueabihf-gcc`、binutils、pkg-config、ALSA headers/tools、Python、BSP、rootfs/sysroot、环境变量和已有工程。
- [ ] 按桌面 i.MX6ULL 资料再次核对 DTS、defconfig、rootfs/SDK 与目标 ABI。
- [ ] 只补缺失或不兼容的包/工具；系统修改前记录命令和影响。Ubuntu 24 原生 multiarch 仅可作为候选，优先使用厂家 Buildroot SDK/sysroot。
- [ ] 在 VM 内跑 host tests、ARMHF full-link、ELF ABI 门禁、打包安全测试；保存完整日志与哈希。
- [ ] 若无实体 i.MX6ULL/WM8960 在线，只能报告 VM 构建/静态 ABI 结果，不能报告板测。

### P2：`NUEDC_2026/dsp` 融合与统一风格

- [ ] 克隆/更新用户指定仓库并固定审核提交 SHA；确认真实目录名、许可证、远端和工作树状态。
- [ ] 形成 MCU DSP 功能矩阵：FFT、FIR/IIR、IQ、AM/FM/BPSK、PLL、THD、相关、拟合等，与 AUDIO_DSP 现有合成/效果模块逐项去重。
- [ ] 提取可验证的代码风格规则：目录/文件命名、类型/API 前缀、返回码、配置结构、静态状态、注释、格式、测试和构建入口。
- [ ] 在不破坏已验证 H743 CubeMX/Keil 工程的前提下融合；为新增模块加离线向量测试、Keil 编译和资源记录。
- [ ] 更新来源/许可证文件与模块 manifest。

### P3：`BX71-DSP-ProMax` 融合与 FPGA 原生工程

- [ ] 克隆/更新 `BX71-DSP-ProMax` 并固定审核提交 SHA、许可证和已验证 capability profile。
- [ ] 审计可复用模块：DDC/CIC/FIR/匹配滤波、功率/峰值/统计、FFT profile、SPI/UART/AXI 控制、STM32 host API、testbench 和 Vivado 2018.3 工程结构。
- [ ] 决定与当前音频 I2S/DC/NCO/delay/AXI 核的融合边界，避免把不具备的 FFT/实时能力写入 capability。
- [ ] 交付并提交至少 selftest 与 soc_i2s 两个可直接打开的 native `.xpr`；路径必须相对/可移植，工程源/BD/XDC/仿真源均可追溯。
- [ ] 使用 Vivado 2018.3 重跑 XSim、综合、实现、时序和 DRC；分别记录 Error/Critical Warning/Warning 数，不能把“DRC Error=0”写成“DRC=0”。
- [ ] 生成 bitstream 和 HDF；用 XSDK/XSCT 2018.3 构建软件。若 PS DDR 未启用，ELF 每个 LOAD 段必须位于实际 OCM 范围并做自动门禁。
- [ ] 打开已提交 `.xpr` 做最终 smoke test；不能只打开 `fpga/build/` 的临时项目。

### P4：清理、全量验证、分阶段提交与推送

- [ ] 删除或更新旧 `fpga/README.md`、`fpga/VERIFICATION.md` 与 Tcl-only 叙述，保留唯一权威入口。
- [ ] 核实根文档/PDF清理只删除重复或过时内容，保留算法、来源、许可证和实用手册。
- [ ] 全量运行 PowerShell 语法、XML/XPR 可解析性、C/C++/RTL/Tcl、冲突标记、绝对路径、许可证和 `git diff --check` 检查。
- [ ] 按 MCU 融合、MPU Ubuntu24 兼容、FPGA 原生工程、文档清理分别暂存与提交；每次提交后记录完整 SHA。
- [ ] `git push origin main` 后核对远端 SHA；禁止 force push，除非用户另行明确授权。

## 8. 动作时间线（仅追加）

### W-20260830-001：建立专职工作记录并补齐旧记录

- 时间：2026-08-30T14:26:32+08:00
- 用户要求：为大项目建立强制工作记录；记录用户要求和全部动作；每次动作前先审阅；委派专职子代理并补齐历史。
- 前置审阅：仓库中没有 `PROJECT_WORKLOG`、`WORKLOG`、`HISTORY` 或同类过程记录；因此本次建立属于第 0 节第 7 条所述的一次性例外。建立前已审阅 Git 历史/状态、根及三平台 README、开源调研、旧 FPGA 验证说明和本项目历史摘要。
- 执行动作：只新增 `docs/PROJECT_WORKLOG.md`，没有编辑其他文件，没有执行 Git 暂存/提交/推送。
- 使用的只读证据命令：
  - `git status --short --branch`
  - `git log --date=iso-strict --pretty=format:...`
  - `git remote -v`
  - `git diff --stat` / `git diff --cached --stat`
  - `rg --files docs`
  - 阅读 `README.md`、`docs/OPEN_SOURCE_SURVEY.md`、`mcu/README.md`、`mpu/README.md`、`fpga/README.md`、`fpga/VERIFICATION.md`
  - 枚举 `fpga/projects` 与 `fpga/build/output`
- 结果：已补录原始目标、平台/工具链、完成度纠正、验证边界、Ubuntu 24 迁移、既有库融合、提交账本、当前状态、决策和未决项。
- 不确定项：旧阶段未在当前轮重新执行的构建结果均标为“历史记录”；当前 FPGA 未提交工作树的可打开性和结果一致性仍待核实。
- Git：未提交。

### W-20260830-002：主代理启用记录门禁并确定阶段顺序

- 时间：2026-08-30T14:31:03+08:00
- 关联要求/未决项：D-008、P0、P1、P2、P3、P4。
- 动作前审阅：主代理报告已完整审阅 `docs/PROJECT_WORKLOG.md`，确认 D-008 与 P0 的动作前审阅、动作后记录门禁正式生效；记录代理在追加本条前也重新完整读取本文件。
- 计划动作：先只暂存本工作记录，形成独立提交并推送；随后并行开展 Ubuntu 24.04/i.MX6ULL 环境核验、`NUEDC_2026` 与 `BX71-DSP-ProMax` 审计，以及 FPGA 原生 Vivado 工程闭环。
- 本条实际动作：只更新本记录中的 P0 状态并追加本条，没有修改其他文件。
- 结果与证据：主代理通过代理消息明确给出上述审阅确认和阶段计划；尚未收到 Git 暂存、提交或远端 SHA，因此不提前标记为已提交/已推送。
- 验证边界：本条只证明流程门禁已启用；不代表 Ubuntu 24.04 可达、开发环境完整、两个外部库已审计或 FPGA `.xpr` 已验收。
- Git：截至本条追加时未暂存、未提交、未推送；等待主代理回传结果。
- 下一步：记录独立提交/推送的完整 SHA；之后逐项追加并行任务的命令、修改、测试结果和未完成边界。

### W-20260830-003：工作记录独立提交的首次校验误报并中止

- 时间：2026-08-30T14:32:05+08:00
- 关联要求/未决项：D-008、P0、P4。
- 动作前审阅：主代理按门禁先审阅本记录；记录代理在追加本条前再次完整读取本文件。
- 计划动作：精确检查工作树，只暂存 `docs/PROJECT_WORKLOG.md`，确认暂存区没有其他路径后形成独立提交并推送。
- 实际命令/步骤：主代理先运行 `git status`、`git diff --check`，随后执行 `git add -- docs/PROJECT_WORKLOG.md`；暂存后用 PowerShell 变量 `$staged` 校验暂存路径。
- 失败：当暂存区只有一行输出时，PowerShell 将 `$staged` 作为单行标量字符串；表达式 `$staged[0]` 取得首字符而不是第一条路径，因而误报 `Unexpected staged paths: docs/PROJECT_WORKLOG.md`。脚本在 `git commit` 之前主动中止。
- 文件/暂存影响：首次尝试只暂存了 `docs/PROJECT_WORKLOG.md`；其他未提交文件未被暂存。本条追加后工作记录又产生新内容，重试前必须再次精确暂存该文件。
- 结果与证据：没有创建提交，没有推送；失败属于校验脚本的单元素数组处理错误，不是 Git 路径污染，也不能被后续成功记录覆盖。
- 验证边界：本条只记录暂存门禁失败；不改变 MCU/MPU/FPGA 交付或验证状态。
- Git：工作记录已部分暂存并因本次追加存在新未暂存内容；无提交 SHA、无远端变化。
- 下一步：将校验结果强制包装为 `@(...)` 数组，重新读取记录、运行 diff 检查、只暂存该文件、核对暂存路径，再提交和推送。

## 9. 后续追加模板

```markdown
### W-YYYYMMDD-NNN：动作标题

- 时间：YYYY-MM-DDTHH:mm:ss+08:00
- 关联要求/未决项：P?
- 动作前审阅：已阅读本记录；当前基线/风险为……
- 计划动作：……
- 命令/工具：`……`
- 修改文件：……（无修改则写“无”）
- 结果与原始证据：……
- 验证边界：……
- Git：未暂存 / 已暂存 / 提交 `<full SHA>` / 已推送并核对
- 下一步：……
```
