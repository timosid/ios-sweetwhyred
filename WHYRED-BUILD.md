# whyred diagnostic XNU build

Apply this overlay to a fork of dipa-studio/ios-sweet on branch
`codex/whyred-early-fb`. Pushing these files starts the macOS workflow.
The two patch files replace their SWEET originals; keep patch 0011.

The workflow checks the upstream XNU commit and whyred framebuffer
constants before compiling. A successful run uploads
`xnu-whyred-early-fb` with the kernel, SHA256SUMS and source report.
The boot image must then be packaged locally using the device-specific
stock boot backup. No device backup belongs in the CI repository.
