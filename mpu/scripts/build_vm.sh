#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
sysroot=${1:-${AUD_MPU_SYSROOT:-}}

"$script_dir/build-host.sh"
[ -n "$sysroot" ] || {
    echo "host verification passed; full build still requires an ARMHF ALSA sysroot" >&2
    echo "usage: build_vm.sh /path/to/armhf-sysroot" >&2
    exit 2
}
"$script_dir/build-target.sh" "$sysroot"
echo "MPU_VM_FULL_BUILD_OK=1"
