# MPU library - 100ASK i.MX6ULL

目标环境：VMware Ubuntu 18.04，ARMHF 交叉编译，目标 Linux 通过 ALSA/ASoC 驱动板载 WM8960。

该目录提供可独立交叉编译的 DSP 静态库、ALSA 双工引擎、xrun 统计、实时线程配置和目标板应用。根目录纯 C 模块是算法集合；大型 Eurorack C++ 模块按内存与许可证单独启用。

## 构建

在 Ubuntu 18.04 虚拟机仓库根目录执行：

```sh
sh mpu/scripts/build_vm.sh
```

脚本先用主机 GCC/ALSA 构建 `audio_dsp_imx6ull` 并运行单元测试，再用 `arm-linux-gnueabihf-gcc` 构建 ARMv7-A hard-float 静态库。由于虚拟机未配置 ARMHF ALSA sysroot，默认交叉阶段只构建 DSP core；要交叉链接目标应用，设置 `AUD_MPU_SYSROOT` 指向目标 rootfs 并保持 `AUD_MPU_BUILD_ALSA=ON`。

`AUD_DSP_ENABLE_LGPL` 默认关闭；打开时静态库会改名为 `libaudio_dsp_portable_lgpl.a`，避免与 permissive 产物混淆。分发该产物前必须履行根目录 `LICENSE` 中的 LGPL-2.1-only 要求。

目标板运行示例：

```sh
./audio_dsp_imx6ull --capture hw:0,0 --playback hw:0,0 --rate 48000 --period 128 --format s32
```

默认链为：输入增益 -> DC blocker -> SVF -> overdrive -> 可交叉反馈的 stereo delay -> soft clip。ALSA 循环预分配全部应用缓冲，统计 capture/playback xrun；实时优先级失败时会警告并继续运行。
