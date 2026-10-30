#!/usr/bin/env python3
"""Seed the ephemeral whyred /private/var image for the built-in AKS provider."""
import argparse
import json
import os
from pathlib import Path
import plistlib
import stat

# Required by tools/mkiosroot.sh and sweet-root-tweaks.py in the same project.
DIRECTORIES = {
    "keybags": (0, 0, 0o700),
    "db/diagnostics": (272, 272, 0o751),
    "db/diagnostics/timesync": (272, 272, 0o755),
    "db/uuidtext": (272, 272, 0o755),
    "containers/Bundle": (33, 33, 0o755),
    "containers/Data/System": (0, 0, 0o755),
    "containers/Shared/SystemGroup": (0, 0, 0o755),
    "containers/Temp": (0, 0, 0o755),
    "mobile/Library/Preferences": (501, 501, 0o755),
    "mobile/Library/Caches": (501, 501, 0o755),
    "mobile/Library/Logs": (501, 501, 0o755),
    "mobile/Media": (501, 501, 0o755),
    "root/Library/Preferences": (0, 0, 0o755),
}
MARKERS = (
    "db/diagnostics/.migrated",
    "db/diagnostics/timesync/.migrated",
    "db/uuidtext/.migrated",
)


def checked_path(root, relative):
    path = root / relative
    if not path.resolve().is_relative_to(root):
        raise ValueError(f"path escapes mounted image: {relative}")
    return path


def metadata(path, uid, gid, mode):
    st = path.stat()
    if (st.st_uid, st.st_gid, stat.S_IMODE(st.st_mode)) != (uid, gid, mode):
        raise ValueError(f"incorrect owner or mode: {path.name}")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("mountpoint", type=Path)
    ap.add_argument("--verify", action="store_true")
    args = ap.parse_args()
    root = args.mountpoint.resolve()
    if root in (Path("/"), Path("/private/var")) or not all(
            (root / item).is_dir() for item in ("db", "mobile", "root", "tmp")):
        raise SystemExit("expected a mounted, initialized whyred var image")
    if not args.verify:
        if os.geteuid() != 0:
            raise SystemExit("run this image seeding step as root")
        for relative, (uid, gid, mode) in DIRECTORIES.items():
            path = checked_path(root, relative)
            path.mkdir(parents=True, exist_ok=True)
            os.chown(path, uid, gid)
            os.chmod(path, mode)
        # SWEET is the existing unlocked AKS provider's synthetic bag payload.
        # This file contains no passcode or device key material.
        bag = checked_path(root, "keybags/systembag.kb")
        with bag.open("wb") as stream:
            plistlib.dump({"KeyBagKeys": b"SWEET"}, stream, fmt=plistlib.FMT_XML)
        os.chown(bag, 0, 0)
        os.chmod(bag, 0o600)
        for relative in MARKERS:
            path = checked_path(root, relative)
            path.touch()
            os.chown(path, 272, 272)
            os.chmod(path, 0o644)
    for relative, (uid, gid, mode) in DIRECTORIES.items():
        path = checked_path(root, relative)
        if not path.is_dir():
            raise ValueError(f"required directory missing: {relative}")
        metadata(path, uid, gid, mode)
    bag = checked_path(root, "keybags/systembag.kb")
    metadata(bag, 0, 0, 0o600)
    with bag.open("rb") as stream:
        if plistlib.load(stream) != {"KeyBagKeys": b"SWEET"}:
            raise ValueError("synthetic system keybag does not match the AKS provider")
    for relative in MARKERS:
        path = checked_path(root, relative)
        metadata(path, 272, 272, 0o644)
        if path.stat().st_size != 0:
            raise ValueError(f"logd migration marker is not empty: {relative}")
    print(json.dumps({"required_directories": len(DIRECTORIES), "keybag": "verified",
                      "logd_migration_markers": len(MARKERS), "owners_and_modes": "verified"}))


if __name__ == "__main__":
    main()
