# whyred diagnostic XNU build

Apply this overlay to a fork of dipa-studio/ios-sweet on branch
`codex/whyred-early-fb`. Pushing these files starts the macOS workflow.
The two patch files replace their SWEET originals; keep patch 0011.

The workflow checks the upstream XNU commit, whyred framebuffer geometry,
and the UARTDM source guard before compiling. It preserves the firmware's
115200 8-N-1 UART configuration and enables TX-only kprintf diagnostics.
A successful run uploads `xnu-whyred-uartdm-debug` with the kernel,
SHA256SUMS and source report.
The boot image must then be packaged locally using the device-specific
stock boot backup. No device backup belongs in the CI repository.
