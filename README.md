# ios-sweetwhyred

An experimental bring-up of Apple XNU (the iOS 16 generation, xnu-8796) on the Xiaomi Redmi Note 5 (whyred, Qualcomm SDM636 / Snapdragon 636).

This branch targets whyred. The inherited sweet documentation described a Redmi Note 10 Pro running a complete iOS userland; those claims do not describe the current state of this branch.

> **Early development.** This is not a usable phone OS. Keep a verified stock boot.img backup. Do not store personal data on experimental builds.

## Current status — 2026-10-29

The v36 diagnostic image reached the project's static test init as PID 1 and verified the timed wait/wakeup path on the Redmi Note 5 whyred. It is not a usable phone OS, and it has not started Apple launchd, backboardd, SpringBoard, or the home screen.

- The v36 image was built from [Actions run 40878122126](https://github.com/timosid/ios-sweetwhyred/actions/runs/40878122126), packaged locally, and flashed only to `boot`.
- GIC redistributor readback is `0x80002`: SGI 1 and virtual timer PPI 19 are enabled. UART records delivered INTID 19 interrupts, two timer callbacks, and `SWEET init: one-shot select returned OK`.
- The PID 1 test intentionally prints `SWEET init: one-shot diagnostic complete` and parks. The boot therefore confirms XNU, VM, BSD mockfs root, PID 1, and timer wakeup; it does not demonstrate a normal iOS boot.
- v36 kernel SHA-256: `c192b3a149abe8e5a358f1db996454e6b9572e1f5a1f4bf6a07486db4bfd29bf`
- v36 boot image SHA-256: `4e5cc717ee89df7308cc1cdf7ebad29b081788a9b5f7015780e5ffa8df2b31ab`
- The linked diagnostic kernel was checked for unsupported instructions: zero LSE and PAN instructions.

**Next major blocker:** whyred's storage is eMMC through Qualcomm SDHCI. This XNU port has no whyred eMMC block driver, so the HFS+ iOS root image cannot yet be mounted. Storage support must come before replacing the mockfs init with Apple's launchd and attempting backboardd/SpringBoard.

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
