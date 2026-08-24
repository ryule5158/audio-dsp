#!/bin/sh
set -eu

name=audio-dsp-imx6ull
unit=$name.service
init_script=/etc/init.d/$name
config_dir=/etc/audio-dsp-imx6ull
config=$config_dir/audio-dsp.conf

purge=0
case "${1:-}" in
    "") ;;
    --purge) purge=1 ;;
    --help) echo "usage: uninstall-target.sh [--purge]"; exit 0 ;;
    *) echo "usage: uninstall-target.sh [--purge]" >&2; exit 2 ;;
esac
[ "$#" -le 1 ] || { echo "usage: uninstall-target.sh [--purge]" >&2; exit 2; }
[ "$(id -u)" -eq 0 ] || { echo "run as root" >&2; exit 1; }

if command -v systemctl >/dev/null 2>&1 && [ -d /run/systemd/system ]; then
    init_system=systemd
    systemctl disable --now "$unit" >/dev/null 2>&1 || true
else
    init_system=sysv
    if [ -x "$init_script" ]; then
        "$init_script" stop >/dev/null 2>&1 || true
    fi
fi

# Remove SysV registration even on a systemd image so a rootfs that changed
# init systems cannot retain a stale autostart entry.
if command -v update-rc.d >/dev/null 2>&1; then
    update-rc.d -f "$name" remove >/dev/null 2>&1 || true
fi
if command -v chkconfig >/dev/null 2>&1; then
    chkconfig "$name" off >/dev/null 2>&1 || true
    chkconfig --del "$name" >/dev/null 2>&1 || true
fi
rm -f -- \
    /etc/init.d/S99audio-dsp-imx6ull \
    /etc/rc0.d/K01audio-dsp-imx6ull \
    /etc/rc1.d/K01audio-dsp-imx6ull \
    /etc/rc2.d/S99audio-dsp-imx6ull \
    /etc/rc3.d/S99audio-dsp-imx6ull \
    /etc/rc4.d/S99audio-dsp-imx6ull \
    /etc/rc5.d/S99audio-dsp-imx6ull \
    /etc/rc6.d/K01audio-dsp-imx6ull

rm -f -- \
    /usr/bin/audio_dsp_imx6ull \
    /usr/libexec/audio-dsp-imx6ull/board-check \
    /usr/libexec/audio-dsp-imx6ull/uninstall \
    /lib/systemd/system/audio-dsp-imx6ull.service \
    /etc/init.d/audio-dsp-imx6ull \
    /usr/share/audio-dsp-imx6ull/profile.env \
    /usr/share/audio-dsp-imx6ull/audio-dsp.conf.default \
    /usr/share/audio-dsp-imx6ull/MANIFEST.sha256 \
    /usr/share/doc/audio-dsp-imx6ull/ELF_ABI.txt \
    /usr/share/doc/audio-dsp-imx6ull/LICENSE \
    /usr/share/doc/audio-dsp-imx6ull/THIRD_PARTY_NOTICES.md \
    /usr/share/doc/audio-dsp-imx6ull/LICENSES/MIT.txt \
    /usr/share/doc/audio-dsp-imx6ull/LICENSES/LGPL-2.1-only.txt \
    /usr/share/doc/audio-dsp-imx6ull/LICENSES/GPL-3.0-or-later.txt

rmdir /usr/share/doc/audio-dsp-imx6ull/LICENSES 2>/dev/null || true
rmdir /usr/share/doc/audio-dsp-imx6ull 2>/dev/null || true
rmdir /usr/libexec/audio-dsp-imx6ull 2>/dev/null || true
rmdir /usr/share/audio-dsp-imx6ull 2>/dev/null || true

if [ "$purge" -eq 1 ]; then
    rm -f -- "$config"
    rmdir "$config_dir" 2>/dev/null || true
fi

if command -v systemctl >/dev/null 2>&1 && [ -d /run/systemd/system ]; then
    systemctl daemon-reload
fi
echo "MPU_TARGET_INIT_SYSTEM=$init_system"
echo "MPU_TARGET_UNINSTALL_OK=1"
