#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
host_build="$repo_root/build/mpu-host"
arm_build="$repo_root/build/mpu-armv7a"

mkdir -p "$host_build" "$arm_build"

cd "$host_build"
cmake "$repo_root/mpu" \
  -DCMAKE_BUILD_TYPE=Release \
  -DAUD_MPU_BUILD_ALSA=ON \
  -DAUD_MPU_BUILD_TESTS=ON \
  -DAUD_DSP_ENABLE_LGPL=OFF
cmake --build . -- -j2
ctest --output-on-failure

cd "$arm_build"
cmake "$repo_root/mpu" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$repo_root/mpu/cmake/arm-linux-gnueabihf.cmake" \
  -DAUD_MPU_BUILD_ALSA=OFF \
  -DAUD_MPU_BUILD_TESTS=OFF \
  -DAUD_DSP_ENABLE_LGPL=OFF
cmake --build . -- -j2

file "$arm_build/libaudio_dsp_portable.a" "$arm_build/libaud_mpu_core.a"
printf '%s\n' "MPU_VM_HOST_TESTS_OK=1" "MPU_ARMV7A_CROSS_BUILD_OK=1"
