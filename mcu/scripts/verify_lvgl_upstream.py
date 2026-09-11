"""Compare the vendored subset with an official, fixed-commit LVGL tar.gz.

Read-only: does not extract the archive, edit vendor files, or trust paths
from archive members. lv_conf.h and ORIGIN.md are local and excluded.
"""

import argparse
import hashlib
from pathlib import Path
import subprocess
import tarfile


COMMIT = "85aa60d18b3d5e5588d7b247abf90198f07c8a63"
ROOT_FILES = {
    "lvgl.h", "lvgl_private.h", "lv_version.h", "lv_conf_template.h",
    "LICENCE.txt", "COPYRIGHTS.md",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--index", action="store_true", help="Also verify the staged Git blobs, without checkout or newline conversion")
    args = parser.parse_args()
    if not args.archive.is_file():
        parser.error(f"Archive missing: {args.archive}")
    vendor = Path(__file__).resolve().parents[1] / "project/STM32H743_Audio/App/LVGL"
    local = {p.relative_to(vendor).as_posix(): p for p in (vendor / "src").rglob("*") if p.is_file()}
    local.update({name: vendor / name for name in ROOT_FILES})
    expected = {}
    prefix = f"lvgl-{COMMIT}/"
    with tarfile.open(args.archive, "r:gz") as archive:
        for member in archive:
            if not member.isfile() or not member.name.startswith(prefix):
                continue
            relative = member.name[len(prefix):]
            if not (relative.startswith("src/") or relative in ROOT_FILES):
                continue
            if relative in expected:
                raise SystemExit(f"Duplicate upstream member: {relative}")
            with archive.extractfile(member) as source:
                expected[relative] = hashlib.sha256(source.read()).digest()
    if len(expected) != 1136 or set(local) != set(expected):
        missing = sorted(set(expected) - set(local))
        extra = sorted(set(local) - set(expected))
        raise SystemExit(f"Manifest mismatch: official={len(expected)} local={len(local)} missing={missing[:10]} extra={extra[:10]}")
    mismatches = [name for name, path in local.items()
                  if not path.is_file() or hashlib.sha256(path.read_bytes()).digest() != expected[name]]
    if mismatches:
        raise SystemExit(f"SHA-256 mismatch: {mismatches}")
    print(f"LVGL_UPSTREAM_OK commit={COMMIT} files={len(local)} sha256_mismatches=0")
    if args.index:
        repo = Path(__file__).resolve().parents[2]
        names = sorted(expected)
        prefix = vendor.relative_to(repo).as_posix()
        requests = "".join(f":{prefix}/{name}\n" for name in names).encode()
        result = subprocess.run(["git", "cat-file", "--batch"], cwd=repo,
                                input=requests, stdout=subprocess.PIPE, check=True)
        offset = 0
        for name in names:
            end = result.stdout.index(b"\n", offset)
            header = result.stdout[offset:end].split()
            if len(header) != 3 or header[1] != b"blob":
                raise SystemExit(f"Missing staged blob: {name}")
            size = int(header[2])
            start = end + 1
            blob = result.stdout[start:start + size]
            if hashlib.sha256(blob).digest() != expected[name]:
                raise SystemExit(f"Staged Git byte mismatch: {name}")
            offset = start + size + 1
        if offset != len(result.stdout):
            raise SystemExit("Unexpected trailing Git batch output")
        print(f"LVGL_GIT_INDEX_OK files={len(names)} sha256_mismatches=0")


if __name__ == "__main__":
    main()
