# ios-sweetwhyred

An experimental bring-up of Apple XNU (the iOS 16 generation, xnu-8796) on the Xiaomi Redmi Note 5 (whyred, Qualcomm SDM636 / Snapdragon 636).

This branch targets whyred. The inherited sweet documentation described a Redmi Note 10 Pro running a complete iOS userland; those claims do not describe the current state of this branch.

> **Early development.** This is not a usable phone OS. Keep a verified stock boot.img backup. Do not store personal data on experimental builds.

## Current status — 2026-10-30

XNU boots on whyred through VM and BSD, mounts the project's RAM-backed mockfs root and runs the static test PID 1. The handset has confirmed timer wakeups and `SWEET init: one-shot select returned OK`. Apple launchd, backboardd and SpringBoard have not started.

- v47 established eMMC EXT_CSD reads after fixing the Qualcomm SDHCI FIFO selection.
- v48 read and validated the primary GPT; the handset reported all 64 partitions with valid CRCs.
- v49 registered bounded, read-only BSD block devices for those partitions, then continued with `BSD root: md0`. [Build run 40949733049](https://github.com/timosid/ios-sweetwhyred/actions/runs/40949733049) succeeded. Only the boot image was flashed.
- v50 adds an opt-in one-sector system-partition filesystem probe; it is built but has not been flashed.
- Patch 0135 adds a gated read-only APFS reader/VFS port adapted from HakiOS, without its platform drivers, write modules or scheduler hooks. [Actions run 41048205578](https://github.com/timosid/ios-sweetwhyred/actions/runs/41048205578) passed the full Apple-clang kernel build and host parser cases. A handset APFS mount remains untested. [APFS scope and checks](docs/whyred-apfs.md).
- The tools can build a 4 MiB diagnostic APFS root with the project's static PID 1. This test image contains no Apple iOS userland. Root mounting requires `whyred-apfs-root=1`; the default boot continues to use mockfs.
- The tested kernel uses one Gold core and avoids unsupported LSE/PAN instructions.

**Next:** verify mounting and executing the small diagnostic APFS root on the handset. Original iOS 16.5 root/OS/App source images have been downloaded locally from Apple and CRC-checked; a prepared whyred iOS system image, writable runtime state and additional userspace/platform adaptation are still needed before attempting the graphical interface. Apple images are not included in this repository or its CI artifacts.

## Display status

A photo from the device shows the early XNU framebuffer console and its colored stage strip. This verifies that the kernel can write pixels to the replacement panel. The normal display stack and graphical userspace are not confirmed.

## Hardware facts used by this port

- Target: Redmi Note 5 whyred, Qualcomm SDM636 / Snapdragon 636, heterogeneous Kryo 260 cores.
- The verified boot core is Gold MPIDR 0x100. XNU topology is deliberately limited to that one core; secondary-core startup is not validated.
- The Gold core supports 16 KiB translation granules and reports no LSE atomics or PAN. The diagnostic kernel is built without LSE/PAN instructions.
- The observed interrupt route uses the GIC virtual timer, INTID 19. UARTDM is the diagnostic console.
- The ADT's timer-frequency property is 19.2 MHz; v36 UART confirms runtime `CNTFRQ=0x124f800` (19.2 MHz) and delivery of virtual timer PPI 19.

## Build and diagnostics

GitHub Actions builds the ARM64 XNU kernel using [the workflow](.github/workflows/xnu.yml). Whyred-specific source patches are in [xnu/patches](xnu/patches/) and apply from the XNU source root. The workflow uploads the kernel, build log, and build tree as artifacts.

Current diagnostic milestones:

1. XNU handoff, the Gold boot CPU, VM, early display/UART, IOKit, and GIC initialization.
2. BSD setup, `md0` mockfs root mount, and transition to the static test PID 1.
3. Timer PPI 19 delivery, wait-queue wakeup, timer callbacks, and a successful five-second `select()` return.
4. Real eMMC/SDHCI storage support and mounting the iOS root are not implemented; Apple launchd and graphical userspace remain future work.

A successful CI build is not evidence of a successful device boot; confirm runtime milestones from UART.

## Scope and limitations

The current root is the `md0` mockfs diagnostic ramdisk. Whyred uses eMMC through the Qualcomm SDHCI controller at `0x0c0c4000` (`qcom,sdhci-msm-v5`); this XNU tree has no whyred eMMC block driver. The inherited `sweet_ufs.c` expects a UFS host and a `/sweet-ufs` ADT node, which whyred does not provide, so the old `rd=ufsroot0` path cannot mount the HFS+ image built by `mkiosroot.sh`. A real iOS root and Apple launchd remain blocked on storage integration. The inherited SWEET touch poller targets SE7/GT9896/Goodix/Focaltech; whyred's DTS lists Synaptics DSX (`0x20`) and Novatek NVT (`0x62`). Its key poller is PM6150L-specific, while whyred uses PM660/PM660L. The current IOKit HID services expose the event interface, but there is no whyred hardware input driver feeding them. These legacy pollers are not activated by the `md0` diagnostic boot and must not be enabled on whyred as-is. Battery telemetry, radios, GPU acceleration, suspend, and power management are also unvalidated; the built-in AMFI/HFS components and several platform services are bring-up implementations. This project is for kernel/platform experiments, not daily use.

## Credits

This work builds on Apple’s open-source XNU sources, the XNU build tooling from [blacktop/darwin-xnu-build](https://github.com/blacktop/darwin-xnu-build), and published Qualcomm/Xiaomi kernel sources used to identify hardware details. Apple’s iOS binaries and firmware are not distributed here.
