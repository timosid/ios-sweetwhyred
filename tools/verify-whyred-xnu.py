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
            r"MOV64\s+x9,\s*0x0c170000",
            r"\.macro WHYRED_UART_TX",
            r"WHYRED_UART_WORD 0x30554e58",
            r"WHYRED_UART_WORD 0x31554e58",
            r"WHYRED_UART_WORD 0x32554e58",
            r"WHYRED_UART_WORD 0x33554e58",
            r"WHYRED_UART_WORD 0x34554e58",
            r"WHYRED_UART_WORD 0x35554e58",
            r"WHYRED_UART_WORD 0x36554e58",
            r"MOV64\s+x10,\s*0x0c000000",
            r"ARM_TTE_BLOCK_ATTRINDX\(CACHE_ATTRINDX_DISABLE\)",
            r"Lwhyred_boot_exception:",
            r"mrs x14, ESR_EL1",
            r"mrs x14, ELR_EL1",
            r"mrs x14, FAR_EL1",
        ],
        "osfmk/arm64/sweet_early.c": [
            r"early_fb\s*=\s*\(volatile uint32_t \*\)\(uintptr_t\)0x9d400000;",
            r"#define EARLY_WIDTH 1080u",
            r"#define EARLY_HEIGHT 2160u",
            r"#define EARLY_STRIDE \(4320u / 4\)",
            r"XNU_C",
            r"whyred_boot_uart_disabled = 1;",
            r"XNU_EARLY_PANIC",
        ],
        "osfmk/arm64/arm_vm_init.c": [
            r"sweet_early_relocate\(SWEET_EARLY_FB_VA \+ 0x01400000\);",
        ],
        "pexpert/arm/pe_serial.c": [
            r"#define WHYRED_UARTDM_PA\s+0x0c170000ULL",
            r"ml_io_map\(WHYRED_UARTDM_PA, WHYRED_UARTDM_SIZE\)",
            r"\[whyred-XNU\] UARTDM console ready",
            r"whyred_uartdm_read\(WHYRED_UARTDM_ISR\) & \(1u << 7\)",
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
    startup = (root / "osfmk/arm64/start.s").read_text().split("LEXT(start_first_cpu)", 1)[1]
    if not (startup.index("WHYRED_UART_WORD 0x30554e58") < startup.index("OSLAR_EL1") <
            startup.index("Lsweet_mark_pixel:")):
        raise SystemExit("UART entry marker must precede OSLAR and framebuffer writes")
    vector_base = startup.rindex("EXT(LowExceptionVectorBase)@page")
    mmu_marker = startup.index("WHYRED_UART_WORD 0x34554e58")
    mmu_enable = startup.index("MSR_SCTLR_EL1_X0")
    first_stage_enable = startup.index("WHYRED_UART_WORD 0x35554e58")
    stage6 = startup.index("WHYRED_UART_WORD 0x36554e58")
    restore_vbar = startup.rindex("EXT(ExceptionVectorsBase)@page")
    if not (vector_base < mmu_marker < mmu_enable < first_stage_enable < stage6 < restore_vbar):
        raise SystemExit("Diagnostic VBAR must span MMU enable and restore before arm_init")
    commit = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    if args.expected_upstream_commit and commit != args.expected_upstream_commit:
        raise SystemExit(f"Unexpected upstream XNU commit: {commit}")
    report = {
        "source_commit": commit, "source_sha256": hashes,
        "framebuffer": {"physical_base": "0x9d400000", "width": 1080,
                        "height": 2160, "stride": 4320,
                        "l2_block_base": "0x9c000000", "alias_offset": "0x01400000"},
        "uartdm": {"physical_base": "0x0c170000", "baud": 115200,
                   "mode": "firmware-configured 8-N-1", "direction": "TX-only PIO",
                   "entry_marker": "XNU0 before OSLAR and framebuffer", "tx_idle_wait": True},
        "bootstrap_uart": {"stages": ["XNU" + str(i) for i in range(7)],
                           "device_block_base": "0x0c000000", "memory_type": "Device-nGnRnE",
                           "exception_registers": ["ESR_EL1", "ELR_EL1", "FAR_EL1"],
                           "vbar_spans_mmu": True,
                           "c_stages": True},
        "result": "whyred framebuffer and UARTDM requirements passed",
    }
    args.report.write_text(json.dumps(report, indent=2) + "\n")
    print(report["result"])


if __name__ == "__main__":
    main()
