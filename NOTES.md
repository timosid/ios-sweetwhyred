## Current whyred work (2026-11-01, USB Ethernet/live console candidate)

- The user resumed USB work and selected Ethernet plus live logs first. SSH is
  outside this candidate. Patch 0159 ports the existing CDC-ECM driver to the
  whyred SDM660 registers and powered bootloader handoff; see `WHYRED-USB.md`.
  `whyred-usb=2` explicitly enables the gadget; the default remains off.
- Startup runs after network initialization, independently of UFS. The existing
  in-memory console ring feeds bounded nonblocking UDP to 192.168.7.1:5140.
  No System image changes or persistent storage writes are added.
- Local UBSan passes 566 checks executing 27 actual kernel functions with
  modeled MMIO/kernel services. The two changed ARM64 units compile, with zero
  LSE/PAN instructions. Exact late-patch replay matches all three reviewed files;
  the UFS source remains unchanged. The macOS interface helper self-test passes.
- Initial DMA cache publication is checked before event-ring installation or
  STARTTRANSFER. The strengthened harness rejects the previous missing-clean
  version; the corrected driver passes. Physical cache coherence remains untested.
- Source `7ccdf8dda98cdc2ead91f171c51607ddcae29890`, full workflow
  [41319898167](https://github.com/timosid/ios-sweetwhyred/actions/runs/41319898167),
  job `124888314335`: all 26 steps succeeded, including 566 USB checks with
  ASan+UBSan, the full linked ARM64 kernel and all retained checks. Earlier USB
  run 41319155786 was superseded and cancelled; it supplies no candidate artifacts.
- Both final ZIPs match GitHub SHA256/length and pass CRC. All four USB/ring/init
  sources match the reviewed buildtree and test report. Linked-code audit found
  zero LSE/PAN instructions; the linked cache-maintenance path includes physical
  translation, `dc civac` and `dsb sy`. Loader/ADT/stock DTB, layout and size guards
  passed. Boot v81 retains the v80 diagnostics below and installed System v70.
- Boot v81: 22,315,008 bytes, payload 21,504,080 bytes, 496-byte command line with
  `whyred-usb=2`; image SHA256
  `8b1300ed5f5c9524bc5b61722479a5f363b8df4391b8b71b06a10130f1706b41`.
- Temporary `fastboot boot` returned OKAY, but the phone remained in Fastboot;
  the user confirmed this command does not work on this handset. The user then
  explicitly authorized flashing boot. Identity/product/unlocked/64-MiB partition,
  v81 SHA256 and the preserved v79 rollback image passed fresh checks.
- Sending/writing boot v81 and reboot each returned OKAY. The installed baseline
  is now boot v81 / System v70; System was not written. Fastboot disappeared after
  reboot. No CDC Ethernet interface or UDP console bytes have yet been observed;
  enumeration, ping and actual delivery remain unconfirmed.
- Cold PHY power-up is unsupported; powered handoff, DMA visibility and host
  compatibility remain physical assumptions. Next: obtain the user's UART
  `WHYRED USB:` snapshot/stopped reason and screen observation, then diagnose that
  evidence. Do not infer a register/power fix from host tests alone.
- macOS 26 exposes USB via IOService IOUSBHostDevice / SPUSBHostDataType. The
  legacy IOUSB-plane/SPUSBDataType query returned empty even with Fastboot present;
  use the current inventory. Raw UART/serials remain private and user-supplied.

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
