#!/bin/sh
set -eu

[ "$#" -ge 1 ] && [ "$#" -le 2 ] || {
    echo "usage: deploy.sh root@BOARD [PACKAGE.tar.gz]" >&2
    exit 2
}

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mpu_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
target=$1
package=${2:-$mpu_root/out/audio-dsp-imx6ull-armv7a.tar.gz}
[ -r "$package" ] || { echo "package not found: $package" >&2; exit 1; }
command -v ssh >/dev/null 2>&1 || { echo "ssh is required" >&2; exit 1; }
command -v scp >/dev/null 2>&1 || { echo "scp is required" >&2; exit 1; }

remote_dir=
cleanup() {
    [ -n "$remote_dir" ] || return 0
    ssh "$target" "rm -rf -- '$remote_dir'" >/dev/null 2>&1 || true
}
trap cleanup 0
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

# Create the upload directory on the target.  Restrictive ownership and mode
# prevent another target user from replacing the archive or installer.
remote_dir=$(ssh "$target" '
    set -eu
    [ "$(id -u)" -eq 0 ] || {
        echo "deployment requires a root SSH account" >&2
        exit 1
    }
    command -v mktemp >/dev/null 2>&1 || {
        echo "mktemp is required on the target" >&2
        exit 1
    }
    umask 077
    upload_dir=$(mktemp -d /tmp/audio-dsp-imx6ull-deploy.XXXXXX)
    chmod 0700 "$upload_dir"
    printf "%s\n" "$upload_dir"
')

case "$remote_dir" in
    /tmp/audio-dsp-imx6ull-deploy.*)
        suffix=${remote_dir#/tmp/audio-dsp-imx6ull-deploy.}
        case "$suffix" in
            ""|*[!ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789]*)
                echo "target returned an unsafe temporary path" >&2
                exit 1
                ;;
        esac
        ;;
    *)
        echo "target returned an unsafe temporary path" >&2
        exit 1
        ;;
esac

scp "$package" "$target:$remote_dir/package.tar.gz"
scp "$script_dir/install-target.sh" "$target:$remote_dir/install-target.sh"
ssh "$target" "chmod 0700 '$remote_dir/install-target.sh' && sh '$remote_dir/install-target.sh' '$remote_dir/package.tar.gz' --enable"
echo "MPU_REMOTE_DEPLOY_OK=1"
