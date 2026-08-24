#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mpu_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
repo_root=$(CDPATH= cd -- "$mpu_root/.." && pwd)
work=$(mktemp -d /tmp/audio-dsp-package-test.XXXXXX)
cleanup() { rm -rf -- "$work"; }
trap cleanup 0
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

stage=$work/stage
mkdir -p \
    "$stage/usr/bin" \
    "$stage/usr/libexec/audio-dsp-imx6ull" \
    "$stage/usr/share/audio-dsp-imx6ull" \
    "$stage/usr/share/doc/audio-dsp-imx6ull/LICENSES" \
    "$stage/lib/systemd/system" \
    "$stage/etc/init.d"

printf '#!/bin/sh\nexit 0\n' > "$stage/usr/bin/audio_dsp_imx6ull"
cp "$mpu_root/scripts/board-check.sh" \
    "$stage/usr/libexec/audio-dsp-imx6ull/board-check"
cp "$mpu_root/scripts/uninstall-target.sh" \
    "$stage/usr/libexec/audio-dsp-imx6ull/uninstall"
cp "$mpu_root/config/audio-dsp.conf" \
    "$stage/usr/share/audio-dsp-imx6ull/audio-dsp.conf.default"
cp "$mpu_root/board/100ask-imx6ull/profile.env" \
    "$stage/usr/share/audio-dsp-imx6ull/profile.env"
cp "$mpu_root/packaging/audio-dsp-imx6ull.service" \
    "$stage/lib/systemd/system/audio-dsp-imx6ull.service"
cp "$mpu_root/packaging/audio-dsp-imx6ull.init" \
    "$stage/etc/init.d/audio-dsp-imx6ull"
cp "$repo_root/LICENSE" "$stage/usr/share/doc/audio-dsp-imx6ull/LICENSE"
cp "$repo_root/THIRD_PARTY_NOTICES.md" \
    "$stage/usr/share/doc/audio-dsp-imx6ull/THIRD_PARTY_NOTICES.md"
cp "$repo_root/LICENSES/MIT.txt" \
    "$stage/usr/share/doc/audio-dsp-imx6ull/LICENSES/MIT.txt"
cp "$repo_root/LICENSES/LGPL-2.1-only.txt" \
    "$stage/usr/share/doc/audio-dsp-imx6ull/LICENSES/LGPL-2.1-only.txt"
cp "$repo_root/LICENSES/GPL-3.0-or-later.txt" \
    "$stage/usr/share/doc/audio-dsp-imx6ull/LICENSES/GPL-3.0-or-later.txt"

manifest=$stage/usr/share/audio-dsp-imx6ull/MANIFEST.sha256
(
    cd "$stage"
    find . -type f ! -name MANIFEST.sha256 -print0 | LC_ALL=C sort -z |
        xargs -0 sha256sum
) > "$manifest"

valid=$work/valid.tar.gz
tar -C "$stage" -czf "$valid" .
sh "$mpu_root/scripts/install-target.sh" "$valid" --verify-only |
    grep -Fx 'MPU_TARGET_PACKAGE_VERIFY_OK=1' >/dev/null

printf '\nchecksum-tamper\n' >> \
    "$stage/usr/share/audio-dsp-imx6ull/audio-dsp.conf.default"
tampered=$work/tampered.tar.gz
tar -C "$stage" -czf "$tampered" .
if sh "$mpu_root/scripts/install-target.sh" "$tampered" --verify-only \
    >/dev/null 2>&1; then
    echo "tampered package was accepted" >&2
    exit 1
fi

cp "$mpu_root/config/audio-dsp.conf" \
    "$stage/usr/share/audio-dsp-imx6ull/audio-dsp.conf.default"
printf 'unexpected\n' > "$stage/usr/share/audio-dsp-imx6ull/not-allowlisted"
unexpected=$work/not-allowlisted.tar.gz
tar -C "$stage" -czf "$unexpected" .
if sh "$mpu_root/scripts/install-target.sh" "$unexpected" --verify-only \
    >/dev/null 2>&1; then
    echo "package containing a non-allowlisted path was accepted" >&2
    exit 1
fi
rm -f -- "$stage/usr/share/audio-dsp-imx6ull/not-allowlisted"

rm -f -- "$stage/usr/bin/audio_dsp_imx6ull"
if ln -s /etc/passwd "$stage/usr/bin/audio_dsp_imx6ull" 2>/dev/null; then
    linked=$work/symlink.tar.gz
    tar -C "$stage" -czf "$linked" .
    if sh "$mpu_root/scripts/install-target.sh" "$linked" --verify-only \
        >/dev/null 2>&1; then
        echo "package containing a symlink was accepted" >&2
        exit 1
    fi
else
    echo "MPU_PACKAGE_SYMLINK_TEST_SKIPPED=host_cannot_create_symlink" >&2
fi

echo "MPU_PACKAGE_VALIDATION_TEST_OK=1"
