#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mpu_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
build_dir=$mpu_root/build/host
jobs=${AUD_MPU_JOBS:-2}

command -v cmake >/dev/null 2>&1 || { echo "cmake is required" >&2; exit 1; }
command -v cc >/dev/null 2>&1 || { echo "a native C compiler is required" >&2; exit 1; }
mkdir -p "$build_dir"
cd "$build_dir"

cmake "$mpu_root" \
    -DCMAKE_BUILD_TYPE=Release \
    -DAUD_MPU_BUILD_RUNTIME=ON \
    -DAUD_MPU_BUILD_TESTS=ON \
    -DAUD_DSP_ENABLE_LGPL=OFF
cmake --build . -- -j"$jobs"
ctest --output-on-failure

./audio_dsp_imx6ull --config "$mpu_root/config/audio-dsp.conf" \
    --check-config >/dev/null
./audio_dsp_imx6ull --capture null --playback null --no-realtime \
    --allow-unlinked --rate 48000 --period 256 --periods 4 --format s16 --probe
./audio_dsp_imx6ull --capture null --playback null --no-realtime \
    --allow-unlinked --rate 48000 --period 256 --periods 4 --format s16 --run-seconds 1

echo "MPU_HOST_BUILD_OK=1"
echo "MPU_HOST_TESTS_OK=1"
echo "MPU_HOST_ALSA_NULL_SMOKE_OK=1"
