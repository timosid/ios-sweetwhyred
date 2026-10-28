# ios-sweetwhyred

An experimental bring-up of the Apple XNU kernel (the iOS 16 generation, xnu-8796) on the Xiaomi Redmi Note 5 (`whyred`, Qualcomm SDM636 / Snapdragon 636).

This branch targets `whyred`. The inherited `sweet` documentation described a Redmi Note 10 Pro running a complete iOS userland; those claims do not describe the current state of this branch.

> **Early development.** This is not a usable phone OS. Keep a verified stock `boot.img` backup. Do not store personal data on experimental builds.

## Current status — 2026-10-28

The v27 diagnostic kernel has booted on the phone and passed these milestones:

- XNU entered on the Kryo 260 Gold core at MPIDR `0x100`.
- The kernel initialized its MMU and VM, the GIC virtual timer (INTID 19), UARTDM console, IOKit, one boot CPU, BSD, and the built-in AMFI/HFS registrations.
- BSD selected the test RAM disk as `md0`; `vfs_mountroot()` returned success.
- Calendar initialization completed. The UART trace contains all `WHYRED_CLOCK` C1–C14 markers.

The v27 image came from [Actions run 40698136953](https://github.com/timosid/ios-sweetwhyred/actions/runs/40698136953). The follow-up [run 40702435098](https://github.com/timosid/ios-sweetwhyred/actions/runs/40702435098) built the v28 root-handoff diagnostics. Its UART capture confirmed the `md0` root mount, then stopped at the `IOSecureBSDRoot` wait for `IOPlatformExpert`. Source review found that the whyred diagnostic fallback did not explicitly register that service; [patch 0114](xnu/patches/0114-whyred-platform-expert-register.patch) adds registration and tracing. [Run 40707929849](https://github.com/timosid/ios-sweetwhyred/actions/runs/40707929849) completed successfully with patch 0114 applied. The v29 image (`b425a38df5c3ef1b206f7206c2930029a878dfc36c535b7616017aec77f9893c`) was flashed to `boot` on the unlocked `whyred` (Fastboot serial `de9ec666`). Its UART confirms the platform expert registers, the secure-root wait returns, BSD mounts `devfs`, and the static test init runs as PID 1. This init is not Apple's launchd; `backboardd` and SpringBoard are not present or confirmed.

**Userspace status:** The boot image still provides only the project's static test init on a mockfs RAM disk; it is not Apple's `launchd`, and `backboardd`, SpringBoard, and the iOS home screen have not been reached. The v30 UART shows one successful `select()` return, then an error marker and a conflicting second entry marker; the user waited about one more minute without output.

The first v31 workflow [40716443013](https://github.com/timosid/ios-sweetwhyred/actions/runs/40716443013) built XNU but exposed a missing `alive` label still used by `adr x2, alive` in the init assembly (`unknown AArch64 fixup kind`). The reference now uses `hello_tail_end`. The init-only [Actions run 40719859405](https://github.com/timosid/ios-sweetwhyred/actions/runs/40719859405) succeeded; signed init SHA-256: `dd897881eb5cb817c092df8d8e5dca40fc2a6d801170df434331e97e8975d594`. Packaged v31 image SHA-256: `9c51d3efded9e3420c321a140013cc8beed72f7cf9a02622d99ebdb3fa385dd0`; its kernel SHA-256 is `212b854d3b96e7da7b8784933485015d0257e31c3283db8ee435f221809f63fc`, with patch 0114 applied and LSE/PAN instruction counts both zero. The image was flashed only to `boot` on the unlocked `whyred` (Fastboot serial `de9ec666`), and reboot returned OKAY. Await the one-shot UART result: PID 1 now calls `select()` once, prints success or its two-digit errno, then parks.

## Display status

A photo from the device shows the early XNU framebuffer console and its colored stage strip. This verifies that the new panel can display kernel-written pixels. The screen remains on the early boot text: XNU disables that early framebuffer console after UART initialization, and the `serial=1` boot argument routes later console output to UARTDM. The photo is not evidence that the normal display stack or graphical userspace has started.

## Hardware facts used by this port

- Target: Redmi Note 5 `whyred`, Qualcomm SDM636 / Snapdragon 636, heterogeneous Kryo 260 cores.
- The verified boot core is Gold MPIDR `0x100`; the current XNU topology is deliberately limited to that one core. Secondary-core startup is not validated.
- The Gold core supports 16 KiB translation granules and reports no LSE atomics or PAN. The tested kernel is built without LSE/PAN instructions.
- The observed interrupt path uses the GIC virtual timer, INTID 19. UARTDM is the diagnostic console.

## Build and diagnostics

GitHub Actions builds the ARM64 XNU kernel using the workflow in [`.github/workflows/xnu.yml`](.github/workflows/xnu.yml). Whyred-specific source patches are in [`xnu/patches/`](xnu/patches/) and apply from the XNU source root. The build includes a signed static mockfs test init and uploads the kernel, build log, and source tree as workflow artifacts.

The current diagnostic sequence checks:

1. XNU handoff, CPU, VM, IOKit, and calendar initialization.
2. BSD setup and root selection/mount from `rd=md0`.
3. Root vnode setup, `devfs`, and the transition to the test PID 1.

Do not treat a successful CI build as a successful device boot; use the UART log to confirm runtime milestones.

## Scope and limitations

Storage drivers, touch, buttons, battery telemetry, radios, GPU acceleration, suspend, and power management are not validated by the current boot. The built-in AMFI/HFS components and several platform services are bring-up implementations. This project is for kernel/platform experiments, not daily use.

## Credits

This work builds on Apple's open-source XNU sources, the XNU build tooling from [blacktop/darwin-xnu-build](https://github.com/blacktop/darwin-xnu-build), and published Qualcomm/Xiaomi kernel sources used to identify hardware details. Apple's iOS binaries and firmware are not distributed here.

