# i.MX6ULL Linux 音频工程模板

这是可直接交叉编译、打包并部署到 100ASK i.MX6ULL Pro 的 ALSA 全双工效果器工程，不再只是 DSP 接口骨架。目标产物包含 ARMv7-A 应用、严格配置文件、板级自检、systemd/SysV 服务、事务安装/卸载工具和远程部署脚本。

## 已固定的板级事实

用户资料中的 `Linux-4.9.88/arch/arm/boot/dts/100ask_imx6ull-14x14.dts` 明确配置：

- 声卡模型 `wm8960-audio`，ALSA 预期 card ID 为 `wm8960audio`；
- WM8960 位于 I2C2 地址 `0x1a`；
- CPU DAI 为 SAI2，MCLK 为 12.288 MHz；
- BCLK/SYNC/TX/RX/MCLK 分别复用 JTAG_TDI、JTAG_TDO、JTAG_TRST_B、JTAG_TCK、JTAG_TMS；
- 内核配置启用了 FSL SAI、i.MX WM8960 machine driver 和 WM8960 codec driver。

这些事实同时固化在 `board/100ask-imx6ull/profile.env`，可用下列命令检查一份 BSP：

```sh
sh mpu/scripts/verify-bsp-profile.sh /path/to/Linux-4.9.88
```

板端 ABI 版本来自用户资料中的 `files/rootfs.ext4`，不是 Ubuntu 18.04 构建主机：该镜像 SHA-256 为 `a0a393b4e7bdd6f58314156e1ac4e4a533f8346c18aa82f883189d2bfc4a5921`，大小为 `734003200` 字节。镜像内的 Buildroot libc banner 为 2.30；libasound 字符串表中的版本为 1.2.1.2。可用 binutils `strings`、`sha256sum` 复核原始镜像：

```sh
sh mpu/scripts/verify-rootfs-profile.sh /path/to/files/rootfs.ext4
```

底板原理图第 9 页显示耳麦座 J10 的 MIC 经 C108 进入 WM8960 `LINPUT1`，但厂家 DTS 将 `Mic Jack` 路由到 `LINPUT2/3`。若使用 J10 耳麦输入且板上实测无采集信号，可在自己的 BSP 分支应用 `board/100ask-imx6ull/fix-headset-mic-routing.patch`，重新编译 DTB 后再测；不要在没有板测时直接替换量产 DTB。板载 ECM 麦克风的 `RINPUT1/2` 路由不受此补丁影响。

## Ubuntu 18.04 构建环境

原生构建依赖：

```sh
sudo apt-get update
sudo apt-get install build-essential cmake libasound2-dev alsa-utils
```

完整 ARMHF 交叉链接还需要交叉 libc 和 ARM 版 ALSA 开发库：

```sh
sudo dpkg --add-architecture armhf
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabihf libc6-dev-armhf-cross libasound2-dev:armhf
```

目标 sysroot 的 ALSA 开发头版本必须与上述板端 rootfs profile 一致；不能只因 Ubuntu 中存在一个 ARM `libasound.so` 就假定与板端 rootfs 兼容。Ubuntu 18.04 只是构建主机，它自身的 glibc/ALSA 版本不是门禁来源。如果 Ubuntu multiarch 根目录 `/` 与板端版本不同，应使用从目标 Buildroot SDK/rootfs 导出的 sysroot。所有构建产物只写入 `mpu/build` 和 `mpu/out`。

仅做主机编译、4 个 C 单元测试、包安全回归测试、配置检查和 ALSA `null` 双工烟雾测试：

```sh
sh mpu/scripts/build-host.sh
```

执行主机验证、交叉链接完整 ARM 应用并生成部署包：

```sh
sh mpu/scripts/build_vm.sh /
```

也可把参数换成独立 ARMHF sysroot。它必须包含 `usr/include/alsa/asoundlib.h` 和 ARM `libasound.so` 开发链接。成功包位于：

```text
mpu/out/audio-dsp-imx6ull-armv7a.tar.gz
```

`AUD_DSP_ENABLE_LGPL` 默认关闭；显式开启后静态库改名为 `libaudio_dsp_portable_lgpl.a`，分发时遵守仓库根许可证说明。

交叉构建会对 `profile.env` 执行硬门禁：ELF32/ARM little-endian hard-float、解释器 `/lib/ld-linux-armhf.so.3`、动态依赖 allowlist、最高 GLIBC 符号版本和 ALSA 开发头版本。结果写入包内 `usr/share/doc/audio-dsp-imx6ull/ELF_ABI.txt`；任一不匹配都会停止打包。内部 `MANIFEST.sha256` 必须覆盖除自身外的每个普通文件。

## 部署与运行

目标 rootfs 必须提供 ARMHF `libasound.so.2`；板级自检还需要 `alsa-utils` 的 `aplay`/`arecord`。安装脚本使用 POSIX shell 及常见的 `tar`、`mktemp`、`sha256sum`、`awk`、`find`、`sort`、`cmp` 等工具，100ASK 镜像若裁剪了其中任一工具，应先在自己的 rootfs 构建系统中启用。

100ASK 系统一般可使用 root SSH。脚本不保存密码，也不会绕过 host-key 检查：

```sh
sh mpu/scripts/deploy.sh root@BOARD_IP
```

部署端先在板上以 `mktemp` 创建 root 所有、`0700` 的上传目录。安装器只接受显式 allowlist 中的相对路径，拒绝 `..`、绝对路径、重复条目、符号/硬链接、设备、FIFO 等特殊条目；归档只解压一次，逐文件 SHA-256 验证后从同一 root-only staging 安装，避免二次解压的 TOCTOU。安装前会停止旧实例，失败时回滚旧文件及原先的启用/运行状态；已存在的 `/etc/audio-dsp-imx6ull/audio-dsp.conf` 始终保留。也可以把包与 `scripts/install-target.sh` 复制到板上后手工执行：

```sh
sh install-target.sh audio-dsp-imx6ull-armv7a.tar.gz --enable
```

不写入 rootfs、仅执行归档结构/类型/manifest 校验：

```sh
sh install-target.sh audio-dsp-imx6ull-armv7a.tar.gz --verify-only
```

`--enable` 会根据实际 init 系统启用并启动服务：systemd 使用 unit；100ASK BusyBox/SysV 使用 init script，存在 `update-rc.d`/`chkconfig` 时调用它，否则建立明确的 `S99`/runlevel 链接。板上诊断：

```sh
/usr/libexec/audio-dsp-imx6ull/board-check --probe
systemctl status audio-dsp-imx6ull.service
journalctl -u audio-dsp-imx6ull.service -f
# BusyBox/SysV rootfs：
/etc/init.d/audio-dsp-imx6ull status
```

卸载会对 systemd 与 SysV 做对称的停止、禁用和注册清理；默认保留用户配置，增加 `--purge` 才删除配置：

```sh
/usr/libexec/audio-dsp-imx6ull/uninstall
/usr/libexec/audio-dsp-imx6ull/uninstall --purge
```

## 音频配置与开发入口

默认是低延迟但保守的 `48 kHz / S16_LE / 256 frames / 4 periods`。完整参数在 `config/audio-dsp.conf`；配置解析器会拒绝未知字段、超范围参数和非有限浮点数，服务启动前还会实际打开采集与播放 PCM。

默认处理链：输入增益 -> DC blocker -> SVF 低通混合 -> overdrive -> stereo/cross-feedback delay -> soft clip。`drive=0` 是严格旁路，非零值同时控制过载强度及 dry/wet 交叉淡化，不会再让默认链静音。要开发新的效果器，保持 ALSA 与 PCM 层不变，在 `AudMpuFxChain_Process()` 中替换或扩展处理链，并给 `tests/` 增加离线向量测试。

直接调试应用：

```sh
audio_dsp_imx6ull --config /etc/audio-dsp-imx6ull/audio-dsp.conf --check-config
audio_dsp_imx6ull --config /etc/audio-dsp-imx6ull/audio-dsp.conf --probe
audio_dsp_imx6ull --config /etc/audio-dsp-imx6ull/audio-dsp.conf --run-seconds 30
```

工程不会自动修改 WM8960 mixer 增益，因为 J10 耳麦、板载 ECM、耳机和外接扬声器所需路由与安全音量不同。上板先用 `amixer controls`/`alsamixer` 确认采集源并从低音量开始，随后再保存经过实测的 `alsactl` 状态。

## 完成边界

构建成功只证明源码、工具链、ARM ELF 和部署包一致。最终硬件验收仍应记录：实际 DTB 名称、`aplay -l`/`arecord -l`、WM8960 mixer 状态、持续运行 xrun 数、端到端延迟、耳机/扬声器电平、噪声和削顶。没有这些数据时，不应把“可部署工程”写成“已通过整机音质测试”。
