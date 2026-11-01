## Current whyred work (2026-11-01, USB Ethernet/live console candidate)

- The user resumed USB work and selected Ethernet plus live logs first. SSH is
  outside this candidate. Patch 0159 ports the existing CDC-ECM driver to the
  whyred SDM660 registers and powered bootloader handoff; see `WHYRED-USB.md`.
  `whyred-usb=2` explicitly enables the gadget; the default remains off.
- Startup runs after network initialization, independently of UFS. The existing
  in-memory console ring feeds bounded nonblocking UDP to 192.168.7.1:5140.
  No System image changes or persistent storage writes are added.
- Local UBSan passes 565 checks executing 27 actual kernel functions with
  modeled MMIO/kernel services. The two changed ARM64 units compile, with zero
  LSE/PAN instructions. Exact late-patch replay matches all three reviewed files;
  the UFS source remains unchanged. The macOS interface helper self-test passes.
- Full kernel CI now includes USB ASan+UBSan checks alongside all retained
  checks. The next milestone is a successful full build, authenticated artifacts
  and a guarded v81 boot package retaining the v80 diagnostics below.
- No physical USB enumeration, ping or console delivery is confirmed. Cold PHY
  power-up is unsupported; DMA visibility and host compatibility need a device
  test. No phone command, flash or reboot has occurred on the current host.

## Retained v80 RAMFS diagnostics

- Boot v79 and System v70 are the inherited hardware baseline. Filesystem
  tracing reproduced container metadata write ENOSPC; the quota versus
  allocation/reclamation cause is unresolved. A working graphical scene is
  unconfirmed. USB Ethernet/live console work is active as described above.
- Patch 0158 adds default-off development diagnostics for data quota, missing
  data pages, nonwaiting page-array allocations and attribute snapshot failures.
  It retains the existing quotas, allocation flags, operation results and storage
  write policy. See `WHYRED-RAMFS-PRESSURE.md` for record semantics and limits.
- Local host checks pass with UBSan: five injected failure branches, short-write
  residuals, open-unlinked and named-vnode reclamation, 1.1 million failures for
  the output budget and 10,000 allocation/reclaim lifetimes. Existing 66 namespace,
  readdir64 and sync checks pass on the candidate. The changed C unit compiles to
  ARM64 using the authenticated v79 generated headers.
- Local AddressSanitizer cannot run in the current host environment: even a
  standalone hello-world sanitizer binary exits with SIGILL before main. This
  is not a passing local ASan result. Both ASan and UBSan passed on the CI runner.
- Source `2e17298133f5639668ce5366f99b7334bdc01e11`, full workflow
  [41314180439](https://github.com/timosid/ios-sweetwhyred/actions/runs/41314180439),
  job `124870632893`: all 25 steps succeeded. This includes the full linked ARM64
  kernel, the new pressure suite and all retained storage/graphics/AKS checks.
- Both downloaded CI ZIPs matched their GitHub SHA256 digests. Exact RAMFS source
  and all retained reviewed runtime files match the buildtree. Linked-code audit
  found zero LSE/PAN instructions. Boot v80 was packaged with the proven loader,
  ADT and stock DTB tail; layout and payload guards passed.
- Boot v80: 22,310,912 bytes, payload 21,503,680 bytes; image SHA256
  `a52274cdec6a8afbb0f2f7564ac6a1548925edc4137165fd202e3de63664367a`.
  It enables `whyred-ramfs-trace=1`. No phone command, flash or v80 hardware test
  has occurred; the inherited installed System v70 remains required.
- Next: obtain RAMFS pressure records during an explicitly authorized boot-only
  test. The observed ENOSPC cause and the relation to container crashes/empty
  frames remain unresolved. Private manifests, UART, decompilation and transfer
  verification remain outside this repository.
