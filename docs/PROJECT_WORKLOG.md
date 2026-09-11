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

- `main` 与 `origin/main` 在工作记录独立提交后都指向 `5d5cf9cf5b3fecfc2c5ec90b85acc8e25fb08e26`；本文件后续追加内容会再次形成未提交差异，需随下一阶段记录提交。
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
- 已在忽略目录建立 `NUEDC_2026` 审计副本并固定 SHA `416270a254795843b6f1854074380015637d1f95`；真实 MCU 路径是大写 `DSP/`，分别位于三个 STM32H743 变体和一个 TI 工程内。仓库根没有 LICENSE/COPYING/NOTICE/AUTHORS，现有 ST/CMSIS/TI 组件许可证不能推定为用户自研 DSP 许可。API/依赖/测试/风格矩阵、授权元数据和正式融合仍未完成。

### 3.3 MPU

- 已有 CMake/ALSA 全双工应用、严格配置解析、主机测试、ARMHF 打包、事务安装/回滚、systemd/SysV 入口、100ASK BSP/rootfs profile。
- 历史主机测试：`MPU_PCM_TEST_OK`、`MPU_FX_TEST_OK`、`MPU_CONFIG_TEST_OK`、`MPU_IO_STATE_TEST_OK`；BSP/rootfs profile：`MPU_100ASK_BSP_PROFILE_OK=1`、`MPU_100ASK_ROOTFS_PROFILE_OK=1`。
- 目标 rootfs 历史事实：镜像 SHA-256 `a0a393b4e7bdd6f58314156e1ac4e4a533f8346c18aa82f883189d2bfc4a5921`，Buildroot glibc 2.30，ALSA 1.2.1.2，ARMHF 解释器 `/lib/ld-linux-armhf.so.3`。这些需在 Ubuntu 24.04 迁移环境中重新核验并用于 sysroot/链接门禁。
- 旧的 Ubuntu 18.04 VM 没有完成本轮 ARMHF 全链接、板端部署和 WM8960 测试。
- Ubuntu 24 已建立独立 SSH known_hosts 信任并通过公钥核验：用户 `hry`、hostname `ubuntu24.04`、Ubuntu `24.04.4 LTS`、内核 `7.0.0-30-generic`、地址 `192.168.79.149`。资源快照显示系统空闲且 system 级单元无失败。终端启动故障已定位到自定义 `~/.config/systemd/user/terminal-warmup.service` 在图形 DISPLAY 可用前强启服务并报 `Cannot open display`；系统终端/portal 随后自恢复，修复动作尚未执行。交叉环境有 ARM GCC/binutils，但缺 ARM G++、ALSA 开发头、foreign architecture 和 QEMU；`/home/hry/toolchains` 仅 64 KiB、其中归档仅 58,351 字节且未发现 SDK/compiler，`/home/hry/imx6ull` 约 977 MiB、当前只确认 Linux 4.9.88 源码树，完整性/ABI 仍待核验。
- 用户曾报告 Ubuntu 24.04 GUI 卡住且无法打开终端；历史主机诊断发现 VMnet8 曾落到 link-local 地址。2026-08-30 当前主机侧复核已确认 VMnet8 恢复为 `192.168.79.1/24`，Guest 的 ICMP、22/TCP 和 MAC 都匹配；因此不需要再提权改 VMnet8。SSH host key、Guest 身份和开发环境仍待独立核验。

### 3.4 FPGA

- 旧提交 `09664830f188551a6a7dea4cbd0e2f119d0a40c9` 只提供初版 RTL/Tcl/说明，未达到用户要求的原生 Vivado 工程验收。
- 当前未提交工作树已出现 `fpga/projects/audio_dsp_bx71_selftest/audio_dsp_bx71_selftest.xpr`（13,576 字节，历史文件时间 2026-08-24 13:47:15），但第 6 行 `Path=` 仍含 `C:/Users/LENOVO/Desktop/AUDIO_DSP/...` 绝对路径，尚不具备仓库移植性，也未证明与当前 RTL 同步。
- 截至本快照，`fpga/projects/` 中只有 selftest `.xpr`；`soc_i2s` `.xpr` 只存在于被忽略的 `fpga/build/...`，不满足用户的已提交原生工程门槛。
- `fpga/build/output/` 历史上出现 selftest/soc_i2s `.bit` 和 soc_i2s `.hdf`，但它们对应当前未提交修改的可追溯性、最终时序/DRC、软件 ELF 地址范围仍需重新核验。
- `fpga/README.md` 和 `fpga/VERIFICATION.md` 仍描述旧的 Tcl-only/synthesis-only 状态，与当前重做方向冲突，必须在最终提交前更新或删除冗余说明。
- 已在忽略目录建立 `BX71-DSP-ProMax` 审计副本并固定 SHA `3ff859aa888a7e77996f6fc3b32b92be8352f69c`；初审未找到 LICENSE/COPYING/NOTICE、SPDX 或版权/许可文本，因此暂按“无明确许可证、不能复制源码”处理。能力需继续审计，正式融合只能使用有权采用的实现。
- 本机已核验 Vivado 2018.3 64-bit（SW 2405991 / IP 2404404）；`E:\Vivado\2018.3\bin\xsct.bat` 不存在，XSDK/XSCT 的实际安装入口仍需定位。当前 `open_project.ps1` 仍打开被忽略的 `build/` 临时工程，尚不符合最终 GUI 入口要求。

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
| 2026-08-30 14:32 | `5d5cf9cf5b3fecfc2c5ec90b85acc8e25fb08e26` | Add authoritative project work record | 已推送并由 `git ls-remote` 核对 | 首版权威工作记录；后续条目持续追加 |

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
| D-009 | NUEDC MCU 融合只选一个经审计的 H743 canonical 源，不导入三个重复固件快照 | 三个 H743 DSP 树绝大多数同名文件哈希相同；重复导入会制造维护分叉 | 生效，仍受许可证/授权门禁限制 |

## 7. 未决项与下一步门禁

### P0：记录与协作门禁

- [x] 建立并补齐首版 `docs/PROJECT_WORKLOG.md`；已由独立提交 `5d5cf9cf5b3fecfc2c5ec90b85acc8e25fb08e26` 推送。
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

### W-20260830-004：工作记录独立提交并完成远端核对

- 时间：2026-08-30T14:33:05+08:00
- 关联要求/未决项：D-008、P0、P4。
- 动作前审阅：主代理和记录代理均按门禁重新审阅本记录；确认 W-20260830-003 的失败原因和“只提交工作记录”范围。
- 计划动作：修正单元素路径校验，只暂存并提交 `docs/PROJECT_WORKLOG.md`，推送后核对远端分支 SHA。
- 实际命令/步骤：将暂存路径读取修正为 `@(git diff --cached --name-only)`，重新暂存工作记录并核对唯一暂存路径；提交信息为 `Add authoritative project work record`；执行 `git push origin main`，再用 `git ls-remote` 核对 `origin/main`。
- 结果与原始证据：成功创建提交 `5d5cf9cf5b3fecfc2c5ec90b85acc8e25fb08e26`；`git push origin main` 成功；`git ls-remote` 返回的 `origin/main` 与该完整 SHA 一致。
- 文件/暂存影响：该提交只包含 `docs/PROJECT_WORKLOG.md`。根目录和 `fpga/` 的其他未提交改动仍保留且未暂存，没有被意外带入提交。
- 验证边界：这次成功只证明记录文件已进入本地/远端历史；不代表任何平台的新实现、VM 环境或硬件验证完成。
- Git：提交并推送成功，完整 SHA 为 `5d5cf9cf5b3fecfc2c5ec90b85acc8e25fb08e26`。本条是在该提交完成后追加，因此将作为后续记录差异随下一阶段提交。
- 下一步：按既定顺序并行执行 P1、P2、P3，并让各阶段结果回传本记录。

### W-20260830-005：读取 VMware 专用规则并隔离 Ubuntu 24 身份

- 时间：2026-08-30T14:34:53+08:00
- 关联要求/未决项：P1。
- 动作前审阅：主代理先完整审阅本记录；记录代理在追加本条前也重新完整读取本文件。
- 计划动作：在访问 VMware Guest 前，先按项目级 AGENTS 规则审阅既有 SSH 配置的适用范围。
- 命令/工具：读取 `C:\Users\LENOVO\Documents\Codex\2026-06-14\vmware-ubuntu\AGENTS.md`。
- 结果与原始证据：该文件只描述并授权原 Ubuntu 18.04 Guest `hry@ubuntu`，且其 host-key alias 属于旧 Guest 身份。它不能直接套用到 Ubuntu 24.04 `hry@ubuntu24` / `192.168.79.149`。
- 决策：Ubuntu 24 必须单独核验用户名、hostname、OS、当前地址和 SSH host key；不得用旧 `HostKeyAlias` 绕过新 Guest 的身份检查，也不得因 IP 相邻而假定是同一 VM。
- 修改文件/系统：无；此次只读取规则，没有连接 VM、修改 SSH 配置、修改系统或仓库文件。
- 验证边界：尚未证明 Ubuntu 24 SSH 可达或环境完整。
- Git：无暂存、提交或推送。
- 下一步：建立独立且可核验的 Ubuntu 24 SSH 身份，再做只读健康与工具链盘点。

### W-20260830-006：克隆并固定 NUEDC_2026 审计基线

- 时间：2026-08-30T14:34:53+08:00
- 关联要求/未决项：D-006、D-007、P2。
- 动作前审阅：`/root/nuedc_mcu_audit` 报告已完整执行并阅读 `Get-Content -Raw docs\PROJECT_WORKLOG.md`，确认审计范围；记录代理追加前再次读取最新记录。
- 计划动作：只在被忽略的临时目录建立上游审计副本，固定来源提交、远端和工作树状态，不触碰已验证 MCU 工程。
- 命令/工具：在仓库根目录执行 `git clone --filter=blob:none --no-tags https://github.com/ryule5158/NUEDC_2026.git tmp/upstream/NUEDC_2026`，随后执行 `git rev-parse HEAD`、`git remote -v`、`git status --short --branch`。
- 结果与原始证据：克隆成功；固定审核 SHA 为 `416270a254795843b6f1854074380015637d1f95`；`origin` 为 `https://github.com/ryule5158/NUEDC_2026.git`；状态为 `main...origin/main` 且工作树干净。
- 修改范围：只新增被忽略的 `tmp/upstream/NUEDC_2026` 审计副本；没有编辑 AUDIO_DSP MCU 工程，没有暂存、提交或推送。
- 验证边界：当前只证明来源、固定提交和 clean checkout；许可证、真实 `dsp/` 路径、功能矩阵、代码风格规则和融合安全性均未完成审计。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：逐文件核对仓库许可证、DSP 真实目录、API/测试/构建入口和可复用功能，再形成去重与风格矩阵。

### W-20260830-007：克隆并固定 BX71-DSP-ProMax 审计基线

- 时间：2026-08-30T14:34:53+08:00
- 关联要求/未决项：D-006、P3。
- 动作前审阅：`/root/fpga_native` 报告在首次动作前已完整审阅本记录；记录代理追加前再次读取最新记录。
- 计划动作：在被忽略目录建立上游 FPGA 库审计副本并固定精确提交，不修改当前正式 FPGA 工程。
- 命令/工具：在 `tmp/upstream/BX71-DSP-ProMax` 使用 `git clone --filter=blob:none --no-checkout https://github.com/ryule5158/BX71-DSP-ProMax`，随后 detached checkout 并核验 HEAD、远端和状态。
- 结果与原始证据：成功固定 HEAD `3ff859aa888a7e77996f6fc3b32b92be8352f69c`；远端为 `https://github.com/ryule5158/BX71-DSP-ProMax`；工作树 clean 且处于 detached HEAD。
- 修改范围：只新增 `tmp/` 下被忽略的审计副本；没有修改正式 `fpga/` 工程，没有暂存、提交或推送。
- 验证边界：当前只证明来源克隆、固定提交和干净副本；许可证、能力声明、Vivado 工程结构、可复用 RTL/软件和融合方案尚未审计。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：审计许可证、capability profile、RTL/控制面/测试、Vivado 2018.3 工程结构，并与当前音频核做去重和兼容性评估。

### W-20260830-008：BX71-DSP-ProMax 结构与许可证初审

- 时间：2026-08-30T14:35:49+08:00
- 关联要求/未决项：D-006、P3。
- 动作前审阅：`/root/fpga_native` 和记录代理均在动作前完整审阅最新工作记录，继续保持只读上游审计范围。
- 计划动作：枚举固定提交的全部文件、顶层文档和许可证候选，识别可借鉴能力与是否具有可复制许可。
- 命令/工具：在固定 SHA `3ff859aa888a7e77996f6fc3b32b92be8352f69c` 上执行 `git ls-tree -r --name-only HEAD`，枚举顶层/README/许可证候选，并执行 `rg -i 'license|copyright|SPDX'`。
- 结构结果：上游包含 CIC、FIR、mixer、NCO、matched filter、Goertzel、power/peak、FFT wrapper/profile、SPI/UART/control、STM32 host API、6 个 testbench 和多套 Vivado Tcl。
- 原生工程结果：上游 README 明确忽略 `vivado/` 和 `build/`，固定提交本身没有纳入 `.xpr`。因此不能把该仓库当作满足本项目 native `.xpr` 要求的直接模板；只能选择性参考结构和脚本思路。
- 许可证结果：没有发现 LICENSE、COPYING、NOTICE 文件，也没有发现 SPDX、copyright 或 license 文本。当前决策是按“无明确许可证”处理：不得把上游源码直接复制到 AUDIO_DSP；除非后续获得明确许可/授权，只能做只读审计并独立实现可借鉴的架构/接口思想。
- 上游声明边界：README 中的 Vivado 2018.3、资源、时序和仿真数字只是上游自述，本轮尚未复现，不能记作 AUDIO_DSP 的验证证据。
- 修改范围：没有修改正式工程，没有暂存、提交或推送。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：继续核对能力/接口与现有音频 RTL 的重复关系；在不复制无许可源码的前提下完成本仓库自有 native `.xpr`、测试和工具链闭环。

### W-20260830-009：FPGA 正式工作树与 native `.xpr` 门槛审计

- 时间：2026-08-30T14:36:15+08:00
- 关联要求/未决项：D-005、P3、P4。
- 动作前审阅：`/root/fpga_native` 和记录代理均按门禁完整审阅最新记录；确认只读审计现有未提交工作，不覆盖或回退并行改动。
- 计划动作：区分正式待提交源、已跟踪历史、未跟踪候选与被忽略的构建产物，确认仓库中真正可交付的 native `.xpr` 数量和移植性。
- 命令/工具：`git status --short --branch`、`git diff --stat -- fpga`、`git diff --name-status -- fpga`、`git ls-files fpga`、`rg --files fpga`，并搜索 `.xpr`、绝对路径及 Vivado 工具候选。
- 工作树结果：正式未提交变更包括 4 个 RTL 修改、testbench 修改、旧 XDC/header/build Tcl 删除，以及新的 build/open/software/BD/profile/native-project 文件。审计没有覆盖、恢复或删除这些改动。
- 原生工程结果：正式 `fpga/projects/` 中只有 `audio_dsp_bx71_selftest/audio_dsp_bx71_selftest.xpr`；`soc_i2s` `.xpr` 只存在于被忽略的 `fpga/build/...`，不能替代版本化交付。
- 移植性缺陷：selftest `.xpr` 第 6 行 `Path=` 使用 `C:/Users/LENOVO/Desktop/AUDIO_DSP/...` 绝对路径，必须改为可迁移方式并在另一路径/干净检出中验证。
- 修改/构建：无；没有运行 Vivado 构建，没有编辑正式工程，没有暂存、提交或推送。
- 验证边界：只完成现状审计；当前仍未满足用户要求的 selftest + soc_i2s 可直接打开、已提交、可移植 native 工程门槛。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：修复 selftest 路径，物化并版本化 soc_i2s `.xpr`，然后从已提交候选工程执行 Vivado 2018.3 打开/仿真/综合/实现/bitstream/HDF/软件验收。

### W-20260830-010：Ubuntu 24.04 主机侧网络与 SSH 端口复核

- 时间：2026-08-30T14:36:50+08:00
- 关联要求/未决项：P1。
- 动作前审阅：主代理和记录代理均按门禁完整审阅最新记录，确认只做主机侧只读连通性检查，不修改 VMnet8。
- 计划动作：验证此前 VMnet8 APIPA 故障是否仍存在，并把 Guest IP/MAC 与用户提供的 `ip addr` 对齐；只有网络身份一致后才进入 SSH host-key 核验。
- 命令/工具：`Get-NetIPConfiguration`、`Get-NetIPAddress`、两次 ping、`Test-NetConnection -Port 22` 和 ARP 表检查。
- 结果与原始证据：VMnet8 当前为 `192.168.79.1/24`，`PrefixOrigin` 显示 DHCP、`AddressState` 为 Preferred；`192.168.79.149` 两次 ping 均成功且报告 0 ms；22/TCP 成功，源地址为 `192.168.79.1`；ARP MAC 为 `00-0c-29-29-8c-b9`，与用户提供的 Guest MAC 完全一致。
- 结论：此前 VMnet8 link-local/APIPA 故障当前已恢复，不需要提权或重新配置 VMnet8。当前可以进入 SSH 身份核验，但端口可达不等于 host key、用户名、hostname 或 OS 已核验。
- 修改文件/系统：无；没有改网络、VM、SSH 配置或仓库文件。
- 验证边界：尚未建立独立 Ubuntu 24 host key 信任，也未登录 Guest；不能据此声称系统卡顿原因或开发环境状态已解决。
- Git：无暂存、提交或推送。
- 下一步：读取 192.168.79.149 当前 SSH host key，以独立 known-hosts 记录核验后登录并检查 `whoami`、hostname、OS、负载、内存和工具链。

### W-20260830-011：NUEDC_2026 全树结构与许可证候选初筛

- 时间：2026-08-30T14:36:50+08:00
- 关联要求/未决项：D-006、D-007、P2。
- 动作前审阅：`/root/nuedc_mcu_audit` 报告已完整读取包含 W-006 的最新工作记录；记录代理追加前再次完整读取本文件。
- 计划动作：对固定 SHA `416270a254795843b6f1854074380015637d1f95` 枚举目录、工程、DSP 路径和许可证候选，先建立结构边界，不修改正式工程。
- 命令/工具：`git ls-tree -r --name-only HEAD`、顶层目录与工程文件统计、匹配 `(^|/)dsp(/|$)` 的路径，并用 `rg` 搜索 SPDX/license/copyright。
- 结构结果：固定提交共有 3,655 个跟踪文件；主要树为 `STM32H743` 957 个、`STM32H743_LTC2208` 956 个、`STM32H743_AD9238` 955 个、`TI_MSPM0G3507` 606 个，另有三个 FPGA 目录。
- 许可证结果：只确认到 ST/CMSIS 驱动等组件级 LICENSE 候选，尚未看到仓库根 LICENSE。全仓许可证文本搜索被大量 vendor HAL/CMSIS/TI SDK 命中淹没且输出被截断，因此本动作**不能**得出自研 DSP 的许可证结论，也不能确认全部真实 DSP 路径。
- 决策边界：不得把 vendor LICENSE 推定为用户自研 DSP 代码许可；在复制/融合任何自研源码前，必须排除 Drivers/third_party 后对目标路径逐文件核验。
- 修改范围：只读上游审计；没有修改正式工程，没有暂存、提交或推送。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：聚焦 STM32 自研 DSP、公共 Modules 和对应测试/Keil 工程，排除 vendor 路径后重新统计文件、API、风格和许可证标记。

### W-20260830-012：FPGA 工具链与现有工程脚本能力审计

- 时间：2026-08-30T14:37:28+08:00
- 关联要求/未决项：D-005、P3、P4。
- 动作前审阅：`/root/fpga_native` 和记录代理均按门禁完整审阅最新记录，保持只读脚本/工具检查范围。
- 计划动作：确认本机 Vivado 版本，审阅当前构建、物化、打开、BD、软件和门禁脚本能做什么，并找出与用户 native `.xpr` 验收冲突的入口。
- 审阅文件：`fpga/.gitignore`、`build.ps1`、`open_project.ps1`、`build_software.ps1`、`vivado/create_project.tcl`、`materialize_native_projects.tcl`、`check_native_project.tcl`、`flow.tcl`、SoC BD Tcl、native selftest `.xpr`、`software/build_xsdk.tcl`、baremetal `main` 和 XDC profiles。
- 工具证据：`E:\Vivado\2018.3\bin\vivado.bat -version` 返回 Vivado 2018.3 64-bit，软件版本 2405991、IP 版本 2404404。`E:\Vivado\2018.3\bin\xsct.bat` 不存在；这只说明该候选路径错误，后续必须定位实际 SDK/XSDK 2018.3 入口。
- 脚本能力：materialize 目标声明为 selftest + soc_i2s；native check 要求 `xc7z020clg400-2`、top、自包含 `fpga/` 路径及 BD validate；flow 对 XSim marker、综合、`RAMB36>=24`、`RAM64M=0`、实现、DRC Error/Critical=0、WNS/WHS>=0、bit/HDF 设置硬门禁。
- PS 软件边界：SoC BD 明确 `PCW_EN_DDR=0`；XSDK Tcl 计划把 DDR section 改映射到 `ps7_ram_0`，并逐个 ELF LOAD 段要求落在 `0x00000000..0x00030000` OCM。该门禁尚未在本动作执行。
- 入口缺陷：`open_project.ps1` 当前打开 `fpga/build/` 中的临时 `.xpr`，不能作为用户要求的仓库原生 GUI 入口，必须改为打开 `fpga/projects/` 的已提交工程。
- 修改/构建：无；没有编辑、运行 Vivado flow、生成产物、暂存或提交。
- 验证边界：这里只证明工具存在和脚本意图，不证明脚本可成功执行、工程可打开、时序/DRC/ELF 合格或硬件工作。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：定位正确 SDK/XSCT，修复原生工程和 GUI 入口，再实际执行全部门禁。

### W-20260830-013：NUEDC_2026 自研 DSP 路径聚焦审计

- 时间：2026-08-30T14:37:28+08:00
- 关联要求/未决项：D-006、D-007、P2。
- 动作前审阅：`/root/nuedc_mcu_audit` 报告已完整读取含 W-009 的最新记录；记录代理追加前再次完整读取本文件。
- 计划动作：排除全仓 vendor 噪声，确认用户所称 `dsp/` 的真实大小写、位置、文件集、工程形态和根许可证状态。
- 命令/工具：对固定 SHA 使用 `git -c core.quotepath=false ls-tree`，结合受限 PowerShell 过滤枚举 DSP、自研源、README 和根许可证候选。
- 路径结果：真实目录是大写 `DSP/`，存在于 `STM32H743/DSP`、`STM32H743_AD9238/DSP`、`STM32H743_LTC2208/DSP` 和 `TI_MSPM0G3507/DSP`。
- H743 功能文件：基础集合包含 Adaptive、Correlate、DSP_ProMax、Demod、FFT、Filter、FilterEx、Fit、IQ、Measure、ModelFit、SoftPll、periodic_analyzer 的 `.c/.h` 与 `DSP_Header.h`；AD9238/LTC2208 变体另有 `ad9238_fir307_coeffs.h`。
- 工程形态：三个 H743 树都包含 BSP/Core/Drivers/DSP/Examples/MDK-ARM/Modules/output/tests，属于完整竞赛固件快照，而不是可直接整目录导入的单一可移植库。
- 许可证结果：根目录没有 LICENSE/COPYING/NOTICE/AUTHORS；仅有组件级 ST/CMSIS LICENSE 和子 README。自研 DSP 的再分发许可证元数据仍不明确，不能把 vendor 许可外推给这些文件；在公开 AUDIO_DSP 远端融合前需明确授权/许可证并保留来源。
- 工具限制：CMSIS DSP 路径仍造成部分输出截断，但本次已聚焦确认自研目录位置和根许可证缺失；不能因此声称所有 API/依赖均已审完。
- 修改范围：只读审计；没有修改正式文件、暂存、提交或推送。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：对四个自研 DSP 目录做 blob/hash 去重，审计 API、内部依赖、测试和代码风格，形成可融合清单。

### W-20260830-014：Ubuntu 24.04 SSH host-key 只读探测

- 时间：2026-08-30T14:38:18+08:00
- 关联要求/未决项：P1。
- 动作前审阅：主代理和记录代理均按门禁完整审阅最新记录，确认不得复用 Ubuntu 18 的 HostKeyAlias，并且本动作只扫描公钥。
- 计划动作：确认 `192.168.79.149` 是否已有 known_hosts 记录，获取当前 SSH 服务公钥指纹，为 Ubuntu 24 建立独立信任材料；不读取或输出任何私钥内容。
- 命令/工具：`ssh-keygen -F 192.168.79.149`、`ssh-keyscan -T 5`，以及仅枚举本机候选私钥的文件名。
- known_hosts 结果：当前没有 `192.168.79.149` 的既有条目。
- 公钥指纹：RSA `SHA256:MKWFUfI6y0cEU7yDzbpDF79SsWBJ/ws7IDYO7IzJ83I`；ECDSA `SHA256:6Mg/1lNUHNcwrUXaZv8RUMo752OQDmzY20JdNwx07vI`；ED25519 `SHA256:ltLz46cAbNGzsBOA6h4rRd+qJfR5xDahEus5U0dMcNA`。
- 本机身份候选：只枚举到文件名 `id_rsa`、`codex_vmware_ubuntu_ed25519`、`codex_jetson_usb_ed25519`；没有读取、打印或暴露任何私钥内容。
- 修改文件/系统：无；此次没有写 known_hosts、登录 Guest、修改 SSH 配置或仓库文件。
- 验证边界：`ssh-keyscan` 只取得服务端当前公开密钥，尚未通过 Guest 登录证明 `hry@ubuntu24` 身份；接受前仍要结合已匹配的 NAT IP/MAC，并使用 Ubuntu 24 独立条目。
- Git：无暂存、提交或推送。
- 下一步：在已匹配 IP/MAC 的 VMware NAT 内用 `accept-new` 建立 Ubuntu 24 独立 known_hosts 条目，再以 BatchMode 验证身份；绝不使用 Ubuntu 18 HostKeyAlias。

### W-20260830-015：NUEDC_2026 DSP 哈希去重、许可复核与 include 汇总失败

- 时间：2026-08-30T14:38:18+08:00
- 关联要求/未决项：D-006、D-007、D-009、P2。
- 动作前审阅：`/root/nuedc_mcu_audit` 报告已完整读取最新记录；记录代理追加前再次完整读取本文件。
- 计划动作：对四个自研 DSP 目录逐文件比较 SHA-256、字节数和行数，复核许可证标记并汇总 include 依赖，确定 canonical 源和平台差异。
- 数量结果：`STM32H743/DSP` 27 个文件，`STM32H743_AD9238/DSP` 28 个，`STM32H743_LTC2208/DSP` 28 个，`TI_MSPM0G3507/DSP` 23 个。
- 去重结果：三个 H743 树绝大多数同名文件完全相同；AD9238 与 LTC2208 的全部公共 DSP 内容相同。基础 H743 与这两个变体仅 `periodic_analyzer.c/h` 不同，后两者另共享 `ad9238_fir307_coeffs.h`。TI 版多数 `.c/.h` 有平台适配差异，但 `Fit.h`、`IQ.h`、`Measure.h` 与 H743 版本完全相同。
- 专有模块：H743 专有 `DSP_ProMax.*` 和 `FilterEx.*`；TI 专有 `ADC_FFT.*`。
- 许可证结果：四个自研 DSP 目录仍未检出 SPDX/license/copyright 标记，进一步确认不能在公开 AUDIO_DSP 中直接复制这些源码，必须先明确授权/许可证元数据和来源记录。
- 失败记录：命令最后用 `rg -h` 试图汇总 include；本机 ripgrep 将 `-h` 解释为 help，输出帮助文本，include 依赖汇总没有完成。此失败保留，不由前述哈希成功掩盖。
- 修改范围：只读审计；没有修改正式工程，没有暂存、提交或推送。
- 核心决策：融合时只选一个 H743 canonical 版本，禁止把三份重复副本全部导入；平台差异通过显式适配层或经测试的模块选择处理。
- Git：AUDIO_DSP 无新增提交/推送。
- 下一步：改用 `rg --no-filename` 重试 include 依赖汇总，再审计 API/内部依赖、测试覆盖和代码风格。

### W-20260830-016：Ubuntu 24.04 独立 SSH 身份核验通过

- 时间：2026-08-30T14:42:00+08:00
- 关联要求/未决项：P1。
- 动作前审阅：主代理分两段完整审阅最新工作记录，确认当前 IP、MAC、端口和公开 host-key 指纹已记录，且不得复用 Ubuntu 18 的 `HostKeyAlias`。
- 计划动作：用非交互公钥认证首次接受且只记录 `192.168.79.149` 的 Ubuntu 24 ED25519 host key，然后只读核验 Guest 身份；任一身份不符即停止。
- 命令/工具：Windows OpenSSH；指定 `codex_vmware_ubuntu_ed25519`、`IdentitiesOnly=yes`、`BatchMode=yes`、`ConnectTimeout=8`、`StrictHostKeyChecking=accept-new`，远端只执行 `id -un`、`hostname`、读取 `/etc/os-release`、`uname -r` 和 `hostname -I`。
- 结果与原始证据：首次把该地址的 ED25519 key 加入用户 known_hosts；登录成功，返回 `USER=hry`、`HOST=ubuntu24.04`、`VERSION_ID=24.04`、`PRETTY_NAME=Ubuntu 24.04.4 LTS`、`KERNEL=7.0.0-30-generic`、`IPV4=192.168.79.149`。
- 身份边界：没有使用 Ubuntu 18 的旧 `HostKeyAlias`；该结果与此前已匹配的 VMware NAT IP/MAC 和预扫描 ED25519 指纹共同建立本轮 Ubuntu 24 身份链。
- 修改文件/系统：仅 OpenSSH 按 `accept-new` 写入当前 Windows 用户的 known_hosts 条目；Guest 无文件或系统修改，仓库除本记录外无修改。
- 验证边界：只证明 SSH 身份和 OS；尚未证明 GUI 卡顿原因、资源健康、交叉工具链、BSP/sysroot、ALSA ABI 或 AUDIO_DSP 构建可用。
- 记录代理状态：专职记录代理本轮因服务额度暂停；主代理依照既定模板追加本条，待记录代理恢复后继续由其汇总，不以此取消专职记录要求。
- Git：本条未暂存、未提交、未推送。
- 下一步：通过该已核验连接执行只读资源、服务、日志和开发工具链盘点，再与桌面 i.MX6ULL 资料逐项对齐。

### W-20260830-017：Ubuntu 24 资源、GUI 故障和交叉开发环境只读盘点

- 时间：2026-08-30T14:44:08+08:00
- 关联要求/未决项：P1。
- 动作前审阅：主代理分两段完整审阅含 W-016 的最新记录，确认本动作只读、不安装包、不修改服务或用户配置。
- 命令/工具：通过已核验 SSH 连接一次性执行 `uptime`、`free`、`swapon`、`df`、`vmstat`、`systemctl`、进程排序、boot error journal、工具命令/版本、`dpkg-query`、multiarch、显式环境变量、共享目录和有限深度的 BSP/SDK 路径搜索。
- 资源证据：Guest 实见 4 vCPU、3.8 GiB RAM、1.8 GiB available、5.3 GiB swap（仅 64 KiB 使用）、根盘 98 GiB 中使用 18 GiB（20%）、inode 6%；三次 `vmstat` 的 CPU idle 为 99/100/100%，无持续 I/O wait。当前没有资源耗尽证据。
- 服务证据：system 级 failed unit 为 0；`open-vm-tools`、GDM/display-manager 和 SSH 均 active。GNOME Shell、Xorg、用户态 vmtoolsd 均运行；当前较大进程为 gnome-shell 约 276 MiB、update-manager 约 204 MiB，没有持续高 CPU 进程。
- GUI/终端故障证据：本次 boot journal 明确记录 `gnome-terminal-server.service` 与 `terminal-warmup.service` 启动失败，同时有 GNOME keyring、IBus session 和 `xdg-desktop-portal` 失败；因此用户报告的“终端打不开”有可复现方向，不能归因于当前 CPU/磁盘耗尽。进程快照后来又出现 gnome-terminal server，仍需读取用户单元状态和详细 journal 确认是否自恢复及根因。
- native 工具：git 2.43、CMake 3.28.3、Ninja 1.11.1、Make 4.3、GCC/G++ 13.3、Python 3.12.3、pkg-config 1.8.1、ALSA utils/aplay 1.2.9 均存在。
- ARMHF 工具：`arm-linux-gnueabihf-gcc` 13.3 和 binutils/readelf/objdump 存在；`-dumpmachine` 为 `arm-linux-gnueabihf`，但 `-print-sysroot` 为 `/`。`arm-linux-gnueabihf-g++` 缺失，说明迁移并不完整。
- ALSA/ABI 缺口：`libasound2-dev` 与 `libasound2-dev:armhf` 都缺失，`pkg-config alsa` 失败；没有 foreign architecture；QEMU user 工具缺失。Ubuntu 24 原生工具不能替代板卡 Buildroot glibc 2.30 / ALSA 1.2.1.2 目标 sysroot。
- 路径结果：没有 `/mnt/hgfs`；明确环境变量 `CROSS_COMPILE/ARCH/SYSROOT/SDKTARGETSYSROOT/OECORE_TARGET_SYSROOT/PKG_CONFIG_SYSROOT_DIR` 都为空；发现 `/home/hry/toolchains` 与 `/home/hry/imx6ull`，`/opt` 和 `/usr/local` 未发现候选。非交互 sudo 不可用，任何包安装都不能假定自动提权。
- 修改文件/系统：Guest 无修改；仓库仅追加本记录。
- 验证边界：尚未读取用户 service 的详细失败原因，也未核验 `/home/hry/toolchains` 和 `/home/hry/imx6ull` 的来源、版本、完整性或与桌面 BSP/rootfs 的 ABI 一致性。
- Git：本条未暂存、未提交、未推送。
- 下一步：只读检查 GNOME Terminal 用户单元和 journal，同时枚举现有 toolchains/imx6ull 内容、编译器 sysroot、BSP/rootfs 哈希；确定最小修复集后再做系统修改。

### W-20260830-018：GNOME Terminal 用户单元与现有 i.MX6ULL 迁移目录深查

- 时间：2026-08-30T14:47:00+08:00
- 关联要求/未决项：P1。
- 动作前审阅：主代理分两段完整审阅含 W-017 的最新记录；确认继续只读，不禁用服务、不删除文件、不解包归档。
- 命令/工具：通过已核验 SSH 查询 `loginctl` 会话、四个用户单元的 show/status/cat、terminal/portal/IBus user journal、用户 unit 文件、终端进程；并用受限 `du/find/git` 枚举 `/home/hry/toolchains` 与 `/home/hry/imx6ull`。
- 会话结果：本地图形会话为 active X11，另有本轮 SSH tty；用户 DBus 和 `/run/user/1000` 可用。
- 终端根因：唯一自定义用户单元是 `/home/hry/.config/systemd/user/terminal-warmup.service`（时间 2026-08-24），其 `ExecStart` 为 `systemctl --user start gnome-terminal-server` 且被挂到 `default.target`。启动 14:18:03 时 gnome-terminal-server 明确报 `Failed to parse arguments: Cannot open display:`，warmup 随之失败。该设计在图形会话环境变量尚未进入 user manager 时存在确定竞态。
- 自恢复证据：gnome-terminal-server 在 14:20:58 后为 active/running，portal 同时恢复 active；GNOME Terminal 版本 3.52.0，当前有 `gnome-terminal --wait`、real wrapper 和 server 进程。故早期打不开有证据，但当前服务已自恢复，尚未执行 GUI 点击验收。
- 其他用户单元：IBus unit 失败原因为 `current session already has an ibus-daemon`，属于重复启动；portal 初次等待 GNOME backend 超时，后来成功。本动作没有把这些次生状态误判为 CPU/内存不足。
- 迁移目录：`/home/hry/toolchains` 总计仅 64 KiB，只有 `arm-gcc.tar.xz`，字节数 58,351；限定搜索未发现任何 `*gcc`、`*g++` 或 `environment-setup-*`。这不能视为已安装或完整的厂家 SDK。
- BSP 目录：`/home/hry/imx6ull` 约 977 MiB，明确包含 `Linux-4.9.88`；限定 rootfs/sysroot/archive 搜索没有发现可用目标 rootfs 或 SDK，只命中内核文档/脚本等。尝试对发现的 `.git` 候选执行 `git -C` 返回 not a git repository，因此当前不能用 Git 元数据证明来源/版本。
- 输出边界：内核树枚举输出过长被工具截断；本动作足以证明终端竞态、目录大小和缺少 SDK/compiler 候选，但不能据此声称内核树每个文件或归档内容已审完。
- 修改文件/系统：Guest 无修改；仓库仅追加本记录。
- Git：本条未暂存、未提交、未推送。
- 下一步：先用 `file/xz -t/tar -tf/hash` 核验 58,351 字节归档是否损坏；与 Windows 桌面板卡资料的 BSP/rootfs/工具链逐项对照。终端修复候选是保留 unit 文件但禁用有竞态的 warmup，并重启/验证用户终端服务。

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
### W-019 — i.MX6ULL 桌面资料与虚拟机工具链归档核验（只读）

- 执行前状态：已按本记录约束完整复核 `docs/PROJECT_WORKLOG.md` 后再开始本阶段。
- 用户资料入口：`C:\Users\LENOVO\Desktop\imx6ull.lnk` 指向 `D:\Desktop_Archive\imx6ull`；顶层包含 Linux 4.9.88 源码、`files`、手册/工具、NXP Yocto 源码归档及多份烧写工具资料。
- 主根文件系统镜像：`D:\Desktop_Archive\imx6ull\files\rootfs.ext4`，大小 `734003200` 字节，SHA-256 为 `a0a393b4e7bdd6f58314156e1ac4e4a533f8346c18aa82f883189d2bfc4a5921`。烧写工具目录中还存在若干 `rootfs.ext4`、UBI、JFFS2、IMG 候选副本；本次未逐一散列，因此不宣称它们彼此相同。
- 未在候选归档列表中发现明显可直接使用的 Buildroot SDK/交叉工具链包；可见的主要是 NXP Yocto 源码包和根文件系统镜像。
- 板级音频证据来自 `Linux-4.9.88\arch\arm\boot\dts\100ask_imx6ull-14x14.dts`：WM8960 位于 I2C 地址 `0x1a`，CPU DAI 使用 SAI2，含 SAI2 时钟/引脚配置，分配时钟为 `12288000` Hz。
- `100ask_imx6ull_defconfig` 启用 `CONFIG_SND_SOC_FSL_SAI=y` 与 `CONFIG_SND_SOC_WM8960=y`，但未启用 `CONFIG_SND_SOC_FSL_ASOC_CARD`；后续工程必须以该板实际 machine driver/设备树为准，不能把通用声卡配置当作已验证板级配置。
- 虚拟机中的 `/home/hry/toolchains/arm-gcc.tar.xz` 不是工具链：大小 `58351` 字节，SHA-256 为 `40e71db1eeedc62d2542e28c76ea1c6b1f552fecdb425672f657c823f489b61e`，文件内容以 `<!DOCTYPE html>` 开始；`xz -t` 返回 1（格式无法识别），`tar` 列表返回 2 且无成员。
- 结论：该 `.tar.xz` 是误命名的 HTML 下载页/错误页，禁止解压或加入构建路径。暂不删除，保留为迁移失败证据；后续应取得真实工具链/目标 sysroot，再做交叉编译验证。
- 本阶段全部为只读检查，没有修改桌面资料、虚拟机归档或 DSP 源码。

### W-020 — 禁用有竞态的 GNOME Terminal warmup 用户单元

- 时间：2026-08-30T14:50:13+08:00
- 关联要求/未决项：P1；用户报告 Ubuntu 24.04 图形终端打不开。
- 动作前审阅：主代理分两段完整读取含 W-019 的最新记录，确认此前已建立 Ubuntu 24 独立 SSH 身份链，且根因是用户自建 warmup 在 DISPLAY 导入前强启 terminal server；本动作限于 `hry` 用户级 systemd，不使用 sudo。
- 计划动作：保留 `~/.config/systemd/user/terminal-warmup.service` 文件作为证据，只撤销其登录自动启用；清除相关 failed 状态、重新加载用户 manager，并确认 GNOME Terminal 服务保持可用。
- 命令/工具：通过已核验 SSH 执行 `systemctl --user disable --now terminal-warmup.service`、`reset-failed`、`daemon-reload`、`start gnome-terminal-server.service`，随后查询 enable/active/failed 状态、全部 failed user units、源文件和 wants 链接。
- 修改结果：`terminal-warmup.service` 从 `enabled` 变为 `disabled`，移除了 `/home/hry/.config/systemd/user/default.target.wants/terminal-warmup.service`；源文件仍保留。没有修改系统级服务、软件包、BSP、rootfs 或 AUDIO_DSP 源码。
- 服务证据：执行前 warmup 为 `enabled/inactive`、terminal server 为 `active`；执行后 warmup 为 `disabled/inactive`、terminal server 为 `active`，`systemctl --user --failed` 无条目。`systemctl --user is-failed gnome-terminal-server.service` 输出 `active`，说明它不在 failed 状态。
- 失败/提示保留：禁用后再对 warmup 执行 `reset-failed` 返回 `Unit terminal-warmup.service not loaded`；这是单元已卸载后的状态清理提示，未阻止后续 daemon reload、terminal start 和状态验证成功。
- 验证边界：已消除已定位的登录竞态并验证用户服务状态，但 SSH 状态检查不能代替用户在桌面点击/快捷键打开终端的 GUI 验收，也尚未验证重启后的登录路径。
- Git：本条未暂存、未提交、未推送。
- 下一步：核对图形会话环境后做一次无交互终端启动 smoke test；再处理损坏工具链、目标 rootfs/sysroot 和交叉依赖。

### W-021 — Ubuntu 24.04 GNOME Terminal 图形会话 smoke test

- 时间：2026-08-30T14:51:38+08:00
- 关联要求/未决项：P1；W-020 后的图形终端验收。
- 动作前审阅：主代理分两段完整读取含 W-020 的 535 行工作记录；确认允许在当前图形会话短暂启动终端，但临时文件必须精确创建、验证并清理。
- 计划动作：从正在运行的 `gnome-shell` 进程读取当前用户的 DISPLAY、DBus 和 Xauthority 环境，通过 `gnome-terminal --wait` 启动一次短命令，在 `mktemp` 创建的独立 `/tmp/audio_dsp_terminal_smoke.*` 目录写标记，验证后由 trap 精确清理；同时只读确认 rootfs 检查工具和远端目标状态。
- 原始证据：GNOME Shell PID `2501`，`DISPLAY=:0`，DBus 为 `unix:path=/run/user/1000/bus`，Xauthority 存在；终端子命令成功产生 `GUI_TERMINAL_SMOKE_OK`，结果 `GUI_TERMINAL_SMOKE_OK=1`。
- 服务复核：`gnome-terminal-server.service=active`、`terminal-warmup.service=disabled`、用户级 failed unit 数为 0。
- 清理：由 shell trap 删除本动作创建的临时 result 文件并移除精确的 `mktemp` 目录；未删除用户原有文件。
- rootfs 前置结果：VM 已有 `/usr/sbin/debugfs` 和 `/usr/bin/file`；`/home/hry/imx6ull/files` 及其 `rootfs.ext4` 当前均不存在。
- 修改范围：除短暂打开/关闭一个 GNOME Terminal 窗口和已清理的 `/tmp` 临时文件外无持久 Guest 修改；仓库只追加本记录。
- 验证边界：证明当前登录会话中的 GNOME Terminal 启动与命令执行链可用；尚未通过注销/重启验证下次登录，也不代表交叉开发环境已修复。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后，把已核验哈希的 100ASK `rootfs.ext4` 复制到用户目录，复制前后校验大小与 SHA-256，再用 `debugfs` 只读提取 ABI/ALSA/sysroot 证据。

### W-022 — rootfs 安全传输脚本首次尝试在 PowerShell 解析阶段失败

- 时间：2026-08-30T14:52:54+08:00
- 关联要求/未决项：P1；把开发板实际 rootfs 引入 Ubuntu 24 作为 ABI/sysroot 证据。
- 动作前审阅：主代理分两段完整读取含 W-021 的 550 行记录，确认只能向精确用户目录写入、不得覆盖已有镜像，且必须使用临时名、前后散列和失败清理。
- 计划动作：本地复核大小/散列，远端 preflight，SCP 到 `rootfs.ext4.part-codex-20260830`，校验一致后原子改名。
- 失败：PowerShell 在解析包含远端 shell 校验语句的 `$verify` 字符串时报告 `Unexpected token`；错误位于嵌套的 `` `$actual_size`` / 引号组合。该脚本在 PowerShell 解析期即中止。
- 修改范围：由于解析失败发生在任何语句执行之前，本地 `Get-FileHash`、SSH preflight、`mkdir`、SCP 和远端校验均未运行；Windows 和 Guest 均无文件/目录变化。
- 验证边界：没有发生传输，也没有新增 rootfs 校验证据；后续成功不得覆盖本失败记录。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后，以分步、低嵌套命令重试；每一步单独检查 `$LASTEXITCODE`，避免复杂 PowerShell/远端 shell 引号。

### W-023 — 100ASK rootfs 镜像安全传输与端到端散列通过

- 时间：2026-08-30T14:54:05+08:00
- 关联要求/未决项：P1、D-004；建立与开发板资料一致的目标 ABI/sysroot 基线。
- 动作前审阅：主代理分两段完整读取含 W-022 的 562 行记录；确认保留首次失败、禁止覆盖已有目标，并采用低嵌套的四阶段传输。
- 本地源：`D:\Desktop_Archive\imx6ull\files\rootfs.ext4`；重新核验大小 `734003200` 字节、SHA-256 `a0a393b4e7bdd6f58314156e1ac4e4a533f8346c18aa82f883189d2bfc4a5921`，与 W-019 一致。
- 远端 preflight：明确要求最终文件和临时文件均不存在后才创建 `/home/hry/imx6ull/files`；可用空间 `80867684352` 字节，满足传输要求。
- 传输方法：SCP 先写 `/home/hry/imx6ull/files/rootfs.ext4.part-codex-20260830`；若 SCP 失败，脚本只删除该精确临时文件。此次 SCP 成功。
- 传输后验证：临时文件大小 `734003200`、SHA-256 与预期完全一致；验证通过后用 `mv` 原子改名为 `/home/hry/imx6ull/files/rootfs.ext4`。最终文件大小与 SHA-256 再次完全一致，输出 `ROOTFS_TRANSFER_OK=1`。
- 文件类型证据：Guest `file` 识别该镜像为 Linux rev 1.0 ext4 filesystem，UUID `491f6117-415d-4f53-88c9-6e0de54deac6`，含 extents/large files/huge files 特性。
- 修改范围：Guest 新增目录 `/home/hry/imx6ull/files` 和一个经散列核验的 `rootfs.ext4`；没有删除/覆盖原文件，没有修改系统包、BSP 源码或 AUDIO_DSP 源码。
- 验证边界：只证明镜像传输完整和文件系统类型；尚未证明镜像内部 ABI、ALSA 文件、开发头或可直接作为链接 sysroot。
- Git：镜像位于 VM 用户目录，不属于 AUDIO_DSP Git；本条未暂存、未提交、未推送。
- 下一步：完整复核记录后用 `debugfs` 只读检查镜像 superblock、Buildroot 标识、ELF 解释器/glibc、ALSA 库和开发头；据此决定提取 sysroot 的最小范围与交叉编译器兼容方案。

### W-024 — rootfs 内部 Buildroot、glibc 与 ALSA 开发能力审计

- 时间：2026-08-30T14:55:38+08:00
- 关联要求/未决项：P1、D-004；区分目标运行时 rootfs 与可编译 SDK/sysroot。
- 动作前审阅：主代理分两段完整读取含 W-023 的 577 行记录；确认镜像只读、不挂载、不修改，先判断是否具备目标 ABI 库和开发文件。
- 文件系统证据：`debugfs stats` 返回 UUID `491f6117-415d-4f53-88c9-6e0de54deac6`、状态 clean、块大小 4096、179200 blocks/44832 inodes；创建和最后写入时间均为 2021-05-18。
- 系统身份：`/usr/lib/os-release` 为 `NAME=Buildroot`、`VERSION=2020.02-g6a1d083-dirty`、`PRETTY_NAME="Buildroot 2020.02"`；`/etc/issue` 明确显示 `Welcome to imx6ull buildroot system !`、weidongshan/100ASK 来源。
- 保留的读取提示：对 `/etc/os-release` 直接执行 `debugfs cat` 报 `short read`，与其链接形态一致；本动作没有用该失败覆盖成功读取的 `/usr/lib/os-release`。`/etc/buildroot-release` 不存在。
- glibc/ABI 文件：`/lib/ld-linux-armhf.so.3 -> ld-2.30.so`，真实 loader 大小 158748；`/lib/libc.so.6 -> libc-2.30.so`，真实 libc 大小 1241860。另见 `libm-2.30.so`、`libpthread-2.30.so`、`librt-2.30.so`。
- ALSA 运行时：`/usr/lib/libasound.so` 与 `.so.2` 均指向 `libasound.so.2.0.0`，真实库大小 758792；`/usr/lib/alsa-lib` 目录和 `/usr/bin/aplay`（55288 字节）存在。
- 开发文件缺口：镜像中 `/usr/include` 整体不存在，因此 `/usr/include/alsa/asoundlib.h` 不存在；`/usr/lib/pkgconfig/alsa.pc` 与 `/usr/share/pkgconfig/alsa.pc` 也都不存在。
- 附带结果：`/bin/busybox` 不存在，不能假定该 Buildroot 使用单一 BusyBox 路径；这不影响已确认的 ELF/ALSA 文件结论。
- 结论：该镜像可作为目标运行库与 ABI 基线，但不是完整编译 SDK/sysroot。必须从匹配的 Buildroot 输出/SDK取得目标头文件和链接元数据，或构造明确版本匹配的补充开发 sysroot；不能拿 Ubuntu 24 的 ARMHF headers 伪装成板卡 SDK。
- 修改范围：对镜像和 Guest 均为只读；仓库只追加本记录。
- 验证边界：尚未对 ELF header/ARM attributes、`aplay` 的动态依赖和 ALSA 源码版本字符串做二进制验证，也尚未建立可链接 sysroot。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后只读导出少量 ELF 到临时目录，使用 `readelf/strings` 验证 EABI5 hard-float、解释器、NEEDED/GLIBC 版本和 ALSA 工具版本，然后精确清理临时目录。

### W-025 — rootfs ELF hard-float、依赖与 ALSA 版本核验

- 时间：2026-08-30T14:56:58+08:00
- 关联要求/未决项：P1、D-004；确认可执行文件/运行库真实 ABI 和链接合同。
- 动作前审阅：主代理分两段完整读取含 W-024 的 595 行记录；确认只从镜像导出四个指定 ELF 到 `mktemp` 目录，使用非递归精确清理。
- 临时导出：`/usr/bin/aplay` 55288 字节、`/lib/ld-2.30.so` 158748、`/lib/libc-2.30.so` 1241860、`/usr/lib/libasound.so.2.0.0` 758792；大小与镜像 stat 一致。
- `aplay` ABI：ELF32 little-endian ARM executable，EABI5 hard-float；目标 CPU attribute 为 Cortex-A7/ARMv7 Application profile，VFPv4、NEONv1 Fused-MAC，`Tag_ABI_VFP_args=VFP registers`。`file` 报目标 GNU/Linux 4.9.0。
- 动态加载合同：程序解释器为 `/lib/ld-linux-armhf.so.3`；`aplay` NEEDED 包括 `librt.so.1`、`libasound.so.2`、`libatopology.so.2`、`libm.so.6`、`libdl.so.2`、`libpthread.so.0`、`libc.so.6`。其 GLIBC symbol versions 为 2.4、2.17、2.28。
- `libasound` ABI：ELF32 little-endian ARM DYN，EABI5 hard-float，Cortex-A7/VFPv4/NEON/VFP-register ABI；SONAME `libasound.so.2`，依赖 libc、libm、libdl、libpthread、librt 和 loader。
- 版本证据：字符串中出现 `1.2.1.2`（ALSA library）和 `%s: version 1.2.1 by Jaroslav Kysela...`（aplay/alsa-utils）；libc version info 的最高条目为 GLIBC 2.30，与文件名一致。
- 清理证据：只删除本动作在 `/tmp/audio_dsp_rootfs_elf.gM4biX` 创建的四个导出文件并 `rmdir`；随后 `test ! -e` 通过，输出 `TEMP_CLEANUP_OK=1`。
- 结论：目标是 i.MX6ULL Cortex-A7 的 ARMv7 hard-float ABI，不能用软浮点或 AArch64 工具链；链接/打包验证必须保留 `/lib/ld-linux-armhf.so.3`、glibc 2.30 与 ALSA 1.2.1.2 合同，并检查 `libatopology.so.2` 等间接依赖。
- 修改范围：镜像保持只读；Guest 无持久文件变化；仓库只追加本记录。
- 验证边界：仍缺与该 Buildroot 输出匹配的 C 开发头、启动对象和链接脚本/SDK；二进制 ABI 证据不能代替一次成功的目标全链接或板端运行。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后查找桌面 Buildroot 2020.02 的源码/config/output 线索和官方/100ASK SDK 获取路径；优先恢复匹配 SDK，否则重建同版 Buildroot SDK，再运行 AUDIO_DSP ARMHF 全链接。

### W-026 — 桌面 i.MX6ULL 资料中的 Buildroot/SDK/toolchain 全量名称与配置检索

- 时间：2026-08-30T14:58:19+08:00
- 关联要求/未决项：P1；优先使用用户开发板资料恢复匹配 SDK。
- 动作前审阅：主代理分两段完整读取含 W-025 的 612 行记录；确认本动作仅对 `D:\Desktop_Archive\imx6ull` 做文件名和受限文本检索，不解压、不修改资料。
- 文件总量：`rg --files` 枚举 58629 个文件。
- 名称候选：仅 7 个；其中 5 个位于 `06_NXP官方参考学习资料\Buildroot学习参考资料`，分别是一个 89 字节网址文本和四份 Buildroot 教学 PDF；其余两个是 Linux 4.9.88 源码内的 `ARM-gcc.h` 与 `toolchain-flags`，均不是 SDK。
- 配置文本检索：在 `.config`、`*defconfig*`、`.mk`、`.sh`、`.txt`、`.md`、`.cfg`、Makefile 和 CMake 文件内搜索 `BR2_TOOLCHAIN`、`BR2_PACKAGE_ALSA_LIB`、`arm-buildroot-linux-gnueabihf`、`Buildroot 2020.02`、`environment-setup-`，命中文件数为 0。
- 归档候选：关键词归档只有 `07_裸机教程及源码\6_Makefile与GCC.zip`，大小 20233 字节；名称、大小和路径均表明它是教程附件，不是交叉工具链/SDK。
- 结论：当前桌面资料没有可识别的 Buildroot source/output、SDK、sysroot、environment setup 或 ARM Buildroot GCC 归档。Linux 内核树与 NXP Yocto 源码包不能替代生成该 rootfs 的 Buildroot SDK。
- 修改范围：纯只读；没有解压、写文件、下载、修改 Guest 或仓库源码。
- 验证边界：还未读取 89 字节参考网址，也尚未查询 100ASK 官方仓库/发布件；不能据此断言互联网上不存在匹配来源。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后读取参考网址，并通过 100ASK/weidongshan 与 Buildroot 官方来源定位 2020.02 板级配置或可重建 SDK 的固定提交。

### W-027 — 开发板资料指定网址与 100ASK 官方来源初筛

- 时间：2026-08-30T15:00:52+08:00
- 关联要求/未决项：P1、D-004；必须先依据开发板资料补齐 Ubuntu 24 的匹配开发环境，再继续 MPU DSP。
- 动作前审阅：主代理分两段完整读取含 W-026 的 627 行工作记录；确认先读取资料明确给出的来源，再限于 100ASK/weidongshan、DongshanPI 与 Buildroot 官方页面核查，不采用第三方打包工具链替代板卡 SDK。
- 本地来源：读取 `D:\Desktop_Archive\imx6ull\06_NXP官方参考学习资料\Buildroot学习参考资料\参考网址.txt`，内容仅为 `http://wiki.100ask.org/Buildroot`、`https://buildroot.org/`、`https://buildroot.org/docs.html`。
- 官方来源初筛：weidongshan 的 Gitee 主页固定项目说明显示其 Buildroot 仓库基于官方 Buildroot，并为 100ASK STM32MP157 与 i.MX6ULL 开发板适配及增加自定义软件包；尚需打开该项目链接，固定准确仓库 URL、分支和提交。
- SDK 目录结构旁证：DongshanPI 的 `lv_100ask_linux_desktop` 项目文档给出旧版 100ASK i.MX6ULL SDK 路径 `~/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot`；这与当前 rootfs 的 ARMHF/Buildroot 证据一致，并证明应恢复带开发头、启动对象和 sysroot 的 Buildroot SDK，而不是只用 Ubuntu 裸交叉编译器。
- 官方资料获取线索：100askTeam 的 `Linux-doc_and_source_for_drivers` 项目指向 `https://e.coding.net/weidongshan/linux/doc_and_source_for_drivers.git` 和 `http://download.100ask.net/`；它们是后续核查官方发行材料的入口，但本动作未假定其中一定含匹配 SDK。
- 失败/保留：直接打开 100ASK Wiki 页面时工具返回内部错误；该页面内容尚未取得，不能把搜索摘要当成已验证页面正文。
- 修改范围：只读本地 89 字节文本并浏览公开页面；未克隆、下载、安装或修改 Windows、Guest、开发板资料和 AUDIO_DSP 源码；仓库只追加本记录。
- 验证边界：尚未确定 `Buildroot 2020.02-g6a1d083-dirty` 中 `6a1d083` 是否存在于 100ASK 仓库，也未取得与 2021-05-18 rootfs 完全匹配的 `.config` 或 SDK。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后打开 Gitee 固定项目，获取准确 clone URL，并用 `git ls-remote`/受控克隆检查提交 `6a1d083`、i.MX6ULL defconfig、ALSA 配置和 SDK 生成路径。

### W-028 — 100ASK Buildroot 直接访问与远端探测失败（保留）

- 时间：2026-08-30T15:36:00+08:00
- 关联要求/未决项：P1、D-004；取得与板卡 rootfs 匹配的 Buildroot 源码/SDK。
- 动作前审阅：主代理已分两段完整读取 W-027 后的 642 行记录；确认本动作只做公开来源读取和 Git 只读远端探测，不输入凭据、不修改本地/Guest。
- 计划动作：点击 weidongshan Gitee 主页固定项目链接，直接打开 `https://gitee.com/weidongshan/Buildroot`，再以 `git ls-remote --symref` 查询公开仓库的分支/提交。
- 页面结果：浏览工具点击返回 HTTP 405 Method Not Allowed；直接打开返回 HTTP 400 timeout。页面正文和准确可用 clone URL 均未取得，不能把主页链接视为已下载来源。
- Git 结果：`git ls-remote --symref https://gitee.com/weidongshan/Buildroot.git HEAD` 及 heads/tags 探测触发 Gitee 用户名提示；在非交互环境中因无 TTY/未提供凭据中止，返回 `could not read Username for 'https://gitee.com'`。没有输入、保存或暴露任何凭据，也没有建立克隆目录。
- 修改范围：无本地文件、Guest、桌面资料或仓库源代码修改；仅追加本记录。未删除失败残留（未生成残留目录）。
- 验证边界：该失败只说明当前 Gitee HTTPS 入口在本环境不可直接匿名读取；不说明仓库不存在，也不证明 `6a1d083` 是否匹配 rootfs。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后，使用公开网页搜索/官方 GitHub/Coding/下载中心线索定位同一 Buildroot fork 或可验证 SDK；先用 `git ls-remote`/HTTP HEAD 做只读可达性检查，再决定是否在 VM 用户目录受控克隆。

### W-029 — 通过 Gitee API 确认 100ASK Buildroot 仓库与 2020.02 分支

- 时间：2026-08-30T15:36:00+08:00（后续查询同一阶段）
- 关联要求/未决项：P1、D-004；定位与 rootfs `Buildroot 2020.02-g6a1d083-dirty` 相符的官方来源。
- 动作前审阅：主代理已完整分块审阅 W-028 后的工作记录；本阶段仍限定为公开网页/API 只读查询，不输入 Gitee 凭据、不克隆或修改任何工程。
- 搜索结果：公开搜索再次定位 `weidongshan/Buildroot`、100ASK 驱动资料仓库及 100ASK SDK 文档；搜索摘要仅作导航，未当作版本证据。
- Gitee 页面/API 结果：`Invoke-WebRequest https://gitee.com/weidongshan/Buildroot` 返回 HTTP 200；页面元数据给出 clone URL `https://gitee.com/weidongshan/Buildroot.git`。公开 API `https://gitee.com/api/v5/repos/weidongshan/Buildroot` 返回 `private=false`、`public=true`、描述为基于官方 Buildroot 适配 100ASK STM32MP157/i.MX6ULL、仓库许可证字段为 `GPL-2.0`、默认分支 `master`、最近推送 `2021-09-01T17:38:09Z`。
- 分支证据：API `.../branches` 返回 `2020.02.x`，头提交 `b0a4559fe452bc5586cab39bc984f9a81337219d`；`master` 头为 `85b7ab8d5b17b179777f8ab613df127f25d4cb27`。API tags 当前为空。
- 文件/下载结果：`.../contents/configs?ref=master` 可公开读取并返回大量 Buildroot config 文件；`raw/master/README` 返回 404（README 路径/分支形态待进一步确认）。仓库 archive URL 返回 Gitee 下载 HTML/captcha 页面而非 ZIP，未保存或解包。
- 提交定位结果：API 对缩写 `6a1d083` 返回 404；这尚不能排除它是官方 Buildroot 基础提交、非该 fork 公开分支提交或由 dirty tree 生成的 describe 缩写。
- 修改范围：无本地/Guest/桌面资料/仓库源修改；没有建立下载归档或凭据文件，仅追加本记录。
- 验证边界：已确认官方仓库身份、许可证字段和 2020.02.x 分支，但尚未证明 `b0a4559` 与 rootfs 完全一致，尚未取得该分支的 `.config`、SDK 归档或 `6a1d083` 对应提交。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后查询官方 Buildroot GitHub 的 `6a1d083`、Gitee 分支提交树和 i.MX6ULL 配置/包路径；若 API 足够，采用逐文件 raw 下载或在 VM 用户目录受控获取源码，避免 Gitee archive/captcha。

### W-030 — 桌面 Buildroot 教程 PDF 与本地 PDF 工具链审阅

- 时间：2026-08-30T15:36:00+08:00（同一资料审阅阶段）
- 关联要求/未决项：P1、D-004；依据桌面资料确认 Buildroot SDK 生成方式和可迁移环境要求。
- 动作前审阅：主代理已完整分块读取 W-029 后的 670 行记录；按 PDF 技能要求先读取本地 PDF 技能说明，确认本阶段只读、不生成/编辑 PDF，不需 artifact marker。
- 资料枚举：`D:\Desktop_Archive\imx6ull\06_NXP官方参考学习资料\Buildroot学习参考资料` 中有四份 PDF：`buildroot-slides.pdf`（5,287,700 B）、`Buildroot2.pdf`（713,045 B）、`Getting-Started-With-Buildroot-Slides-ELC2018.pdf`（1,966,760 B）和 `The Buildroot user manual.pdf`（1,671,764 B）。
- 工具发现：系统默认 `py -3` 指向 Python 3.9 且无 `pdfplumber`；系统未提供 `pdftotext`，但 `pdfinfo`/`pdftoppm` 位于 Codex bundled Poppler。Python 3.13（`C:\ProgramData\miniconda3\python.exe`）已具备 `pdfplumber 0.11.10`、`pypdf 6.14.2`、`PyMuPDF 1.27.2.3`，可直接做文本/页级审阅。
- 失败保留：首次用 Python 3.9 提取时返回 `ModuleNotFoundError: No module named 'pdfplumber'`；第二次批量提取遇到 Windows GBK 输出无法编码 `U+25B6`，在第二份 PDF 中止；随后改用 Python 3.13、UTF-8/replace 重新提取，未修改任何 PDF。
- 资料证据（Buildroot2/ELC 讲义）：Buildroot 的 `output/host` 包含主机工具链和 `<tuple>/sysroot`，sysroot 提供目标 headers/libraries；target 目录通常去掉开发头；内部/外部 toolchain 后端、`O=` out-of-tree 构建、`make savedefconfig`、`BR2_PACKAGE_ALSA` 等机制均被明确说明。该证据支持“必须恢复 SDK/sysroot，不能只拿运行时 rootfs”的既有决策。
- 资料边界：这些 PDF 主要是通用 Buildroot 教程（部分内容基于 2011.11/2019.02），没有发现能直接证明当前 `Buildroot 2020.02-g6a1d083-dirty` 配置的板卡专用 SDK；批量输出过长被工具截断，后续如需引用具体页应按文件分开提取/渲染。
- 修改范围：无桌面资料、Guest、仓库或临时文件修改；只读枚举和文本提取。
- 验证边界：已确认 SDK 结构和 Buildroot 工作流旁证，但尚未获得 100ASK 的实际 `.config`、toolchain 版本或可下载 SDK 归档。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核记录后，查询官方 Buildroot/Gitee 的精确提交与 100ASK i.MX6ULL 配置文件；再在 VM 用户目录按临时名受控获取源码或构建 SDK。

### W-20260830-031：Buildroot 2020.02 来源、提交与 100ASK 工具链规格定版（只读）

- 时间：2026-08-30T15:48:52+08:00（阶段性记录；来源查询约在此前数分钟完成）
- 关联要求/未决项：P1、D-004；用户要求先修复 Ubuntu 24 的 i.MX6ULL 开发环境，并保留可复核来源。
- 动作前审阅：主代理已完整分块读取本记录末尾至 W-030；确认本阶段只做忽略目录中的源码获取、公开 API/官方页面读取，不安装软件、不改 Guest、不覆盖仓库工程文件。
- 计划动作：受控获取官方 Buildroot 2020.02.x 无工作树副本，查找 rootfs 标识中的 `6a1d083`；读取 100ASK fork 的提交树、配置片段和官方资料，判断能否声称“原始 SDK”。
- 命令/工具：Windows Git `clone --filter=blob:none --no-checkout --single-branch --branch 2020.02.x https://github.com/buildroot/buildroot.git tmp/upstream/buildroot-official`；Gitee REST API/Raw；官方 Buildroot Git refs；官方 100ASK 论坛与下载中心页面。
- 官方源码证据：临时副本 HEAD 为 `1a6bd98fa87996f50f42a27857a9e9f029cc83e0`（2020.02.x/2020.02.12），可见历史约 51,692 个提交；在全部可见提交和对象中没有 `6a1d083`，`git rev-parse --verify '6a1d083^{commit}'` 返回无效对象。因此不能把 rootfs 的 `g6a1d083` 归因于公开官方分支。
- 100ASK fork 证据：Gitee API 确认 `weidongshan/Buildroot` 为公开仓库、GPL-2.0 字段、`2020.02.x` 头 `b0a4559fe452bc5586cab39bc984f9a81337219d`；该提交父为无父的导入快照 `83752440fe347ca4d8a2ece84eb774cdbe3e67e2`。快照树 14858 项，没有生成时的完整 `.config`/output/SDK，仅有 `configs/100ask/100ask_imx6ull_{Bootloaders,Kernel}.config`、Core filesystem、Hostutilities 等片段。`package/alsa-lib/alsa-lib.mk` 与官方 2020.02 内容相同；Makefile 的 `BR2_VERSION_EPOCH`、顶层 `Config.in` 和 package 索引有 fork 差异。故它是可追溯的重建输入，不是 rootfs 的逐字节 SDK 证明。
- 100ASK 配置证据：Bootloader 片段使用 `mx6ull_14x14_evk`、自定义 U-Boot Coding URL 和 `DTB_IMX`；Kernel 片段使用 Linux 4.9.88 Coding URL、defconfig `100ask_imx6ull`、DTS `100ask_imx6ull-14x14`、zImage/gzip/perf；ALSA 包版本由官方 2020.02 `alsa-lib.mk` 固定为 `1.2.1.2`，与已审 rootfs 字符串一致。
- 官方 100ASK 环境规格旁证：论坛页面给出的实际 SDK 路径为 `~/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot`，交叉编译器目标 `arm-buildroot-linux-gnueabihf`，GCC `7.5.0`，`--with-cpu=cortex-a7 --with-fpu=neon-vfpv4 --with-float=hard`，并明确以 Buildroot sysroot 编译内核/驱动。下载中心的 i.MX6ULL PRO 页面列出官方配套资料入口及 Linux/驱动 Coding 源码地址，但没有公开可直接匿名下载的 SDK 归档。
- 结果：已排除 Ubuntu 24 自带 GCC 13.3/空 sysroot 和损坏 HTML 归档作为目标 SDK；当前可确认的修复路线是取得真实 100ASK SDK，或以官方 2020.02 + 100ASK 片段和目标内核重建一个明确标注“重建版”的 SDK。不能宣称已经恢复与 `Buildroot 2020.02-g6a1d083-dirty` 完全一致的工具链。
- 临时文件/修改范围：只在被 `.gitignore` 覆盖的 `tmp/upstream/buildroot-official` 创建无工作树源码副本；没有修改桌面资料、Guest、AUDIO_DSP 跟踪文件或用户凭据。仓库原有的 FPGA/文档等未提交改动保持不变。
- 验证边界：官方 clone 的无工作树状态会显示索引文件为 deleted，这是 `--no-checkout` 的预期状态；尚未对 100ASK Coding 仓库输入凭据或下载；尚未证明 SDK 可构建，也尚未做 ARMHF 全链接。
- Git：本条仅追加工作记录，未暂存、未提交、未推送。
- 下一步：完整复核本条后，先在 Ubuntu 24 读取目标 rootfs ELF 的 `.comment`/GCC 版本及 Buildroot 主机依赖；再检查官方/100ASK 下载入口是否能取得真实 SDK，若不能则在用户目录建立可复现的重建 SDK profile，并用它编译 `mpu` 工程。

### W-20260830-032：Ubuntu24 rootfs 编译器标记与 Buildroot 主机依赖复核（只读）

- 时间：2026-08-30T15:50:02+08:00
- 关联要求/未决项：P1、D-004；确认迁移环境是否能重建与目标 rootfs ABI 相符的 MPU SDK。
- 动作前审阅：主代理已完整分块读取含 W-031 的 702 行记录，并再次读取 VMware 访问规则；确认使用独立 Ubuntu24 host key、只读 SSH 命令、临时 ELF 导出和精确清理，不安装包、不改变系统配置。
- 命令/工具：以已核验身份 `hry@192.168.79.149` 执行 host tool/version、`dpkg-query`、foreign-architecture、交叉编译器 `-v/-dumpmachine/-print-sysroot`；用 `debugfs` 从 `/home/hry/imx6ull/files/rootfs.ext4` 导出四个指定 ELF 到 `mktemp` 目录，运行 `file/readelf/strings` 后由 trap 清理。
- Guest 身份复核：`hry`、`ubuntu24.04`、Ubuntu 24.04.4 LTS、kernel `7.0.0-30-generic`、地址 `192.168.79.149` 均匹配既有身份链。
- 主机依赖证据：native GCC/G++ 13.3.0、CMake 3.28.3、Ninja 1.11.1、Git 2.43、Python 3.12.3、pkg-config 1.8.1、Bison 3.8.2、Flex 2.6.4、rsync 3.2.7、cpio 2.15、unzip、bc、dtc 均存在；`build-essential`、`libc6-dev:amd64`、`libncurses-dev:amd64` 已安装。`libasound2-dev`、`qemu-user-static` 不存在，foreign architecture 列表为空。
- 迁移交叉环境证据：系统 `arm-linux-gnueabihf-gcc` 为 Ubuntu 13.3.0、binutils 2.42，`-dumpmachine=arm-linux-gnueabihf` 但 `-print-sysroot=/`；`arm-linux-gnueabihf-g++` 缺失。其配置为 `armv7-a+fp`、hard-float、默认 Thumb，不能作为目标 Buildroot SDK。
- 目标 ELF 证据：rootfs 的 `aplay`、loader、libc、libasound 均为 ARM EABI5、Cortex-A7/ARMv7、VFPv4、`Tag_ABI_VFP_args=VFP registers`；解释器 `/lib/ld-linux-armhf.so.3`。`libc-2.30.so` 字符串明确写出 `GNU C Library ... 2.30`、`Compiled by GNU CC version 7.5.0`，从而与官方 100ASK 论坛给出的 GCC 7.5.0 规格相互印证。`readelf -p .comment` 对这些 stripped ELF 均提示无 `.comment`，不能把该提示误写成版本缺失。
- ALSA/Buildroot 证据：rootfs `/usr/lib/os-release` 为 `Buildroot 2020.02-g6a1d083-dirty`；先前已核实 ALSA runtime 1.2.1.2，当前导出的 `libasound` 保持同一 ARMHF ABI。`debugfs dump` 对非 root 临时文件输出 “Operation not permitted while changing ownership”，但文件大小、`file/readelf` 均成功；该权限提示不影响读取结论，临时目录由 trap 删除。
- 结果：Ubuntu24 的通用 host 工具足以作为 Buildroot 主机，但迁移仍缺真实 100ASK GCC 7.5/sysroot、G++、目标 ALSA headers/pkg-config、可能的 qemu；不能直接用当前 GCC 13 或运行时 rootfs 进行目标全链接。下一步应优先取得/重建真实 SDK，并把版本/ABI 门禁写入 MPU 工程。
- 修改范围：Guest 无持久修改；只创建并清理 `/tmp/audio_dsp_elfcheck.*` 临时目录；仓库未改（本条记录将在本文件追加）。
- 验证边界：没有运行目标程序、没有挂载/修改 rootfs、没有安装缺失包；尚未证明 Buildroot 全量构建在 Ubuntu24 成功，也未取得精确 `6a1d083` 源码或 SDK。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核本条后，在不破坏现有迁移目录的前提下探测官方 100ASK Coding/下载中心是否能匿名取得源码或 SDK；若仍不可达，设计用户目录内的“重建 SDK”流程和最小配置，先做可中止的依赖/空间检查。

### W-20260830-033：用户禁止 Gitee 登录；改用匿名 manifest 与 Coding 直连

- 时间：2026-08-30T18:18:41+08:00
- 关联要求/未决项：P1、D-004；用户明确表示没有 Gitee 账号，要求不要让其登录。
- 新增硬约束：不要求用户创建/登录 Gitee，不索要、输入、保存或输出 Gitee 用户名、密码、令牌；任何提示认证的 Gitee Git 入口立即停止。Gitee 仅允许使用无需认证的公开 Raw/REST API 作为只读取证来源；实际源码/SDK 优先使用可匿名访问的 100ASK Coding 或 Buildroot/GitHub 官方入口。
- 动作前审阅：主代理已完整分块读取含 W-032 的 719 行记录；随后只做公开来源与 `git ls-remote` 探测，没有建立凭据或修改 Guest。
- 官方 manifest 证据：无需登录即可读取 `https://gitee.com/weidongshan/manifests/raw/linux-sdk/imx6ull/100ask_imx6ull_linux4.9.88_release.xml`，文件大小 1915 字节，API blob SHA `e39cc221c399f9d8174339ca42d5314f09d6c502`。它明确列出 Buildroot、下载缓存、两套 ToolChain、Linux、U-Boot 和环境配置的实际项目路径。
- 工具链来源：manifest 的 Buildroot GCC 7 工具链项目是 `ToolChain-7.x/ToolChain-7.x`，目标路径 `ToolChain`，Coding remote 基址 `https://e.coding.net/weidongshan/`；另有 Linaro 6.2.1 项目，但当前 rootfs libc 已证明由 GCC 7.5.0 构建，因此优先审核 Buildroot GCC 7 项目。
- 匿名可达性证据：`git ls-remote` 对以下 Coding URL 无凭据成功：Buildroot `https://e.coding.net/weidongshan/buildroot/buildroot.git`（2020.02.x=`b0a4559...`，master=`37ba456...`）、ToolChain-7.x（master=`f11f3a59043f5703faec676cde5a5556d8f3b00a`）、Buildroot dl imx6ull（`a69a3f5...`）、Linux 4.9.88（`d0b34fd...`）、U-Boot 2017.03（master=`8ba4c5b...`）及 manifest Coding mirror（linux-sdk=`5d04edb...`）。
- Gitee Git 结果：`git ls-remote https://gitee.com/weidongshan/manifests.git` 返回认证失败；按照用户新约束，该入口永久不作为本任务依赖，也不会再次要求登录。公开 Raw/API 已足够取得 manifest，不构成阻塞。
- 结果：现在有一条完全不依赖用户登录的官方恢复路径：从匿名 Coding 固定提交取得 ToolChain-7.x，并对其 GCC/sysroot/ALSA/glibc 与 rootfs 做门禁，再决定是否需要 Buildroot 源码重建。
- 修改范围：本阶段仅公开网络只读探测；未克隆新仓库、未修改 Guest、未输入凭据；仓库除追加本记录外无新增修改。
- 验证边界：`ls-remote` 只证明远端和提交可达，不证明 ToolChain 仓库内容完整、不是 Git LFS pointer、可在 Ubuntu24 执行或与 rootfs ABI 一致。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核本条后，以 `--filter=blob:none --no-checkout --depth=1` 在忽略目录审计 ToolChain-7.x 树、大小、LFS 和许可证；通过后再以临时目录在 Ubuntu24 获取并做哈希、编译器/sysroot/ALSA ABI 验证，全程无 Gitee 登录。

### W-20260830-034：ToolChain-7.x 匿名克隆前置检查

- 时间：2026-08-30T18:24:12+08:00
- 关联要求/未决项：P1、D-004；落实用户“不登录 Gitee”硬约束并准备获取匹配 ARMHF SDK。
- 动作前审阅：主代理已完整分块审阅 `docs/PROJECT_WORKLOG.md` 最新 735 行，确认仅允许匿名 Coding/GitHub 来源，禁止覆盖既有迁移文件；本动作只检查目标路径和磁盘，不下载、不修改 Guest。
- 计划动作：确认 `tmp/upstream/ToolChain-7.x` 不存在且 `tmp/` 由 `.gitignore` 覆盖，再执行受控无工作树克隆。
- 命令/工具：PowerShell `Test-Path`、`Get-PSDrive`、`git check-ignore -v`。
- 结果与原始证据：目标不存在（`DEST_EXISTS=False`），父目录已存在；工作盘可用 `32419639296` 字节；`.gitignore:6:/tmp/` 命中目标路径。未触发网络访问，未建立克隆目录。
- 修改文件：无。
- 验证边界：空间值仅是克隆前快照；尚未证明远端对象大小/内容、工具链可执行或 sysroot 与 rootfs ABI 一致。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核本条后，以 `GIT_TERMINAL_PROMPT=0` 从匿名 Coding URL 执行 `--filter=blob:none --no-checkout --depth=1` 克隆，并审计固定 HEAD、树大小、LFS 指针和许可证。

### W-20260830-034：记录员接管与索引核对

- 时间：2026-08-30T18:20:46+08:00
- 关联要求/未决项：D-008、P0、P1、W-20260830-033。
- 动作前审阅：记录员已完整读取 `docs/PROJECT_WORKLOG.md`，重点核对 W-20260830-033 的时间、用户“没有 Gitee 账号、禁止要求登录”的硬约束及其匿名来源结论。
- 接管结果：确认后续不得引导用户登录 Gitee，不得索取、保存或输出 Gitee 凭据；公开 Raw/API 与无需认证的 Coding 入口可继续作为只读证据来源。
- 索引核对：确认 W-20260830-033 已记录 manifest SHA、Coding 远端可达性、Gitee Git 认证失败及其不再使用的决策；未发现与该硬约束冲突的后续条目。
- 实际动作：仅追加本条工作记录；未执行系统、网络、VM、仓库代码或 Git 操作。
- 验证边界：本条只证明记录员恢复并完成索引核对，不代表 Ubuntu24 工具链或 MPU 工程已完成。
- Git：本条追加形成未提交工作记录差异；无新的提交或推送。
- 下一步：继续要求所有实质动作先审阅本记录，完成后把命令、结果、边界和 SHA 回传记录员。

### W-20260830-037：匿名 Coding ToolChain-7.x 克隆完成及中途状态勘误

- 时间：2026-08-30T18:29:30+08:00
- 关联要求/未决项：P1、D-004、W-20260830-033、W-20260830-036。
- 动作前审阅：主代理与记录员已完整读取最新工作记录；确认继续使用无需登录的 Coding 入口，保留 W-036 的“中途无 HEAD”异常，不以中途快照覆盖历史。
- 动作：等待并核对匿名 Coding ToolChain-7.x partial clone 后台进程自然完成；未执行 Gitee 登录、未修改 Guest 系统。
- 中途状态勘误：首次检查时仓库尚无 HEAD，`in-pack=0`、`garbage=1`；这是异步 clone 尚未完成的中途状态，W-036 原记录保留，不代表最终仓库损坏。
- 完成证据：后台 clone 约 18:25 自然结束；固定 HEAD=`f11f3a59043f5703faec676cde5a5556d8f3b00a`。过滤克隆输出包含 warning；最终 no-checkout 状态显示 42,462 个 `D`，属于预期未检出工作树，不代表对象缺失。
- 规模证据：最终对象树约 42,462 blobs；逻辑文件总字节 `1,679,159,533`；pack 约 `519,831,503` 字节。顶层仅 `arm-buildroot-linux-gnueabihf_sdk-buildroot`。
- ABI/工具链证据：`gcc`/`g++` 符号链接到 `toolchain-wrapper`，包含 `relocate-sdk.sh`；sysroot 存在 `ld-linux-armhf.so.3`、`libc-2.30.so`、ALSA headers、ALSA 版本 `1.2.1.2`、`alsa.pc` 和 `libasound.so.2.0.0`。
- 修改范围：仅在被忽略临时目录完成匿名来源审计副本；未修改正式 AUDIO_DSP 工程、未暂存、未提交、未推送，未改 Ubuntu24 Guest。
- 验证边界：clone/树/文件存在与 ABI 文件名证据不等于 SDK 可执行、relocate 后路径正确、ARMHF 编译可用、与目标 rootfs 符号版本一致或 MPU 板上运行通过；仍需在 Ubuntu24 内做只读 SDK 盘点和最小编译/ABI 门禁。
- Git：AUDIO_DSP 无新增提交或推送。
- 下一步：在 Ubuntu24 独立身份核验后，使用该固定 SHA 的 SDK 做最小 toolchain/sysroot/ALSA 编译检查；全程继续遵守匿名来源和 Gitee 禁止登录约束。

### W-20260830-038：Ubuntu24 独立 SSH 预检与迁移环境状态核验

- 时间：2026-08-30T18:31:13+08:00
- 关联要求/未决项：P1、D-004、W-20260830-005、W-20260830-037。
- 动作前审阅：主代理与记录员已完整读取最新工作记录，并按 VMware 专用 AGENTS 规则确认 Ubuntu24 不复用 Ubuntu18 HostKeyAlias；本次仅使用已核验的 Ubuntu24 独立 key/IP 做只读 SSH 预检。
- 命令/工具：通过独立 Ubuntu24 SSH 身份执行用户、hostname、OS、内核、磁盘、Git 和工具链目录检查；同时匿名执行 Coding `git ls-remote`。
- Guest 身份证据：`USER=hry`、`HOST=ubuntu24.04`、`VERSION_ID=24.04`、`PRETTY=Ubuntu 24.04.4 LTS`、kernel `7.0.0-30-generic`，与用户提供的 Ubuntu24 Guest 身份链一致。
- 资源/工具证据：`/home` 可用 `78,255,396 KiB`；Guest Git 版本 `2.43`。
- 迁移环境缺口：目标 `/home/hry/toolchains/100ask-toolchain-7.x` 和 staging 目录均不存在；此前坏归档仍为 `58,351 B`，不能作为 SDK 使用。
- 来源复核：匿名 Coding `git ls-remote` 返回 master=`f11f3a59043f5703faec676cde5a5556d8f3b00a`，与 W-037 固定提交一致；全过程未登录或访问 Gitee Git。
- 修改范围：只读 SSH/远端预检；未修改 Ubuntu24 Guest、未安装包、未删除坏归档、未修改正式代码或工程、未暂存/提交/推送。
- 验证边界：预检证明独立 SSH、Guest 身份和基本资源可用，不证明 SDK 已部署、交叉编译器可执行、sysroot/ALSA ABI 可链接或 MPU 板端可运行；坏归档仍需隔离处理，不能直接解压覆盖。
- Git：AUDIO_DSP 无新增提交或推送。
- 下一步：在 Ubuntu24 用户目录内安全取得/审计固定 ToolChain-7.x，验证 wrapper、relocate、GCC/G++、sysroot、ALSA headers/pkg-config 与目标 rootfs ABI，再进行 MPU 构建。

### W-20260831-039：用户放弃 MPU，验收范围收敛至 MCU 与 FPGA

- 时间：2026-08-31T13:58:48+08:00
- 关联要求/未决项：用户最新范围变更；D-004、P1、P4。
- 动作前审阅：记录员已完整读取最新 `docs/PROJECT_WORKLOG.md`，核对 W-038 的 Ubuntu24 预检、ToolChain 状态以及 W-033 的 Gitee 禁止登录约束。
- 范围变更：用户明确放弃 MPU，不再进行任何 MPU 环境、代码、VM、下载、工具链、部署或验证工作。
- 停止动作：立即停止 `mpu_build_audit` 及所有 P1/i.MX6ULL/Ubuntu24 后续任务，不得继续克隆、下载、安装、修改 Guest 或运行 MPU 构建/测试。
- 保留策略：现有 `mpu/` 文件、已有 MPU 提交和 W-001～W-038 中的 MPU/Ubuntu24 记录全部保留为历史证据，不删除、不改写；但后续报告不得把它们宣称为当前完成或继续中的交付。
- 验收范围：项目最终验收和剩余主动工作收敛至 MCU（STM32H743IIT6 CubeMX+Keil）与 FPGA（BX71 Zynq-7020 Vivado 2018.3/XSDK）两条线；MCU/FPGA 仍须满足 native 工程、可复现构建和明确的硬件验证边界。
- Gitee 约束：用户“不登录 Gitee”硬约束继续有效；既有匿名 Raw/API 与 Coding 记录仅作为历史来源证据，不授权开展新的 MPU 下载或环境动作。
- 修改文件/系统：本条仅追加工作记录；未执行 MPU、VM、网络、下载、代码或 Git 操作。
- 验证边界：本条只证明范围收敛和停止门禁，不代表 MPU 当前或历史工程已完成，也不代表 MCU/FPGA 新验收已完成。
- Git：无新增提交或推送；本条追加形成待后续阶段提交的工作记录差异。
- 下一步：主代理仅继续 MCU/FPGA 审计、融合、工程化和验证，并将每阶段结果回传记录员；任何偏离该范围的动作须取得用户新授权。

### W-20260830-035：并发记录编号勘误（保留原始条目）

- 时间：2026-08-30T18:26:31+08:00
- 关联要求/未决项：D-008、P0；工作记录并发写入的一致性。
- 动作前审阅：主代理已完整分块审阅包含两个 W-034 的最新工作记录；确认不得删除或改写任何原始时间线，只能追加勘误。
- 勘误内容：由于主代理与恢复的记录员在同一阶段并发追加，文件中出现两个同名编号 W-20260830-034。较早时间 `18:20:46` 的条目是记录员接管核对，`18:24:12` 的条目是 ToolChain-7.x 克隆前置检查；两条原始内容均保留，后续阶段使用新的唯一编号并以时间字段区分。
- 结果与边界：未删除、覆盖或重排历史；今后主代理先向记录员发送阶段摘要，由记录员分配编号，避免再次并发复用编号。
- 修改文件：仅追加本条 `docs/PROJECT_WORKLOG.md`。
- Git：本条未暂存、未提交、未推送。
- 下一步：完整复核本勘误后执行 ToolChain-7.x 匿名克隆；克隆结果使用下一个唯一编号记录。

### W-20260830-036：ToolChain-7.x 匿名克隆未形成有效仓库（保留失败）

- 时间：2026-08-30T18:31:04+08:00
- 关联要求/未决项：P1、D-004；取得可执行的 ARMHF GCC 7.5/sysroot，且不使用 Gitee 登录。
- 动作前审阅：主代理已完整分块审阅包含 W-035 的最新工作记录，确认目标目录此前不存在、仅允许匿名 Coding、不得覆盖用户文件；记录员暂不并发写入。
- 计划动作：从 `https://e.coding.net/weidongshan/ToolChain-7.x/ToolChain-7.x.git` 以 `GIT_TERMINAL_PROMPT=0`、空 credential helper、`--filter=blob:none --no-checkout --depth=1` 克隆到被忽略的 `tmp/upstream/ToolChain-7.x`，随后读取固定 HEAD 和树摘要。
- 命令/工具：Windows Git clone；随后 `git rev-parse`、`remote -v`、`status`、`count-objects`、`ls-tree`。
- 结果与原始证据：clone 输出 `Cloning into ...` 后仅有 `warning: filtering not recognized by server, ignoring`；命令单元最终没有产生 HEAD。目标目录存在但 `git rev-parse HEAD` 和 `git ls-tree ... HEAD` 均返回 `ambiguous argument/Not a valid object name HEAD`；`git status` 显示 `##` 后跟 warning，`count-objects` 显示 `in-pack: 0`、`packs: 0`、`garbage: 1`、`size-garbage: 211260`。因此这是不完整临时仓库，不可当作工具链来源或固定提交。
- 修改文件：仅创建了被 `.gitignore` 覆盖的临时目录 `tmp/upstream/ToolChain-7.x` 及其不完整 `.git` 对象；正式工程、Guest、桌面资料和凭据均未修改。
- 验证边界：尚未判断失败是 Coding 服务对 partial clone 的兼容性、网络中断还是 Git 进程异常；尚未读取任何有效工具链 blob，也不能声称仓库不存在。需先审阅目录内部状态，再决定精确清理或采用不带 filter 的浅克隆重试。
- Git：本条未暂存、未提交、未推送；不完整目录被忽略。
- 下一步：完整复核本条后只读检查 `.git/config`、pack 临时文件和进程/网络错误；若确认无有效 refs，则在确认精确目标后删除该不完整临时目录并用 `--no-tags --depth=1 --no-single-branch`（不带 filter）匿名重试，保留本失败证据。

### W-20260831-040：用户“继续”后的 MCU/FPGA 只读接管盘点

- 时间：2026-08-31T14:11:28+08:00
- 关联要求/未决项：用户 2026-08-31 回复“继续”；W-20260831-039、P2、P3、P4。
- 动作前审阅：主代理完整审阅了当时最新的 829 行 `docs/PROJECT_WORKLOG.md`；记录员也逐行完整复核同一版本并确认追加优先、唯一编号及验证边界。本阶段没有执行 MPU、VM 或 Gitee 动作。
- 本阶段动作：主代理只读盘点 MCU 与 FPGA 的 Git/目录/工程现状，准备在用户已收敛的 MCU/FPGA 范围内接管后续工作；截至本阶段回传，没有收到可据以认定正式源码修改的代理报告。
- MCU Git 与工程基线：`mcu/` 相对 `HEAD` 无修改；STM32H743 工程已有可重新生成的 `.ioc`、可打开的 `.uvprojx`、SelfTest 与 WM8960 两个 target，并已纳入 49 个 permissive 根 DSP 源。NUEDC canonical 尚未正式融合。
- NUEDC 审计基线：审计副本继续固定在 `416270a254795843b6f1854074380015637d1f95`；canonical `STM32H743/DSP` 共 27 个文件（13 个 `.c`、14 个 `.h`），覆盖 FFT、FIR/IIR、IQ、AM/FM/BPSK、PLL、THD、相关、拟合、自适应与周期分析，并依赖 CMSIS-DSP。
- 许可证边界：该源树没有根 LICENSE，目标 DSP 文件也未发现可据以声明许可证的 SPDX 元数据。用户已明确要求整合，但在来源/授权元数据明确前不得擅自宣称许可证，也不得把只读审计写成已经完成融合。
- NUEDC 风格结论：可复核规则包括 `模块名_函数` 命名、显式状态 `enum`、`stdint` 定宽类型、`NULL`/范围检查、静态工作区且不使用堆、中文功能注释和头文件保护；正式适配仍需保持现有 H743 工程可重生成和双 target 边界。
- MCU 融合计划：在 `mcu/` 内建立隔离、可选的 `NUEDC Analysis` target；重分析不得放入实时 SAI ISR；补齐 CMSIS-DSP 接入、离线向量测试以及来源/授权说明后再做 Keil 构建与资源核验。
- FPGA 工具与预检：本机 Vivado 2018.3 64-bit，软件版本 2405991、IP 版本 2404404。`check_native_project` 的 selftest 检查退出码为 1；检查中确认 `.xpr` 的 `Project Path` 含 `C:/Users/LENOVO/Desktop/AUDIO_DSP` 绝对路径，并出现 Vivado 安装目录 Tcl store 无写权限提示。
- FPGA 原生工程缺口：`fpga/projects/` 当前只有 selftest `.xpr`，`soc_i2s` 仍缺少版本化 native `.xpr`，所以尚未满足 selftest + soc_i2s 双原生工程门槛。原 FPGA 代理与上一记录代理随后因额度中断，未回传正式修改报告；主代理已接管 FPGA 后续工作。
- 修改文件：本条仅追加 `docs/PROJECT_WORKLOG.md`；未修改 MCU/FPGA 源码或工程，未执行 MPU/VM/Gitee。
- Git：未暂存、未提交、未推送。
- 验证边界：以上为只读盘点与计划，不等于 MCU/FPGA 构建、仿真、综合、实现、bitstream、软件构建或真实硬件验证通过；文件存在和工具版本也不等于 native 工程已可移植验收。
- 下一步：主代理按上述隔离边界推进 MCU 可选分析 target 与 FPGA native 工程闭环；每次实际修改、构建或验证后把命令、文件、原始结果和未完成边界回传记录员追加。

### W-20260831-041：Windows 已安装 CMSIS Pack 只读盘点

- 时间：2026-08-31T14:15:11+08:00
- 关联要求/未决项：W-20260831-040、P2；为 NUEDC Analysis 可选 target 确认 CMSIS-DSP 接入来源。
- 动作前审阅：主代理在完整复核含 W-20260831-040 的最新工作记录后开始本动作；记录员追加前也逐行完整复核了当时最新的 847 行记录，确认本阶段只允许 MCU 本机环境只读检查，不涉及 MPU、VM 或 Gitee。
- 单一只读动作：主代理检查 Windows 已安装的 CMSIS Pack 路径和目录内容；使用本机文件系统只读路径/目录检查，没有安装、复制、移动或修改 Pack。
- CMSIS-DSP 证据：`C:\Users\LENOVO\AppData\Local\Arm\Packs\ARM\CMSIS-DSP\1.16.2` 完整存在，包含 `ARM.CMSIS-DSP.pdsc`、Apache-2.0 `LICENSE`、`Include/`、`PrivateInclude/` 和 `Source/`。
- CMSIS Core 证据：`C:\Users\LENOVO\AppData\Local\Arm\Packs\ARM\CMSIS\6.3.0` 存在；另行检查的两个 Keil Pack 候选路径不存在，因此不得把候选路径当作有效依赖来源。
- 结论与许可证边界：NUEDC Analysis 可优先通过已安装的 CMSIS-DSP 1.16.2 Pack 接入，不需要把 CMSIS-DSP 整库复制进仓库。Apache-2.0 证据只对应该 CMSIS-DSP Pack，不能外推为 NUEDC 自研 DSP 源码的许可证。
- 修改文件/系统：除本条追加 `docs/PROJECT_WORKLOG.md` 外无修改；未修改源码、MCU 工程或 Windows 系统，未执行 MPU、VM、Gitee。
- Git：未暂存、未提交、未推送。
- 验证边界：本动作只证明指定 Pack 路径和关键文件/目录存在；尚未验证 PDSC 组件选择、Keil target 依赖解析、头文件/源文件配置、编译链接或真实硬件。
- 下一步：在再次完整复核记录后，读取 `ARM.CMSIS-DSP.pdsc` 的组件定义并为隔离的 NUEDC Analysis target 设计最小 Pack 接入；实际编辑与 Keil 构建结果另行追加。

### W-20260831-042：CMSIS-DSP PDSC 与 H743 RTE 接入设计审计（只读）

- 时间：2026-08-31T14:18:37+08:00
- 关联要求/未决项：W-20260831-040、W-20260831-041、P2；为第三个 NUEDC Analysis target 选择可控的 CMSIS-DSP 接入方式。
- 动作前审阅：主代理在完整复核含 W-20260831-041 的最新记录后执行本阶段；记录员追加前也逐行完整读取了当时最新的 861 行日志，确认只做 PDSC/RTE 设计审计，不修改工程或触发构建。
- 只读动作：读取已安装 `ARM.CMSIS-DSP.pdsc` 中的组件定义，并枚举当前 STM32H743 工程的 RTE 内容；没有让 Keil/RTE 管理器生成、复制或更新任何文件。
- PDSC 依赖证据：`CMSIS:DSP:Source` 1.16.2 组件要求 `CMSIS:CORE`；它向工程提供 `Include/`、`PrivateInclude/`，以及 BasicMath、CommonTables、Complex、Filtering、Statistics、Support、Transform 等聚合源。
- 冗余范围：整组件还会带入许多当前 NUEDC Analysis 不需要的功能分类和 F16 源；因此不能仅因 Pack 已安装就把整个 `CMSIS:DSP:Source` 无选择加入目标。
- 当前 RTE 证据：STM32H743 工程的 RTE 目录当前仅见 Device `dbgconf` 文件，未见 CMSIS-DSP RTE 内容；本阶段没有改变该状态。
- 设计结论：直接选用整组件会引入明显冗余，并通过 `CMSIS:CORE` 建立 RTE 依赖；这与仓库当前手工 vendored CMSIS core 以及工程再生成策略存在潜在冲突。当前只确认风险，不把它写成已复现的 Keil 冲突。
- 修改文件/系统：除本条追加 `docs/PROJECT_WORKLOG.md` 外无修改；未修改源码、RTE、Keil 工程或 Windows 系统，未执行 MPU、VM、Gitee。
- 构建/Git：未构建；未暂存、未提交、未推送。
- 验证边界：本阶段只读核对 PDSC 定义和现有 RTE 文件，不等于 Keil 能解析该组件、依赖可正确解析、最小源集可编译链接或真实硬件通过。
- 下一步：先验证 NUEDC 所需的最小 CMSIS-DSP 头文件/源文件组合，或建立明确、可诊断的 Pack 路径门禁；确认不会破坏手工 CMSIS core 与再生成流程后，再设计第三个隔离 Analysis target。

### W-20260831-043：本机 CMSIS-DSP Keil 参考工程与 RTE 元数据审计（只读）

- 时间：2026-08-31T15:57:15+08:00
- 关联要求/未决项：W-20260831-041、W-20260831-042、P0、P2；为新 Analysis target 寻找已验证配置形态的本机参考工程。
- 动作前审阅事实：主代理在本动作前读取了最新日志尾部 W-20260831-040～W-20260831-042，并依赖此前对 861 行版本的完整分块审阅；但没有再次分块输出并复核当时最新的全部 876 行。因此本动作**不能**记作符合第 0 节规则的严格全量复核，属于流程缺口。记录员在追加本条前已完整分块读取 876 行版本，但该补读不能追溯性消除主代理动作前的缺口。
- 只读动作：主代理在本机检索已配置 CMSIS-DSP 的 Keil 工程，并读取候选 `.uvprojx`、RTE 元数据和 RTE 目录结构；没有打开 RTE 管理器写回工程，也没有修改任何参考工程。
- 当前版本参考：找到 `C:\Users\LENOVO\Desktop\Keil_Cubemx_Projects\NUEDC_MAIN(NO_RTOS)\MDK-ARM\NUEDC_MAIN(NO_RTOS).uvprojx`。其 RTE XML 明确登记 `CMSIS:CORE` 6.2.0（来源 `ARM.CMSIS` 6.3.0）与 `CMSIS:DSP:Source` 1.16.2（来源 `ARM.CMSIS-DSP` 1.16.2），组件 `targetInfo` 绑定该工程的 Keil target。
- RTE 物化证据：该参考工程的 RTE 目录只额外生成 `RTE_Components.h` 和 Device `dbgconf`，没有把 CMSIS Pack 全树复制进工程。这说明规范 RTE 元数据可以引用已安装 Pack，但尚不能直接证明 AUDIO_DSP 当前工程会得到相同结果。
- 旧版本参考：另找到 `DSP_FFT` 工程，其配置使用 CMSIS 5.4 与 CMSIS-DSP 1.5.2；该工程只作为历史 RTE 结构参考，不采用其旧组件版本。
- 设计结论：AUDIO_DSP 可以尝试复用 `NUEDC_MAIN(NO_RTOS)` 的规范 RTE 元数据结构，并只把 CMSIS 组件的 `targetInfo` 绑定到新的 Analysis target，避免影响现有 SelfTest/WM8960 targets。此结论是待验证设计，不等于已完成工程配置。
- 修改文件/系统：除本条追加 `docs/PROJECT_WORKLOG.md` 外无修改；未修改参考工程、AUDIO_DSP 源码/RTE/Keil 工程或 Windows 系统，未执行 MPU、VM、Gitee。
- 构建/Git：未构建；未暂存、未提交、未推送。
- 验证边界：只读 XML/RTE 结构证据不等于 Keil 能为新 target 解析 Pack 源与依赖、编译链接成功、资源满足或硬件通过；必须用实际 Keil build 证明。
- 下一步：任何后续实质动作前，主代理必须重新分块完整审阅包含本条在内的最新日志；随后在隔离 Analysis target 中最小化复用 RTE 元数据，并以实际 Keil 依赖解析与构建日志验收。

### W-20260831-044：MCU Analysis 挂接点与 NUEDC tests 大范围只读审计（输出截断）

- 时间：2026-08-31T16:03:23+08:00
- 关联要求/未决项：W-20260831-039、W-20260831-043、P0、P2；用户范围继续冻结为仅 MCU+FPGA，MPU 已放弃，且不做 VM/Gitee。
- 追加前审阅：记录员按 W-20260831-043 的纠正要求，逐行完整读取当时最新的 891 行 `docs/PROJECT_WORKLOG.md`，核对追加优先、唯一编号、范围冻结和验证边界后才追加本条。
- 单一只读命令：主代理在一次 PowerShell exec 中依次执行：`rg -n -i -C 2 'AUDIO_DSP|NUEDC|BX71|H743|PROJECT_WORKLOG' C:\Users\LENOVO\.codex\memories\MEMORY.md`；枚举并 `Get-Content` `tmp/upstream/NUEDC_2026/STM32H743/tests` 全部文件；读取 `mcu/project/STM32H743_Audio/App/Src/audio_app.c`、`App/Inc/audio_app.h`、`Core/Src/main.c`；尝试读取 `C:\Users\LENOVO\Desktop\Keil_Cubemx_Projects\NUEDC_MAIN(NO_RTOS)\RTE\_Target_1\RTE_Components.h`；最后执行 `git status --short --branch`。
- 命令结果与截断边界：命令退出码为 0，耗时约 1.9 秒；原始输出约 46,766 tokens/4,410 行，因工具输出过大被截断。因此本次不能声称已完整捕获或审完 NUEDC `tests` 全部内容，后续必须按文件或小范围分块复核。
- 记录门禁旁证：`MEMORY.md` 命中 AUDIO_DSP 任务组及两项既有硬要求——每次实质动作前读取 `PROJECT_WORKLOG.md`，以及源码/工程/构建证据不等同于开发板实测；该记忆检索只作流程旁证，不替代当前源码、构建或硬件证据。
- MCU 源码证据：`AudioApp_Init` 先运行 `AudioApp_RunSelfTest`，随后初始化 `AudioDsp`；只有启用 `AUDIO_BOARD_ENABLE_WM8960_STREAM` 时才启动 WM8960/SAI DMA。`audio_app.h` 的状态枚举当前延伸到 `DMA_FAILED=-4`；`main.c` 调用 `AudioApp_Init` 与 `AudioApp_Service`。
- 设计结论：第三个 NUEDC Analysis target 可用独立宏挂接到非实时的启动自检路径，并与现有 SelfTest/WM8960 target 隔离；FFT、拟合等重分析不得放入实时 SAI ISR。该结论仍是待实现、待构建验证的设计，不代表 target 已创建。
- RTE 失败/保留：预期的参考路径 `RTE\_Target_1\RTE_Components.h` 不存在，结果为 `MISSING`；后续应先搜索实际 target 目录，不能继续假定 `_Target_1` 路径，也不能据此否定 W-20260831-043 已从参考 `.uvprojx` 取得的 RTE XML 证据。
- Git/修改范围：`git status` 仍显示既有脏工作树；`mcu/` 没有新增修改。本次审计纯只读，记录员仅追加本文件；未修改 MCU/FPGA 源码或工程，未执行 MPU、VM、Gitee，也未暂存、提交或推送。
- 验证边界：没有运行 Keil 构建、Vivado 仿真/综合/实现/bitstream、软件构建或真实硬件测试；只读源码与路径盘点不能证明 Pack/RTE 可解析、Analysis target 可链接、实时预算满足或板上音频通过。
- 下一步：再次完整复核最新日志后，按文件/小范围分块完整审阅 NUEDC tests，搜索参考工程真实 RTE target 目录，再据此设计隔离的第三个 Analysis target；任何实际编辑和构建结果另行追加。

### W-20260831-045：NUEDC tests 清单与真实 RTE 路径窄化审计（输出仍截断）

- 时间：2026-08-31T16:07:33+08:00
- 关联要求/未决项：W-20260831-039、W-20260831-043、W-20260831-044、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 追加前审阅：记录员重新逐行完整读取当时最新的 906 行 `docs/PROJECT_WORKLOG.md`，确认 W-20260831-044 的“大输出不可视为完整测试审计”边界、唯一编号和追加优先规则后才追加本条。
- 只读动作：PowerShell 枚举 `tmp/upstream/NUEDC_2026/STM32H743/tests` 的文件、大小和行数，以 `rg` 检索测试 include、entrypoint、PASS/FAIL/assert marker；在本机 `NUEDC_MAIN(NO_RTOS)` 参考工程下搜索 RTE 文件与 `RTE_Components.h`，并读取参考 `.uvprojx` 的 `TargetName`、CMSIS component 和 `targetInfo`。
- 命令结果与截断边界：命令退出码为 0，耗时约 2.2 秒；输出约 15,215 tokens/702 行，超过 15,000-token 上限并被截断。因此不能声称已完整捕获全部 assertion marker 或全部递归 vendor RTE 内容；但顶层 `tests` 文件 inventory 已完整显示。
- 测试清单：`tests` 共 25 个文件；主要 C host tests 包括 `adaptive_e`（214 行）、`fpga_promax`（358 行）、`fractional_delay`（188 行）、`g_tjc_plot_scale`（217 行）、`measure_thd`（193 行）和 `periodic_analyzer`（861 行），另有 Python 数值/验证脚本、PowerShell runner 与 host stub。
- MCU 验证取舍：可复用于 MCU Analysis 的重点是 Adaptive、FilterEx fractional delay、Measure THD 和 PeriodicAnalyzer；`fpga_promax` 与 `g_tjc_plot_scale` 属于应用/FPGA host interface，不应被当作 MCU canonical 13 个 DSP 源的全量自检。该筛选是测试范围设计，不代表这些测试已经执行或通过。
- 真实 RTE 路径：参考工程的 RTE root 位于 `MDK-ARM/RTE`，而非项目根 `RTE`；实际文件为 `C:\Users\LENOVO\Desktop\Keil_Cubemx_Projects\NUEDC_MAIN(NO_RTOS)\MDK-ARM\RTE\_NUEDC_MAIN_NO_RTOS_\RTE_Components.h`。
- RTE 文件内容：该 `RTE_Components.h` 只是仅含 include guard 的空宏壳，不列出 CMSIS-DSP 宏；这支持组件由 `.uvprojx`/RTE Pack 元数据解析的判断，但尚不证明 AUDIO_DSP 新 target 能同样解析和链接。
- `.uvprojx` 元数据：参考工程只有一个 target `NUEDC_MAIN(NO_RTOS)`；`CMSIS:CORE` 6.2.0（pack 6.3.0）和 `CMSIS:DSP:Source` 1.16.2 的 `targetInfo` 均绑定该 target。递归搜索还命中 `Drivers/CMSIS` 自带 RTE 示例，它们属于 vendor 噪声，不能作为该目标工程的组件配置证据。
- 修改/构建/Git：本阶段正式文件无修改，未构建，未执行 MPU、VM、Gitee；主代理未做 Git 暂存、提交或推送。记录员仅追加本日志，保留既有脏工作树不动。
- 验证边界：inventory、文本 marker 和 RTE XML 检查不等于测试运行、断言全覆盖、Keil Pack 解析、编译链接、实时资源或硬件验证通过；输出截断部分仍须以更小范围复核。
- 下一步：再次完整复核最新日志后，逐个读取四类 MCU 重点 host test 及其 runner/stub，形成可移植向量与依赖矩阵；随后只为隔离 Analysis target 设计 CMSIS/RTE 元数据并以实际 Keil build 验收。

### W-20260831-046：NUEDC tests 清单与真实 RTE 路径窄化审计（编号冲突顺延）

- 时间：2026-08-31T16:12:00+08:00
- 关联要求/未决项：W-20260831-039、W-20260831-043、W-20260831-044、W-20260831-045、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员先完整读取 `docs/PROJECT_WORKLOG.md` 最新全文并核对动作前门禁、验证边界与当前未决项；发现用户指定的 W-20260831-045 已存在，故为避免覆盖或复用编号，本条顺延为唯一编号 W-20260831-046。
- 只读动作：主代理在 AUDIO_DSP 根目录用 PowerShell 枚举 `tmp/upstream/NUEDC_2026/STM32H743/tests` 全部文件的相对路径、大小、行数；用 `rg --no-heading` 搜索所有测试 includes/entrypoints，并搜索 `PASS|FAIL|assert|return|printf|TEST`；枚举本机参考 `C:\Users\LENOVO\Desktop\Keil_Cubemx_Projects\NUEDC_MAIN(NO_RTOS)\RTE` 文件；递归搜索并读取 `RTE_Components.h`；用 `rg` 检查参考 `.uvprojx` 的 `TargetName`、CMSIS component 与 `targetInfo`。
- 结果与截断边界：命令 exit 0，约 2.2 秒；首层 inventory 完整，但总输出约 15,215 tokens/702 行，受 15,000-token 工具上限截断，不能声称全部 marker 或递归 RTE 内容均已完整捕获。`tests` 顶层共 25 个文件；重点文件行数为 `adaptive_e_host_test.c` 214、`adaptive_e_numeric_test.py` 176、`fpga_promax_host_test.c` 358、`fractional_delay_host_test.c` 188、`g_tjc_plot_scale_host_test.c` 217、`measure_thd_host_test.c` 193、`periodic_analyzer_host_test.c` 861，另有 runners/stubs。
- MCU 取舍结论：适合 MCU 分析向量验证的重点是 Adaptive、FilterEx fractional delay、Measure THD、PeriodicAnalyzer；`fpga_promax`/`g_tjc_plot_scale` 是 host/FPGA interface 证据，不能代替 MCU 13 个 canonical DSP 源的全量自检。该结论是范围设计，不代表测试已执行或通过。
- RTE 证据：参考工程真正 RTE 位于 `MDK-ARM/RTE/_NUEDC_MAIN_NO_RTOS_/RTE_Components.h`，文件为空 guard 壳；DSP 由 `.uvprojx`/RTE Pack 元数据解析。递归命中的 `Drivers/CMSIS` 示例 RTE 属 vendor 噪声，不能作为目标配置证据。参考 target 为 `NUEDC_MAIN(NO_RTOS)`，CMSIS CORE 6.2.0（pack ARM CMSIS 6.3.0）与 CMSIS DSP Source 1.16.2，`targetInfo` 均绑定该 target。
- 修改/构建/Git：正式 MCU/FPGA 源码和工程无修改；未构建，未触碰 MPU/VM/Gitee；未暂存、提交或推送。本条仅追加 `docs/PROJECT_WORKLOG.md`，保留既有脏工作树。
- 验证边界：inventory、文本 marker 与 RTE/uvprojx 只读审计不等于所有输出已捕获、测试运行/断言覆盖、Keil Pack 解析、编译链接、实时资源满足或真实硬件验证通过；未执行 Keil/Vivado 构建及板测。
- 下一步：再次完整复核最新日志后，逐文件/小范围读取四类 MCU 重点 host test 及 runner/stub，形成可移植向量与依赖矩阵；随后仅为隔离 Analysis target 设计 CMSIS/RTE 元数据并以实际 Keil build 验收。

### W-20260831-047：MCU/NUEDC canonical DSP 与工程脚本只读源码审计

- 时间：2026-08-31T16:20:00+08:00
- 关联要求/未决项：W-20260831-046、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员先完整读取当前 935 行 `docs/PROJECT_WORKLOG.md`，核对 W-046、追加优先、唯一编号、源码/工程不改动及构建/硬件验证边界后再记录本阶段。
- 只读动作：PowerShell 对固定 NUEDC SHA `416270a254795843b6f1854074380015637d1f95` 的 `STM32H743/DSP` 枚举文件大小、行数和 SHA；用 `rg` 检索 `includes`、`arm_math.h`、`main.h`、HAL、`printf`、`malloc`；读取 `DSP_Header.h`、重点 `Adaptive.h`、`FilterEx.h`、`Measure.h`、`periodic_analyzer.h`、`FFT.h`、`DSP_ProMax.h`；读取重点 host test/runners；读取当前 `mcu/scripts/postgenerate_keil.ps1`、`verify_all.ps1`、`build_keil.ps1` 和 `.uvprojx` summary。
- 命令结果与截断边界：命令 exit 0，约 4.8 秒；原始输出约 21,243 tokens/1,673 行，受工具输出上限截断，因此不能声称所有头文件或脚本全文已捕获；本条仅采用可可靠核对的汇总结果，不以截断输出替代完整源码审阅。
- canonical DSP 结果：`STM32H743/DSP` 共 27 个文件，即 13 个 C 与 14 个 H；`Demod.c` 为 93,539 B/2,583 行，`periodic_analyzer.c` 为 66,498 B/1,755 行。公共依赖包括 `arm_math.h`、`arm_const_structs.h`、部分 `main.h`、标准 `math/string/stdint`；`DSP_Header.h` 聚合 11 个模块。
- 测试与移植取舍：重点 host test/runners 仍筛选 Adaptive、FilterEx fractional delay、Measure、PeriodicAnalyzer，用于后续 MCU 分析向量验证；不把 host/FPGA interface 测试当作 MCU canonical DSP 全量自检，且本阶段未执行这些测试。
- 工程脚本/工程证据：`postgenerate_keil.ps1` 当前强制两个 targets，移除 CMSIS CORE RTE，并生成 Application/Audio 与 49 个 permissive sources；`verify_all.ps1` 调 regenerate 后 build；`build_keil.ps1` 的 ValidateSet 仅 SelfTest/WM8960/Both，并检查两个 targets、49 objects 与 DMA map。现有 `.uvprojx` 为两个 target，RTE components empty。
- 修改/构建/Git：未修改正式文件，未构建，未触碰 MPU/VM/Gitee/Git；本条仅追加 `docs/PROJECT_WORKLOG.md`，未暂存、提交或推送。
- 验证边界：文件清单、源码检索、脚本及 `.uvprojx` 摘要不等于所有源码/脚本全文已复核，不等于 NUEDC 测试执行、CMSIS/RTE 解析、Keil 编译链接、实时资源满足或真实 MCU/FPGA 硬件验证通过。
- `git diff --check`：对本记录追加后的工作树执行，exit 0，通过。
- 下一步：再次完整复核最新日志后，以小范围/逐文件方式补齐重点 host test、runner/stub 与依赖矩阵，再设计隔离 NUEDC Analysis target；实际编辑和 Keil build 结果另行记录。

### W-20260831-048：MCU CMSIS-DSP 依赖与 RTE 工程只读审计

- 时间：2026-08-31T16:26:00+08:00
- 关联要求/未决项：W-20260831-047、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员先完整读取当前 950 行 `docs/PROJECT_WORKLOG.md`，核对 W-047、追加优先、唯一编号、只读范围及验证边界后执行本条追加。
- 只读动作：PowerShell 检查 `mcu/project/STM32H743_Audio/Drivers/CMSIS` 树，确认本地 `Drivers/CMSIS/Include/arm_math.h` 不存在；统计已安装 CMSIS-DSP 1.16.2 Source 分类；检索 canonical NUEDC DSP 的 `arm_*` 符号与未解析 project headers；读取参考 `NUEDC_MAIN(NO_RTOS)` `.uvprojx` 的 RTE XML/分组和当前 `.uvprojx` RTE 尾部。
- 结果：命令 exit 0，约 1.8 秒。本地 H743 CMSIS 只有 Core/device headers，无 `arm_math.h`；Pack Source 分类包含 BasicMath、CommonTables、ComplexMath、Filtering、Statistics、Support、Transform 等。
- 依赖窄化：NUEDC canonical DSP 只读到 `arm_cfft_f32`、instances/const structs、`arm_cmplx_mag_f32`、`arm_fir_f32`/init、`arm_max_f32` 及 `arm_sq` 相关宏，并引用部分 `main.h`；因此不能把整套 Pack Source 无选择加入 Analysis target。
- RTE 证据：参考工程 `<RTE><components>` 登记 `CMSIS:CORE` 6.2.0（pack 6.3.0）与 `CMSIS:DSP:Source` 1.16.2，`RTE_Components.h` 为空壳；当前 AUDIO_DSP `.uvprojx` 的 RTE components 为空。
- 截断与历史边界：此前 W-047 相关读取总输出约 21,243 tokens/1,673 行并被工具截断，不能声称所有头文件/脚本全文已捕获；本条只记录可靠汇总，不能以其替代逐文件复核。
- 修改/构建/Git：无正式文件修改，未构建，未触碰 MPU/VM/Gitee/Git；未暂存、提交或推送，本条仅追加工作记录。
- 验证边界：依赖检索、Pack 分类和 RTE XML 只读证据不等于 CMSIS/RTE 解析、最小源集配置、Keil 编译链接、资源满足或真实 MCU/FPGA 硬件验证通过。
- `git diff --check`：追加后执行，exit 0，通过。
- 下一步：再次完整复核最新日志后，建立隔离 Analysis target 的最小 CMSIS-DSP 依赖映射，并以实际 Keil 解析/构建结果验收；任何工程编辑另行记录。

### W-20260831-049：CMSIS-DSP 精确依赖与 H743 RTE 结构审计动作前计划

- 时间：2026-08-31T16:31:00+08:00
- 关联要求/未决项：W-20260831-048、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员已分块完整读取当前 965 行 `docs/PROJECT_WORKLOG.md`，核对 W-048 的依赖结论、追加优先、唯一编号和只读验证边界。
- 计划动作：主代理在完整审阅本条后，只读检查已安装 CMSIS-DSP 1.16.2 的逐文件精确源文件/头文件依赖，并审阅当前 H743 `.uvprojx`/RTE 结构；不修改正式文件、不构建、不触碰 MPU/VM/Gitee。
- 预期证据与门禁：保留 Pack 文件路径、组件/源头映射、当前工程 RTE 元数据和未决缺口；不得把计划或文件存在性写成 Pack 解析、Keil 编译链接或真实硬件通过。若输出截断，必须明确截断范围并缩小后续读取。
- 当前动作结果：本条仅记录动作前计划，未执行 CMSIS 检查、未构建、未修改正式文件，未执行 MPU/VM/Gitee/Git 操作。
- Git/验证边界：未暂存、提交或推送；后续实际只读审计结果须另行追加，且仍不代表 MCU/FPGA 工程构建或板级验证通过。

### W-20260831-050：CMSIS-DSP 1.16.2 精确文件依赖只读审计

- 时间：2026-08-31T16:38:00+08:00
- 关联要求/未决项：W-20260831-049、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员先完整读取当前 975 行工作记录，核对 W-049 计划门禁、追加优先、只读范围及验证边界后追加本条。
- 只读动作：检查 `C:\Users\LENOVO\AppData\Local\Arm\Packs\ARM\CMSIS-DSP\1.16.2` 的 `Include`、`PrivateInclude`、`Source`；核对目标源文件及其 include：`arm_cfft_f32.c`、`arm_cfft_init_f32.c`、`arm_common_tables.c`、`arm_const_structs.c`、`arm_cmplx_mag_f32.c`、`arm_fir_f32.c`、`arm_fir_init_f32.c`、`arm_max_f32.c`。
- 结果与截断边界：命令 exit 0；输出约 10,137 tokens/878 行并被工具上限截断，不能声称全部源文件、头文件或 include 关系已完整捕获。上述 Pack 路径和目标文件存在性/可读性结果作为可靠汇总保留。
- 修改/构建/Git：未修改正式文件，未构建，未触碰 MPU/VM/Gitee/Git；未暂存、提交或推送。
- 验证边界：Pack 文件存在及部分 include 读取不等于最小依赖闭包已证明、不等于 Keil/RTE 解析、编译链接、资源满足或 MCU/FPGA 板级验证通过。
- 下一步：再次完整复核最新日志后，按单文件和小范围继续读取上述源文件的直接/间接 include，形成可审计的最小 CMSIS-DSP 依赖映射，并另行记录完整捕获结果。

### W-20260831-051：NUEDC DSP 依赖与 AUDIO_DSP uvprojx 结构审计动作前计划

- 时间：2026-08-31T16:44:00+08:00
- 关联要求/未决项：W-20260831-050、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员已分块完整读取当前 986 行工作记录，核对 W-050 的 Pack 截断边界、追加优先、唯一编号和只读验证要求。
- 计划动作：主代理在完整审阅本条后，逐文件审阅 NUEDC canonical `STM32H743/DSP` 全部 `#include`、外部符号与宏依赖，并解析 AUDIO_DSP 当前 `.uvprojx` XML 的真实节点结构，为可编译隔离 target 设计依赖适配。
- 门禁：仅执行只读源码/工程 XML 分析，不修改任何正式文件，不构建，不触碰 MPU/VM/Gitee；如输出需分块，必须明确未读取部分，不将截断输出写成完整审计或构建通过证据。
- 当前动作结果：本条仅记录动作前计划，未执行上述审计；未修改正式文件、未构建、未执行 MPU/VM/Gitee/Git 操作。
- Git/验证边界：未暂存、提交或推送；后续结果须另行追加，并继续区分依赖设计、Keil 编译链接与真实 MCU/FPGA 硬件验证。

### W-20260831-052：NUEDC DSP 逐文件依赖与 AUDIO_DSP uvprojx 结构只读审计

- 时间：2026-08-31T16:52:00+08:00
- 关联要求/未决项：W-20260831-051、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员先完整读取当前 996 行工作记录，核对 W-051 计划、追加优先、只读范围和验证边界后追加本条。
- 只读动作：枚举固定 NUEDC SHA `416270a254795843b6f1854074380015637d1f95` 的 `tmp/upstream/NUEDC_2026/STM32H743/DSP` 27 个文件及大小/行数；分析 include map；解析当前 AUDIO_DSP `.uvprojx` XML 节点与两个 target。
- 结果：命令 exit 0。include map 显示仅 canonical headers 与标准 C 头；`Demod.c` 依赖 `arm_const_structs.h`；按筛选确认未解析外部头仅 `main.h`。当前 `.uvprojx` 的 RTE 位置为 `TargetOption.TargetCommonOption.RTE`，两个 target 均为空/无有效组件；分组计数约为 MDK1、HAL20、CMSIS1、Core7、Audio3、DSP50。
- 结论：可据此设计隔离 target 的依赖适配边界，但现有工程 RTE 未提供有效组件配置，不能直接声称 NUEDC 依赖已接入。
- 修改/构建/Git：未修改正式文件，未构建，未触碰 MPU/VM/Gitee/Git；未暂存、提交或推送。
- 验证边界：本条为只读摘要，不等于全部源码逐行内容、全部 XML 节点/分组细节、Pack/RTE 解析、Keil 编译链接、实时资源满足或真实 MCU/FPGA 硬件验证通过。
- 下一步：再次完整复核最新日志后，在小范围内补齐未读取的源文件依赖细节与 XML 节点核对，再设计并验证隔离 target。

### W-20260831-054：MCU 融合前源码、脚本与 RTE 结构只读审计

- 时间：2026-08-31T17:08:00+08:00
- 关联要求/未决项：W-20260831-053、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员先完整读取当前 1018 行工作记录，核对 W-053 融合计划、来源与授权边界、追加优先和验证门禁后追加本条。
- 只读动作：读取 `postgenerate_keil.ps1`、`build_keil.ps1`、`verify_all.ps1`、`audio_app.c/h`、`main.c`、当前 `.uvprojx` target/groups，以及参考 `.uvprojx` RTE 节点和 NUEDC 重点 API/test marker。
- 结果与截断边界：首个聚合输出约 11,462 tokens/1,230 行并被工具截断；随后窄化查询 exit 0。因此不能声称所有脚本、头文件、XML 或测试内容已完整捕获。
- 可靠结论：当前脚本仍为 2 targets、49 objects，并清空 RTE；参考 RTE root `/Project/RTE` 使用 CMSIS CORE 6.2.0（pack 6.3.0）与 DSP Source 1.16.2，且只绑定单 target。FFT、Measure、Adaptive、FilterEx API 具备用于启动时自检的候选接口。
- 修改/构建/Git：未修改正式文件，未构建，未触碰 MPU/VM/Gitee/Git；未暂存、提交或推送。
- 验证边界：本条是只读摘要，不等于全源码/脚本/XML 已审完，不等于第三个 target 已编辑、Pack/RTE 已解析、Keil 已构建或真实 MCU/FPGA 硬件通过。
- 下一步：再次完整复核最新日志后，按 W-053 计划在隔离目录实施可追溯融合与第三个 Analysis target；编辑、静态检查和构建结果分别记录。

### W-20260831-053：MCU NUEDC canonical 融合与 Analysis target 动作前计划

- 时间：2026-08-31T17:00:00+08:00
- 关联要求/未决项：W-20260831-052、P0、P2；主动范围仍冻结为 MCU+FPGA，不恢复 MPU，且不做 VM/Gitee。
- 动作前审阅：记录员已分块完整复核当前 1008 行工作记录，核对 W-052 的只读依赖/RTE 结论、来源 SHA、追加优先及验证边界。
- 计划动作：主代理在完整审阅本条后，将固定 SHA `416270a254795843b6f1854074380015637d1f95` 的 H743/DSP canonical 27 文件以可追溯来源复制到正式 MCU 工程隔离目录；新增来源/授权边界说明、CMSIS-DSP 1.16.2 依赖映射和自检适配层；扩展 CubeMX/Keil 脚本与 `.uvprojx` 为第三个 NUEDC Analysis target，同时保持既有 SelfTest/WM8960 不变；随后按计划运行静态检查与 Keil 构建。
- 门禁：本条仅记录计划；主代理必须再次完整审阅最新日志后才可编辑。复制/融合不得把无明确授权写成已获许可；构建、静态检查和工程级结果不得写成真实硬件通过。
- 当前动作结果：本条未执行复制、编辑、脚本扩展、静态检查或构建，未触碰 MPU/VM/Gitee。
- Git/验证边界：未暂存、提交或推送；既有脏工作树保持不动。后续必须分别记录修改文件、源 SHA、构建原始输出、失败和硬件验证缺口。

### W-20260831-055：MCU NUEDC 融合与第三 Analysis target 编辑阶段动作前计划

- 时间：2026-08-31T17:15:00+08:00
- 关联要求/未决项：W-20260831-053、W-20260831-054、P0、P2；主动范围冻结为 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 继续停止。
- 动作前审阅：记录员已完整分块读取 `docs/PROJECT_WORKLOG.md` 最新全文（当前 1031 行，含 W-20260831-054 与 W-20260831-053 的既有插入顺序异常），核对用户范围、追加优先、NUEDC 固定来源 SHA、许可证/授权边界和“构建不等于硬件通过”门禁；历史条目不改写。
- 计划动作：主代理仅在 MCU H743 工程执行以下编辑：将固定 NUEDC canonical `STM32H743/DSP` 27 文件复制到正式工程隔离目录；新增来源/授权与 CMSIS-DSP 1.16.2 依赖说明；新增非 ISR 的分析自检适配层；扩展 CubeMX/Keil 再生成脚本及 `.uvprojx` 为第三个 `NUEDC Analysis` target，同时保持既有 `SelfTest`、`WM8960 Stream` target 不变。编辑完成后回传精确修改路径、源 SHA、命令和结果，记录员再追加实际结果；在结果追加前不开始构建。
- 禁止/范围：本阶段不触碰 `mpu/`、Ubuntu/VM、任何 Gitee 入口、FPGA 源码或 Vivado 工程；不删除历史记录或无关脏工作树；不得把 NUEDC 无明确许可证写成已获公开再分发许可。
- 当前动作结果：本条仅记录编辑前计划；截至追加时没有执行复制、编辑、静态检查、构建、Git 暂存/提交/推送或硬件操作。
- 验证边界：计划本身不证明文件已复制、第三 target 已创建、CMSIS/RTE 已解析、Keil 可编译链接、实时资源满足或 MCU/FPGA 板测通过；后续必须分别记录成功、失败、原始输出和边界。
- Git：未暂存、未提交、未推送；既有工作树状态保持不动。
- 下一步：主代理按本计划完成隔离 MCU 编辑并回传精确结果；记录员在追加实际结果前继续要求其先完整复核本记录。

### W-20260831-056：MCU 融合计划记录的差异格式检查

- 时间：2026-08-31T17:16:00+08:00
- 关联要求/未决项：W-20260831-055、P0、P2、P4。
- 动作前审阅：记录员已核对 W-055 追加后的最新记录及其只读/追加边界；本次只验证工作记录自身格式，不进入源码编辑或构建。
- 命令/工具：`git diff --check -- docs/PROJECT_WORKLOG.md`。
- 修改文件：无（命令为只读检查）。
- 结果与原始证据：命令退出码 0、无输出，说明当前工作记录差异未发现空白/冲突标记格式错误。
- 验证边界：该检查只覆盖 Git diff 空白错误，不证明 MCU/FPGA 源码、Keil/Vivado 工程或硬件状态；未执行 MPU/VM/Gitee 动作。
- Git：未暂存、未提交、未推送；既有工作树保持原状。
- 下一步：等待主代理按 W-055 完成 MCU 编辑并回传精确文件、命令、结果和失败边界；其后再追加实际编辑结果并允许构建阶段开始。

### W-20260831-057：NUEDC canonical 文件机械复制首次尝试失败（保留）

- 时间：2026-08-31T17:29:00+08:00
- 关联要求/未决项：W-20260831-055、W-20260831-056、P0、P2；主动范围仍仅 MCU/FPGA，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已在动作前完整复核 W-055/W-056 最新记录；记录员依据已完整分块审阅的记录核对复制范围、固定来源 SHA、追加优先及“失败不得覆盖”门禁后追加本条。
- 计划动作：把固定 NUEDC canonical `STM32H743/DSP` 内容复制到 `mcu/project/STM32H743_Audio/App/NUEDC` 的隔离目录；本次只记录首次机械复制尝试，不进入构建。
- 命令/工具：PowerShell `Copy-Item -LiteralPath (Join-Path $src '*') -Destination $dst -Force`。
- 结果与原始错误：命令失败，PowerShell 报 `Cannot find path ...\DSP\*`；通配符作为 `-LiteralPath` 未展开。命令只创建了精确目标空目录 `mcu/project/STM32H743_Audio/App/NUEDC`，随后 `COUNT=0`；没有复制任何 NUEDC 源文件。
- 修改文件/系统：新增目标空目录；无源文件被覆盖，无其他 MCU/FPGA/系统/VM 修改。失败目录未删除，以便下一次精确重试并保留证据。
- 验证边界：本条仅证明首次复制命令失败及空目录状态，不证明来源文件已融合、许可证/授权已解决、第三 target 已配置、Keil 可编译或硬件通过；未执行 MPU/VM/Gitee、静态检查、构建、Git 暂存/提交/推送。
- Git：未暂存、未提交、未推送；既有工作树其他改动保持不动。
- 下一步：主代理在再次完整复核本记录后，改用 `Get-ChildItem` 管道或逐文件精确复制，核对 27 文件及固定 SHA，再回传结果；记录员追加成功或后续失败后才进入工程脚本编辑/构建。

### W-20260831-058：NUEDC canonical 27 文件精确复制成功及补丁解析失败（保留）

- 时间：2026-08-31T17:52:00+08:00
- 关联要求/未决项：W-20260831-055、W-20260831-057、P0、P2；主动范围仍仅 MCU/FPGA，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已在重试前完整复核 W-055～W-057 最新记录；记录员核对首次复制失败、精确目标范围、追加优先及“解析失败不得写成修改成功”门禁后追加本条。
- 计划动作：在已创建的隔离目录内精确复制固定 NUEDC canonical DSP 文件，并随后新增来源/适配说明；每个补丁调用单独保留解析失败。
- 复制命令/工具：PowerShell 使用 Get-ChildItem 管道，从 `tmp/upstream/NUEDC_2026/STM32H743/DSP` 复制到 `mcu/project/STM32H743_Audio/App/NUEDC`，复制后统计并逐文件计算 SHA-256。
- 复制结果与证据：`SRC_COUNT=27`、`DST_COUNT=27`、`MISMATCH_COUNT=0`；逐文件 SHA-256 全部相同；来源 HEAD 为 `416270a254795843b6f1854074380015637d1f95`。这证明 27 个文件已按固定审计源复制到隔离目录。
- 补丁失败（全部保留）：新增适配/说明文件的第一次 apply_patch 调用在 functions.exec JavaScript 解析阶段因 `\\1` 触发 `Octal escape sequences are not allowed in strict mode`；第二、第三次使用 String.raw 模板又分别因补丁正文中的 Markdown 反引号触发 `Unexpected identifier 'STM32H743'` 与 `Unexpected identifier 'mcu'`。三次均发生在 apply_patch 真正调用前，因此没有因这些失败新增或改写文件。
- 同阶段其他修改状态：`mcu/scripts/postgenerate_keil.ps1` 的先前 apply_patch 已实际写入修改，但尚未完成静态检查；主代理已发现其中 XPath 字符串疑似多余括号，待下一次小补丁复核/修正。除该脚本外，来源/适配说明文件、audio_app、THIRD_PARTY 尚未改变。
- 验证边界：复制哈希证明只覆盖 27 文件传输完整性，不证明许可证/授权已解决、适配层已存在、脚本语法/目标配置正确、Keil 编译链接或真实 MCU/FPGA 硬件通过；本阶段未构建、未执行 MPU/VM/Gitee、未做 Git 暂存/提交/推送。
- Git：未暂存、未提交、未推送；其他并行 FPGA/文档脏工作保持不动。
- 下一步：主代理在完整复核本条后，将补丁拆成无反斜杠/无反引号的小块，先核对 postgenerate 脚本差异，再新增来源/适配文件并回传精确 diff 与静态检查结果；记录员追加后才允许进入构建。

### W-20260831-059：MCU NUEDC 隔离融合编辑完成（尚未构建）

- 时间：2026-08-31T18:00:00+08:00
- 关联要求/未决项：W-20260831-055、W-20260831-058、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已按 W-058 完整复核最新记录；记录员核对复制哈希、补丁解析失败、许可证边界及“先编辑后静态检查/构建”门禁后追加本条。
- 实际编辑文件与内容：
  - 已确认隔离目录 `mcu/project/STM32H743_Audio/App/NUEDC` 内固定来源的 27 个 canonical 文件保持完整。
  - 新增 `mcu/project/STM32H743_Audio/App/Src/nuedc_analysis_selftest.c` 与 `.h`；启动时自检覆盖 NUEDC Measure、Adaptive statistics、FilterEx biquad，以及 CMSIS-DSP 64-point CFFT/magnitude；适配层明确不在 SAI/DMA ISR 中执行重分析。
  - 新增 `mcu/project/STM32H743_Audio/App/NUEDC/ORIGIN.md`，记录源仓库、固定 SHA、无许可证断言和用户授权边界；新增 `CMSIS-DSP_DEPENDENCY.md` 依赖说明。
  - 修改 `mcu/project/STM32H743_Audio/App/Inc/audio_app.h`，新增状态码 `-5`；修改 `audio_app.c`，增加 `AUDIO_BOARD_ENABLE_NUEDC_ANALYSIS` 条件宏，并在启动自检阶段调用适配层。
  - 修改 `THIRD_PARTY_NOTICES.md`，增加 NUEDC provenance 条目。
- 同阶段未决修改：`mcu/scripts/postgenerate_keil.ps1` 的第三 target/RTE patch 仍在工作树，尚未修复或验证 XPath 字符串疑似多余括号；因此 CubeMX/Keil 工程配置尚不能视为完成。
- 结果：上述 apply_patch 均成功，新增/修改内容已写入工作树；本轮没有再次出现解析失败。未触碰 FPGA、MPU、VM 或 Gitee。
- 验证边界：本条只证明源码/说明文件编辑落盘及设计意图，不证明 C 语法、CMSIS/RTE 解析、第三 target 工程配置、Keil 编译链接、实时资源或真实 MCU/FPGA 硬件通过；截至追加时未执行静态检查、构建、Git 暂存/提交/推送。
- Git：未暂存、未提交、未推送；并行 FPGA/文档脏工作保持不动。
- 下一步：主代理在再次完整复核本条后先核对 `postgenerate_keil.ps1` 精确 diff、修正 XPath/target 配置并执行 PowerShell/XML/源文件静态检查；把完整结果回传记录员，追加后才运行 Keil 构建。

### W-20260831-060：MCU Keil 后处理脚本 XPath 修正尝试（尚未构建）

- 时间：2026-08-31T21:03:27+08:00
- 关联要求/未决项：W-20260831-059、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已在 W-059 后继续遵守最新记录门禁；记录员已完整分块审阅当时最新的 1098 行工作记录，核对 W-059 的编辑状态、`postgenerate_keil.ps1` 未决风险、追加优先及“静态解析不等于工程构建”边界后追加本条。
- 本阶段范围：主代理只对 `mcu/scripts/postgenerate_keil.ps1` 进行语法检查和补丁修正尝试，尚未运行工程生成或构建；未触碰 FPGA、MPU、VM 或 Gitee。
- PowerShell 语法证据：Parser 静态解析返回 `PARSE_OK=True`。该结果只证明当时脚本可被 PowerShell parser 解析，不证明 XPath 在运行时正确，也不证明 Keil target、RTE、group 或文件列表配置正确。
- 第一次补丁失败（保留）：第一次 `apply_patch` 因同一文件包含 multiple operations 被工具拒绝；该次调用没有产生修改。
- 第二次补丁失败（保留）：第二次合并补丁因上下文不匹配失败；该次调用没有产生修改。
- 成功修正：随后单行 `apply_patch` 成功，把用于移除 CMSIS 组件的 XPath 从末尾含多余右括号的字符串修正为 `component[@Cclass='CMSIS' and (@Cgroup='CORE' or @Cgroup='DSP')]`。
- 修改文件：本阶段实际修改仅为 `mcu/scripts/postgenerate_keil.ps1` 上述单行 XPath；未修改其他 MCU/FPGA 文件。
- 未完成项：尚未逐项核对脚本其余 target/group/RTE 节点，尚未运行 XML 解析、CubeMX/工程再生成、Keil 编译链接或任何硬件测试。
- 验证边界：`PARSE_OK=True` 与单行补丁落盘不等于 XPath 运行时选择正确、第三个 Analysis target 可生成、CMSIS-DSP Pack/RTE 可解析、Keil 通过或真实硬件通过。
- Git：未暂存、未提交、未推送；既有并行 FPGA/文档脏工作保持不动。
- 下一步：主代理必须先完整复核 W-060；随后只读核对当前脚本精确结构，再用 `apply_patch` 修复剩余三目标生成问题并执行 PowerShell/XML/源文件静态检查。结果回传记录员追加后，才进入 Keil 构建。

### W-20260831-061：MCU 三目标脚本编辑完成及首次静态检查在 Parser 门禁失败

- 时间：2026-08-31T21:11:12+08:00
- 关联要求/未决项：W-20260831-060、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已完整复核截至 W-060 的工作记录全文和新增尾部，确认先编辑三目标脚本、再执行静态门禁且失败不得写成构建成功；记录员依据已审阅基线核对追加优先和验证边界后追加本条。
- 成功编辑：`apply_patch` 已修改 `mcu/scripts/postgenerate_keil.ps1`、`mcu/scripts/regenerate_cubemx.ps1`、`mcu/scripts/build_keil.ps1`、`mcu/scripts/verify_all.ps1` 四个文件；本阶段没有修改其他路径。
- `postgenerate_keil.ps1` 编辑内容：把 two-target 注释改为 three-target；将 Analysis group 改为查找复制后的 `Groups` 再追加；增加三个 target、`.uvoptx`、group 与 RTE 断言。
- `regenerate_cubemx.ps1` 编辑内容：把双目标门禁扩展为三个目标，并增加 Analysis define、group、source 与 RTE 检查。
- `build_keil.ps1` 编辑内容：`ValidateSet` 扩展为 `SelfTest/WM8960/NUEDC/Both/All`，默认 `All`；增加三个 target、RTE/Pack 门禁与 NUEDC 14 object 检查；构建超时设为 300 秒。
- `verify_all.ps1` 编辑内容：构建调用显式指定 `-Target All`。
- 首次静态检查：随后执行包含四脚本 PowerShell Parser、临时 smoke 复制/运行、XML/hash/diff 检查的一体命令；命令在第一个 `postgenerate_keil.ps1` Parser 门禁即失败并中止，后续 smoke、XML、hash 与 diff 检查均未执行。
- 原始错误摘要：Parser 报 `Unexpected token '{'`、`foreach` 缺少 closing `)`、method call 缺 `)`，并在 `$source`、`(`、`nuedc_analysis_selftest.c`、adapter header 等附近产生连锁错误。
- 当前判断：该失败证明 `postgenerate_keil.ps1` 仍存在 PowerShell 语法错误，当前不能进入 CubeMX/Keil 构建；尚未通过 Parser Error Extent 和逐行定位确认唯一根因，不得直接把全部连锁错误归结为某一处。
- 未执行项：未运行临时 smoke、XML/hash/diff 静态门禁，未执行 CubeMX 再生成、Keil 编译链接或硬件测试；未触碰 FPGA、MPU、VM、Gitee，也未执行 Git 暂存、提交或推送。
- 验证边界：四文件补丁落盘不等于脚本可解析或运行；Parser 失败更不等于第三个 target、CMSIS-DSP/RTE、NUEDC objects、资源或真实硬件已验证。
- Git：未暂存、未提交、未推送；既有并行 FPGA/文档脏工作保持不动。
- 下一步：主代理必须先复核 W-061，再只读输出 Parser Error Extent 与精确行号；随后以最小 `apply_patch` 修复并重跑全部静态门禁，结果另行回传记录员后才允许进入构建。

### W-20260831-062：MCU 脚本 Parser 精确修复通过及 smoke 运行时类型冲突失败

- 时间：2026-08-31T21:14:38+08:00
- 关联要求/未决项：W-20260831-060、W-20260831-061、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已复核 W-061 尾部并遵守“先定位、最小修复、重跑静态门禁、失败即停止”边界；记录员核对此前完整基线、W-060/W-061 新增内容与追加优先规则后追加本条。
- Parser 精确定位：只读 Error Extent 显示 line 214 的 `foreach` 缺少一个闭合 `)`；lines 288、291、294、299 的多行命令缺少续行反引号，其他报错均为这些语法缺陷造成的连锁错误。
- W-060 勘误边界：line 214 原有三重 `)))` 分别闭合 `SelectNodes`、`@()` 和 `foreach` 条件，不是多余括号。W-060 的“多余右括号”判断保留为历史误判，本条以 Parser 精确证据纠正，不改写原条。
- 最小修复：`apply_patch` 成功把 line 214 恢复为三重 `)))`，并在四处 `New-SourceFileNode` 多行调用补入 PowerShell 续行反引号。
- Parser 重验：`postgenerate_keil.ps1`、`regenerate_cubemx.ps1`、`build_keil.ps1`、`verify_all.ps1` 四个脚本均返回 `PARSER_OK`。这只证明 PowerShell 语法解析通过，不证明运行时、XML 或 Keil 工程正确。
- 临时 smoke：在 `mcu/build` 下的忽略临时目录复制当前 `.uvprojx`/`.uvoptx` 并运行 `postgenerate_keil.ps1`；脚本运行至 `New-RteComponent` line 229 失败，原始错误为 `Cannot convert the System.Xml.XmlElement value ... to type System.Collections.Hashtable`。
- 运行时根因：PowerShell 变量名大小写不敏感；函数参数 `[hashtable]$Package` 与局部变量 `$package = CreateElement(...)` 冲突。把 `XmlElement` 赋给受参数类型约束的同名变量时触发转换失败。
- 中止边界：smoke 失败后，后续 XML、hash 和 diff 门禁均未执行；没有修改正式 `.uvprojx`，仅在 `mcu/build` 下留下被忽略的临时 smoke 文件。
- 未执行项：未运行 CubeMX 再生成、Keil 编译链接或硬件测试；未触碰 FPGA、MPU、VM 或 Gitee；未执行 Git 暂存、提交或推送。
- 验证边界：四脚本 `PARSER_OK` 与根因定位不等于 postgenerate 可运行完成、三 target/RTE/XML 正确、Keil 可构建或真实硬件通过。
- Git：未暂存、未提交、未推送；正式 `.uvprojx` 未变，既有并行 FPGA/文档脏工作保持不动。
- 下一步：主代理必须先复核 W-062，再以最小补丁把函数参数/局部变量重命名为无冲突的 `PackageAttributes`/`packageNode`，随后从头重跑完整静态门禁；结果另行回传记录员后才允许进入 Keil 构建。

### W-20260831-063：MCU postgenerate 运行时冲突修复及临时副本完整静态门禁通过

- 时间：2026-08-31T21:17:20+08:00
- 关联要求/未决项：W-20260831-062、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已复核 W-062，确认只做变量冲突最小修复并从头重跑全部静态门禁；记录员核对此前完整基线、运行时失败根因、追加优先及“临时副本不等于正式工程”边界后追加本条。
- 最小修复：`apply_patch` 成功把 `New-RteComponent` 参数 `$Package` 重命名为 `$PackageAttributes`，并把局部 XML 节点 `$package` 重命名为 `$packageNode`，消除 PowerShell 大小写不敏感造成的参数类型约束冲突。
- 完整静态门禁结果：从头执行的一体命令 exit 0；`postgenerate_keil.ps1`、`regenerate_cubemx.ps1`、`build_keil.ps1`、`verify_all.ps1` 四脚本均返回 `PARSER_OK`。
- 临时 smoke：在被忽略的 `mcu/build/postgenerate_smoke_20260831_211648` 中复制当前正式 `.uvprojx`/`.uvoptx` 并运行 `postgenerate_keil.ps1`，脚本成功完成。
- 临时 XML targets：project/options 中 targets 精确为 `SelfTest`、`WM8960_Stream`、`NUEDC_Analysis` 三个；SelfTest 与 WM8960 各 `ANALYSIS_GROUPS=0`，Analysis 为 `ANALYSIS_GROUPS=1`，并且 `DIRECT_TARGET_GROUPS=0`，证明 Analysis group 位于 `Groups` 节点下而不是直接挂在 target 下。
- 临时宏配置：三个 target 的 stream/analysis 宏分别为 `0/0`、`1/0`、`0/1`。
- 临时 RTE 配置：`CMSIS:CORE` 6.2.0 来自 ARM CMSIS Pack 6.3.0；`CMSIS:DSP:Source` 1.16.2 来自 ARM CMSIS-DSP Pack 1.16.2；二者 `targetInfo` 均只绑定 Analysis target。
- 来源完整性：固定 canonical 来源与正式隔离目标各有 27 个文件，逐文件 SHA-256 `MISMATCH=0`。
- 差异检查：本阶段指定路径的 `git diff --check` exit 0。
- 正式工程边界：上述证据只来自忽略目录中的临时 `.uvprojx`/`.uvoptx` 副本及静态 XML/hash 检查；正式 `.uvprojx` 仍是旧双 target，尚未执行正式 CubeMX 再生成、Keil 编译链接或硬件测试。
- 范围/Git：未触碰 FPGA、MPU、VM 或 Gitee；未暂存、未提交、未推送；既有并行 FPGA/文档脏工作保持不动。
- 验证边界：临时 postgenerate/XML/hash 门禁通过不等于正式三 target 已生成、Keil Pack/RTE 实际编译链接成功、资源满足或真实硬件通过。
- 下一步：主代理必须先复核 W-063，再运行正式 `regenerate_cubemx.ps1`（其会调用 postgenerate）并检查正式三 target；正式再生成结果追加记录后，才允许执行 Keil `All` 构建。

### W-20260831-064：MCU Keil 脚本只读子代理审计及 W-060 时点结论校正

- 时间：2026-08-31T21:19:01+08:00
- 关联要求/未决项：W-20260831-060～W-20260831-063、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 审计基线：只读子代理 `mcu_keil_audit` 完整读取了 W-060 时点的 1114 行工作记录，再读取四个 MCU 脚本、正式 `.uvprojx`/`.uvoptx`、DebugConfig/RTE、参考 NUEDC RTE 与 CMSIS-DSP PDSC。该代理没有编辑、构建、Git、FPGA、MPU、VM 或 Gitee 动作。
- 时序校正：子代理当时报告的 `postgenerate` 21 个 parser errors、Group 直挂 Target、其余脚本仍为双目标，均是 W-060 时点的原始 P0；这些问题已由后续 W-061～W-063 的主代理补丁与临时 smoke 静态门禁处理，不能写成当前仍存在的缺陷。
- 当前仍有效的隔离建议：`../App/NUEDC` include 只能加入 Analysis clone，不能先加入 base target 后让三个 target 继承。
- 当前仍有效的可重入建议：清理旧分组时应同时覆盖 `Library/NUEDC Analysis`，避免重复运行后残留或重复 group。
- 当前仍有效的保存后断言：必须确认不存在 Target/Group 直挂；Application 组精确为 3 个 C；permissive DSP 精确为 49 个 C；所有 `FilePath` 非空；三个 target 宏矩阵正确；只有 Analysis target 含 NUEDC include/group，且该 group 精确为 14 个 C 与 15 个 H；RTE 组件版本与 `targetInfo` 精确；`.uvoptx` 仅 SelfTest 为 current。
- DebugConfig 建议：直接运行 postgenerate 时，若 CubeMX base `dbgconf` 不存在，应从已有 final `dbgconf` 补齐 Analysis；正式 regenerate 后还必须断言三份 target 调试配置均存在。
- 调用建议：`regenerate_cubemx.ps1` 调用 postgenerate 时应显式传入 `ProjectPath`，避免依赖隐式默认路径。
- CMSIS-DSP 边界：`CMSIS:DSP:Source` 是聚合全组件，不是最小源集；是否可接受必须以后续实际 Keil build 与资源占用记录为依据，不能把 Pack/RTE 元数据存在写成最小依赖闭包或资源合格。
- 正式工程现状：正式 `.uvprojx` 仍是旧双 target 且 RTE empty；这与 W-063 已记录的待物化状态一致。子代理只读审计未改变该状态。
- 修改/构建/Git：本阶段除追加工作记录外无正式文件修改，未运行 CubeMX、Keil 或硬件测试，未触碰 FPGA/MPU/VM/Gitee，未暂存、提交或推送。
- 验证边界：子代理的 XML/PDSC/脚本审计与建议不等于增强项已实现、正式三 target 已生成、Keil 编译链接或真实硬件通过。
- 下一步：主代理先复核 W-064，补强 Analysis 隔离、可重入清理、保存后断言、DebugConfig 与显式 `ProjectPath` 调用；随后重新运行临时 smoke 并记录结果，之后才允许正式 regenerate。

### W-20260831-065：MCU 三目标后处理补强补丁已落盘（尚未验证）

- 时间：2026-08-31T21:31:48+08:00
- 关联要求/未决项：W-20260831-063、W-20260831-064、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已复核 W-064 后再执行本次单一 `apply_patch`；记录员追加本条前完整分块逐行读取当前 1184 行工作记录，核对物理末尾为 W-20260831-064、最大唯一编号为 064、历史重复 W-034 保留，以及“补丁落盘不等于脚本/工程/构建通过”的验证边界。
- 实际动作：一次 `apply_patch` 成功修改 `mcu/scripts/postgenerate_keil.ps1`、`mcu/scripts/regenerate_cubemx.ps1`、`mcu/scripts/build_keil.ps1` 三个文件；本阶段没有修改第四个 `verify_all.ps1`。
- `postgenerate_keil.ps1` 补强：从 base target include path 移除 `../App/NUEDC`，只向 Analysis clone 添加；清理旧 `Library/NUEDC Analysis` group 以支持重入；DebugConfig 源优先使用 CubeMX base、否则使用已有 final 配置，补齐三份目标配置并避免源/目标相同时自复制。
- `postgenerate_keil.ps1` 保存后断言：新增 Application 精确 3 个 C、permissive DSP 精确 49 个 C、三个 target 宏矩阵、仅 Analysis 含 NUEDC include/group、Analysis group 精确 14 个 C 与 15 个 H、无 Target/Group 直挂、无空 `FilePath`、RTE CORE 6.2.0 / ARM CMSIS Pack 6.3.0 与 DSP Source 1.16.2 精确版本及 Analysis-only `targetInfo`、`.uvoptx` 仅 SelfTest 为 current 等门禁。
- `regenerate_cubemx.ps1` 补强：调用 postgenerate 时显式传入 `ProjectPath`，并在再生成结果中断言三份 target DebugConfig 均存在。
- `build_keil.ps1` 补强：构建前新增三 target 宏矩阵、group/include 隔离和三份 DebugConfig 存在性断言，以便在调用 Keil 前阻止错误工程进入构建。
- 结果：补丁工具报告成功，上述三文件修改已写入工作树；截至本条追加时尚未运行四脚本 Parser、临时 postgenerate smoke、XML/hash/diff 门禁、正式 CubeMX 再生成、Keil 编译链接或硬件测试，因此当前修改仍可能含 PowerShell 语法、运行时或 XML/工程结构错误。
- 范围/Git：未触碰 FPGA、MPU、VM 或 Gitee；未暂存、未提交、未推送；既有 FPGA/文档脏工作继续保留。
- 验证边界：补丁成功只证明文本编辑落盘，不证明三 target/RTE/DebugConfig 可生成、CMSIS-DSP Pack 可解析、Keil 可构建、资源满足或真实 MCU/FPGA 硬件通过。
- 下一步：主代理必须先完整复核本条，再从头运行四脚本 PowerShell Parser 与临时 postgenerate smoke；同时执行 XML/来源 hash/`git diff --check` 门禁。任一失败即停止并回传原始错误，全部通过后才允许正式 regenerate。

### W-20260831-066：MCU 三目标后处理双次可重入 smoke 与完整静态门禁通过

- 时间：2026-08-31T21:36:30+08:00
- 关联要求/未决项：W-20260831-064、W-20260831-065、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理报告已复核 W-065 后才执行本阶段；记录员追加本条前再次完整分块逐行读取当时最新的 1199 行工作记录，确认物理末尾 W-20260831-065、最大唯一编号 065、历史重复 W-034 保留，以及“临时副本静态通过不等于正式工程或构建通过”的边界。
- 命令/工具：主代理执行一体 PowerShell 静态门禁，依次解析四个 MCU 脚本、建立忽略目录临时工程副本、在同一临时副本上连续两次运行 `postgenerate_keil.ps1`、解析临时 `.uvprojx`/`.uvoptx`、核对 DebugConfig、来源哈希并对指定路径运行 `git diff --check`。
- 总结果：一体静态门禁 exit 0；`postgenerate_keil.ps1`、`regenerate_cubemx.ps1`、`build_keil.ps1`、`verify_all.ps1` 均输出 `PARSER_OK`。
- 可重入 smoke：在被忽略目录 `mcu/build/postgenerate_smoke_20260831_213446` 中复制正式旧双 target `.uvprojx`、`.uvoptx` 及 DebugConfig；对同一临时工程连续运行 postgenerate 两次，两次均输出已配置并同步三 target，证明当前测试输入和断言覆盖范围内可重入。
- 临时 XML target/group 证据：project/options 的 target 精确为 `SelfTest|WM8960_Stream|NUEDC_Analysis`；每个 target 的 Application C 精确为 3、permissive DSP C 精确为 49；SelfTest/WM8960 的 Analysis group 均为 0 且不含 NUEDC include，Analysis 的对应 group 为 1 且含 NUEDC include；三个 target 均无 Target 直挂 Group。
- 临时宏/RTE/options 证据：三个 target 的 stream/analysis 宏矩阵依次为 `0/0`、`1/0`、`0/1`；RTE CORE 6.2.0 来自 ARM CMSIS Pack 6.3.0，DSP Source 1.16.2 来自 ARM CMSIS-DSP Pack 1.16.2，二者均只绑定 Analysis；`.uvoptx` 仅 SelfTest 为 current。
- 临时 DebugConfig 与来源证据：临时工程内 target DebugConfig 精确为三份；canonical 来源和正式隔离目标均为 27 个文件，逐文件 SHA-256 mismatch 为 0。
- 差异检查：本阶段指定路径的 `git diff --check` exit 0。
- 正式工程边界：正式 `.uvprojx`/`.uvoptx` 仍未再生成；本阶段没有运行正式 `regenerate_cubemx.ps1`、CubeMX 生成、Keil 编译链接或硬件测试。临时双次 postgenerate 成功只证明当前后处理脚本在该临时输入和断言范围内通过，不证明正式工程已物化、Pack 编译资源合格或板上音频工作。
- 范围/Git：未触碰 FPGA、MPU、VM 或 Gitee；未暂存、未提交、未推送；既有 FPGA/文档脏工作继续保留。
- 下一步：主代理必须先完整复核 W-066，再运行正式 `regenerate_cubemx.ps1`；若失败立即停止并记录原始错误。只有正式再生成通过且正式三 target/DebugConfig/RTE 等门禁复核完成并另行记录后，才允许执行 Keil `All` 构建。

### W-20260831-067：长任务中断恢复与持久检查点新增硬要求（动作前计划）

- 时间：2026-08-31T21:39:49+08:00
- 关联要求/未决项：用户最新长期记录硬要求、D-008、P0、P4、W-20260831-066；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 用户最新硬要求：本长任务会不定期中断后再由用户要求“继续”；必须妥善保存工作记录，并允许主代理自主采取必要的项目内恢复措施，保证每次恢复都能无缝衔接，不丢失已完成动作、失败、验证证据、边界与下一门禁。
- 动作前审阅：记录员在追加本条前再次完整分块逐行读取当时最新的 1215 行工作记录，确认物理末尾 W-20260831-066、最大唯一编号 066、历史重复 W-034 保留，以及正式 MCU regenerate 仍未执行的当前阶段边界。
- 权威历史决策：`docs/PROJECT_WORKLOG.md` 继续作为 append-only 权威过程历史；不得用摘要、检查点或记忆覆盖、删改、重排其中任何成功或失败记录。
- 可恢复性交付门禁：计划新增一个非冗余、机器可读的“当前检查点”，只保存当前有效范围、最后记录编号、HEAD/远端状态、当前阶段、已验证证据、失败/验证边界和唯一下一动作；它是恢复索引，不复制完整时间线，也不替代本工作记录。
- 只读恢复入口计划：计划新增一个只读恢复脚本，一次输出工作记录完整性、当前检查点、Git/远端状态、关键 MCU/FPGA 原生工程入口和下一门禁；脚本不得修改工程、工作树、远端、工具链或硬件。
- 持续同步计划：每个阶段完成并记录后更新机器可读检查点，随对应阶段小提交一并推送并核对远端，使本地中断、对话中断或代理更换后仍可从版本化证据恢复。
- Codex 记忆计划：因用户本条明确要求保存长期记录，主代理计划按 Codex 记忆更新规则仅在 `C:\Users\LENOVO\.codex\memories\extensions\ad_hoc\notes\` 新增一条小型 update note，提示未来恢复先读取权威工作记录与检查点，并保持主动范围仅 MCU+FPGA；不直接编辑现有 MEMORY 文件。
- 当前动作结果：本条只记录需求、架构决策和动作前计划；截至追加时尚未盘点现有恢复/状态脚本，尚未创建机器可读检查点、只读恢复脚本或 ad-hoc memory note，也未修改 MCU/FPGA 工程。
- 构建/Git/范围边界：未运行正式 `regenerate_cubemx.ps1`、CubeMX、Keil、Vivado 或硬件测试；未触碰 MPU、VM、Gitee；未暂存、未提交、未推送。
- 验证边界：记录计划不证明恢复机制已实现、可解析、可在中断后重现状态或已进入远端，也不改变 W-066 的临时静态证据与正式工程尚未物化边界。
- 下一步：主代理必须先完整复核 W-067，再只读盘点仓库现有恢复/状态脚本以避免新增冗余；随后使用 `apply_patch` 实现最小检查点与只读恢复机制并单独记录验证结果，之后再回到 MCU 正式 regenerate 门禁。

### W-20260831-068：中断恢复入口与仓库状态只读盘点

- 时间：2026-08-31T21:45:00+08:00
- 关联要求/未决项：用户关于长任务中断后无缝衔接的硬要求、W-20260831-067、D-008、P0、P4；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理在执行本阶段前已完整分块逐行读取 `docs/PROJECT_WORKLOG.md` 全文 1231 行；物理末尾为 W-20260831-067，时间线最大序号为 067，历史重复序号仅为 W-20260830-034，均保留。记录员在追加本条前再次核对该基线和追加优先规则。
- 只读盘点命令：在仓库根执行 `Get-ChildItem -Force`；用 `rg --files -g '!build/**' -g '!tmp/**'` 检索恢复/状态/checkpoint/context 候选和现有 `*.ps1`；读取 `.gitignore`；执行 `git status --short --branch`、`git remote -v`、`git rev-parse HEAD`、`git ls-remote origin refs/heads/main`。本阶段未编辑源码、工程或系统。
- 结果与原始证据：根目录现有 `docs/`、`mcu/`、`fpga/`、历史 `mpu/`、`tmp/`、`build/` 等；排除 `build/`/`tmp/` 后没有现成的机器可读恢复/checkpoint 入口。检索命中的 `performance_state` 与 `mpu_io_state` 属算法/历史 MPU 文件，不是项目恢复机制，不能作为新增入口的替代。现有脚本为 MCU 四个（`mcu/scripts/build_keil.ps1`、`postgenerate_keil.ps1`、`regenerate_cubemx.ps1`、`verify_all.ps1`）及 FPGA `build.ps1`、`build_software.ps1`、`open_project.ps1`，没有重复的最小恢复脚本。
- Git 只读证据：状态为 `## main...origin/main`，工作树保留既有 FPGA/文档/MCU 修改且必须按精确路径暂存；远端为 `git@github.com:ryule5158/audio-dsp.git`；本地 `HEAD=5d5cf9cf5b3fecfc2c5ec90b85acc8e25fb08e26`；`git ls-remote origin refs/heads/main` 返回同一 SHA。因此本阶段没有新的本地/远端提交变化。
- 计划而非已实施：下一步拟新增非冗余 `docs/PROJECT_CHECKPOINT.json`、`tools/resume_context.ps1` 和一条 Codex ad-hoc memory update note。恢复脚本默认只读、离线运行，只有显式 `-CheckRemote` 才查询 `origin`；它将解析工作记录编号/hash、checkpoint、local Git、关键 MCU/FPGA native 入口并标 stale，不触碰 Gitee、工程、系统或硬件。该段仅为计划，不能写成文件已创建或已验证。
- 修改/验证边界：本条追加本工作记录属于唯一持久修改；未创建 checkpoint、恢复脚本或 memory note，未运行 CubeMX/Keil/Vivado、未构建/仿真/上板、未触碰 MPU/VM/Gitee。文件存在、Git SHA 和远端一致不等于平台工程或硬件通过。
- Git：本条追加形成 `docs/PROJECT_WORKLOG.md` 未提交差异；未暂存、未提交、未推送。
- 下一步：主代理先完整复核 W-068，再用 `apply_patch` 实现最小检查点、只读恢复入口和 ad-hoc memory note；实现后先记录 Parser/JSON/只读 smoke 与 stale 边界，再回到 MCU 正式 regenerate 门禁。

### W-20260831-069：恢复盘点记录的格式、行数与末尾编号核验

- 时间：2026-08-31T21:48:00+08:00
- 关联要求/未决项：W-20260831-068、用户长任务中断恢复硬要求、D-008、P0、P4；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员在本次追加前已完整分块逐行读取最新工作记录全文（含 W-20260831-068，共 1244 行），确认物理末尾、最大序号和追加优先规则；历史重复 W-20260830-034 继续保留。
- 验证命令：`git diff --check -- docs/PROJECT_WORKLOG.md`；随后只读计算行数、时间线标题序号、最后标题和物理最后一行。
- 结果与原始证据：`git diff --check` 退出码 0、无输出；追加前统计为 `LINE_COUNT=1244`、`LAST_HEADING=### W-20260831-068：中断恢复入口与仓库状态只读盘点`、`MAX_SEQUENCE=68`，物理最后一行是 W-068 的“下一步”行。此前一次分块复核命令的函数编排因 JavaScript `SyntaxError: Unexpected identifier 'max_output_tokens'` 未进入子命令；重试成功且未产生文件或系统修改，该失败不影响本次格式核验。
- 修改/验证边界：本条仅追加工作记录；没有修改源码、工程、系统、远端或临时构建产物，未运行 CubeMX/Keil/Vivado、未构建/仿真/上板，未触碰 MPU/VM/Gitee。
- Git：本条追加后 `docs/PROJECT_WORKLOG.md` 形成新的未提交差异；未暂存、未提交、未推送。行数和末尾序号将在下一次追加前重新核对。
- 下一步：主代理必须先完整复核 W-069，再实施 W-068 计划中的 `docs/PROJECT_CHECKPOINT.json`、`tools/resume_context.ps1` 与 ad-hoc memory note；实现和静态 smoke 结果另行追加后才回到 MCU 正式 regenerate 门禁。

### W-20260831-070：W-069 追加后的物理末尾复核

- 时间：2026-08-31T21:49:00+08:00
- 关联要求/未决项：W-20260831-069、D-008、P0、P4；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员已完整分块读取含 W-069 的最新工作记录，确认其格式核验结果、追加优先规则和不得把恢复计划写成已实施的边界。
- 只读核验：使用 PowerShell `Get-Content` 统计全文行数、时间线标题序号、最大序号、最后标题和物理最后一行；未执行任何工程或系统修改。
- 结果与原始证据：W-069 追加后的统计为 `LINE_COUNT=1255`、`LAST_HEADING=### W-20260831-069：恢复盘点记录的格式、行数与末尾编号核验`、`MAX_SEQUENCE=69`；物理最后一行与 W-069 的“下一步”一致。工作记录此前 `git diff --check` 已为 exit 0。
- 修改/验证边界：本条只追加工作记录；未修改源码、工程、系统、远端或临时构建产物，未运行 CubeMX/Keil/Vivado、未构建/仿真/上板，未触碰 MPU/VM/Gitee。
- Git：本条追加形成新的 `docs/PROJECT_WORKLOG.md` 未提交差异；未暂存、未提交、未推送。
- 下一步：主代理先完整复核 W-070，再执行 W-068 计划中的最小 checkpoint/恢复入口实现，并把每个实现、解析、只读 smoke 和 stale 结果单独回传记录员。

### W-20260831-071：恢复检查点补丁编排在 JavaScript 解析阶段失败（保留）

- 时间：2026-08-31T21:55:00+08:00
- 关联要求/未决项：用户关于长任务中断后无缝衔接的硬要求、W-20260831-067～W-20260831-070、D-008、P0、P4；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理已完整分块读取 W-070 物理末尾及其前序工作记录，确认本阶段只允许实现非冗余恢复索引、只读恢复脚本和 Codex ad-hoc note；记录员追加本条前再次核对最新全文、最大序号和历史重复 W-20260830-034。
- 计划动作：使用 functions.exec 编排 apply_patch，一次创建 docs/PROJECT_CHECKPOINT.json、tools/resume_context.ps1 和 C:\Users\LENOVO\.codex\memories\extensions\ad_hoc\notes\20260831-audio-dsp-resume.md。
- 原始失败：functions.exec 在真正调用 apply_patch 之前解析补丁正文时，正文中的 Markdown 反引号破坏了 JavaScript 模板字符串，抛出 SyntaxError: Unexpected identifier 'docs'。
- 失败边界：由于错误发生在 JavaScript 解析阶段，apply_patch 没有被调用；没有创建、修改或删除上述三个文件/笔记，没有修改任何 MCU/FPGA 工程、系统、远端、工具链、硬件或临时构建物，也没有运行 CubeMX、Keil、Vivado、MPU/VM 或 Gitee 动作。
- 结果与验证：只确认编排调用失败；当前恢复检查点、恢复脚本和 ad-hoc note 仍不存在，不能把计划写成已实施或已验证。
- Git：未暂存、未提交、未推送；既有工作树差异保持不动。
- 下一步：主代理先完整复核本条，改用不含模板字符串反引号的最小 apply_patch；落盘后分别验证 JSON/PowerShell 解析、只读恢复 smoke 和 stale 边界，再回传结果追加。该失败保留，不由后续成功覆盖。

### W-20260831-072：恢复脚本补丁再次在 JavaScript 解析阶段失败（保留）

- 时间：2026-08-31T21:58:00+08:00
- 关联要求/未决项：用户关于长任务中断后无缝衔接的硬要求、W-20260831-067～W-20260831-071、D-008、P0、P4；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理已完整读取含 W-071 的最新工作记录，确认恢复索引和只读入口仍未创建；本阶段仅重试创建 docs/PROJECT_CHECKPOINT.json、tools/resume_context.ps1 及 Codex ad-hoc note，不进入 MCU/FPGA 工程动作。
- 计划动作：使用 functions.exec 配合 JavaScript String.raw 编排 apply_patch，创建检查点 JSON、PowerShell 恢复脚本和长期恢复 note。
- 原始失败：补丁正文的 PowerShell 换行正则表达式包含 `r?`n；该反引号序列破坏了 JavaScript 模板字符串，functions.exec 在真正调用 apply_patch 前抛出 SyntaxError: Unexpected identifier 'r'。
- 失败边界：apply_patch 未被调用；docs/PROJECT_CHECKPOINT.json、tools/resume_context.ps1 和 C:\Users\LENOVO\.codex\memories\extensions\ad_hoc\notes\20260831-audio-dsp-resume.md 均未创建、未修改、未删除。没有发生其他源码、工程、系统、远端、工具链、硬件、CubeMX、Keil、Vivado、MPU、VM 或 Gitee 动作。
- 结果与验证：只确认 JavaScript 编排解析失败；恢复机制仍未实施或验证，不能把 W-067/W-068 的计划写成现状。
- Git：未暂存、未提交、未推送；既有工作树差异保持不动。
- 下一步：主代理先完整复核本条，改用不含反引号的补丁表示或拆分为安全的小补丁，再逐项验证 JSON/PowerShell 解析、只读恢复 smoke 与 stale 边界；该失败保留，不由后续成功覆盖。

### W-20260831-073：恢复入口跨 PowerShell 版本的 UTF-8 行数兼容性缺陷（保留）

- 时间：2026-08-31T22:05:00+08:00
- 关联要求/未决项：用户关于长任务中断后无缝衔接的硬要求、W-20260831-067～W-20260831-072、D-008、P0、P4；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理已完整读取含 W-072 的最新工作记录，并确认恢复文件已经实际落盘但尚未形成可接受的双版本验证结论；本阶段只核查恢复入口兼容性，不运行工程或平台构建。
- 验证动作：执行 tools/resume_context.ps1 的初次恢复 smoke，并在当前 PowerShell 7.6.4、Windows PowerShell 5.1、Windows PowerShell 显式 UTF-8 读取和 [IO.File]::ReadAllLines UTF8 路径之间比较工作记录行数与 SHA-256。
- 原始缺陷：Windows PowerShell 5.1 中恢复脚本使用 Get-Content 默认编码读取 UTF-8 工作记录，脚本报告 LINE_COUNT=411；同一文件在 PowerShell 7.6.4、Windows PowerShell 显式 -Encoding UTF8 以及 [IO.File]::ReadAllLines UTF8 均报告 1290。UTF-8 正确读取的 SHA-256 为 F53E...3787，彼此一致。
- 其余结果：初次验证的 JSON 解析、PowerShell Parser、恢复脚本退出码均为 0，checkpoint stale=false；但错误的 411 行计数使该运行不能接受为最终通过，不能把 stale=false 单独当作完整性证明。
- 修改与失败边界：本阶段尚未修改代码、工程、恢复文件、工作记录以外的系统文件或远端；没有运行 CubeMX、Keil、Vivado、构建、仿真、上板、MPU、VM 或 Gitee 动作。
- Git：未暂存、未提交、未推送；现有工作树差异保持不动。
- 下一步：主代理先完整复核本条，将恢复脚本读取工作记录的调用改为显式 -Encoding UTF8（兼容 Windows PowerShell 5.1），随后在两种 PowerShell 版本重测行数、SHA、JSON/Parser、stale 和只读边界；本缺陷保留，不由后续成功覆盖。

### W-20260831-074：中断恢复检查点、只读恢复入口与长期记忆落盘并通过双版本验证

- 时间：2026-09-01T01:56:23+08:00
- 关联要求/未决项：用户关于长任务中断后无缝衔接的硬要求、W-20260831-067～W-20260831-073、D-008、P0、P4；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理与记录员均完整分块逐行读取含 W-073 的最新工作记录；记录员追加前复核为 1302 行、物理末尾 W-20260831-073、W-074 不存在、历史重复 W-20260830-034 共两条且继续保留。
- 新增检查点：创建 docs/PROJECT_CHECKPOINT.json，67 行、3148 字节、SHA-256 F2CBE04FA270E23B6B5222DAC713285C2B773CA0A244D14D7F83652894F6594E；snapshot 记录当前 MCU/FPGA 范围、HEAD、当前工序、已验证证据、失败/验证边界与唯一下一动作，不替代 append-only 工作记录。
- 新增恢复入口：创建 tools/resume_context.ps1，275 行、10148 字节、SHA-256 499462303A5DB9ADC7280D87DFD9D001FFEEC98A9E925C6D7097A55781157A5E；默认离线只读，只有显式 -CheckRemote 才查询 origin，支持 -Json，并解析工作记录行数/SHA/末条/重复序号、checkpoint stale、Git、关键 MCU/FPGA 路径和 native XPR 清单。
- 新增长期恢复 note：依据用户对长期记录的明确授权，只在 C:\Users\LENOVO\.codex\memories\extensions\ad_hoc\notes\20260901-audio-dsp-resume.md 新增小型更新说明，SHA-256 3C788BA184C8FCAEDB9B1321384B970ECF9E5B19E9122FF87C748F0141B889E6；没有直接编辑 MEMORY.md。
- W-073 缺陷修复：工作记录读取改为 Get-Content 显式 -Encoding UTF8；checkpoint 同步为 W-20260831-073、1302 行、工作记录 SHA-256 4F95C87E3DCCDFE59721A6D35F94D54640FBEC3409CDC841041C6FA8D2187132。
- 验证命令/结果：PowerShell 7 parser 通过；Windows PowerShell 5.1 与 PowerShell 7 的 human/JSON 四种恢复运行均 exit 0，均报告 line_count=1302、last_entry=W-20260831-073、SHA-256 4F95C87E3DCCDFE59721A6D35F94D54640FBEC3409CDC841041C6FA8D2187132、checkpoint stale=false、remote_checked=false；运行前后工作记录 SHA 不变。
- 原生工程恢复证据：恢复入口报告 native XPR 数量为 1，selftest XPR 存在而 soc_i2s XPR 不存在；因此 selftest 文件存在不能写成 FPGA 双工程或 FPGA 交付完成。
- 差异检查：git diff --check exit 0。
- 记录追加失败保留：本条首次 apply_patch 的上下文行把 Markdown 列表连字符误作补丁删除标记，导致 verification failed；该次调用未修改文件，更正统一 diff 上下文后重试成功。
- 修改/网络/构建边界：本阶段只新增上述 checkpoint、恢复脚本、ad-hoc note 并修复恢复脚本/检查点内容；未访问网络或 Gitee，未运行 CubeMX、Keil、Vivado、构建、仿真、上板，也未触碰 MPU/VM。
- 检查点前进语义：本条作为 append-only 日志追加后，checkpoint 将按设计相对最新日志变为 stale；这是提示恢复者重新阅读新条目的安全信号，不是实现失败。
- Git：未暂存、未提交、未推送；既有 MCU/FPGA/文档脏工作保持不动。
- 验证边界：JSON/Parser/双版本只读 smoke 只证明恢复入口及其状态解析可用，不证明 MCU 正式三 target 已物化、Keil 构建、FPGA soc_i2s 原生工程、Vivado 流程或任何真实硬件通过。
- 下一步：主代理完整复核 W-074，运行只读恢复入口确认预期 stale，再返回 W-066 门禁，执行正式 MCU regenerate；任一失败须停止并追加原始证据。

### W-20260901-075：W-074 追加后的预期 stale 恢复信号验证通过

- 时间：2026-09-01T02:00:59+08:00
- 关联要求/未决项：用户关于长任务中断后无缝衔接的硬要求、W-20260831-074、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理先完整复核 W-074 后运行恢复入口；记录员在追加本条前再次完整分块逐行读取当前 1321 行工作记录，确认物理末尾 W-20260831-074、W-20260901-075 不存在、追加优先和正式 MCU regenerate 尚未执行的边界。
- 只读验证动作：主代理使用 Windows PowerShell 5.1 以默认离线模式运行 tools/resume_context.ps1 -Json；命令 exit 0，未使用 -CheckRemote。
- 工作记录结果：恢复输出 last_entry=W-20260831-074、line_count=1321、SHA-256 1687A095F91882CAAE5B8A90F15B117CDB501F6EE665306C3C188122DE183CCE；运行前后工作记录哈希不变。
- checkpoint 结果：stale=true；原因精确为两项：worklog last_entry differs from checkpoint、worklog SHA-256 differs from checkpoint。remote_checked=false。
- 结论：W-074 作为 append-only 新记录追加后，恢复入口按设计将相对 W-073 快照标为 stale；该预期安全信号工作正常，不是恢复脚本失败。next_action 仍指向完成验证后完整复核最新工作记录并正式运行 MCU regenerate。
- 修改/网络/构建边界：除记录员追加本条外无文件修改；未访问网络或 Gitee，未运行 CubeMX、Keil、Vivado、构建、仿真、上板，也未触碰 MPU/VM。
- Git：未暂存、未提交、未推送；既有 MCU/FPGA/文档脏工作保持不动。
- 验证边界：本条只证明离线恢复入口能识别 append-only 日志前进及其两项 stale 原因，不证明正式 MCU 三 target 已物化、Keil 构建、FPGA 原生双工程、Vivado 流程或真实硬件通过。
- 下一步：主代理完整审阅 W-075 后执行正式 mcu/scripts/regenerate_cubemx.ps1；任一失败立即停止并追加原始证据。

### W-20260901-076：正式 MCU CubeMX 再生成与独立三目标审计通过

- 时间：2026-09-01T02:07:57+08:00
- 关联要求/未决项：用户要求 MCU H743IIT6 交付可直接开发、已配置并编译验证的 CubeMX + Keil 完备工程模板；关联 W-20260901-075、D-008、P0、P2。主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理已完整复核 W-075 后才进入正式 MCU regenerate；记录员在追加本条前完整分块逐行读取当前 1335 行工作记录，确认物理末尾 W-20260901-075、W-20260901-076 不存在，并保持失败证据与验证边界。
- 正式再生成：主代理使用 Windows PowerShell 5.1 运行 `mcu/scripts/regenerate_cubemx.ps1`，开始于 2026-09-01T02:02:25+08:00，结束于 2026-09-01T02:02:57+08:00，exit 0；正式 `.uvprojx` 与 `.uvoptx` 已修改。
- 原始输出第一行：`Configured synchronized three-target Keil MDK project for ArmClang 6.24, App sources and optional NUEDC analysis.`
- 原始输出第二行：`CubeMX regeneration and three-target SAI/PLL3/DMA/RTE contract checks passed.`
- 首次独立审计失败保留：主代理自写查询误用了组名 `Application`、`Library/Audio DSP`，同时使用了错误 XML root 和非递归 DebugConfig 路径，因此误报 APP=0、DSP=0、options empty、debug=0；这是审计查询错误，不是工程再生成失败，后续纠正不覆盖该错误记录。
- 纠正审计：读取真实 XML 结构与脚本断言后重新查询并 PASS；三个 target 精确为 `SelfTest`、`WM8960_Stream`、`NUEDC_Analysis`，每个 target 均有 APP=3、DSP permissive=49。
- 可选分析隔离：`SelfTest` 与 `WM8960_Stream` 的 Analysis group 均为 0 且 NUEDC include=false；`NUEDC_Analysis` 的 Analysis group=1，包含 C=14、H=15，且 NUEDC include=true。
- RTE/选项/调试审计：RTE CORE 6.2.0 与 DSP Source 1.16.2 仅绑定 `NUEDC_Analysis`；`.uvoptx` 含三个 target 且仅 `SelfTest` 为 current；递归核验 DebugConfig=3。
- 哈希与差异检查：正式 project SHA-256 为 7D0ED49EBEACF369F762967CFF4B86521424D457A7CACEE72138143FED2C97B0，正式 options SHA-256 为 647B057C733B179C7A981B98E03ED356B83BDD1FFCEF3161C807951D903438D2；`git diff --check -- mcu` 为 OK。
- 工作树边界：状态仍包含此前 MCU 源码、脚本与 NUEDC 的未提交差异；本次新增正式 `.uvprojx`/`.uvoptx` 修改，并出现仅 `NUEDC_Analysis` 的 dbgconf 未跟踪项；`SelfTest`/`WM8960_Stream` 调试文件此前已被跟踪。本条不清理、不覆盖这些差异。
- 构建/平台/硬件边界：尚未运行 Keil build、烧录或真实硬件验证；未触碰 FPGA、MPU、VM 或 Gitee，未运行 Vivado，也未访问网络。本次 regenerate 与结构审计通过不能替代三 target 的实际 Keil 编译和板级验证。
- Git：未暂存、未提交、未推送。
- 下一步：主代理必须先完整复核 W-076，随后才可运行 `mcu/scripts/build_keil.ps1 -Target All`；任一 target 失败即停止并追加原始证据，不继续后续动作。

### W-20260901-077：MCU Keil All 首次正式构建在 SelfTest 失败并中止

- 时间：2026-09-01T02:32:34+08:00（构建运行于 02:09:56～02:10:10）
- 关联要求/未决项：用户要求 MCU H743IIT6 交付可直接开发、已配置并编译验证的 CubeMX + Keil 完备工程模板；关联 W-20260901-076、D-008、P0、P2。主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理在 W-076 正式再生成与结构审计通过后，按门禁完整复核最新记录才运行构建；记录员追加本条前完整分块逐行读取当前 1353 行工作记录，确认物理末尾 W-20260901-076、W-20260901-077 不存在、历史重复 W-20260830-034 保留。
- 构建命令：Windows PowerShell 5.1 运行 `mcu/scripts/build_keil.ps1 -Target All`；开始时间 2026-09-01T02:09:56+08:00，结束时间 02:10:10，exit 1。
- 首个 target 与工具链证据：日志首先输出 `Rebuild target 'STM32H743_Audio_SelfTest'`，使用 ArmClang V6.24。
- 意外编译范围：尽管 W-076 的独立 XML 结构审计显示 SelfTest 不含 NUEDC Analysis group 或 NUEDC include，SelfTest 实际构建仍编译了 NUEDC 的 Adaptive、Demod、Filter、DSP_ProMax、Correlate、FFT、IQ、Measure、Fit、SoftPll、ModelFit、periodic_analyzer、FilterEx 以及 `nuedc_analysis_selftest.c`。
- 编译错误证据：共 9 个 error；Demod、Filter、DSP_ProMax、FFT、IQ、Measure、SoftPll、periodic_analyzer 各因 `arm_math.h` file not found 失败，`nuedc_analysis_selftest.c` 因 `Adaptive.h` file not found 失败。
- 最终 Keil 汇总：`STM32H743_Audio_SelfTest.axf - 9 Error(s), 0 Warning(s). Target not created.`；最终 warning 汇总为 0。构建脚本 line 198 的 0-error/0-warning 门禁随即抛错并返回 exit 1。
- 中止边界：因第一个 SelfTest target 失败，本次 All 构建立即中止；后续 `WM8960_Stream` 与 `NUEDC_Analysis` targets 均未运行，不能推断它们成功或失败。
- 当前判断：W-076 的 XML group/include 隔离不能替代真实 build 隔离证据。必须定位为何 Keil 在 SelfTest 构建中仍串入 NUEDC 源；在确认根因前不得修复、重跑或把结构审计写成编译隔离通过。
- 修改范围：未修改正式源码或工程配置；只产生常规构建输出。未烧录、未做真实 MCU 硬件/WM8960 验证，未触碰 FPGA、MPU、VM 或 Gitee，也未运行 Vivado。
- Git：未暂存、未提交、未推送；既有 MCU/FPGA/文档脏工作保持不动。
- 验证边界：本条只证明正式 Keil All 首次构建在 SelfTest 编译阶段失败并按门禁中止；不证明任何 target 编译通过、Pack/RTE 解析正确、资源满足、FPGA 工程完成或真实硬件通过。
- 下一步：主代理完整复核 W-077 后，只读检查 `build_keil.ps1` 的 Keil 调用、Keil 构建日志、`.uvprojx` target groups/RTE 行为与输出目录；在确定根因前不修改或重跑。

### W-20260901-078：Keil SelfTest 串入 NUEDC 的只读根因调查

- 时间：2026-09-01T02:49:54+08:00
- 关联要求/未决项：W-20260901-077、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅与范围：主代理每批诊断动作前均核对最新门禁为 W-077，只允许只读根因调查；记录员追加本条前完整分块逐行读取当前 1370 行记录，确认物理末尾 W-077 且 W-078 不存在。全过程未重跑 Keil、未修改正式工程或源码、未触碰 FPGA/MPU/VM/Gitee。
- 构建脚本调用审计：`build_keil.ps1` 对每个 target 正式调用 `UV4.exe -r <uvprojx> -t <target> -j0 -o <log>`，按 target 使用独立日志并在失败后立即停止。Keil 官方命令行文档 https://www.keil.com/support/man/docs/uv4/uv4_commandline.asp 明确 `-r` 重译指定 target、`-t` 选择 target、`-j0` 隐藏 GUI；因此命令行 target 选择语法不是本次串入根因。
- 正式磁盘工程证据：`.uvprojx` SHA-256 为 `7D0ED49EBEACF369F762967CFF4B86521424D457A7CACEE72138143FED2C97B0`，mtime 为 02:02:56；SelfTest 与 WM8960 各有 6 groups、82 files、0 NUEDC files，Analysis 有 7 groups、111 files、29 NUEDC files。NUEDC group 只在第三 target，构建没有改写磁盘 XML。
- SelfTest 依赖文件证据：SelfTest 的 `.dep` 创建/修改时间为 02:10:09，SHA-256 为 `DA0620A2BEA6396D84483A9B1851607DACB82110FDBDA50CE6F72CF5D1A90325`；其内容明确标识 Target SelfTest、宏 `WM8960=0`/`NUEDC_ANALYSIS=0` 和 `RTE/_SelfTest`，却包含 14 个 NUEDC C 编译命令与 15 个 headers，证明 Adaptive 等 NUEDC C 确实由 SelfTest 构建执行。
- RTE 边界：该 `.dep` 尾部还列出 CMSIS-DSP Pack sources，但这些 `F` entries 的命令选项为空、未实际编译。因此 RTE `targetInfo` 隔离与手工 group 串入是两个不同现象；本次证据不能证明 Analysis target 的 CMSIS-DSP 实际编译或链接。
- Keil 项目属性证据：官方 Properties 文档 https://www.keil.com/support/man/docs/uv4cl/uv4cl_dg_property.htm 明确 `Include in Target Build` 可按项目 target 排除 group/source；本机 `project_projx.xsd` 将 `IncludeInBuild` 定义为 group/file `CommonProperty`。本机 Keil Pack 样例 CAN_RTR 在同一文件清单上使用 `FileOption/IncludeInBuild=0` 排除 target 文件，H743 Blinky 使用 `GroupOption/IncludeInBuild=0`；扫描前 250 个本机 Pack `.uvprojx` 未找到 multi-target 文件数量不一致样例。
- 有证据支持的根因结论：`postgenerate_keil.ps1` 只向第三 target 追加额外的 NUEDC group/29 files，破坏了 uVision 多 target 共享项目项的同步建模；uVision 装载时把该项目项集合合并到 SelfTest，再使用 SelfTest target options 编译。该结论定位为 Keil 项目建模缺陷，不是命令行 target 选择错误。
- 修复方向：三个 targets 都应生成完全相同的 NUEDC group/file list；SelfTest 与 WM8960 对该 group 或其 C files 显式设置 `IncludeInBuild=0`，Analysis 保持启用。结构断言必须从“前两 target 无 group”改为“三 target 项目项同步、前两显式排除、Analysis 启用”，并继续核对 include 与 RTE 隔离。
- 日志哈希：本次失败的 `keil_build.log` SHA-256 为 `6CFAC0523480931F24C9A442FC71D6AB6844DA0F8D244FF5CD7089E7587B7E10`。
- 诊断副作用保留：主代理曾错误尝试 `UV4.exe -h` 查询帮助；Keil 没有输出 CLI help，而是启动 GUI。该进程 PID 为 29164，CreationDate 为 2026-09-01 02:39:13，ExecutablePath 为 `C:\Users\LENOVO\AppData\Local\Keil_v5\UV4\UV4.exe`，CommandLine 末尾为 `-h`，追加本条时仍在运行。它在 02:10 构建失败之后才启动，不是原失败原因；不得广泛终止 UV4。
- 修改/构建/Git 边界：除只读诊断及上述误启动 GUI 进程外，没有修改正式工程/源码，没有重跑 Keil，没有烧录或硬件验证，没有执行 FPGA/Vivado、MPU/VM/Gitee；未暂存、未提交、未推送。
- 验证边界：本条是工程/工具模型根因证据，不是任何 target 构建通过、CMSIS-DSP 实际编译链接、资源合格、FPGA 完成或真实硬件证明。
- 下一步：主代理完整审阅 W-078 后，先精确核验并关闭仅由代理启动的 PID 29164；再用 `apply_patch` 最小修改 `postgenerate_keil.ps1` 及必要的构建/结构门禁。编辑结果先独立记录，正式 regenerate 与 build 必须分阶段执行并分别记录，任一失败即停。

### W-20260901-079：UV4 代理进程精确关闭尝试失败（保留）

- 时间：2026-09-01T02:52:00+08:00
- 关联要求/未决项：W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理在本次尝试前已完整分块逐行读取含 W-078 的最新工作记录；记录员追加本条前核对物理末尾 W-078、最大序号 078、历史重复 W-20260830-034 继续保留，并确认只允许对代理误启动的 PID 29164 做严格身份匹配后关闭。
- 计划/命令：PowerShell 读取 PID 29164 的 CIM 进程属性，核对 PID、ExecutablePath、CommandLine 和 CreationDate 四项后再执行 Stop-Process；目标仅为 W-078 记录的 C:\Users\LENOVO\AppData\Local\Keil_v5\UV4\UV4.exe、命令行末尾 -h 的代理进程。
- 原始失败：在身份检查阶段把 CIM CreationDate 传给 ManagementDateTimeConverter 转换时抛出 Specified argument was out of the range of valid values (dmtfDate)。当前 CIM 返回值已经是 DateTime，不应再次按 DMTF 字符串转换。
- 失败边界：异常发生在身份检查输出完成及 Stop-Process 执行之前；PID 29164 未终止，未对任何其他 UV4 进程采取动作。没有修改文件、工程、系统配置、工具链、远端或 Git，也没有构建、联网、触碰 FPGA、MPU 或 VM。
- 结果与验证边界：本次只证明严格关闭尝试在 DateTime/DMTF 类型兼容处理前失败；不能声称代理进程已关闭，也不能把 W-078 的进程身份记录写成当前存活状态已重新核验。后续必须在执行终止前同时严格匹配四项身份字段，并记录匹配结果。
- Git：本条追加形成 docs/PROJECT_WORKLOG.md 未提交差异；未暂存、未提交、未推送。
- 下一步：主代理先完整复核 W-079，采用兼容 DateTime/DMTF 的严格四项匹配后重试仅关闭 PID 29164；成功/失败结果另行追加，随后才进入 postgenerate_keil.ps1 最小补丁。

### W-20260901-080：UV4 代理进程已消失，严格关闭门禁安全中止

- 时间：2026-09-01T03:00:00+08:00
- 关联要求/未决项：W-20260901-079、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理在本次查询前完整复核 W-079；记录员追加本条前再次完整分块逐行读取最新工作记录，确认物理末尾 W-079、最大序号 079、历史重复 W-20260830-034 保留，并确认只能针对该代理曾启动的 PID 29164 做四项身份门禁。
- 查询脚本与门禁：使用兼容 DateTime/DMTF 的 PowerShell 严格脚本，显式设置 gate=W-079；先以 Get-CimInstance Win32_Process -Filter ProcessId=29164 查询，再要求 PID、ExecutablePath、CommandLine 末尾 -h 和 CreationDate 四项全部匹配后才允许 Stop-Process。
- 原始结果：Get-CimInstance Win32_Process -Filter ProcessId=29164 返回 null；脚本立即抛出 PID 29164 is no longer running; refusing implicit action。由于对象不存在，没有进入四项匹配通过分支。
- 安全边界：Stop-Process 未被调用，未终止或影响任何其他 UV4/进程；本结果只证明目标 PID 已不再可查询，不能声称曾执行关闭成功，也不能据此推断其他 UV4 实例归属。
- 修改/动作边界：本次没有修改文件、工程、系统配置、工具链、远端或 Git；没有构建、联网、触碰 FPGA、MPU、VM 或 Gitee。
- 验证边界：严格脚本安全拒绝对不存在 PID 做隐式动作，避免 PID 重用风险；尚未完成“无其他代理启动 UV4 实例”的只读盘点，也尚未修改 Keil 生成器或结构断言。
- Git：本条追加形成 docs/PROJECT_WORKLOG.md 未提交差异；未暂存、未提交、未推送。
- 下一步：主代理先完整复核 W-080，只读核对当前不存在其他由代理启动的 UV4 实例；确认后再按 W-078 方案用 apply_patch 修改 Keil 生成器及三 target IncludeInBuild 结构断言，编辑与验证分阶段记录。

### W-20260901-081：UV4 进程只读核对通过，允许进入 Keil 结构修复

- 时间：2026-09-01T10:04:45+08:00
- 关联要求/未决项：W-20260901-080、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理在完整复核 W-080 后执行本次只读检查；记录员追加本条前已完整逐行复核全文，确认物理末尾为 W-080、最大序号为 080，且不得在目标 PID 消失时隐式终止其他进程。
- 只读命令与 gate：PowerShell 只读运行 Get-CimInstance Win32_Process -Filter "Name='UV4.exe'"，gate=W-080；未调用 Stop-Process。
- 结果与原始输出：UV4_COUNT=0；READ_ONLY_PROCESS_CHECK=True。未发现任何 UV4/uVision 进程。
- 安全边界：未终止进程，未修改文件、工程、系统配置、工具链、远端或 Git；未构建、联网，也未触碰 FPGA、MPU、VM 或 Gitee。
- 下一步：允许主代理按 W-078 方案使用 apply_patch 修改 Keil 生成器及三 target IncludeInBuild 结构断言；编辑、静态检查、正式 regenerate 和构建仍须分阶段记录并遵守失败即停门禁。
- Git：本条仅追加工作记录；未暂存、未提交、未推送。

### W-20260901-083：Keil 多目标项目项同步建模缺陷只读审查与修复门禁

- 时间：2026-09-01T10:22:00+08:00
- 关联要求/未决项：W-20260901-082、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员在追加本条前已完整 UTF-8 分块逐行读取当前工作记录，确认物理末尾为 W-20260901-082、总行数 1435、历史重复 W-20260830-034 保留，并核对当前门禁只允许对 Keil 目标建模做只读审查。
- 主代理只读审查：在 gate=W-20260901-082 下读取 mcu/scripts/postgenerate_keil.ps1（528 行，SHA-256 8F639DC6A621AF261A9F289640DAB40F3818A238CA5EED8B4321634DB35ED450）、mcu/scripts/regenerate_cubemx.ps1 与 mcu/scripts/build_keil.ps1（238 行，SHA-256 2988660F9EDDF4990BF144E3A7F62292EC3BB4D6D10A618170996A9C871FFD6C），没有写入这些文件。
- 根因证据：postgenerate_keil.ps1 的 New-SourceFileNode 只写 FileName、FileType、FilePath；target loop 只对 NUEDC_Analysis 追加 NUEDC group。其前三处结构断言仍把 group count 与 expectedAnalysis 比较，因而与“仅 Analysis 有 group”的生成模型绑定，形成同一项目项建模缺陷；构建后的 object/dep 检查即使发现 NUEDC 泄漏也已经太晚，不能阻止 SelfTest/WM8960 先被错误源集合污染。
- 最小修复方案（尚未实施）：三个 target 生成完全相同的 NUEDC 14 个 C + 15 个 H 文件树；14 个 C 均写完整 FileOption/CommonProperty/IncludeInBuild，其中 SelfTest/WM8960_Stream 为 0、NUEDC_Analysis 为 1。只有 Analysis target 保留 NUEDC include path、analysis 宏和 CMSIS/RTE 组件；三处门禁改为断言三 target 均有 1 个同步 group、14 C/15 H 清单一致，并逐一核对 14 个 IncludeInBuild 标志。构建前可拒绝已存在的 UV4/uVision 进程，但绝不自动终止用户 IDE；构建后继续核验 fresh .dep 与 object 清单，确认没有跨 target 泄漏。
- 记录员失败保留：上一记录智能体 project_recorder_11 曾尝试追加 W-083，但因 usage limit 工具回传额度耗尽；没有追加 W-083，也没有造成文件、工程、系统、Git 或远端修改。该失败不能被本条覆盖；本条是唯一实际追加的 W-083。
- 修改/验证边界：上述全部为只读脚本/模型审查；未修改脚本或工程，未运行 Parser、静态检查、CubeMX、Keil、构建、网络、FPGA、MPU、VM 或 Gitee。
- Git：仅追加 docs/PROJECT_WORKLOG.md；未暂存、未提交、未推送。其他工作树差异保持不动。
- 验证边界：本条只定位 Keil 多目标项目项同步缺陷并记录最小修复设计，不证明补丁已落盘、Parser/XML 门禁通过、三 target 可生成、Keil/CMSIS-DSP 可编译链接、资源合格或真实 MCU/FPGA 硬件通过。
- 下一步：主代理必须先完整复核 W-083，再用 apply_patch 修改三脚本及必要门禁；修改后只先运行 Parser/static checks 和 XML/清单审计，不先 regenerate 或 build，结果另行追加。

### W-20260901-082：记录员动作勘误（W-081 前编排失败与误调用）

- 时间：2026-09-01T10:15:00+08:00
- 关联要求/未决项：W-20260901-081、W-20260901-080、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员在本次追加前已完整 UTF-8 分块逐行读取最新工作记录至 W-081，确认物理末尾为 W-081、最大序号为 081，并保留所有先前成功与失败记录。
- 勘误一（W-081 成功追加前）：首次 apply_patch 编排因误写数组项，在 functions.exec 真正调用 apply_patch 前出现原始错误：apply_patch verification failed: invalid hunk at line 6, Expected update hunk to start with a @@ context marker, got: 'NaN'。该次未调用 apply_patch，未修改文件。
- 勘误二：随后误调用一次 read_thread_terminal，仅返回 No app terminal session is attached to this thread yet.（isError=true），没有终端会话状态修改。
- 共同边界：上述两项均未改变源码、工程、系统、工具链、远端或 Git；未构建、未联网，未触碰 FPGA、MPU、VM 或 Gitee。其失败证据由本条保留，不能被随后成功追加 W-081 覆盖。
- 下一步：主代理须完整复核 W-082 后，再检查并用 apply_patch 修复 Keil 生成器及三 target IncludeInBuild 结构断言；编辑、静态检查、正式 regenerate 与构建继续分阶段记录。
- Git：本条仅追加工作记录；未暂存、未提交、未推送。

### W-20260901-084：W-083 插入位置勘误与 append-only 门禁恢复

- 时间：2026-09-01T10:30:00+08:00
- 关联要求/未决项：W-20260901-083、W-20260901-082、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员已完整 UTF-8 分块逐行读取当前工作记录，确认当前物理末尾为 W-20260901-082、总行数 1449、最大序号为 083，并确认 append-only 规则禁止删除、移动或重排既有条目。
- 勘误一（保留 W-083）：W-083 的首次成功 apply_patch 使用了一个过于宽泛的相同 Git 行作为上下文，因而把新条目插入在 W-081 与物理末尾的 W-082 之间；W-083 内容保持原样，不删除、不移动、不重排。当前 W-083 仍唯一存在，后续门禁以本条 W-084 为最新记录。
- 勘误二（编排失败保留）：在该成功调用之前，另一次 functions.exec 编排在真正调用 apply_patch 前抛出原始 JavaScript 错误 SyntaxError: Unexpected token **；apply_patch 未调用，没有文件、工程、系统、工具链、远端或 Git 修改。该失败不能被后续成功追加覆盖。
- 当前状态：除工作记录自身的 W-083/W-084 追加外，没有修改 mcu 脚本、Keil 工程、源码、FPGA 工程或其他文件；未运行 Parser、静态检查、CubeMX、Keil、构建、网络、Vivado、MPU、VM 或 Gitee。
- Git/验证边界：未暂存、未提交、未推送；本条只恢复记录顺序门禁并说明 W-083 的历史插入位置，不证明任何脚本、三 target、Keil/CMSIS-DSP、FPGA native 工程或真实硬件通过。
- 下一步：主代理必须先完整复核 W-084，再用 apply_patch 修改三脚本及必要门禁；随后仅运行 Parser/static checks 与 XML/清单审计，不先 regenerate 或 build，结果另行追加。

### W-20260901-085：Keil 三目标同步补丁落盘后发现确定脚本缺陷并停止

- 时间：2026-09-01T13:41:11+08:00
- 关联要求/未决项：W-20260901-084、W-20260901-083、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理在 gate W-20260901-084 下执行本阶段；本次 turn 中断后，主代理重新完整读取工作记录并只读核对工作树，确认 W-20260901-085 仍为 0、三脚本状态与下述缺陷不变，没有额外部分执行。记录员追加本条前也完整 UTF-8 分块逐行读取当前 1460 行工作记录，确认物理末尾为 W-20260901-084、W-085 不存在、历史重复 W-20260830-034 继续保留。
- 实际补丁：主代理使用受控 apply_patch 修改 mcu/scripts/postgenerate_keil.ps1、mcu/scripts/regenerate_cubemx.ps1、mcu/scripts/build_keil.ps1，意图为三个 target 同步生成 NUEDC 14 个 C + 15 个 H 文件树，并为 14 个 C 写文件级 IncludeInBuild（SelfTest/WM8960=0、Analysis=1），同时同步三处结构门禁。apply_patch 返回空对象，但 git diff、git status 与文件 SHA-256 证实三脚本修改已落盘。mcu/scripts/verify_all.ps1 的修改是此前既有差异，本动作没有修改该文件。
- 补丁后只读缺陷一：postgenerate_keil.ps1 当前约 line 136 的 $analysisFileOptionTemplate = $baseTarget.SelectSingleNode(...) 位于 $baseTarget 约 line 326 初始化之前；在 StrictMode 下该路径必然失败。
- 补丁后只读缺陷二：postgenerate_keil.ps1 当前约 line 540 残留字面文本 throw "NUEDC target file tree is not the synchronized 14-C/15-header manifest"+    }；这是补丁拼接残留及确定的解析缺陷。
- Parser 诊断失败保留：用于汇总三脚本 parser 错误的 PowerShell 命令自身误写为 ($errs|%{$_.Message}-join ' | ')，返回 Cannot bind parameter 'RemainingScripts'. Cannot convert the '-join' value ... to ScriptBlock。因此本阶段尚未取得三个脚本的 parser 结论，绝不能写成 Parser 通过。
- 当前脚本 SHA-256：postgenerate_keil.ps1 为 DEECC6DFC6E81071A65A1785892A4511881CE2F9FF4377CCF01E2E8BDE952321；regenerate_cubemx.ps1 为 A2F89CD2AF06CA5DF6D77F45ED838147CFBE7143E4A79355878444EF3D38C59E；build_keil.ps1 为 3D0980A14A1F5B5ACC25DDB7B9319688290872AB8249F9EA7D2978740227AF68。
- 记录员失败保留：project_recorder_13 与 project_recorder_12 先后受托追加 W-085，均因 usage limit 失败；二者没有修改工作记录或工程。后续成功追加不得覆盖这两次失败。
- 修改/构建/范围边界：本阶段只修改上述三个脚本；没有修改生成工程 XML 或应用源码，没有运行 CubeMX、Keil、Vivado 或任何 build，没有联网，也没有触碰 FPGA、MPU、VM 或 Gitee；未暂存、未提交、未推送。
- 验证边界：补丁落盘、git diff/status 与哈希证据不证明 PowerShell 可解析、三 target XML 可生成、IncludeInBuild 能隔离 Keil 编译、CMSIS-DSP 可链接、资源合格或真实 MCU/FPGA 硬件通过；当前两个确定脚本缺陷使正式 regenerate/build 门禁继续关闭。
- 下一步：主代理必须先完整复核 W-085，再用 apply_patch 仅修正模板初始化位置与残留 + }；随后使用正确的 Parser 表达式并运行 git diff --check 做静态验证，先记录 W-086，不运行 regenerate 或 build。

### W-20260901-086：Keil 同步建模脚本两处定点修复与静态门禁通过

- 时间：2026-09-01T13:44:19+08:00
- 关联要求/未决项：W-20260901-085、W-20260901-083、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理完整复核 W-20260901-085 后才执行本阶段；记录员追加本条前重新完整 UTF-8 分块逐行读取当前 1475 行工作记录，确认物理末尾为 W-20260901-085、W-086 不存在、历史重复 W-20260830-034 继续保留。
- 定点修复一：主代理仅对 mcu/scripts/postgenerate_keil.ps1 使用 apply_patch，删除位于 $baseTarget 初始化之前的 analysis FileOption template 初始化；在确认 $baseTarget 非空后，使用 XPath Groups/Group/Files/File[FileType='1' and FileOption]/FileOption 获取完整 FileOption 模板。
- 定点修复二：同一补丁修复 manifest 断言 throw 行残留的 +    } 拼接文本。apply_patch 返回空对象，但 git diff 与文件 SHA-256 证实修改已落盘。本阶段没有修改 regenerate_cubemx.ps1 或 build_keil.ps1。
- Parser 静态结果：postgenerate_keil.ps1、regenerate_cubemx.ps1、build_keil.ps1 三脚本的 PowerShell Parser Errors 均为 0。
- 顺序与残留检查：postgenerate 的 BaseTargetLine=318、TemplateLine=322、OrderValid=True；BadPatchMarkerCount=0；SetHelperCount=1；NewGroupCount=1。
- 同步合同检查：三个脚本均包含同步 NUEDC group 与 IncludeInBuild 合同，并且三个脚本均不再包含旧断言 analysisGroups.Count -ne [int]$expectedAnalysis。
- 差异与哈希：对上述三个脚本运行 git diff --check，exit 0。当前 SHA-256：postgenerate_keil.ps1 为 36C00D22C452E905CA71CF850596B14DFCFD9E364C8AFF6538F1250BDC3D444E；regenerate_cubemx.ps1 为 A2F89CD2AF06CA5DF6D77F45ED838147CFBE7143E4A79355878444EF3D38C59E；build_keil.ps1 为 3D0980A14A1F5B5ACC25DDB7B9319688290872AB8249F9EA7D2978740227AF68。
- 修改/构建/范围边界：本阶段只修改 postgenerate_keil.ps1 的上述两处；没有修改生成工程 XML 或应用源码，没有运行 CubeMX、Keil、Vivado 或任何构建，没有联网，也没有触碰 FPGA、MPU、VM 或 Gitee；未暂存、未提交、未推送。
- 验证边界：三脚本 Parser 与静态合同通过只证明当前脚本文本可解析且关键静态门禁存在；不证明 postgenerate 运行时、正式三 target XML 生成、IncludeInBuild 对 Keil 编译的实际隔离、CMSIS-DSP 编译链接、资源合格或真实 MCU/FPGA 硬件通过。
- 下一步：主代理必须先完整复核 W-086，再运行正式 mcu/scripts/regenerate_cubemx.ps1；任一失败立即停止并记录。若成功，先独立审计生成 XML 并另行记录，尚不立即运行 Keil build。

### W-20260901-087：正式 MCU 再生成成功但全工程 XSD 独立审计失败

- 时间：2026-09-01T13:45:38+08:00
- 关联要求/未决项：W-20260901-086、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理先完整复核 W-086 后才运行正式 regenerate；记录员追加本条前再次完整 UTF-8 分块逐行读取当前 1490 行工作记录，确认物理末尾为 W-086、W-087 不存在、历史重复 W-20260830-034 继续保留。
- 正式再生成：Windows PowerShell 5.1 运行 mcu/scripts/regenerate_cubemx.ps1，START=2026-09-01T13:45:12.0114092+08:00，END=2026-09-01T13:45:38.0942747+08:00，EXIT_CODE=0。原始输出为：Configured synchronized three-target Keil MDK project for ArmClang 6.24, App sources and optional NUEDC analysis. 以及 CubeMX regeneration and three-target SAI/PLL3/DMA/RTE contract checks passed. 正式工程已更新，尚未运行 Keil build。
- 独立只读 XML 结构审计：三个 target SelfTest、WM8960_Stream、NUEDC_Analysis 各自 Groups=7、Files=111、AnalysisC=14、AnalysisH=15；IncludeInBuild FlagValues 依次为 0/0/1，NuedcInclude 依次为 False/False/True。三者文件树哈希输出前缀均为 4A6953F8E8AB5571A40...，代码确认唯一文件树哈希值=1。
- RTE/选项/文件节点审计：RTE CORE 6.2.0（来源 CMSIS 6.3.0）与 DSP 1.16.2（来源 CMSIS-DSP 1.16.2）只绑定 NUEDC_Analysis；uvoptx 含三个 target 且仅 SelfTest 的 Current=1；BlankFilePaths=0、NestedFiles=0、AnalysisFileOptions=42、AnalysisIncludeNodes=42、DebugConfigs=3。
- XSD 门禁：使用本机 C:\Users\LENOVO\AppData\Local\Keil_v5\UV4\project_projx.xsd 对正式 uvprojx 执行独立 XML/XSD 审计，得到 XSD_ERRORS=251。代表性错误包括 noNamespaceSchemaLocation 未声明、ComprImg invalid、DebugOption incomplete、pFcarmOut invalid、RvdsMve invalid、useXO invalid、pXoBase invalid，以及 FileArmAds/Aads invalid；另见 noNamespaceSchemaLocation、UseEnv 期望值等 schema 不匹配。
- XSD 失败处理：审计随后抛出 Keil project XSD validation failed，exec exit 1；因此后续 HASHES 与 git diff check 未执行，整体独立审计不能称为通过。该失败只说明当前安装 schema 与工程格式存在 251 项不匹配，尚未通过基线差分确定是 CubeMX/Keil5.43 格式与旧 schema 的普遍差异，还是本次补丁影响，不能归因。
- 记录员失败保留：project_recorder_14 首次受托追加本条因 usage limit 失败，未改工作文件；turn 中断后主代理完整复核当前 worklog/哈希，确认 W087=0、uvprojx SHA-256 A5C52F933CCD48B4D1C65C009E9E843EC403C3AAD60354581A51F47D349CC6EC（长度 277920，mtime 13:45:37）、uvoptx SHA-256 647B057C733B179C7A981B98E03ED356B83BDD1FFCEF3161C807951D903438D2（长度 14142），UV4_COUNT=0，未见额外副作用。
- 当前判断：正式 regenerate 内置合同与独立结构合同通过，但全工程 XSD 门禁失败；251 项可能来自 CubeMX/Keil5.43 工程格式与随安装旧 schema 的不匹配，也可能包含补丁影响，尚未完成基线差分，不能据此放行 Keil build。
- 修改/范围边界：正式 regenerate 修改了生成的 MCU uvprojx/uvoptx；本条只追加工作记录，没有烧录、Vivado/FPGA、MPU/VM/Gitee、网络或其他系统动作，也未进行 Git 暂存、提交或推送。
- 验证边界：再生成和结构审计不证明 XSD 兼容、Keil 编译链接、CMSIS-DSP 实际资源、硬件或 FPGA 通过；XSD 失败后没有执行后续哈希/差异门禁。
- 下一步：主代理必须先完整复核 W-087；只读使用 HEAD/历史 uvprojx 或本机官方 Pack 样例在同一 schema 下分类 XSD 错误，并比较新增 42 个克隆 FileOption 与原模板；先记录诊断，再决定以 Keil 实际加载/build 作为权威门禁。

### W-20260901-088：Keil 工程 XSD 基线差分确认与实际构建门禁回归

- 时间：2026-09-01T15:54:59.0579894+08:00
- 关联要求/未决项：W-20260901-087、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员先以 UTF-8 逐行读取当前工作记录全部 1506 行，确认物理末尾为 W-087、W-088 不存在、W-087 的 XSD 失败边界和“先诊断再以 Keil 实际加载/build 为权威”要求；未修改任何文件后才进行本次只读差分。
- 只读基线差分：在同一 project_projx.xsd 下比较当前正式工程、Git HEAD 历史工程和本机官方 Pack H743 Blinky 样例。Current SchemaVersion=2.1、Targets=3、FileOptions=72；GitHEAD Targets=2、FileOptions=20；OfficialPackH743Blinky Targets=2、FileOptions=0，且其 schemaLocation 为 project_projx.xsd。
- 验证矩阵：Current Errors=251、Unique=13；在内存移除 42 个 NUEDC FileOption 后 Errors=125、Unique=13；GitHEAD Errors=81、Unique=11；OfficialPackH743Blinky Errors=68、Unique=4。所有比较均为只读内存变换或临时解析，没有回写 XML。
- 当前错误分类：Current 的主要类型为 Cads incomplete=75、CommonProperty ComprImg invalid=75、FileArmAds/Aads invalid=72 等；移除 42 个新增 FileOption 后分别降至 33、33、30，Errors delta=126，恰为 42×3。GitHEAD 也已有相同类型错误；官方 Pack H743 Blinky 同样失败，主要为 targetInfo incomplete=64、pCCUsed invalid=2、isTargetSpecific=1、LayerInfo=1。
- FileOption 结构核对：把 IncludeInBuild 归一为 1 后，新增 42 个 Analysis FileOption 与当前 main.c FileOption 模板的 NormalizedStructuralMismatch=0；Flag0=28、Flag1=14、TemplateChildren=2。该结果说明新增节点是原模板结构的克隆，不是补丁制造的异形 XML。
- 结论：随安装的 project_projx.xsd 不能作为当前 Keil/Pack 工程的全局权威兼容门禁。W-087 的 251 项不等于 Keil 工程不可加载；新增 42 个 FileOption 可解释 126 项增量，但这些增量全部来自已有模板字段与旧 schema 的不兼容。XSD 失败历史必须保留，不能改写为通过。
- 修改/验证边界：本阶段仅执行只读 XML/XSD 基线差分和内存结构比较；没有修改文件、系统、工程或工具链，没有运行 CubeMX、Keil、Vivado、FPGA/MPU/VM/Gitee、网络或 Git 暂存/提交/推送动作。
- 下一步门禁：主代理必须先完整复核 W-088，确认 UV4_COUNT=0 后正式运行 mcu/scripts/build_keil.ps1 -Target All；任一 target 失败立即停止并记录原始日志。不要先改代码；Keil 5.43 实际加载/rebuild 及 fresh dep/object 清单作为权威软件工程门禁。

### W-20260901-089：三目标 Keil All 正式构建通过

- 时间：2026-09-01T15:55:59.2636580+08:00 至 2026-09-01T15:56:29.9211869+08:00
- 关联要求/未决项：W-20260901-088、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理先完整复核 W-088；以 Windows PowerShell 5.1 读取 UV4 进程，得到 UV4_COUNT_BEFORE_BUILD=0，确认无遗留 μVision 进程后才执行正式构建。
- 正式命令：powershell.exe -NoProfile -ExecutionPolicy Bypass -File mcu/scripts/build_keil.ps1 -Target All
- 正式构建结果：START=2026-09-01T15:55:59.2636580+08:00，END=2026-09-01T15:56:29.9211869+08:00，EXIT_CODE=0；脚本按顺序完成 SelfTest、WM8960_Stream、NUEDC_Analysis，三个 target 均为 ArmClang V6.24，0 Error(s), 0 Warning(s)。
- SelfTest：Program Size Code=25408，RO-data=1128，RW-data=2068，ZI-data=41012；49 个 portable DSP objects；D2 SRAM DMA placement 脚本合同通过。
- WM8960_Stream：Program Size Code=32608，RO-data=1128，RW-data=2068，ZI-data=41540；49 个 portable DSP objects；D2 SRAM DMA placement 脚本合同通过。
- NUEDC_Analysis：Program Size Code=32328，RO-data=1876，RW-data=2068，ZI-data=41012；实际编译 14 个 NUEDC 分析对象（Adaptive、Correlate、Demod、DSP_ProMax、FFT、Filter、FilterEx、Fit、IQ、Measure、ModelFit、periodic_analyzer、SoftPll 及 adapter）以及 CMSIS-DSP Pack sources；49 个 portable DSP objects；D2 SRAM DMA placement 脚本合同通过。
- 结论：All 脚本退出 0，三目标工程编译/链接门禁通过。该结果是 Keil/ArmClang 工具链工程构建证据，不是烧录、WM8960 器件、SAI 物理时钟、模拟音频链路或长期稳定性证明。
- 修改/范围边界：本阶段只生成/更新 Keil 构建输出与日志；未触碰 FPGA/Vivado、MPU/VM、Gitee、网络或其他系统，未进行 Git 暂存、提交或推送。
- 下一步：主代理必须先完整复核 W-089；独立检查三个输出目录的 AXF/HEX/MAP/LST、fresh .dep/build logs、对象集合及哈希、DMA map/size。发现任何差异都要记录，之后才规划本地仓库提交/远端推送。

### W-20260901-090：三目标 MCU 构建输出独立审计完成

- 时间：2026-09-02T22:21:20+08:00
- 关联要求/未决项：W-20260901-089、W-20260901-088、W-20260901-078、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员先完整读取当前工作记录，确认物理末尾为 W-20260901-089；主代理随后在 W-089 构建门禁下执行本次只读独立输出审计。本次没有编辑源码、工程 XML、脚本或文档，也没有重新构建；构建动作及退出码由 W-089 记录。
- 审计范围：检查 mcu/project/STM32H743_Audio/MDK-ARM/ 下 SelfTest、WM8960_Stream、NUEDC_Analysis 三个输出目录的 fresh AXF/HEX/MAP/build_log、目录文件数、对象清单、依赖文件、HTML 编译摘要、map 符号、构建后 UV4 进程以及 mcu 范围 git diff --check。
- Fresh 主文件结果：三目标各自四类主文件 AXF、HEX、MAP、build_log 均存在，且 mtime 均晚于 W-089 构建开始时间，判定 fresh。SelfTest DirFiles=168、Objects=81、MissingPermissive=0、NuedcCount=0、DepFiles=1、DepNuedcC=13、DepAdapter=1、DepCmsisSources=29、HtmlNuedc=0、HtmlCmsis=0、Summary=1、MapDMA=1；WM8960_Stream 与 SelfTest 的目录/对象/依赖/HTML/map 计数相同，程序尺寸按下述 HTML 结果不同；NUEDC_Analysis DirFiles=254、Objects=124、DepNuedcC=13、DepAdapter=1、DepCmsisSources=29、HtmlNuedc=14、HtmlCmsis=4、Summary=1、MapDMA=1。
- NUEDC 对象证据：NUEDC_Analysis 的 14 个 NUEDC 对象为 Adaptive、Correlate、Demod、DSP_ProMax、FFT、Filter、FilterEx、Fit、IQ、Measure、ModelFit、periodic_analyzer、SoftPll、nuedc_analysis_selftest；其余两个 target 的 NuedcCount=0。
- HTML 尺寸/错误结果：SelfTest Code=25408、RO-data=1128、RW-data=2068、ZI-data=41012；WM8960_Stream Code=32608、RO-data=1128、RW-data=2068、ZI-data=41540；NUEDC_Analysis Code=32328、RO-data=1876、RW-data=2068、ZI-data=41012。三者均为 0 Error(s)、0 Warning(s)；NUEDC HTML 明确出现 14 个分析文件编译行。
- target 隔离证据：SelfTest/WM8960 的 .dep 中虽有 13 个 NUEDC C 条目及 29 个 CMSIS-DSP source 条目，但这些条目的 command 字段为空，属于依赖目录而非编译证据；两者 HTML 无 NUEDC/CMSIS 编译行、object 无 NUEDC/CMSIS 对象，map 的 NUEDC symbols=0。NUEDC_Analysis 的 .dep command 字段、HTML 编译行、object 清单和 map symbols=389/CMSIS=5163 均存在。由此确认 NUEDC 分析代码只进入 NUEDC_Analysis target。
- 其他验证：构建后 UV4_COUNT=0；mcu git diff --check exit 0。当前 MCU worktree 精确 status 仍为：M mcu/project/STM32H743_Audio/App/Inc/audio_app.h；M mcu/project/STM32H743_Audio/App/Src/audio_app.c；M mcu/project/STM32H743_Audio/MDK-ARM/STM32H743_Audio.uvoptx；M mcu/project/STM32H743_Audio/MDK-ARM/STM32H743_Audio.uvprojx；M mcu/scripts/build_keil.ps1；M mcu/scripts/postgenerate_keil.ps1；M mcu/scripts/regenerate_cubemx.ps1；M mcu/scripts/verify_all.ps1；?? mcu/project/STM32H743_Audio/App/NUEDC/；?? mcu/project/STM32H743_Audio/App/Src/nuedc_analysis_selftest.c；?? mcu/project/STM32H743_Audio/App/Src/nuedc_analysis_selftest.h；?? mcu/project/STM32H743_Audio/MDK-ARM/DebugConfig/STM32H743_Audio_NUEDC_Analysis_STM32H743IITx_1.1.1.dbgconf；?? mcu/project/STM32H743_Audio/MDK-ARM/RTE/_STM32H743_Audio_NUEDC_Analysis/。
- 结论：本次只读审计确认三 target 的构建主文件 fresh、尺寸摘要无错误/警告，且 NUEDC/CMSIS-DSP 编译隔离证据与 NUEDC_Analysis 目标一致；.dep 空 command 条目不能误报为编译泄漏。该结论只证明三目标 Keil 构建输出及 target 隔离，不证明烧录、WM8960 ACK/SAI 时钟/模拟音频、FPGA/Vivado、MPU/VM/Gitee 或任何物理硬件。
- 修改/范围边界：本阶段未编辑源码、工程、脚本、系统或文档，未重新构建，未联网，未触碰 FPGA、MPU、VM 或 Gitee，未暂存、未提交、未推送；构建输出由 W-089 既有动作产生并仅被本次读取审计。
- 下一步：主代理必须先完整复核 W-090，再审阅精确 MCU diff 与提交边界；之后才可按用户要求进行 MCU 阶段性提交/远端推送，并继续处理 FPGA native Vivado 2018.3 工程。

### W-20260902-091：LVGL 移植需求追加与 MCU/桌面参考工程只读审计

- 时间：2026-09-02T22:40:00+08:00（记录追加时间；具体执行时间以主代理终端输出为准）
- 关联要求/未决项：用户新增要求“在 STM32H743IIT6 MCU CubeMX+Keil 工程中移植 LVGL”，并补充“如果 Zynq-7020 PS 可移植 LVGL，也一并完成”；W-20260901-090、W-20260901-089、W-20260901-078、D-008、P0、P2；MPU 已按用户此前要求放弃，主动范围仍为 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：主代理在继续工作前完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260901-090；本条为新的 scope-change/audit entry。此前尝试追加 W-091 未成功且未改变文件，本条不覆盖、保持 append-only 记录。
- MCU 只读检查：当前 AUDIO_DSP MCU 工程没有 LVGL、display、LTDC 或 SPI 配置；`.ioc` 当前只有 I2C1 与 SAI1；HAL 配置未启用 DMA2D/SPI；`board_config` 只有 H743 + WM8960 参考合同。未复制或修改 MCU 工程。
- 桌面参考工程只读检查：`C:\Users\LENOVO\Desktop\Keil_Cubemx_Projects\LVGL_Test` 与 `LVGL_FreeRTOS` 均存在 LVGL 8.2.0（`LVGL/lvgl.h` 版本宏为 8.2.0）、`lv_conf.h`、LTDC/DMA2D/FMC 与 `atk_rgblcd` BSP、port 模板及已编译输出；可作为本机 LVGL 参考，未复制、修改或宣称已验证移植。
- 工作树边界：审计确认工作树仍有既有 MCU/FPGA/文档差异；本条动作未暂存、未提交、未推送，未运行构建、CubeMX 生成、网络、硬件、FPGA 或 MPU/VM 操作。
- 当前判断：LVGL 移植必须先依据 H743IIT6 板级资料确定显示控制器、总线、DMA2D/LTDC 或 SPI/并口接线、刷新缓冲与触摸输入，再将可编译的 LVGL 源码、配置、显示/输入端口、CubeMX 外设配置和 Keil target 纳入工程；Zynq PS 侧仅在 BX71 板卡显示硬件、PS 接口与 Vitis/SDK 2018.3 可用性确认后评估裸机 LVGL，PL 侧音频工程不能直接视为 LVGL 已支持。
- 验证边界：本条只证明需求已登记与参考资料/当前工程状态已读取；不证明 LVGL 源码已移植、CubeMX/Keil 或 Vivado/XSDK/Vitis 可编译、显示器件可驱动、触摸可用、帧率达标或真实硬件通过。
- 下一步：主代理必须先完整复核 W-091，再读取 H743IIT6 与 BX71 的显示硬件资料及项目现状，制定并记录 LVGL 接口/资源/构建合同；任何代码、CubeMX 工程、Vivado/PS 工程或构建动作前都必须再次审阅本记录并追加阶段记录。

### W-20260902-093：LVGL 匿名源码获取尝试因本地命令编排失败而未执行

- 时间：2026-09-02T（记录追加时间以本次工具执行为准）
- 关联要求/未决项：用户新增 LVGL 移植要求；当前物理记录末尾为 W-20260902-091，未发现 W-20260902-092；W-20260902-090、W-20260902-091、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：已完整读取 `docs/PROJECT_WORKLOG.md`，并用行数及全文搜索确认 LINE_COUNT=1560、W-20260902-092 出现次数=0、W-20260902-093 出现次数=0；因此不能声称本条是在审阅不存在的 W-092 后执行。该记录保留请求编号缺口，不覆盖或改写任何历史条目。
- 尝试动作：主代理尝试使用匿名命令 `git clone --depth 1 --branch v8.2.0 https://github.com/lvgl/lvgl.git <temp>` 并统计克隆文件。实际 `functions.exec` 在真正启动 PowerShell 前因 `$env:TEMP` 与字符串引号编排触发的失败原文为：`CreateProcess rejected by policy`。
- 结果/错误：命令未执行；该失败是本地命令编排/进程创建策略失败，不是 GitHub 或网络失败。未创建临时目录、未删除临时目录、未下载 LVGL、未统计克隆文件，未修改工程、源码、系统、Git 工作树或远端。
- 验证边界：本条只证明命令执行在进程启动前被拒绝及其无副作用边界；不证明 GitHub 可达、LVGL v8.2.0 克隆成功、源码完整、许可证已核验、MCU/FPGA 工程已移植或任何构建/硬件通过。
- 下一步：主代理必须先完整复核本条，随后使用简单的固定绝对临时路径命令重试匿名获取；重试前仍需再次审阅最新工作记录，成功或失败均追加新的时间线条目。

### W-20260902-095：LVGL v9.4.0 匿名源码克隆仍在运行且工作树为空；通用 DSP 命名要求登记

- 时间：2026-09-02T（记录追加时间以本次工具执行为准）
- 关联要求/未决项：本条在审阅 W-20260902-093 后执行；W-20260902-094 尚未出现；用户新增 LVGL 较新版本手动移植要求，以及“不要再出现 NUEDC_Analysis、通用 DSP 库使用 Generic_DSP/通用 DSP 命名”硬命名要求；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260902-093、尚未存在 W-20260902-094 或 W-20260902-095；W-093 的“固定绝对路径重试匿名获取”下一步门禁仍有效。
- 匿名获取尝试：主代理按用户要求执行 `git clone --depth 1 --branch v9.4.0 https://github.com/lvgl/lvgl.git C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_4_0`。命令输出 `Cloning into ...` 后函数返回无明确退出码。
- 只读后检：临时目录存在但工作树文件数=0；git 进程仍在运行，PID 为 3644、42840、42996、54764。当前只能判定克隆尚未完成、状态待轮询；不能把该动作写成源码获取成功或 LVGL 已可用。
- 命名约束：用户要求历史记录不得篡改；后续当前源码、Keil target、目录、脚本和文档统一移除 `NUEDC_Analysis` 命名，通用 DSP 库改用 `Generic_DSP`/“通用 DSP”。历史时间线中已出现的 NUEDC 文本保留作为历史事实，不回写、不删除。
- 修改/范围边界：本阶段未编辑仓库或系统配置，未构建，未暂存、提交或推送；未触碰 FPGA、MPU、VM 或硬件。只产生了 LVGL 临时克隆进程/目录状态。
- 验证边界：克隆函数未给出明确退出码且工作树为空；在轮询确认完成或安全终止并记录前，不得引用该目录中的 LVGL 源码、版本或许可证作为已获取证据，也不能声称 MCU/FPGA LVGL 移植开始或完成。
- 下一步：主代理必须先完整复核 W-095，再轮询上述 git 进程/临时目录；成功则记录文件数、HEAD、许可证和退出状态，失败或卡死则按安全范围终止并记录原始结果。之后才可进行仓库内 LVGL 手动移植与 `NUEDC_Analysis`→`Generic_DSP` 统一重命名。

### W-20260902-094：LVGL v9.4.0 官方标签匿名核验迟到勘误

- 时间：2026-09-02T（迟到记录；实际追加时间以本次工具执行为准）
- 关联要求/未决项：用户要求改用开源仓库源码手动移植较新 LVGL；目标版本 v9.4.0；不得使用 Keil LVGL Pack/RTE；LVGL 必须与 MCU/FPGA 其他工程隔离；W-20260902-091、W-20260902-093、W-20260902-095；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 物理顺序勘误：本条原计划在 W-20260902-093 后追加，但完整读取记录时发现并发记录员已先追加 W-20260902-095，故当前物理末尾不是 W-093。为遵守 append-only，本条作为迟到勘误追加在 W-095 之后；不插入、不移动、不删除、不伪造时间线顺序，W-095 原文保持不变。
- 迟到动作一：在 W-20260902-091 之后，主代理运行 `git ls-remote https://github.com/lvgl/lvgl.git refs/tags/v8.2.0 refs/tags/v8.2.0^{}`；输出为 `0019fc541f759b3323add63034502b0248afc58f refs/tags/v8.2.0`。project_recorder_20 未能追加 W-092，原因是 project_recorder_21 先追加了 W-093；本条仅补记该证据，没有下载、解包、复制或修改任何 LVGL/工程/系统/Git/远端。
- 迟到动作二（当前最新标签核验）：主代理审阅 W-093 后运行 `git ls-remote https://github.com/lvgl/lvgl.git refs/tags/v9.4.0 refs/tags/v9.4.0^{}`；输出为 `c016f72d4c125098287be5e83c0f1abed4706ee5 refs/tags/v9.4.0`。该动作仅确认官方 v9.4.0 tag，未下载源码、未修改工作树/系统/Git/远端、未构建、未执行硬件动作。
- 用户要求落实：后续采用 LVGL 官方开源仓库源码手动移植，版本目标固定为 v9.4.0；不使用 Keil 自带 LVGL Pack/RTE；所有 LVGL 源码、配置、显示/输入端口、构建节点和脚本必须隔离，不能影响其他 MCU/FPGA 音频工程部分。
- 验证边界：`git ls-remote` 只证明两个官方 tag 在当前匿名网络路径可达及 v9.4.0 标签提交值；不证明源码已获取、许可证文件已固定、端口设计完成、CubeMX/Keil/Vivado/XSDK 可构建、显示/输入硬件可用或任何真实硬件通过。W-095 中的 v9.4.0 clone 状态仍按其原文处理，不能由本条改写为已成功。
- 下一步：主代理必须先完整复核 W-094 与 W-095，随后匿名获取 v9.4.0 源码并固定 LICENSE/提交 SHA；在确认源码完整后设计 MCU 手动端口，并独立评估/实现 Zynq PS 侧 LVGL，保持 LVGL 与既有音频工程隔离；下载、编辑和构建结果分别追加记录。

### W-20260902-096：LVGL v9.4.0 匿名克隆安全终止与命名/隔离要求登记

- 时间：2026-09-02T（实际追加时间以本次工具执行为准）
- 前置审阅：主代理已完整审阅物理末尾当时包含 W-20260902-095 的 `docs/PROJECT_WORKLOG.md`；W-20260902-094 是并发迟到勘误，按 append-only 规则保留在物理末尾之前的记录位置，不插入、不移动、不删除历史条目。
- 只读检查：主代理审阅 W-095 后，仅检查匿名 LVGL v9.4.0 克隆临时目录 `C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_4_0`；轮询两次后仍为 `FILES=20`、`BYTES=27358`，且无 `HEAD`。该状态不能证明源码获取完整。
- 进程归属核验：`Win32_Process` 命令行确认 PID `3644`（主 `git clone`）、`42996`（子 clone）、`54764`（`git remote-https`）、`42840`（`shallow-file`）全部属于本次 v9.4.0 克隆。
- 安全动作：主代理随后仅对上述 4 个精确 PID 执行 `Stop-Process -Force`；终止命令无输出，按原始结果如实记录；保留临时目录，未删除仓库、工程或系统其他内容。后续检查进程确认这些克隆进程已不再运行（不得将无输出误读为成功，进程检查才是确认依据）。
- 用户要求登记：不使用 Keil LVGL Pack/RTE，改用开源源码手动移植；LVGL 版本目标为 v9.4.0；`NUEDC_Analysis` 不得再出现，通用 DSP 库统一改称 `Generic_DSP`/“通用 DSP”；LVGL 不得影响其他工程。
- 范围与边界：本条未构建、未提交、未推送，未触碰 FPGA、MPU、VM 或硬件；仅记录匿名 LVGL 克隆检查与精确进程终止。尚未证明 LVGL 源码完整、许可证已固定、MCU/FPGA 手动移植完成或任何工程/硬件可用。

### W-20260902-097：官方 LVGL 最新版本与手动移植契约核验

- 时间：2026-09-02T（记录追加时间以本次工具执行为准）
- 前置审阅：已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260902-096，且尚未存在 W-20260902-097；W-096 的“源码尚未证明完整、后续需官方固定版本并手动移植”边界继续有效。
- 官方版本核验：主代理通过匿名官方来源查阅 [LVGL GitHub Releases](https://github.com/lvgl/lvgl/releases/latest) 与 [v9.5.0 release](https://github.com/lvgl/lvgl/releases/tag/v9.5.0)，当前页面显示 `v9.5.0` 为 Latest，页面关联提交缩写为 `85aa60d`。本条将 v9.5.0 作为当前移植目标；源码完整性、许可证文件和本地固定副本仍需后续单独核验。
- 官方集成要求核验：阅读 [LVGL 官方 integration overview](https://docs.lvgl.io/master/integration/index.html)。手动移植至少需要把 LVGL 源码复制/纳入应用工程、提供项目自己的 `lv_conf.h`、提供系统 tick（使用 `lv_tick_inc` 或 `lv_tick_set_cb`）、创建 display，并配置绘制 buffer 与 flush callback；主循环必须定期调用 `lv_timer_handler`，tick 必须由 SysTick 或其他可靠时间源持续提供。
- 官方 9.x Display API 核验：阅读 [Display setup](https://docs.lvgl.io/master/details/main-modules/display/setup.html)，确认 9.x 使用 `lv_display_create` 创建 display、`lv_display_set_buffers` 设置 buffer、`lv_display_set_flush_cb` 设置 flush 回调，硬件/传输完成后调用 `lv_display_flush_ready` 完成一次刷新。后续 MCU 与 Zynq PS 端口必须以这些 9.x API 为准，不能沿用旧版 `lv_disp_drv_t`/Keil Pack/RTE 模式。
- 用户约束落实：采用官方开源仓库源码手动移植最新 v9.5.0；不使用 Keil 自带 LVGL Pack/RTE；LVGL 的源码、`lv_conf.h`、显示/输入 port、缓冲区、构建节点和脚本必须与现有音频工程隔离，不得改变其他 DSP/SAI/Codec/FPGA 音频路径。现有 `NUEDC_Analysis` 历史文本不回写、不删除；当前工程新命名继续使用 `Generic_DSP`/“通用 DSP”。
- 本次动作边界：此次仅浏览官方 GitHub Releases 与 docs.lvgl.io 资料并登记结论，未克隆或复制源码，未修改仓库/系统，未构建、未提交、未推送，未触碰 FPGA、MPU、VM 或硬件。
- 验证边界：官方页面核验只证明版本/API/集成要求，不能证明 v9.5.0 源码已在本地完整取得、许可证已固定、MCU CubeMX+Keil 或 Zynq PS Vivado/SDK 2018.3 工程已完成、显示/输入设备可驱动、性能达标或真实硬件通过。
- 下一步：主代理必须先完整复核 W-097，再匿名获取并固定 LVGL v9.5.0 源码与许可证；随后按官方 9.x API 设计独立 MCU 手动移植，并评估/实现隔离的 Zynq PS LVGL 端口；每次获取、编辑、生成和构建动作均须单独追加记录。

### W-20260902-098：官方 LVGL v9.5.0 标签匿名固定校验

- 时间：2026-09-02T（记录追加时间以本次工具执行为准）
- 关联要求/未决项：用户要求使用官方最新 LVGL、严格按官方说明手动移植、不使用 Keil Pack/RTE、LVGL 与既有音频工程隔离；W-20260902-097、W-20260902-096、D-008、P0、P2；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾已因并发追加到 W-20260902-097；同时确认 W-097 已将官方网页核验目标更新为 v9.5.0。本条不插入、不移动、不删除 W-097 或任何历史条目。
- 官方标签命令：主代理审阅工作记录后运行 `git ls-remote https://github.com/lvgl/lvgl.git refs/tags/v9.5.0 refs/tags/v9.5.0^{}`。
- 结果与原始证据：命令返回 `85aa60d18b3d5e5588d7b247abf90198f07c8a63 refs/tags/v9.5.0`；该 SHA 是官方 LVGL v9.5.0 标签对应的完整 commit SHA。
- 结论：当前 LVGL 移植目标从 v9.4.0 更新为官方最新 v9.5.0；后续源码获取、许可证固定、API/端口设计和工程构建均以该版本/提交为准。
- 修改/范围边界：本动作仅匿名读取 GitHub 官方 tag 引用；无下载、无文件/工程/系统/Git 工作树/远端修改，无构建、无硬件动作，未触碰 FPGA、MPU、VM 或 Gitee。
- 验证边界：`git ls-remote` 只证明官方 v9.5.0 tag 在当前匿名网络路径可达并固定到上述完整 SHA；不证明源码已本地完整取得、LICENSE 已核验、MCU CubeMX+Keil 或 Zynq PS Vivado/SDK 工程已完成、显示/输入硬件可用或真实硬件通过。
- 下一步：主代理必须先完整复核 W-098，再匿名获取并固定 v9.5.0 源码与许可证；按官方 9.x API 手动移植到隔离 MCU 工程，并评估/实现隔离的 Zynq PS LVGL 端口。

### W-20260902-099：LVGL v9.5.0 官方压缩包匿名获取失败（迟到记录）

- 时间：2026-09-02T22:34:55.6801245+08:00（本条追加时间；实际尝试时间以主代理终端记录为准）
- 编号/顺序说明：本次执行前完整读取工作记录时，发现并发记录员已经在物理末尾追加了 W-20260902-098（官方 v9.5.0 标签固定校验），因此不能重复使用 W-098 或声称物理末尾仍为 W-097。本条按 append-only 规则使用下一个编号追加，保留原始顺序和 W-098 内容。
- 动作前审阅：已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260902-098；W-097 的官方最新版本/API核验和 W-098 的 v9.5.0 tag SHA `85aa60d18b3d5e5588d7b247abf90198f07c8a63` 继续有效。
- 尝试命令：主代理尝试执行 `Invoke-WebRequest -UseBasicParsing -TimeoutSec 120 -Uri https://github.com/lvgl/lvgl/archive/refs/tags/v9.5.0.zip -OutFile C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0.zip`，通过 `functions.exec` 返回约 30 秒、无输出且未给出文件信息。
- 结果与失败证据：随后只读检查发现 `ZIP_EXISTS=0`，未确认压缩包下载成功；系统中未发现对应下载进程命令行。因此本次判定为源码获取失败/未产生仓库文件修改，不能写成 v9.5.0 源码已获取。
- 用户要求落实：目标为官方最新 LVGL v9.5.0；严格按官方说明手动移植；不使用 Keil Pack/RTE；LVGL 不得影响其他工程；通用 DSP 库使用 `Generic_DSP`/“通用 DSP”命名，`NUEDC_Analysis` 不得出现在后续新命名中。历史记录中的旧名称不回写、不删除。
- 范围与边界：未解包、未编辑、未构建、未提交、未推送，未触碰 FPGA、MPU、VM、硬件或远端；除失败的临时下载尝试外未产生仓库文件修改。保留本次失败证据，不删除或扩大任何目标。
- 下一步：主代理必须先完整复核 W-099，再按匿名来源重新获取并核验 v9.5.0 源码、LICENSE 和固定提交；源码完整性确认前不得开始 LVGL 工程移植或声称构建可用。

### W-20260902-100：空补丁调用被拒绝（保留失败）

- 时间：2026-09-02T（记录追加时间以本次工具执行为准）
- 关联要求/未决项：D-008、P0；W-20260902-098、W-20260902-099；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员已完整读取包含并发追加 W-099 的最新 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-099；本条不重复使用 W-098，不插入、不移动、不删除历史条目。
- 失败动作：在 W-098 成功追加后，记录员误发了一次空的 apply_patch 调用，仅用于尝试触发补丁结果确认。
- 原始错误：工具返回 `patch rejected: empty patch`；由于补丁为空，未进入任何文件更新。
- 结果/范围边界：`docs/PROJECT_WORKLOG.md`、源码、工程、系统、工具链、远端和 Git 工作树均未因该空补丁调用发生修改；未构建、未联网、未触碰 FPGA、MPU、VM 或硬件。W-098 与并发 W-099 内容保持不变。
- 验证边界：本条只记录空补丁被拒绝及无副作用边界，不证明 LVGL v9.5.0 源码已获取、许可证已固定、MCU/FPGA 工程已移植、构建通过或硬件可用。
- Git：未暂存、未提交、未推送。
- 下一步：主代理必须先完整复核 W-100，继续按 W-099 门禁匿名获取并核验 LVGL v9.5.0 源码、LICENSE 与固定提交，然后再开始隔离手动移植。

### W-20260902-101：重复空补丁调用被拒绝（保留失败）

- 时间：2026-09-02T（记录追加时间以本次工具执行为准）
- 关联要求/未决项：D-008、P0；W-20260902-100；主动范围仍仅 MCU H743IIT6 与 FPGA BX71/Zynq7020，MPU/VM/Gitee 停止。
- 动作前审阅：记录员已完整读取最新工作记录，确认物理末尾为 W-100，并在 W-100 后再次确认该空调用没有文件变更；本条按 append-only 规则追加，不修改任何历史条目。
- 失败动作：记录员在追加 W-100 后误发了第二次空的 apply_patch 调用。
- 原始错误：工具返回 `patch rejected: empty patch`；空补丁未调用任何文件更新操作。
- 结果/范围边界：该调用没有修改 `docs/PROJECT_WORKLOG.md`、源码、工程、系统、工具链、远端或 Git 工作树；未构建、未联网、未触碰 FPGA、MPU、VM 或硬件。W-098、W-099、W-100 内容保持不变。
- 验证边界：本条仅记录重复空补丁的拒绝及无副作用边界，不证明 LVGL v9.5.0 源码已获取、许可证已固定、MCU/FPGA 工程已移植、构建通过或硬件可用。
- Git：未暂存、未提交、未推送。
- 下一步：主代理必须先完整复核 W-101，再继续按 W-099 门禁匿名获取并核验 LVGL v9.5.0 源码、LICENSE 与固定提交。

### W-20260902-102：通过 Clash Verge 代理匿名获取 LVGL v9.5.0 官方源码包

- 时间：2026-09-02T22:36:32.5991722+08:00
- 编号/顺序说明：本次执行前已完整读取工作记录；实际物理末尾为并发追加的 W-20260902-101（W-100 与 W-101 已分别记录两次空补丁失败），因此不能重复使用 W-100，也不能声称物理末尾仍为 W-099。本条按 append-only 规则使用下一个编号追加，保留所有历史条目及其顺序。
- 动作前审阅：主代理审阅 W-099 后检查到 PowerShell 环境变量 `ALL_PROXY`、`HTTP_PROXY`、`HTTPS_PROXY` 均为 `http://127.0.0.1:7897`；依据用户已开启 Clash Verge 代理的说明，使用 GitHub 官方 codeload 匿名地址获取固定目标 `v9.5.0`。
- 执行命令：`curl.exe -L --proxy http://127.0.0.1:7897 --http1.1 --connect-timeout 15 --max-time 300 -sS -o C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0.tar.gz https://codeload.github.com/lvgl/lvgl/tar.gz/refs/tags/v9.5.0`
- 结果与原始证据：`CURL_EXIT=0`；临时文件存在，大小 `101,308,654` bytes，时间 `2026-09-02 22:36:05`。这证明通过当前 Clash Verge 代理可匿名取得 GitHub 官方 codeload 的 LVGL v9.5.0 压缩包。
- 验证边界：本次尚未解包，尚未核验归档内 HEAD、完整文件清单、LICENSE 内容或源码哈希；因此不能据此声称本地源码完整性、许可证固定、官方 API/端口移植正确或任何工程构建/硬件可用。
- 修改/范围边界：仓库未修改；未构建、未提交、未推送，未触碰 FPGA、MPU、VM、硬件或远端仓库。源码包仍位于 `C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0.tar.gz`，后续应先核验再进入工程移植。
- 用户要求保留：目标必须为最新 LVGL v9.5.0；严格依照官方说明手动移植；不使用 Keil Pack/RTE；LVGL 与其他工程隔离；通用 DSP 库统一使用 `Generic_DSP`/“通用 DSP”命名，不再产生 `NUEDC_Analysis` 新名称。既有历史记录中的旧名称不回写、不删除。
- 下一步：主代理必须先完整复核本条，再解包并核验 v9.5.0 的 HEAD、文件清单、LICENSE 和源码哈希；核验通过后才可开始 MCU H743IIT6 的隔离手动移植，并评估隔离的 Zynq PS LVGL 端口。

### W-20260902-103：官方 LVGL v9.5.0 codeload 解包与源码只读核验

- 时间：2026-09-02T22:38:08.5015496+08:00
- 关联要求/未决项：W-20260902-101、W-20260902-102、P2；用户要求使用较新且尽可能最新的官方 LVGL，严格依照官方说明手动移植，不使用 Keil Pack/RTE，且 LVGL 必须与现有音频工程隔离；通用 DSP 库统一使用 `Generic_DSP`/“通用 DSP”命名，后续新名称不得出现 `NUEDC_Analysis`。
- 动作前审阅：已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260902-102；并发 W-100/W-101 以及迟到的 W-094/W-098/W-099 等条目均按 append-only 规则保留。本条在审阅 W-101 后已完成的源码解包与只读核验结果，使用下一个编号追加，不插入、不移动、不删除历史内容。
- 解包命令：先执行 `New-Item -ItemType Directory -Path C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0_src -Force`，再执行 `tar -xzf C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0.tar.gz -C C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0_src`。
- 解包结果：`TAR_EXIT=0`；归档根目录 `lvgl-9.5.0` 存在，说明官方 v9.5.0 codeload 压缩包已成功解包到指定临时目录。
- 只读审计结果：总计 `FILES=4847`、`BYTES=177421681`；根目录包含 `CMakeLists.txt`、`COPYRIGHTS.md`、`LICENCE.txt`（1072 bytes）、`lv_conf_template.h`（52731 bytes）、`lv_version.h`（248 bytes）、`lvgl.h`（5971 bytes）和 `README`。分类统计为 `src files=1130 bytes=23770693`、`tests=2379/67095275`、`examples=424/10256861`、`demos=176/30745349`、`env_support=41/8737053`。
- 版本/许可证证据：`lv_version.h` 和 `lv_conf_template.h` 均标记 v9.5.0；官方许可证文件名为 `LICENCE.txt`。后续 MCU 工程只应纳入 `src` 及必要的官方头文件/配置，测试、示例和演示不导入 MCU；该解包动作没有把测试/示例/演示复制到正式工程。
- 修改范围：只新增/使用临时目录 `C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0_src` 并进行只读审计；未修改 AUDIO_DSP 仓库、正式工程、系统配置、工具链或远端，未构建、未提交、未推送，未触碰 MPU、VM、FPGA 硬件或 MCU 硬件。
- 验证边界：本条证明官方 v9.5.0 源码包已解包且目录/规模/版本/许可证文件可读；尚未记录关键文件 SHA-256 或官方 tag/commit 与解包内容的对应关系，尚未开始 MCU 或 Zynq PS 手动端口，也不证明任何 CubeMX/Keil/Vivado/XSDK 构建、显示/输入硬件或真实硬件通过。
- 下一步：主代理必须先完整复核 W-103，再单独固定关键文件哈希与官方提交对应关系；随后按 LVGL 9.x 官方 API 在隔离目录中手动接入 MCU，并评估/实现隔离的 Zynq PS 端口，保持 `Generic_DSP` 命名和既有音频路径不受影响。

### W-20260902-104：导入官方 LVGL v9.5.0 MCU 源码（隔离目录）

- 时间：2026-09-02T（实际追加时间以本次工具执行为准）
- 关联要求/未决项：W-20260902-103、P2；用户要求使用官方最新 LVGL v9.5.0，严格按官方说明手动移植，不使用 Keil 自带 Pack/RTE，且 LVGL 不得影响其他工程。
- 动作前审阅：主代理已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260902-103；已确认官方 v9.5.0 源码解包目录存在，且后续 MCU 接入必须与既有音频工程隔离。
- 导入来源：`C:\Users\LENOVO\AppData\Local\Temp\audio_dsp_lvgl_v9_5_0_src\lvgl-9.5.0`；固定目标版本为官方 v9.5.0，关联官方 tag commit `85aa60d18b3d5e5588d7b247abf90198f07c8a63`（历史核验见 W-20260902-098）。
- 执行命令：使用 `New-Item` 创建目标目录，并使用 `Copy-Item` 从解包目录复制 `src/` 及顶层 `lvgl.h`、`lv_version.h`、`lvgl_private.h`、`lv_conf_template.h`、`LICENCE.txt`、`COPYRIGHTS.md` 到 `mcu/project/STM32H743_Audio/App/LVGL`；明确未复制 `tests/`、`examples/`、`demos/`。
- 结果与原始证据：目标目录统计为 `DEST_FILES=1136`、`DEST_BYTES=23837297`。顶层文件尺寸为：`COPYRIGHTS.md=1914`、`LICENCE.txt=1072`、`lv_conf_template.h=52731`、`lv_version.h=248`、`lvgl_private.h=4668`、`lvgl.h=5971` 字节。
- 修改文件：仓库新增 `mcu/project/STM32H743_Audio/App/LVGL` 官方源码目录；未修改既有 MCU/FPGA/MPU 代码、工程配置或其他目录。
- 验证边界：本动作只证明官方 v9.5.0 指定源码子集已复制到隔离 MCU 目录；尚未把模板变为项目 `lv_conf.h`，尚未固定导入目录逐文件哈希，尚未完成手写 STM32 显示/输入/tick/flush 端口，尚未把 LVGL 加入隔离构建 target，未构建、未提交、未推送、未执行硬件测试。
- Git/范围：没有提交或推送；未触碰 FPGA、MPU、VM、Gitee 或任何硬件。后续必须保持 LVGL 节点与音频 DSP/SAI/Codec 路径隔离，并另行记录配置、端口、工程接入与构建结果。
- 下一步：主代理必须先完整复核 W-104，再把 `lv_conf_template.h` 转为项目隔离配置，核对官方源码/许可证哈希，手写 STM32H743 显示端口与 tick/flush 接口，并仅在独立 LVGL target 中验证 Keil 构建；随后评估隔离的 Zynq PS LVGL 端口。

### W-20260902-105：LVGL v9.5.0 配置副本建立（尚未启用）

- 时间：2026-09-02T（实际追加时间以本次工具执行为准）
- 关联要求/未决项：W-20260902-104、P2；用户要求使用官方最新 LVGL v9.5.0，严格按官方说明手动移植，不使用 Keil Pack/RTE，且 LVGL 必须与既有音频工程隔离。
- 动作前审阅：记录员已完整读取 `docs/PROJECT_WORKLOG.md` 全文；物理末尾实际为 W-20260902-104，不存在以 W-103 作为物理末尾的状态。W-104 的官方源码导入和“先建立项目配置、再接入端口/构建”的下一步继续有效。
- 实际动作：主代理审阅 W-103 后，将仓库内官方 v9.5.0 `mcu/project/STM32H743_Audio/App/LVGL/lv_conf_template.h` 复制为同目录的 `lv_conf.h`；未编辑其他文件。
- 结果与原始证据：`lv_conf.h` 文件长度为 `52731` bytes，文件时间戳为 `2026-02-19 00:09:37`。
- 验证边界：本动作只建立官方模板的项目配置副本；尚未启用或构建 LVGL，未修改 `.ioc`、`.uvprojx` 或其他 target，未完成 tick/display/input/flush 端口，也不证明 CubeMX/Keil、Vivado/XSDK 或任何真实硬件通过。
- 修改/范围边界：仅新增 `mcu/project/STM32H743_Audio/App/LVGL/lv_conf.h`；未提交、未推送，未触碰 FPGA、MPU、VM、硬件或其他工程部分。
- Git：未暂存、未提交、未推送。
- 下一步：主代理必须先完整复核 W-105，再按 LVGL v9.5.0 官方 API 完成隔离配置和 STM32H743 手写端口；随后单独规划构建 target，保持现有音频路径不受影响。

### W-20260902-106：启用并裁剪官方 LVGL v9.5.0 项目配置

- 时间：2026-09-02T22:44:00+08:00（记录追加时间）
- 关联要求/未决项：W-20260902-105、P2；用户要求使用官方最新 LVGL、严格依照官方说明手动移植、不使用 Keil Pack/RTE，并保证 LVGL 不影响既有音频工程；通用 DSP 继续使用 `Generic_DSP`/“通用 DSP”命名。
- 动作前审阅：本次记录动作前已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260902-105；W-104 的官方源码导入和 W-105 的配置副本建立结论继续有效。本条只记录主代理随后对配置文件的已完成修改，不插入、移动或删除历史条目。
- 实际动作：主代理仅修改 `mcu/project/STM32H743_Audio/App/LVGL/lv_conf.h`，将官方 v9.5.0 模板启用为项目配置（文件头由模板守卫改为 `#if 1`），保持 `LV_COLOR_DEPTH=16`/RGB565，设置 `LV_MEM_SIZE=96 KiB`，启用软件绘制单元 1，并仅保留 RGB565 与 RGB565_SWAPPED 支持；保留合成器 UI 所需的 arc/bar/button/chart/label/slider/switch、默认主题和 flex 布局，关闭多余 widgets、simple/mono 主题及 grid 布局；启用 `LV_USE_ST7789=1`（自动启用 Generic MIPI），关闭 ST LTDC；关闭 `LV_BUILD_EXAMPLES=0` 与 `LV_BUILD_DEMOS=0`。
- 结果与原始证据：只读核对显示 `lv_conf.h` 大小为 `52748` bytes；首个配置守卫为 `#if 1 /* Project-local LVGL 9.5.0 configuration is enabled. */`；关键配置行核对结果为 `LV_COLOR_DEPTH 16`、`LV_MEM_SIZE (96 * 1024U)`、`LV_USE_OS LV_OS_NONE`、`LV_USE_DRAW_SW 1`、`LV_DRAW_SW_DRAW_UNIT_CNT 1`、`LV_USE_ST7789 1`、`LV_USE_ST_LTDC 0`、`LV_BUILD_EXAMPLES 0`、`LV_BUILD_DEMOS 0`，裁剪后的 widgets/themes/layouts 与上述选择一致。
- Git/范围：`git status --short -- mcu/project/STM32H743_Audio/App/LVGL/lv_conf.h` 显示该项目配置仍为未跟踪文件（`??`）；`git diff --stat` 对该路径没有已跟踪差异，这是因为 LVGL 目录尚未纳入索引。除该配置文件外，本次动作未修改其他文件；未暂存、未提交、未推送，未构建，未触碰 FPGA、MPU、VM、远端或任何硬件。
- 验证边界：本次只证明配置文件已按 v9.5.0 模板完成项目级启用与裁剪，不证明 LVGL 源码可编译、Keil target 已接入、STM32H743 显示/tick/flush 端口正确、UI 性能达标、显示器件可工作或真实硬件通过；`LV_USE_ST7789` 仅是配置开关，仍需后续手写 HAL 传输端口和独立构建 target 验证。
- 下一步：主代理必须先完整复核 W-106，再以官方 LVGL 9.x API 编写隔离的 STM32H743 tick/display/flush/UI 端口，更新 Keil/CubeMX 生成后脚本增加独立 LVGL target，保持 SelfTest、WM8960 Stream 与 Generic_DSP target 不受影响；随后另行评估隔离的 Zynq PS 端口。

### W-20260902-107：导入并核对 LVGL 独立 target 使用的 STM32H7 HAL SPI 文件

- 时间：2026-09-02T（记录追加时间以本次工具执行为准）
- 关联要求/未决项：W-20260902-106、P2；用户要求从开源仓库手动移植最新版 LVGL，禁止使用 Keil Pack/RTE，且 LVGL 不得影响既有音频 target。
- 动作前审阅：记录员已完整读取 `docs/PROJECT_WORKLOG.md` 全文，确认物理末尾实际为 W-20260902-106；本条按 append-only 规则追加，不插入、移动或删除历史条目。
- 导入来源与目标：从桌面 `C:\Users\LENOVO\Desktop\Keil_Cubemx_Projects\LVGL_Test\Drivers\STM32H7xx_HAL_Driver\Inc\stm32h7xx_hal_spi.h` 与 `...\Src\stm32h7xx_hal_spi.c` 导入到 `mcu/project/STM32H743_Audio/Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_spi.h` 和 `mcu/project/STM32H743_Audio/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c`。这些文件仅供后续独立 `STM32H743_Audio_LVGL_UI` target 使用，未加入 SelfTest、WM8960 Stream 或 Generic_DSP target 的编译组。
- SHA-256 核对命令：PowerShell `Get-FileHash -Algorithm SHA256` 分别读取源文件与目标文件并比较。
- 核对结果：`stm32h7xx_hal_spi.h` 源/目标均为 `58261` bytes，SHA-256=`9EA983FD0E3148A3291B2AA56B1D01709374A3A3845D6B592B3D3356B71F6CB5`；`stm32h7xx_hal_spi.c` 源/目标均为 `127299` bytes，SHA-256=`862A531FAA657484A7AF61412387F146587E4430CBFDCF104749F48683AB419C`。文件内容和尺寸一致。
- 修改/范围边界：本次确认的仓库新增文件仅为上述两个 HAL SPI 文件；未修改 CubeMX `.ioc`、Keil 工程、既有音频代码或 LVGL 配置，未触碰 FPGA、MPU、VM、远端或硬件。
- 验证边界：SHA-256 一致只证明导入文件与桌面参考副本一致，不证明该 HAL SPI 文件与当前 CubeMX/HAL 版本的接口完整性，不证明 LVGL 端口、Keil 独立 target、CubeMX 重新生成、Vivado/XSDK、显示器件或真实硬件通过；尚未编译。
- Git：未暂存、未提交、未推送。
- 下一步：主代理必须先完整复核 W-20260902-107，再按官方 LVGL 9.x display/tick/flush API 编写独立 STM32H743 手写端口，并把 HAL SPI 文件仅挂入 LVGL target，随后验证非 LVGL target 无对象泄漏。

### W-20260902-108：补齐 LVGL target 缺失的 STM32H7 HAL SPI 扩展头

- 时间：2026-09-02T22:55:09+08:00（记录追加时间）
- 关联要求/未决项：W-20260902-107、P2；用户要求使用官方最新版 LVGL 手动移植、禁止 Keil Pack/RTE，并保持 LVGL 与其他音频 target 隔离。
- 动作前审阅：记录员已完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260902-107；本条按 append-only 规则使用下一个编号，不插入、移动或删除历史条目。
- 失败证据：LVGL 独立 target 的 ArmClang 编译暴露缺少 `stm32h7xx_hal_spi_ex.h`（SPI HAL 头文件被 `stm32h7xx_hal_spi.h` 引用），导致该 target 在头文件解析阶段无法继续；该失败不等同于其他三个音频 target 的构建失败。
- 实际动作：从桌面 `C:\Users\LENOVO\Desktop\Keil_Cubemx_Projects\LVGL_Test\Drivers\STM32H7xx_HAL_Driver\Inc\stm32h7xx_hal_spi_ex.h` 导入单个文件到 `mcu/project/STM32H743_Audio/Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_spi_ex.h`，未导入其他 HAL 文件或修改其他路径。
- SHA-256/尺寸核对：源文件与目标文件均为 `2392` bytes，SHA-256 均为 `44D854991D118840677DA5CE86D88B1E555FE811808354522AD2F198AC4FA66C`；目标文件 Git 状态为未跟踪 `??`。该结果证明单文件内容与桌面参考副本一致。
- 修复范围边界：该头文件仅供后续 `STM32H743_Audio_LVGL_UI` target 配套的 HAL SPI 编译使用，尚未证明其与当前 HAL/ArmClang 全部接口兼容；没有修改 CubeMX `.ioc`、现有音频源码、LVGL 配置、FPGA、MPU/VM、系统环境或远端。
- 验证边界：导入后未重新运行 Keil/CubeMX 构建，未验证 LVGL display/tick/flush 端口、非 LVGL target 无对象泄漏、显示器件或真实硬件；未提交、未推送。
- 下一步：主代理必须先完整复核 W-20260902-108，再重新运行仅 LVGL target 的 ArmClang 构建并记录后续失败/通过证据；通过后再执行四 target 隔离回归与 FPGA PS LVGL 评估。

### W-20260904-109：LVGL 独立 target 首次完成链接，但未通过严格零警告门禁

- 时间：2026-09-04（记录追加时间）
- 关联要求/未决项：W-20260902-108、P2；用户要求严格依照官方说明手动移植最新版 LVGL，禁止使用 Keil Pack/RTE，并保证 LVGL 不影响既有音频工程。
- 动作前审阅：主代理执行构建前已完整复核 W-20260902-108；记录员追加本条前又完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾仍为 W-20260902-108。本条只在物理末尾追加，不插入、移动或删除历史内容。
- 实际命令：在仓库根目录运行 PowerShell `& 'mcu/scripts/build_keil.ps1' -Target LVGL`，仅请求构建 `STM32H743_Audio_LVGL_UI` target。
- 构建结果：在 W-108 补入缺失的 HAL SPI 扩展头后，本次底层 Keil/ArmClang 构建实际执行到链接完成；`keil_build.log` 末尾报告 `Program Size: Code=188524 RO-data=16512 RW-data=2068 ZI-data=163524`，并报告 `STM32H743_Audio_LVGL_UI.axf - 0 Error(s), 9 Warning(s)`、`Build Time Elapsed:  00:00:37`。命令结束后核对当前无 `UV4`、`armclang` 或 `armlink` 进程残留。
- 门禁结论：工程已生成 LVGL target 的 AXF 且编译/链接错误数为 0，但 `build_keil.ps1` 的验收规则要求零错误且零警告；现有 9 条 warning 使严格门禁未通过。因此本次不得表述为 clean build 或完整编译验收通过，后续必须逐条定位并消除或以明确规则处理这些 warning 后重跑。
- 隔离与范围：本次只运行 LVGL target 构建，没有执行 SelfTest、WM8960 Stream、Generic_DSP 三个 target 的回归，因此尚未形成“四 target 隔离无泄漏”的新证据；未改动 FPGA、MPU/VM、远端或硬件。
- 验证边界：该结果只证明当前主机上的 Keil/ArmClang 能把 LVGL target 编译并链接成 AXF；不证明 LVGL display/tick/flush 时序正确，不证明 ST7789 引脚、电气连接、色序、刷新性能或真实屏幕工作，也不构成任何板级音频、FPGA 或硬件验证。
- Git/发布：本次未暂存、未提交、未推送。
- 下一步：主代理必须先完整复核 W-20260904-109，提取并分析 `keil_build.log` 中全部 9 条 warning，修复后再次运行仅 LVGL target 的严格构建；达到 0 Error/0 Warning 后再执行四 target 隔离回归与 FPGA PS LVGL 评估。

### W-20260904-110：加入 LVGL 文件级 warning 约束与真实源路径隔离检查，但生成后校验失败

- 时间：2026-09-04（记录追加时间）
- 关联要求/未决项：W-20260904-109、P2；用户要求严格按官方最新版 LVGL 手动移植、不使用 Keil Pack/RTE，并确保 LVGL 不影响其他工程部分。
- 动作前审阅：主代理执行本阶段动作前已完整审阅最新工作记录；记录员追加本条前再次完整读取 `docs/PROJECT_WORKLOG.md`，确认物理末尾为 W-20260904-109。本条只在文件物理末尾追加，不插入、移动或删除历史条目。
- Keil XML 结构审计：为确认文件级编译选项写法，首次使用 `rg` 查询 XSD 时因引号/正则转义错误而失败；随后改用 PowerShell `Select-String` 成功确认 Keil 工程 XSD 中 `Cads`、`VariousControls`、`MiscControls` 是合法结构。该失败已保留为过程证据，没有据失败结果臆造 XML 结构。
- `postgenerate_keil.ps1` 修改：主代理仅用 `apply_patch` 修改 `mcu/scripts/postgenerate_keil.ps1`，新增文件级 compiler flags helper；计划仅对两个已审计的 RGB565 源文件增加 `-Wno-unused-function`，仅对 `lv_tlsf.c` 增加 `-Wno-unused-parameter`，并在生成后增加“必须精确存在三项 suppression”的结构校验。官方 LVGL vendored 源码本身未被修改。
- `build_keil.ps1` 修改：主代理仅用 `apply_patch` 修改 `mcu/scripts/build_keil.ps1`，把 LVGL target 隔离审计从按同 basename 的 `.o` 文件名判断，改为读取 `.d` 依赖文件中的完整源文件路径并逐一映射，消除不同目录同名源文件造成的假阳性。
- 执行与失败：随后运行 `mcu/scripts/postgenerate_keil.ps1`；脚本在第 723 行触发 `LVGL target must have exactly three file-local warning suppressions`，进程以 exit code `1` 结束。当前只确认生成后校验没有发现精确三项预期 suppression，根因仍待定位；不得把这次失败解释为 suppression 已正确写入，也不得跳过该门禁。
- 当前状态：本次失败后尚未重新运行 `build_keil.ps1 -Target LVGL`，因此 W-109 的 `0 Error(s), 9 Warning(s)` 仍是最近一次完整 LVGL 构建证据；没有新的零警告构建或四 target 隔离回归证据。
- 范围与验证边界：本阶段没有修改官方 LVGL 源码，没有验证显示 tick/flush、电气连接、色序、刷新性能、音频共存、FPGA 或真实硬件；没有触碰 MPU/VM 或远端。
- Git/发布：未暂存、未提交、未推送。
- 下一步：主代理必须先完整复核 W-20260904-110，检查 `postgenerate_keil.ps1` 写入文件级 flags 后的实际 XML 节点及 helper 的路径/节点匹配，修复“精确三项 suppression”校验失败；校验通过后再重跑仅 LVGL target，并以完整 warning 列表和严格门禁结果判断是否 clean。

### W-20260909-111：恢复 LVGL 文件级配置并补强构建对象门禁

- 时间：2026-09-09T05:07:39+08:00（阶段汇总追加时间）。
- 前置与范围：主代理已完整复核至 W-20260904-110；记录员按每块 125 行完整读取当时 1775 行全文后，仅在物理末尾追加本条。当前只继续 MCU/FPGA，MPU/VM/Gitee 停止；历史失败、旧命名记录和无关脏工作保留。
- 本机恢复：`git status --short --branch` 为 `main...origin/main`，原有未提交差异仍在，无暂存。只读 MEMORY 提醒与当前日志约束一致。CIM 查询 UV4/armclang/armlink 被系统拒绝访问；改用 `Get-Process -Name UV4,armclang,armlink -ErrorAction SilentlyContinue` 无输出，未发现这些进程；没有终止任何进程。
- 正式 XML 证据：读取正式 `.uvprojx`，三个非 LVGL target 的文件级 warning flags 均为 0；LVGL target 精确三项，为两项 `-Wno-unused-function`、一项 `-Wno-unused-parameter`。由此确认 9 月 4 日中断前的路径正规化修复与 postgenerate 已实际写入预期配置；W-110 的首次失败原文保留，不改写为成功。
- 实际编辑：主代理使用 `apply_patch` 修改 `mcu/scripts/build_keil.ps1`：`Start-Process` 增加 `-WindowStyle Hidden`；读取每个 `.d` 后，先要求其目标 `.o` 实际存在且解析路径位于所选输出目录内，再计入完整源路径映射，防止仅有 `.d` 而没有对象文件时误判通过。已只读检查 `vg_lite_matrix_1.d` 与 `lv_tlsf.d` 的真实目标格式。
- 验证边界：本条只证明当前 XML 配置恢复与脚本补丁落盘；尚未运行本次 Parser 或严格 LVGL 构建，最近完整构建仍为 W-109 的 0 Error(s)/9 Warning(s)。不证明新对象门禁运行正确、clean build、四 target 隔离、显示 tick/flush 或任何真实硬件通过。
- 修改/发布范围：未修改官方 LVGL vendored 源码、FPGA、MPU/VM、系统配置或硬件；未联网、未暂存、未提交、未推送。记录员仅修改本日志，检查点待本轮结束时依据实际结果同步。
- 下一步：运行 PowerShell Parser 后仅构建 LVGL target，逐项记录严格门禁、完整 warning/对象映射结果；通过后再推进 Generic_DSP 统一命名与端口质量修复。

### W-20260909-112：LVGL 严格零警告构建与完整源对象映射通过

- 时间：2026-09-09T10:02:40+08:00（阶段汇总追加时间；实际构建时点以日志为准）。
- 命令与结果：`build_keil.ps1` 的 PowerShell Parser errors=0，`git diff --check -- mcu/scripts/build_keil.ps1` 通过；随后运行 `& mcu/scripts/build_keil.ps1 -Target LVGL`，exec session 40438 最终 exit 0。ArmClang 6.24 报告 `0 Error(s), 0 Warning(s)`、Build Time 24 秒；Code=188524、RO-data=16512、RW-data=2068、ZI-data=163524。完整日志为 `mcu/build/keil_lvgl.log`。
- 对象与隔离证据：脚本确认全部 466 个 LVGL 源路径各自恰好映射一个 `.d`，且对应 `.o` 实际存在、解析路径位于当前输出目录；包含同名源自动编号的 `vg_lite_matrix_1`。LVGL target 未编入 49 个 permissive DSP 对象或 14 个旧 analysis 对象；DMA 地址 `0x30000000`/`0x30000400`、总区域 `0x800` 的合同通过。构建后 `Get-Process` 未发现 UV4、armclang 或 armlink 残留进程。
- 官方资料复核：主代理重新读取 `https://github.com/lvgl/lvgl/releases/latest`，页面仍将 v9.5.0 标为 Latest；`https://docs.lvgl.io/9.5/integration/overview` 可读。猜测的 tick.html 入口返回 Internal Error，该页面内容未取得；后续改从官方导航与本地固定源码确认 API，不把猜测入口当作已读依据。
- 验证边界：本次首次通过当前 LVGL target 的严格零警告门禁，作为后续端口修改前的编译基线；不代表四 target 的本轮全量回归，也不证明 ST7789 电气连接、初始化/刷新时序、色序、tick/flush、真实显示或任何板级音频/FPGA 验证。W-109 的 9 警告与 W-110 的生成失败继续保留。
- 修改/发布范围：仅产生 MCU 构建输出与日志；未修改官方 LVGL 源码、FPGA、MPU/VM、Gitee 或硬件，未暂存、未提交、未推送。记录员仅追加本日志，检查点尚待主代理回传最终状态。
- 下一步：继续 Generic_DSP 统一命名与 LVGL 端口质量修复，修改后重新生成并做四 target 隔离/构建回归；FPGA native 工程及 PS LVGL 仍须另行验证。

### W-20260909-113：Generic_DSP 精确机械迁移与 W-112 对象事实勘误

- 时间：2026-09-09T10:04:24+08:00（阶段汇总追加时间）。
- W-112 勘误：记录员将 LVGL target 的 49 个 permissive DSP 对象错误记为“未编入”。主代理复核的实际结果是 **49 个 portable/permissive DSP 对象全部存在且通过检查，仅 14 个分析对象被排除**。W-112 原文保留，本条纠正其对象隔离结论；其 466 个 LVGL 源对象映射及 0 Error(s)/0 Warning(s) 结果不受此记录错误影响。
- 迁移命令：主代理使用 `apply_patch` 创建被忽略的 `tmp/rename_generic_dsp.py`，随后执行 `python tmp/rename_generic_dsp.py`。迁移前解析全部选定目标，确认均位于工作区且目的路径不存在，没有覆盖已有目标。
- 实际修改：机械更新 13 个选定文本；`App/NUEDC` 迁至 `App/Generic_DSP`，两个适配文件改为 `generic_dsp_selftest.c/h`，对应 DebugConfig 与 RTE 目录改为 Generic_DSP。三个 MCU 脚本、正式 `.uvprojx`/`.uvoptx`、`audio_app.h/c`、适配符号/宏、来源文档与 `THIRD_PARTY_NOTICES.md` 同步新名称。
- 核验证据：保留原始上游 URL 与固定 SHA；27 个 vendor C/H 文件逐文件 SHA-256 全部不变。对当前源码、脚本与正式 XML 的旧名称检索，仅在 `ORIGIN.md` 保留来源 URL 命中。旧名的忽略构建目录及历史日志继续保留，不作为当前有效 target。
- 未完成与范围：尚未运行改名后的工程再生成或构建，仍需修复 regenerate 残留的三 target `.uvoptx` 门禁并完成端口质量修复。未修改 FPGA、MPU/VM、Gitee 或硬件，未进行 Git 暂存、提交或推送；记录员仅追加本日志，未更新检查点。
- 验证边界与下一步：机械迁移、路径门禁及 vendor 哈希不变仅证明命名更新和源码保留；W-112 是改名前构建基线，不能外推为改名后工程、四 target 回归或物理硬件通过。完成剩余门禁/端口修改后，重新生成并执行四 target 构建与隔离验收。

### W-20260909-114：SPI 版本对齐、LVGL 端口加固与四目标门禁修改落盘

- 时间：2026-09-09T10:12:47+08:00（阶段汇总追加时间）。
- SPI 来源与失败保留：与本机 `STM32Cube_FW_H7_V1.13.0` 比较三个 SPI vendor 文件，两个头文件 SHA 一致；`git diff --no-index` 显示旧 `stm32h7xx_hal_spi.c` 存在 RX suspend、DMA abort、Tx ISR 等差异，主代理仅用 `Copy-Item` 更新该 C 文件，现 SHA-256=`4EEB322662C6FC531E42DA9EEF1474B6B32221F848DD7EC4FB0C4F0C408A013B`。首次只读 hash 命令因 `foreach` 后直接接管道触发 ParserError（empty pipe element），未执行；简化后重试通过。
- 生成脚本修改：`apply_patch` 将 `regenerate_cubemx.ps1` 的旧 `.uvoptx` 三目标断言改为四目标，并以 Hidden 启动 CubeMX；`postgenerate_keil.ps1` 清除 base 继承的全部 LVGL include，非 LVGL target 出现部分 LVGL include 也拒绝；临时 base RTE 清理增加绝对目录及 reparse 门禁。
- 板级参考合同：LVGL_UI 新增 `lvgl_board_config.h`，显式约定 240×240 ST7789、SPI1 PA5/PA7 TX-only、PC4～PC7、mode 3、PLL1Q/64（设计值 6.25 MHz），并检查计算时钟不超过 10 MHz；gap/invert 可配置，背光引脚只控制外部限流驱动使能。以上是参考设计合同，不是实测时钟、电气或接线验收。
- 端口与 UI 修改：`lv_init` 后注册 `HAL_GetTick`/`HAL_Delay`；初始化幂等；HAL 发送失败锁存故障、跳过后续传输并关闭背光，分别统计 flush 完成/中止且始终释放 LVGL buffer，拒绝奇数字节。新增 foreground pointer 弱 hook，默认不访问硬件且返回释放状态，未提供实际触摸驱动。slider 事件更新可观察的 cutoff/mix 值但不耦合音频；UI 故障独立，不覆盖 `g_audio_app_status`，Service 仅在初始化成功且无故障时运行。
- 构建脚本修改：`build_keil.ps1` 检测现有 UV4 时拒绝构建，不终止用户 IDE；先保存完整日志再判断通过与否，成功时默认只打印摘要，`-DetailedLog` 可输出全文。
- 已执行检查与资料：`git diff --check -- mcu` 通过，仅有 CRLF 提示；主代理已通过官方导航读到 LVGL 9.5 STM32/ST7789 说明正文，官方 LVGL 源码保持不改。
- 验证边界与下一步：这批修改已落盘，但尚未执行修改后的 Parser、工程再生成或构建；没有屏幕、触摸或硬件验证，W-112 clean build 不代表本批修改通过。下一步先运行 Parser/静态门禁，再生成并执行四 target 构建及隔离回归，成功/失败另行记录。
- 范围/发布：未修改 FPGA、MPU/VM、Gitee 或硬件，未暂存、未提交、未推送；记录员仅在物理末尾追加本日志，检查点未改。

### W-20260909-115：四脚本解析与正式四目标后处理门禁通过

- 时间：2026-09-09T10:13:47+08:00（阶段汇总追加时间）。
- 最小补强：主代理使用 `apply_patch` 明确 RTE reparse 判断括号；构建前逐项要求 LVGL include、`HAL_SPI_MODULE_ENABLED`、`LV_CONF_INCLUDE_SIMPLE` 按目标精确隔离，非 LVGL target 全无、LVGL target 全有。
- 实际验证：`build_keil.ps1`、`postgenerate_keil.ps1`、`regenerate_cubemx.ps1`、`verify_all.ps1` 四脚本均 `PARSER_OK`；直接执行 `& mcu/scripts/postgenerate_keil.ps1`，exit 0，输出已配置同步四 target、Generic DSP 和 LVGL v9.5。正式后处理内置的四 target、完整 manifest、`IncludeInBuild`、三项文件级 warning、DebugConfig/RTE 断言全部通过。
- 验证边界：本次直接后处理更新正式工程，不是 CubeMX 再生成；未运行 CubeMX，也尚未编译 W-113/W-114 改名和端口修改后的源码。Parser/结构门禁通过不代表 Keil 四目标构建、显示/触摸或真实硬件通过。
- 范围与下一步：未触碰 FPGA、MPU/VM、Gitee、硬件或 Git 发布；检查点未改。下一动作运行 `mcu/scripts/build_keil.ps1 -Target All`，逐 target 记录构建、对象隔离与失败证据。

### W-20260911-116：四目标构建证据补录与 CubeMX 再生成清除 SPI 驱动失败

- 时间：2026-09-11T06:32:52+08:00（迟到汇总追加时间；实际动作发生于 2026-09-09）。
- 前置与顺序：记录员此前已完整分块读完 1826 行工作记录；本次核对内容未变化、物理末尾仍为 W-20260909-115 后，仅在末尾追加。前轮记录员因额度失败没有追加 W-116，原历史全部保留；本条按主代理回传的已验证事实补录，不以当前恢复动作伪造原执行时间。
- 四目标构建：2026-09-09 10:14，主代理运行 `mcu/scripts/build_keil.ps1 -Target All`，会话 86210。前三个 target 均通过严格零错误/零警告门禁：SelfTest 用时 4 秒，Code=25408、RO-data=1128、RW-data=2068、ZI-data=41012；WM8960_Stream 用时 3 秒，Code=32608、RO-data=1128、RW-data=2068、ZI-data=41540；Generic_DSP 用时 8 秒，Code=32328、RO-data=1876、RW-data=2068、ZI-data=41012。
- LVGL 落盘日志证据与限制：上述会话最后失去可恢复状态，未取得最终 All 退出码。主代理于 2026-09-09 15:16 读取 `mcu/build/keil_lvgl.log`，确认日志时间为 10:14:25，LVGL_UI 为 0 Error(s)、0 Warning(s)、21 秒，Code=204724、RO-data=16512、RW-data=2084、ZI-data=163580。这可证明该 target 的 Keil 编译/链接摘要，不能替代缺失的 All exit 0，也不能提前认定 All 后续全部对象/隔离门禁通过。
- 正式再生成失败：2026-09-09 15:17 运行 `mcu/scripts/verify_all.ps1`，会话 49912 以 exit 1 结束；正式 CubeMX 再生成后的 `postgenerate_keil.ps1` 在第 230 行报告缺少 `Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c`。CubeMX 因 `.ioc` 未启用 SPI 清除了手动导入的 SPI vendor 驱动；失败发生于再生成/后处理阶段，verify_all 后续构建没有运行。
- 同阶段脚本修改：`verify_all.ps1` 摘要由 three 改为 four；`regenerate_cubemx.ps1` 补充 main 中 LVGL include、Init、Service 条件守卫断言。修改落盘不等于本次失败后的再生成或完整验证已通过。
- 来源比对失败保留：首次 LVGL SHA 比对错误地引用已经不存在的 Temp 解压目录，产生大量 missing；Temp 压缩包也已不存在。此结果只说明比对源路径失效，不能判为仓库 LVGL 源码被篡改，也不能报告源码 hash 一致或验证通过。
- 当前修复计划：主代理正在限定范围内恢复三个已核验的 STM32Cube_FW_H7_V1.13.0 官方 SPI 文件，不通过修改 `.ioc` 启用 SPI，继续保持其他 targets 与 LVGL/SPI 隔离；截至本条未收到修复与重验成功结果，不能记为已修好。
- 范围/验证边界：未进行硬件、屏幕、触摸、音频或 FPGA 验证，未恢复 MPU/VM/Gitee 工作，未暂存、未提交、未推送。编译摘要、文件恢复计划及再生成失败均不代表真实硬件通过。
- 下一步：取得精确 SPI 恢复和再生成保护修改结果后追加记录；重新执行正式 CubeMX 再生成及四 target 严格构建/对象隔离门禁，保留完整日志与最终退出码，再进入 FPGA native 工程和 PS LVGL 工作。

### W-20260911-117：正式 CubeMX 四目标全链验证与 LVGL 官方源码字节比对通过

- 时间：2026-09-11T06:35:36+08:00（阶段汇总追加时间）。
- SPI 再生成修复：主代理使用 `apply_patch` 修改 `mcu/scripts/regenerate_cubemx.ps1`，在 CubeMX 完成后、postgenerate 之前仅恢复官方 STM32Cube_FW_H7_V1.13.0 的 `stm32h7xx_hal_spi.c`、`stm32h7xx_hal_spi.h`、`stm32h7xx_hal_spi_ex.h`。源文件逐项按固定 SHA-256 核验；已有目标 hash 不同时拒绝覆盖。没有为此在 `.ioc` 启用 SPI，其他三个非 LVGL targets 的 SPI/LVGL 隔离仍严格保留。
- SPI 固定来源：上述 C、H、扩展 H 的 SHA-256 分别为 `4EEB322662C6FC531E42DA9EEF1474B6B32221F848DD7EC4FB0C4F0C408A013B`、`9EA983FD0E3148A3291B2AA56B1D01709374A3A3845D6B592B3D3356B71F6CB5`、`44D854991D118840677DA5CE86D88B1E555FE811808354522AD2F198AC4FA66C`；本次恢复沿用并核对这些已固定的官方版本。
- 正式命令与最终退出码：修复脚本 Parser errors=0；随后运行 `mcu/scripts/verify_all.ps1`，会话 84187 最终 exit 0。先输出三个 HAL 文件 hash 恢复通过，再输出正式 CubeMX 四目标 SAI/PLL3/DMA/RTE/LVGL 合同通过，最后明确输出 four-target verification passed；与 W-116 的会话状态丢失不同，本轮取得完整最终成功退出证据。
- 四目标编译/链接：四者均为 0 Error(s)、0 Warning(s)。SelfTest 用时 6 秒，Code=25408、RO-data=1128、RW-data=2068、ZI-data=41012；WM8960_Stream 用时 5 秒，Code=32608、RO-data=1128、RW-data=2068、ZI-data=41540；Generic_DSP 用时 10 秒，Code=32328、RO-data=1876、RW-data=2068、ZI-data=41012；LVGL_UI 用时 34 秒，Code=204724、RO-data=16512、RW-data=2084、ZI-data=163580。
- 对象与隔离门禁：49 个 portable/permissive DSP 对象在四目标中全部存在并通过检查；14 个 Generic_DSP 分析对象只进入 Generic_DSP target。LVGL_UI 完整 source→`.d`→实际 `.o` 映射数为 466，其他三个 target 的 LVGL 源对象映射数为 0；所有 target 的 D2 SRAM DMA map 合同通过。
- 上游来源复验：从 GitHub 官方 codeload 按固定完整提交 `85aa60d18b3d5e5588d7b247abf90198f07c8a63` 重新下载归档至忽略路径 `mcu/build/lvgl-85aa60d.tar.gz`，curl exit 0。新增只读验证脚本 `mcu/scripts/verify_lvgl_upstream.py`，不解压归档，只读取归档成员和本地文件并逐字节比较；运行会话 7008 exit 0，输出 `LVGL_UPSTREAM_OK`、commit 为上述固定提交、files=1136、mismatch=0。项目自有配置与端口不据此冒充上游原文件。
- 警告策略与历史边界：仍只使用三项已文档化的文件级 warning 选项，没有全局压制警告，也未修改官方 LVGL 源码。W-116 的临时来源路径失效、会话退出码缺失和 CubeMX 清除 SPI 驱动失败全部保留，本次成功不回写历史。
- 状态与发布边界：MCU 已通过当前四目标正式再生成、严格构建、对象隔离及官方 LVGL 源码比对；尚未进行显示、触摸、音频或任何真实硬件验证，未进行 FPGA 新构建，PS LVGL 未完成，MPU/VM/Gitee 继续停止。未暂存、未提交、未推送；随后仅同步 `docs/PROJECT_CHECKPOINT.json` 的当前状态、最新日志索引/hash 与未发布边界。
- 下一步：精确审阅 MCU 阶段差异与来源说明，按限定路径进行阶段提交/推送并独立记录完整 SHA 与远端核对；随后物化和验证 FPGA native 工程、Vivado 2018.3 构建及 XSDK，并继续隔离的 Zynq PS LVGL 工作。当前不提前标记上述发布或 FPGA 工作成功。
