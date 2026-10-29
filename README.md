# ios-sweetwhyred

An experimental bring-up of Apple XNU (the iOS 16 generation, xnu-8796) on the Xiaomi Redmi Note 5 (whyred, Qualcomm SDM636 / Snapdragon 636).

This branch targets whyred. The inherited sweet documentation described a Redmi Note 10 Pro running a complete iOS userland; those claims do not describe the current state of this branch.

> **Early development.** This is not a usable phone OS. Keep a verified stock boot.img backup. Do not store personal data on experimental builds.

## Current status — 2026-10-29

The v31 diagnostic image reached XNU startup, initialized VM and IOKit, mounted the test mockfs root as md0, and launched the project’s static test init as PID 1. The init printed:

`SWEET init: one-shot select(5s) begin`

No further UART output appeared after 2–3 minutes. This confirms that XNU and PID 1 started, but the timed select call has not been observed returning. The exact stall within syscall dispatch, wait queue, timer IRQ, or wakeup is still being investigated.

[Actions run 40728953072](https://github.com/timosid/ios-sweetwhyred/actions/runs/40728953072) completed successfully with XNU patch 0115 and the mockfs PID 1 build. Patch [0115](xnu/patches/0115-whyred-select-timer-trace.patch) adds targeted traces at select entry, wait-queue timer enqueue, virtual-timer programming, GIC IRQ delivery, timer callback, and return from sleep.

The v32 diagnostic boot image was packaged locally from that run’s artifacts. Its kernel contains all seven expected trace markers. The v32 ADT declares a 19.2 MHz timebase for the Gold CPU and retains Qualcomm timer PPI order 1, 2, 3, 0, with the virtual timer at INTID 19.

- Kernel SHA-256: `dd462ff2244f9c0503e6420e4b1daabdee50fae70144dd2247833b68b27e94c3`
- Test init SHA-256: `dd897881eb5cb817c092df8d8e5dca40fc2a6d801170df434331e97e8975d594`
- v32 boot image SHA-256: `679e8b9598a699496c75f5688dd74b8f0040fcb9fde08dcf5232155b54965b3c`

The v32 image has not yet been tested on the phone.

Patch [0116](xnu/patches/0116-whyred-display-storage-guard.patch) now matches AppleCLCD to whyred’s `whyred,display` ADT entry and excludes the `interrupt-controller` node from generic IOKit nub creation. [Actions run 40849359706](https://github.com/timosid/ios-sweetwhyred/actions/runs/40849359706) compiled the updated source successfully. This is a compile result only; the service and display stack still need device validation.

A further ADT check found that the custom platform-expert personality still matched the inherited `xiaomi,sweet` identifier, while the generated whyred ADT identifies its root as `xiaomi,whyred`. Patch [0117](xnu/patches/0117-whyred-platform-expert-root-match.patch) corrects the match so the whyred `SweetPlatformExpert` can be selected instead of the generic fallback. [Actions run 40851815254](https://github.com/timosid/ios-sweetwhyred/actions/runs/40851815254) is checking this change; it is not device-tested yet.

**Userspace status:** The ramdisk provides only the project’s static test init as PID 1. It is not Apple launchd; backboardd, SpringBoard, and the iOS home screen have not been reached.

## Display status

A photo from the device shows the early XNU framebuffer console and its colored stage strip. This verifies that the kernel can write pixels to the replacement panel. The normal display stack and graphical userspace are not confirmed.

## Hardware facts used by this port

- Target: Redmi Note 5 whyred, Qualcomm SDM636 / Snapdragon 636, heterogeneous Kryo 260 cores.
- The verified boot core is Gold MPIDR 0x100. XNU topology is deliberately limited to that one core; secondary-core startup is not validated.
- The Gold core supports 16 KiB translation granules and reports no LSE atomics or PAN. The diagnostic kernel is built without LSE/PAN instructions.
- The observed interrupt route uses the GIC virtual timer, INTID 19. UARTDM is the diagnostic console.
- The timer-frequency property in the v32 ADT is 19.2 MHz. Runtime counter and IRQ values still need confirmation from UART.

## Build and diagnostics

GitHub Actions builds the ARM64 XNU kernel using [the workflow](.github/workflows/xnu.yml). Whyred-specific source patches are in [xnu/patches](xnu/patches/) and apply from the XNU source root. The workflow uploads the kernel, build log, and build tree as artifacts.

Current diagnostic milestones:

1. XNU handoff, Gold CPU, VM, IOKit, and GIC setup.
2. BSD setup, mockfs root mount, and transition to the test PID 1.
3. Timed select tracing through wait-queue registration, timer programming, interrupt delivery, callback, and syscall return.

A successful CI build is not evidence of a successful device boot; confirm runtime milestones from UART.

## Scope and limitations

Storage drivers, touch, buttons, battery telemetry, radios, GPU acceleration, suspend, and power management are not validated by the current boot. The built-in AMFI/HFS components and several platform services are bring-up implementations. This project is for kernel/platform experiments, not daily use.

## Credits

This work builds on Apple’s open-source XNU sources, the XNU build tooling from [blacktop/darwin-xnu-build](https://github.com/blacktop/darwin-xnu-build), and published Qualcomm/Xiaomi kernel sources used to identify hardware details. Apple’s iOS binaries and firmware are not distributed here.
