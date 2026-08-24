#!/bin/sh
set -eu

[ "$#" -eq 1 ] || {
    echo "usage: verify-rootfs-profile.sh /path/to/rootfs.ext4" >&2
    exit 2
}

image=$1
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
# shellcheck source=/dev/null
. "$script_dir/../board/100ask-imx6ull/profile.env"
strings_command=${AUD_MPU_STRINGS:-strings}

[ -f "$image" ] && [ ! -L "$image" ] || {
    echo "rootfs image must be a regular file: $image" >&2
    exit 1
}
for command_name in sha256sum wc awk; do
    command -v "$command_name" >/dev/null 2>&1 || {
        echo "$command_name is required" >&2
        exit 1
    }
done
command -v "$strings_command" >/dev/null 2>&1 || {
    echo "strings tool not found: $strings_command" >&2
    exit 1
}

size=$(wc -c < "$image" | awk '{ print $1 }')
[ "$size" = "$AUD_TARGET_ROOTFS_SIZE" ] || {
    echo "rootfs size mismatch: expected $AUD_TARGET_ROOTFS_SIZE, found $size" >&2
    exit 1
}
hash=$(sha256sum "$image" | awk '{ print $1 }')
[ "$hash" = "$AUD_TARGET_ROOTFS_SHA256" ] || {
    echo "rootfs SHA-256 mismatch: $hash" >&2
    exit 1
}

# The ALSA version must occur in the libasound string-table neighbourhood,
# shortly after its diagnostic format string, rather than merely somewhere in
# the filesystem image.  awk consumes the complete stream so strings does not
# receive SIGPIPE and hide an I/O failure.
"$strings_command" -a -n 7 "$image" | awk \
    -v glibc="$AUD_TARGET_GLIBC_MAX" \
    -v alsa="$AUD_TARGET_ALSA_VERSION" \
    -v interpreter="$AUD_ELF_INTERPRETER" '
    index($0, "GNU C Library (Buildroot) stable release version " glibc ".") {
        found_glibc = 1
    }
    $0 == interpreter { found_interpreter = 1 }
    index($0, "ALSA lib ") == 1 { alsa_window = 16 }
    alsa_window > 0 {
        if ($0 == alsa) found_alsa = 1
        --alsa_window
    }
    END {
        if (!found_glibc) print "missing target glibc banner" > "/dev/stderr"
        if (!found_alsa) print "missing libasound version context" > "/dev/stderr"
        if (!found_interpreter) print "missing ARMHF interpreter" > "/dev/stderr"
        exit !(found_glibc && found_alsa && found_interpreter)
    }
' || {
    echo "rootfs ABI profile mismatch" >&2
    exit 1
}

echo "MPU_100ASK_ROOTFS_PROFILE_OK=1"
