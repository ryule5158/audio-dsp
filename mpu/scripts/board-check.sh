#!/bin/sh
set -eu

config=/etc/audio-dsp-imx6ull/audio-dsp.conf
probe=0
quiet=0
profile=/usr/share/audio-dsp-imx6ull/profile.env

while [ "$#" -gt 0 ]; do
    case "$1" in
        --config)
            [ "$#" -ge 2 ] || { echo "--config requires FILE" >&2; exit 2; }
            config=$2
            shift 2
            ;;
        --probe) probe=1; shift ;;
        --quiet) quiet=1; shift ;;
        --help)
            echo "usage: board-check [--config FILE] [--probe] [--quiet]"
            exit 0
            ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done

say() { [ "$quiet" -eq 1 ] || echo "$*"; }
fail() { echo "BOARD_CHECK_FAIL: $*" >&2; exit 1; }
warn() { echo "BOARD_CHECK_WARN: $*" >&2; }

[ -r "$config" ] || fail "missing configuration: $config"
[ -r "$profile" ] || fail "missing board profile: $profile"
# shellcheck source=/dev/null
. "$profile"
case "$(uname -m)" in
    armv7l|armv7*) ;;
    *) fail "expected ARMv7 i.MX6ULL, found $(uname -m)" ;;
esac

[ -r /proc/asound/cards ] || fail "ALSA card list is unavailable"
grep -qi "$AUD_ALSA_CARD_ID" /proc/asound/cards || {
    cat /proc/asound/cards >&2
    fail "wm8960 card not registered"
}

if [ -r /proc/device-tree/sound/model ]; then
    model=$(tr -d '\000' < /proc/device-tree/sound/model)
    [ "$model" = "$AUD_SOUND_MODEL" ] || fail "unexpected DT sound model: $model"
else
    warn "/proc/device-tree/sound/model is unavailable"
fi

codec_found=0
address=$(printf '%04x' "$AUD_CODEC_I2C_ADDRESS")
for node in /sys/bus/i2c/devices/*-"$address"; do
    [ -e "$node" ] || continue
    codec_found=1
    break
done
[ "$codec_found" -eq 1 ] || warn "no bound I2C device at address $AUD_CODEC_I2C_ADDRESS"

grep -F "CARD=$AUD_ALSA_CARD_ID" "$config" >/dev/null ||
    fail "configuration ALSA card does not match profile $AUD_ALSA_CARD_ID"

command -v aplay >/dev/null 2>&1 || fail "alsa-utils aplay is missing"
command -v arecord >/dev/null 2>&1 || fail "alsa-utils arecord is missing"
aplay -l >/dev/null 2>&1 || fail "no playback PCM is available"
arecord -l >/dev/null 2>&1 || fail "no capture PCM is available"

binary=/usr/bin/audio_dsp_imx6ull
if [ "$probe" -eq 1 ]; then
    [ -x "$binary" ] || fail "runtime binary is missing: $binary"
    "$binary" --config "$config" --probe
fi

say "MPU_100ASK_BOARD_PROFILE_OK=1"
