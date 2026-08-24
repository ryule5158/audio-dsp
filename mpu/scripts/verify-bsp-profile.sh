#!/bin/sh
set -eu

[ "$#" -eq 1 ] || {
    echo "usage: verify-bsp-profile.sh /path/to/Linux-4.9.88" >&2
    exit 2
}

bsp=$1
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
# shellcheck source=/dev/null
. "$script_dir/../board/100ask-imx6ull/profile.env"
dts=$bsp/arch/arm/boot/dts/$AUD_DTS_BASENAME
defconfig=$bsp/arch/arm/configs/100ask_imx6ull_defconfig
[ -r "$dts" ] || { echo "missing $dts" >&2; exit 1; }
[ -r "$defconfig" ] || { echo "missing $defconfig" >&2; exit 1; }

require_dts() {
    grep -F "$1" "$dts" >/dev/null || {
        echo "BSP profile mismatch, missing: $1" >&2
        exit 1
    }
}

require_dts "model = \"$AUD_SOUND_MODEL\";"
require_dts "cpu-dai = <&$AUD_CPU_DAI>;"
require_dts "codec: wm8960@$AUD_CODEC_DTS_UNIT_ADDRESS {"
require_dts "compatible = \"$AUD_CODEC_COMPATIBLE\";"
require_dts "reg = <$AUD_CODEC_I2C_ADDRESS>;"
require_dts "$AUD_SAI2_BCLK_PAD"
require_dts "$AUD_SAI2_SYNC_PAD"
require_dts "$AUD_SAI2_TX_PAD"
require_dts "$AUD_SAI2_RX_PAD"
require_dts "$AUD_SAI2_MCLK_PAD"
require_dts "assigned-clock-rates = <0>, <$AUD_MCLK_HZ>;"
grep -F 'CONFIG_SND_SOC_IMX_WM8960=y' "$defconfig" >/dev/null
grep -F 'CONFIG_SND_SOC_WM8960=y' "$defconfig" >/dev/null
grep -F 'CONFIG_SND_SOC_FSL_SAI=y' "$defconfig" >/dev/null

if grep -F '"LINPUT2", "Mic Jack"' "$dts" >/dev/null &&
   grep -F '"LINPUT3", "Mic Jack"' "$dts" >/dev/null; then
    echo "BSP_PROFILE_WARN=headset_mic_route_differs_from_schematic" >&2
fi

echo "MPU_100ASK_BSP_PROFILE_OK=1"
