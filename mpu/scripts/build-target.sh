#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mpu_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
# shellcheck source=/dev/null
. "$mpu_root/board/100ask-imx6ull/profile.env"
sysroot=${1:-${AUD_MPU_SYSROOT:-}}
build_dir=$mpu_root/build/armv7a
stage_dir=$mpu_root/build/stage-armv7a
out_dir=$mpu_root/out
jobs=${AUD_MPU_JOBS:-2}

[ -n "$sysroot" ] || {
    echo "usage: build-target.sh /path/to/armhf-sysroot" >&2
    echo "The sysroot must contain ALSA headers and ARM libasound.so." >&2
    exit 2
}
[ -d "$sysroot" ] || { echo "sysroot not found: $sysroot" >&2; exit 1; }
command -v arm-linux-gnueabihf-gcc >/dev/null 2>&1 || {
    echo "arm-linux-gnueabihf-gcc is required" >&2
    exit 1
}
for command_name in arm-linux-gnueabihf-readelf awk grep sed sort tail head tr \
    cat find xargs sha256sum cmake tar; do
    command -v "$command_name" >/dev/null 2>&1 || {
        echo "$command_name is required" >&2
        exit 1
    }
done

alsa_include=
for candidate in "$sysroot/usr/include" "$sysroot/include"; do
    if [ -r "$candidate/alsa/asoundlib.h" ]; then
        alsa_include=$candidate
        break
    fi
done
[ -n "$alsa_include" ] || {
    echo "ALSA header not found below $sysroot" >&2
    exit 1
}

alsa_library=
for candidate in \
    "$sysroot/usr/lib/arm-linux-gnueabihf/libasound.so" \
    "$sysroot/lib/arm-linux-gnueabihf/libasound.so" \
    "$sysroot/usr/lib/libasound.so" \
    "$sysroot/lib/libasound.so"; do
    if [ -r "$candidate" ]; then
        alsa_library=$candidate
        break
    fi
done
[ -n "$alsa_library" ] || {
    echo "ARM development symlink libasound.so not found below $sysroot" >&2
    exit 1
}

alsa_version_header=$alsa_include/alsa/version.h
[ -r "$alsa_version_header" ] || {
    echo "ALSA version header not found: $alsa_version_header" >&2
    exit 1
}
alsa_version=$(sed -n \
    's/^[[:space:]]*#define[[:space:]][[:space:]]*SND_LIB_VERSION_STR[[:space:]][[:space:]]*"\([^"]*\)".*/\1/p' \
    "$alsa_version_header" | head -n 1)
[ "$alsa_version" = "$AUD_TARGET_ALSA_VERSION" ] || {
    echo "ALSA development ABI profile mismatch: expected $AUD_TARGET_ALSA_VERSION, found ${alsa_version:-unknown}" >&2
    exit 1
}

mkdir -p "$build_dir" "$out_dir"
cd "$build_dir"
AUD_MPU_SYSROOT=$sysroot export AUD_MPU_SYSROOT
cmake "$mpu_root" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$mpu_root/cmake/arm-linux-gnueabihf.cmake" \
    -DALSA_INCLUDE_DIR:PATH="$alsa_include" \
    -DALSA_LIBRARY:FILEPATH="$alsa_library" \
    -DAUD_MPU_BUILD_RUNTIME=ON \
    -DAUD_MPU_BUILD_TESTS=OFF \
    -DAUD_DSP_ENABLE_LGPL=OFF
cmake --build . -- -j"$jobs"

readelf=arm-linux-gnueabihf-readelf
elf_header=$($readelf -h audio_dsp_imx6ull)
printf '%s\n' "$elf_header" | grep -q 'Class:.*ELF32'
printf '%s\n' "$elf_header" | grep -q 'Data:.*little endian'
printf '%s\n' "$elf_header" | grep -q 'Machine:.*ARM'
printf '%s\n' "$elf_header" | grep -q 'Flags:.*hard-float ABI'

interpreter=$($readelf -l audio_dsp_imx6ull |
    sed -n 's/.*Requesting program interpreter: \([^]]*\).*/\1/p')
[ "$interpreter" = "$AUD_ELF_INTERPRETER" ] || {
    echo "ELF interpreter mismatch: expected $AUD_ELF_INTERPRETER, found ${interpreter:-none}" >&2
    exit 1
}

needed=$($readelf -d audio_dsp_imx6ull |
    sed -n 's/.*Shared library: \[\([^]]*\)\].*/\1/p' | LC_ALL=C sort -u)
printf '%s\n' "$needed" | grep -Fx 'libasound.so.2' >/dev/null || {
    echo "ELF does not depend on libasound.so.2" >&2
    exit 1
}
for library in $needed; do
    case " $AUD_ALLOWED_NEEDED " in
        *" $library "*) ;;
        *)
            echo "ELF dependency is outside the board profile allowlist: $library" >&2
            exit 1
            ;;
    esac
done

required_glibc=$($readelf --version-info audio_dsp_imx6ull |
    sed -n 's/.*Name: GLIBC_\([0-9][0-9.]*\).*/\1/p' |
    LC_ALL=C sort -Vu | tail -n 1)
[ -n "$required_glibc" ] || {
    echo "could not determine the ELF GLIBC requirement" >&2
    exit 1
}
highest_glibc=$(printf '%s\n%s\n' "$required_glibc" "$AUD_TARGET_GLIBC_MAX" |
    LC_ALL=C sort -Vu | tail -n 1)
[ "$highest_glibc" = "$AUD_TARGET_GLIBC_MAX" ] || {
    echo "ELF requires GLIBC_$required_glibc, board limit is GLIBC_$AUD_TARGET_GLIBC_MAX" >&2
    exit 1
}

cat > ELF_ABI.txt <<EOF
BOARD_PROFILE=$AUD_BOARD_PROFILE
ELF_CLASS=ELF32
ELF_MACHINE=ARM
ELF_FLOAT_ABI=hard
ELF_INTERPRETER=$interpreter
GLIBC_REQUIRED_MAX=$required_glibc
GLIBC_ALLOWED_MAX=$AUD_TARGET_GLIBC_MAX
ALSA_DEVELOPMENT_VERSION=$alsa_version
ALSA_TARGET_VERSION=$AUD_TARGET_ALSA_VERSION
NEEDED=$(printf '%s' "$needed" | tr '\n' ' ' | sed 's/[[:space:]]*$//')
EOF

case "$stage_dir" in
    "$mpu_root"/build/*) ;;
    *) echo "refusing unsafe staging path: $stage_dir" >&2; exit 1 ;;
esac
rm -rf -- "$stage_dir"
mkdir -p "$stage_dir"
DESTDIR=$stage_dir export DESTDIR
cmake -DCMAKE_INSTALL_COMPONENT=Runtime -P cmake_install.cmake

for required in \
    usr/bin/audio_dsp_imx6ull \
    usr/libexec/audio-dsp-imx6ull/board-check \
    usr/libexec/audio-dsp-imx6ull/uninstall \
    usr/share/audio-dsp-imx6ull/audio-dsp.conf.default \
    usr/share/audio-dsp-imx6ull/profile.env \
    usr/share/doc/audio-dsp-imx6ull/ELF_ABI.txt \
    lib/systemd/system/audio-dsp-imx6ull.service \
    etc/init.d/audio-dsp-imx6ull; do
    [ -f "$stage_dir/$required" ] && [ ! -L "$stage_dir/$required" ] || {
        echo "staged runtime file is missing or not regular: $required" >&2
        exit 1
    }
done
special=$(find "$stage_dir" ! -type d ! -type f -print)
[ -z "$special" ] || {
    echo "staged runtime contains a link or special file: $special" >&2
    exit 1
}

manifest_dir=$stage_dir/usr/share/audio-dsp-imx6ull
mkdir -p "$manifest_dir"
(
    cd "$stage_dir"
    find . -type f ! -name MANIFEST.sha256 -print0 | LC_ALL=C sort -z |
        xargs -0 sha256sum
) > "$manifest_dir/MANIFEST.sha256"
(
    cd "$stage_dir"
    sha256sum -c usr/share/audio-dsp-imx6ull/MANIFEST.sha256 >/dev/null
)

package=$out_dir/audio-dsp-imx6ull-armv7a.tar.gz
tar -C "$stage_dir" -czf "$package" .
echo "MPU_ARMV7A_RUNTIME_CROSS_BUILD_OK=1"
echo "MPU_ARMV7A_PACKAGE=$package"
