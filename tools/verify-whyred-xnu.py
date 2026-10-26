#!/usr/bin/env python3
"""Reject an XNU source tree that lacks whyred framebuffer and UARTDM diagnostics."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--expected-upstream-commit")
    args = parser.parse_args()
    root = args.source.resolve()
    patterns = {
        "osfmk/arm64/start.s": [
            r"MOV64\s+x0,\s*\(0x9d400000\s*\+\s*2128\s*\*\s*4320\)",
            r"MOV64\s+x10,\s*0x9c000000",
        ],
        "osfmk/arm64/sweet_early.c": [
            r"early_fb\s*=\s*\(volatile uint32_t \*\)\(uintptr_t\)0x9d400000;",
            r"#define EARLY_WIDTH 1080u",
            r"#define EARLY_HEIGHT 2160u",
            r"#define EARLY_STRIDE \(4320u / 4\)",
        ],
        "osfmk/arm64/arm_vm_init.c": [
            r"sweet_early_relocate\(SWEET_EARLY_FB_VA \+ 0x01400000\);",
        ],
        "pexpert/arm/pe_serial.c": [
            r"#define WHYRED_UARTDM_PA\s+0x0c170000ULL",
            r"ml_io_map\(WHYRED_UARTDM_PA, WHYRED_UARTDM_SIZE\)",
            r"\[whyred-XNU\] UARTDM console ready",
        ],
        "pexpert/arm/pe_identify_machine.c": [
            r"whyred synthetic ADT provides arm-io reg, not ranges",
        ],
        "pexpert/pexpert/arm/protos.h": [
            r"SERIAL_WHYRED_UARTDM\s*=\s*0x40",
        ],
        "config/MASTER.arm64.sweet": [
            r"options\s+WHYRED_UARTDM\b",
        ],
    }
    hashes = {}
    for relative, required in patterns.items():
        content = (root / relative).read_bytes()
        text = content.decode("utf-8")
        for pattern in required:
            if not re.search(pattern, text):
                raise SystemExit(f"Missing whyred source requirement in {relative}: {pattern}")
        hashes[relative] = hashlib.sha256(content).hexdigest()
    commit = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    if args.expected_upstream_commit and commit != args.expected_upstream_commit:
        raise SystemExit(f"Unexpected upstream XNU commit: {commit}")
    report = {
        "source_commit": commit, "source_sha256": hashes,
        "framebuffer": {"physical_base": "0x9d400000", "width": 1080,
                        "height": 2160, "stride": 4320,
                        "l2_block_base": "0x9c000000", "alias_offset": "0x01400000"},
        "uartdm": {"physical_base": "0x0c170000", "baud": 115200,
                   "mode": "firmware-configured 8-N-1", "direction": "TX-only PIO"},
        "result": "whyred framebuffer and UARTDM requirements passed",
    }
    args.report.write_text(json.dumps(report, indent=2) + "\n")
    print(report["result"])


if __name__ == "__main__":
    main()
