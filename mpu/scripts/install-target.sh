#!/bin/sh
set -eu

name=audio-dsp-imx6ull
unit=$name.service
init_script=/etc/init.d/$name
config_dir=/etc/audio-dsp-imx6ull
config=$config_dir/audio-dsp.conf

fail() {
    echo "MPU_TARGET_INSTALL_FAIL: $*" >&2
    exit 1
}

enable=0
verify_only=0
archive=
while [ "$#" -gt 0 ]; do
    case "$1" in
        --enable) enable=1; shift ;;
        --verify-only) verify_only=1; shift ;;
        --help)
            echo "usage: install-target.sh PACKAGE.tar.gz [--enable] [--verify-only]"
            exit 0
            ;;
        -*) echo "unknown argument: $1" >&2; exit 2 ;;
        *)
            [ -z "$archive" ] || { echo "only one package is accepted" >&2; exit 2; }
            archive=$1
            shift
            ;;
    esac
done

[ -n "$archive" ] || { echo "package is required" >&2; exit 2; }
[ "$verify_only" -eq 0 ] || [ "$enable" -eq 0 ] ||
    fail "--enable and --verify-only cannot be combined"
[ "$verify_only" -eq 1 ] || [ "$(id -u)" -eq 0 ] || fail "run as root"
[ -r "$archive" ] || fail "package not found: $archive"
[ -f "$archive" ] && [ ! -L "$archive" ] ||
    fail "package must be a regular file, not a link or special object"
original_archive=$archive
for command_name in tar mktemp sha256sum find sort uniq awk cmp cp chmod chown \
    mv rm mkdir grep sed rmdir ln dirname head tr id; do
    command -v "$command_name" >/dev/null 2>&1 ||
        fail "$command_name is required on the target"
done

umask 077
work=$(mktemp -d /tmp/audio-dsp-imx6ull-install.XXXXXX) ||
    fail "could not create a temporary directory"
chmod 0700 "$work"
trap 'rm -rf -- "$work"' 0
payload=$work/payload
backup=$work/backup
entries=$work/archive.entries
types=$work/archive.types
actual_files=$work/actual.files
manifest_files=$work/manifest.files
installed_destinations=$work/installed.destinations
backup_destinations=$work/backup.destinations
temporary_destinations=$work/temporary.destinations
mkdir -m 0700 "$payload" "$backup"
: > "$installed_destinations"
: > "$backup_destinations"
: > "$temporary_destinations"

manager=none
was_enabled=0
was_active=0
transaction_started=0
config_created=0
committed=0

sysv_is_enabled() {
    for link in \
        /etc/init.d/S99audio-dsp-imx6ull \
        /etc/rc2.d/S99audio-dsp-imx6ull \
        /etc/rc3.d/S99audio-dsp-imx6ull \
        /etc/rc4.d/S99audio-dsp-imx6ull \
        /etc/rc5.d/S99audio-dsp-imx6ull; do
        [ -L "$link" ] && return 0
    done
    return 1
}

sysv_stop() {
    [ -x "$init_script" ] || return 0
    "$init_script" stop >/dev/null 2>&1 || true
}

sysv_start() {
    [ -x "$init_script" ] || return 1
    "$init_script" start
}

sysv_disable() {
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
}

sysv_enable() {
    if command -v update-rc.d >/dev/null 2>&1; then
        update-rc.d "$name" defaults >/dev/null
        return
    fi
    if command -v chkconfig >/dev/null 2>&1; then
        chkconfig --add "$name"
        chkconfig "$name" on
        return
    fi

    # Minimal BusyBox rootfs images often execute /etc/init.d/S??* directly.
    # Also create conventional runlevel links when those directories exist.
    rm -f -- /etc/init.d/S99audio-dsp-imx6ull
    ln -s "$name" /etc/init.d/S99audio-dsp-imx6ull
    for runlevel in 2 3 4 5; do
        directory=/etc/rc${runlevel}.d
        [ -d "$directory" ] || continue
        rm -f -- "$directory/S99audio-dsp-imx6ull"
        ln -s "../init.d/$name" "$directory/S99audio-dsp-imx6ull"
    done
    for runlevel in 0 1 6; do
        directory=/etc/rc${runlevel}.d
        [ -d "$directory" ] || continue
        rm -f -- "$directory/K01audio-dsp-imx6ull"
        ln -s "../init.d/$name" "$directory/K01audio-dsp-imx6ull"
    done
}

stop_current_service() {
    case "$manager" in
        systemd) systemctl stop "$unit" >/dev/null 2>&1 || true ;;
        sysv) sysv_stop ;;
    esac
}

disable_current_service() {
    case "$manager" in
        systemd) systemctl disable "$unit" >/dev/null 2>&1 || true ;;
        sysv) sysv_disable ;;
    esac
}

restore_file() {
    destination=$1
    source_file=$backup$destination
    temporary=$destination.audio-dsp-rollback.$$
    rm -f -- "$temporary"
    cp -p "$source_file" "$temporary"
    mv -f "$temporary" "$destination"
}

rollback() {
    set +e
    echo "installation failed; rolling back the previous runtime" >&2
    stop_current_service
    disable_current_service

    while IFS= read -r temporary; do
        [ -n "$temporary" ] || continue
        rm -f -- "$temporary"
    done < "$temporary_destinations"

    while IFS= read -r destination; do
        [ -n "$destination" ] || continue
        rm -f -- "$destination"
    done < "$installed_destinations"
    while IFS= read -r destination; do
        [ -n "$destination" ] || continue
        restore_file "$destination"
    done < "$backup_destinations"

    if [ "$config_created" -eq 1 ]; then
        rm -f -- "$config"
    fi
    rmdir "$config_dir" 2>/dev/null || true
    rmdir /usr/share/doc/audio-dsp-imx6ull/LICENSES 2>/dev/null || true
    rmdir /usr/share/doc/audio-dsp-imx6ull 2>/dev/null || true
    rmdir /usr/libexec/audio-dsp-imx6ull 2>/dev/null || true
    rmdir /usr/share/audio-dsp-imx6ull 2>/dev/null || true

    case "$manager" in
        systemd)
            systemctl daemon-reload >/dev/null 2>&1 || true
            [ "$was_enabled" -eq 0 ] || systemctl enable "$unit" >/dev/null 2>&1 || true
            [ "$was_active" -eq 0 ] || systemctl start "$unit" >/dev/null 2>&1 || true
            ;;
        sysv)
            [ "$was_enabled" -eq 0 ] || sysv_enable >/dev/null 2>&1 || true
            [ "$was_active" -eq 0 ] || sysv_start >/dev/null 2>&1 || true
            ;;
    esac
}

finish() {
    status=$?
    trap - 0 HUP INT TERM
    if [ "$transaction_started" -eq 1 ] && [ "$committed" -eq 0 ]; then
        rollback
    fi
    rm -rf -- "$work"
    exit "$status"
}
trap finish 0
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

# Snapshot a manually supplied package into the root-only work directory before
# opening it with tar.  All structural checks and extraction therefore see the
# same immutable-by-unprivileged-users byte stream.
archive=$work/package.tar.gz
cp "$original_archive" "$archive"
chmod 0600 "$archive"

allow_archive_entry() {
    case "$1" in
        ./|\
        ./usr|./usr/|\
        ./usr/bin|./usr/bin/|\
        ./usr/bin/audio_dsp_imx6ull|\
        ./usr/libexec|./usr/libexec/|\
        ./usr/libexec/audio-dsp-imx6ull|./usr/libexec/audio-dsp-imx6ull/|\
        ./usr/libexec/audio-dsp-imx6ull/board-check|\
        ./usr/libexec/audio-dsp-imx6ull/uninstall|\
        ./usr/share|./usr/share/|\
        ./usr/share/audio-dsp-imx6ull|./usr/share/audio-dsp-imx6ull/|\
        ./usr/share/audio-dsp-imx6ull/audio-dsp.conf.default|\
        ./usr/share/audio-dsp-imx6ull/profile.env|\
        ./usr/share/audio-dsp-imx6ull/MANIFEST.sha256|\
        ./usr/share/doc|./usr/share/doc/|\
        ./usr/share/doc/audio-dsp-imx6ull|./usr/share/doc/audio-dsp-imx6ull/|\
        ./usr/share/doc/audio-dsp-imx6ull/LICENSE|\
        ./usr/share/doc/audio-dsp-imx6ull/THIRD_PARTY_NOTICES.md|\
        ./usr/share/doc/audio-dsp-imx6ull/ELF_ABI.txt|\
        ./usr/share/doc/audio-dsp-imx6ull/LICENSES|./usr/share/doc/audio-dsp-imx6ull/LICENSES/|\
        ./usr/share/doc/audio-dsp-imx6ull/LICENSES/MIT.txt|\
        ./usr/share/doc/audio-dsp-imx6ull/LICENSES/LGPL-2.1-only.txt|\
        ./usr/share/doc/audio-dsp-imx6ull/LICENSES/GPL-3.0-or-later.txt|\
        ./lib|./lib/|\
        ./lib/systemd|./lib/systemd/|\
        ./lib/systemd/system|./lib/systemd/system/|\
        ./lib/systemd/system/audio-dsp-imx6ull.service|\
        ./etc|./etc/|\
        ./etc/init.d|./etc/init.d/|\
        ./etc/init.d/audio-dsp-imx6ull)
            return 0
            ;;
        *) return 1 ;;
    esac
}

tar -tzf "$archive" > "$entries" || fail "cannot list package"
[ -s "$entries" ] || fail "package is empty"
while IFS= read -r entry; do
    case "$entry" in
        /*|..|../*|*/..|*/../*|*\\*) fail "unsafe package path: $entry" ;;
    esac
    allow_archive_entry "$entry" || fail "package path is not allowlisted: $entry"
done < "$entries"

LC_ALL=C sort "$entries" | uniq -d > "$work/duplicate.entries"
[ ! -s "$work/duplicate.entries" ] || fail "package contains duplicate entries"

for required in \
    ./usr/bin/audio_dsp_imx6ull \
    ./usr/libexec/audio-dsp-imx6ull/board-check \
    ./usr/libexec/audio-dsp-imx6ull/uninstall \
    ./usr/share/audio-dsp-imx6ull/audio-dsp.conf.default \
    ./usr/share/audio-dsp-imx6ull/profile.env \
    ./usr/share/audio-dsp-imx6ull/MANIFEST.sha256 \
    ./lib/systemd/system/audio-dsp-imx6ull.service \
    ./etc/init.d/audio-dsp-imx6ull \
    ./usr/share/doc/audio-dsp-imx6ull/LICENSE \
    ./usr/share/doc/audio-dsp-imx6ull/THIRD_PARTY_NOTICES.md; do
    grep -F -x "$required" "$entries" >/dev/null 2>&1 ||
        fail "required package entry is missing: $required"
done

# tar verbose mode exposes the archive member type in column one.  Only plain
# files and directories are accepted: links, devices, FIFOs and sockets fail.
tar -tvzf "$archive" > "$types" || fail "cannot inspect package member types"
while IFS= read -r member; do
    type=${member%"${member#?}"}
    case "$type" in
        -|d) ;;
        *) fail "package contains a link or special member (type $type)" ;;
    esac
done < "$types"

# Extract exactly once.  Everything installed below is copied from this
# verified, root-only staging tree; the archive is never reopened for install.
tar -xzf "$archive" -C "$payload" || fail "cannot extract package"
manifest=$payload/usr/share/audio-dsp-imx6ull/MANIFEST.sha256
[ -f "$manifest" ] && [ ! -L "$manifest" ] || fail "package manifest is missing"

(
    cd "$payload"
    find . -type f ! -path ./usr/share/audio-dsp-imx6ull/MANIFEST.sha256 |
        LC_ALL=C sort
) > "$actual_files"

awk '
    {
        path = $2
        sub(/^\*/, "", path)
    }
    NF != 2 || length($1) != 64 || $1 !~ /^[0123456789abcdefABCDEF]+$/ || path !~ /^\.\// {
        exit 1
    }
    { print path }
' "$manifest" > "$work/manifest.unsorted" || fail "manifest syntax is invalid"
LC_ALL=C sort "$work/manifest.unsorted" > "$manifest_files"
[ -s "$manifest_files" ] || fail "manifest has no payload entries"
LC_ALL=C sort "$manifest_files" | uniq -d > "$work/duplicate.manifest"
[ ! -s "$work/duplicate.manifest" ] || fail "manifest contains duplicate paths"
cmp "$actual_files" "$manifest_files" >/dev/null 2>&1 ||
    fail "manifest does not cover exactly the packaged regular files"
(
    cd "$payload"
    sha256sum -c usr/share/audio-dsp-imx6ull/MANIFEST.sha256 >/dev/null
) || fail "package checksum verification failed"

# Refuse any type confusion that the archive listing or extractor may have
# hidden.  The generated package contains only directories and regular files.
while IFS= read -r extracted; do
    [ -d "$extracted" ] || [ -f "$extracted" ] ||
        fail "staging contains a non-regular object: $extracted"
    [ ! -L "$extracted" ] || fail "staging contains a symbolic link: $extracted"
done <<EOF
$(find "$payload" -print)
EOF

if [ "$verify_only" -eq 1 ]; then
    echo "MPU_TARGET_PACKAGE_VERIFY_OK=1"
    exit 0
fi

if command -v systemctl >/dev/null 2>&1 && [ -d /run/systemd/system ]; then
    manager=systemd
    systemctl is-enabled "$unit" >/dev/null 2>&1 && was_enabled=1 || true
    systemctl is-active "$unit" >/dev/null 2>&1 && was_active=1 || true
else
    manager=sysv
    sysv_is_enabled && was_enabled=1 || true
    [ -x "$init_script" ] && "$init_script" status >/dev/null 2>&1 && was_active=1 || true
fi

transaction_started=1
stop_current_service
case "$manager" in
    systemd)
        systemctl is-active "$unit" >/dev/null 2>&1 &&
            fail "could not stop the existing systemd service" || true
        ;;
    sysv)
        [ -x "$init_script" ] && "$init_script" status >/dev/null 2>&1 &&
            fail "could not stop the existing SysV service" || true
        ;;
esac

ensure_directory() {
    directory=$1
    mode=$2
    if [ -L "$directory" ]; then
        fail "installation directory must not be a symlink: $directory"
    fi
    if [ -e "$directory" ]; then
        [ -d "$directory" ] || fail "installation path is not a directory: $directory"
    else
        mkdir -p "$directory"
        chmod "$mode" "$directory"
        chown 0:0 "$directory"
    fi
}

ensure_directory /usr/libexec/audio-dsp-imx6ull 0755
ensure_directory /usr/share/audio-dsp-imx6ull 0755
ensure_directory /usr/share/doc/audio-dsp-imx6ull 0755
ensure_directory /usr/share/doc/audio-dsp-imx6ull/LICENSES 0755
ensure_directory /lib/systemd/system 0755
ensure_directory /etc/init.d 0755

# Record the old files before writing any new payload.  Existing destination
# symlinks and special files are rejected rather than followed.
while IFS= read -r relative; do
    [ -n "$relative" ] || continue
    destination=/${relative#./}
    if [ -L "$destination" ]; then
        fail "destination must not be a symlink: $destination"
    fi
    if [ -e "$destination" ]; then
        [ -f "$destination" ] || fail "destination is not a regular file: $destination"
        backup_file=$backup$destination
        mkdir -p "$(dirname -- "$backup_file")"
        cp -p "$destination" "$backup_file"
        printf '%s\n' "$destination" >> "$backup_destinations"
    fi
done <<EOF
$(find "$payload" -type f -print | sed "s|^$payload/|./|" | LC_ALL=C sort)
EOF

while IFS= read -r relative; do
    [ -n "$relative" ] || continue
    source_file=$payload/${relative#./}
    destination=/${relative#./}
    temporary=$destination.audio-dsp-install.$$
    printf '%s\n' "$temporary" >> "$temporary_destinations"
    case "$destination" in
        /usr/bin/audio_dsp_imx6ull|/usr/libexec/audio-dsp-imx6ull/*|/etc/init.d/audio-dsp-imx6ull)
            mode=0755
            ;;
        *) mode=0644 ;;
    esac
    rm -f -- "$temporary"
    cp "$source_file" "$temporary"
    chmod "$mode" "$temporary"
    chown 0:0 "$temporary"
    mv -f "$temporary" "$destination"
    printf '%s\n' "$destination" >> "$installed_destinations"
done <<EOF
$(find "$payload" -type f -print | sed "s|^$payload/|./|" | LC_ALL=C sort)
EOF

ensure_directory "$config_dir" 0755
if [ -L "$config" ]; then
    fail "configuration must not be a symlink: $config"
fi
if [ -e "$config" ]; then
    [ -f "$config" ] || fail "configuration is not a regular file: $config"
else
    temporary=$config.audio-dsp-install.$$
    printf '%s\n' "$temporary" >> "$temporary_destinations"
    cp /usr/share/audio-dsp-imx6ull/audio-dsp.conf.default "$temporary"
    chmod 0644 "$temporary"
    chown 0:0 "$temporary"
    mv -f "$temporary" "$config"
    config_created=1
fi

/usr/bin/audio_dsp_imx6ull --config "$config" --check-config >/dev/null ||
    fail "installed runtime rejected the configuration"

case "$manager" in
    systemd)
        systemctl daemon-reload
        if [ "$enable" -eq 1 ] || [ "$was_enabled" -eq 1 ]; then
            systemctl enable "$unit"
        else
            systemctl disable "$unit" >/dev/null 2>&1 || true
        fi
        if [ "$enable" -eq 1 ] || [ "$was_active" -eq 1 ]; then
            systemctl start "$unit"
        fi
        ;;
    sysv)
        if [ "$enable" -eq 1 ] || [ "$was_enabled" -eq 1 ]; then
            sysv_enable
        else
            sysv_disable
        fi
        if [ "$enable" -eq 1 ] || [ "$was_active" -eq 1 ]; then
            sysv_start
        fi
        ;;
esac

committed=1
echo "MPU_TARGET_INIT_SYSTEM=$manager"
echo "MPU_TARGET_INSTALL_OK=1"
