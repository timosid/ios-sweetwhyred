## Active v86 USB connection diagnosis (2026-11-01)

- Installed remains boot v85 / System v70. User reports no new USB lines on
  cable connect/disconnect, but sees ue0 rx0/tx0 heartbeat. Thus the event thread
  runs; no frame exchange, Mac gadget enumeration or Ethernet link is established.
- Patch0165 adds read-only register snapshots before takeover, after endpoint/
  VBUS setup and from the event thread at one-second intervals. Runtime output
  requires a state change, ignoring DSTS frame-number-only differences. Maximum
  32 four-line records, then no more sampler MMIO reads. No controller/PHY policy,
  event ACK, DMA ownership, clock/reset/PMIC/PLL or charger-state changes.
- Samples cover UTMI test/control/line state, DWC3 status/event-ring/endpoint
  commands, QSCRATCH mux/VBUS, bus errors/timing and hardware parameters. MiCode's
  PHY high-impedance override is a possible distinguishable state, not an observed
  cause. See WHYRED-USB.md. Service causes remain pending separate UART output.
- Local actual28-function USB suite passes637 UBSan checks, including timing,
  raw output, change filtering, cap and read-only behavior. Both ARM64 units,
  zeroLSE/PAN audits and exact patch replay pass. Both dedup/cap negative controls
  fail their functional assertions after successful compilation. Full CI next; v86 retains v85's
  idle repair, service tracing and RAMFS owners. No new image/flash yet.

## V85 USB hardware observation (2026-11-01)

- User UART confirms powered-PHY wake and EP0 STARTTRANSFER command/result0x6
  with resource0. The driver passes its former guard, applies VBUS override
  and reaches controller RUN. No reset/connect/event lines were supplied yet.
- The Mac found no matching Ethernet interface during a 90-second observation;
  its USB device inventory also had no phone gadget at the sampled time. No
  host network changes were made. Controller RUN does not establish USB
  enumeration, working Ethernet or console delivery. Cable-reconnect feedback
  and further manual UART are pending; no subsequent register change is justified
  by this excerpt alone. Service exit causes and idle stability remain open.

## V85 boot flash milestone (2026-11-01)

- User confirmed Fastboot readiness for the verified v85 test. Fresh private
  identity, whyred product, unlocked state, 64MiB boot geometry, image/kernel and
  v82 rollback hashes passed. Boot send/write and reboot returned OKAY.
- Installed state is now boot v85 / System v70. No other partition was written.
  Device boot progress, idle stability, USB enumeration and service exit causes
  await user-supplied UART and host observation. UART capture remains manual.

## Active v85 follow-up (2026-11-01, idle panic and daemon exit causes)

- Installed phone remains boot v82 / System v70. User supplied an actual kernel
  panic: kernel_task tid102, preemption count1 in ARM64 user return; exact v82
  UUID/symbols verified. This differs from the separate lsd userspace SIGSEGV.
- Found an unsafe interrupt-stack path introduced by patch0120: proximate timer
  wait unmasks IRQs after Idle_context switches to CPU_ISTACKPTR, while the IRQ
  handler resets SP to that same top. Patch0163 uses masked pending-IRQ/WFI wait
  instead. This repairs a concrete overlap defect consistent with the panic;
  physical wakeup/panic absence still need testing. The assertion stays enabled.
- Patch0164 adds default-off whyred-service-trace=1: bounded PID, exec/spawn errno,
  activation state, exit wait status and OS reason for bluetoothd, fairplayd.H2,
  suggestd, lsd and pkd. No daemon cause/fix is yet proven. No jobs disabled.
  See WHYRED-IDLE-SERVICES.md for semantics, limits and evidence.
- Local UBSan799 checks pass, including 8000 concurrent report attempts. Changed
  cpu.c/kern_exec.c/kern_exit.c compile for ARM64; zero LSE/PAN audit, exact replay
  and both negative controls pass. Full CI will retain all v84 tests and add this
  suite. Verified v85 below combines these with USB and RAMFS owner tracing.
- V85 source `276c34608544fa2e3e1874f57500e8befccef022`, workflow
  [41329547329](https://github.com/timosid/ios-sweetwhyred/actions/runs/41329547329),
  job124916560517: all27 steps passed. New799, USB614 and RAMFS1120134 checks pass
  ASan+UBSan. Both ZIP provenance/digests/length/CRC and exact reviewed runtime/
  test sources pass. Linked cpu_idle contains pending-IRQ/DSB/WFI with no DAIFClr;
  linked ISA/cache, trusted loader/ADT/DTB/layout and size guards pass.
- Packaged v85 image SHA256
  `ed46661c5641b49bc59a79fe71df2c2bd8ce87070830de489699d80d89c707d8`,
  22,315,008 bytes, payload21,504,496. Boot args enable service trace, RAMFS owners
  and USB. It is unflashed; next step is physical Fastboot readiness followed by
  fresh device/product/unlock/64MiB/image/rollback checks and boot-only testing.
  No physical USB link, daemon exit cause or panic-resolution claim yet.
- V84 source d4ba66b3b00832f7610a6bd24887f2396d0694d8, CI41327819130/job124911379642:
  all26 steps pass, USB614 and RAMFS1120134 ASan+UBSan. Authenticated/package
  image SHA12e1b309a8f7af21fc4f079541943efb42faf603aef577fdf32595150305e8f8.
  It is unflashed and lacks the new idle fix/daemon trace; prefer v85 next.
- Full UART's first data-quota failure precedes first pkd attempt; Bluetooth/
  FairPlay restart attempts already precede quota exhaustion. Largest RAMFS owners
  remain unknown until TOP output. Preserve quotas and installed System v70.

## Active whyred follow-up (2026-11-01, EP0 activity plus RAMFS owners)

- V82 boot flash/reboot succeeded after the user's Fastboot-ready answer.
  Installed phone is now boot v82 / System v70. User UART confirms PHY wake,
  PLL lock, DWC3 ID 0x5533300a and ue0 address setup. Startup then stopped at the
  driver's strict missing-transfer-index guard before USB RUN; no host link yet.
- Patch 0162 follows Linux EP0's separate BUSY/activity tracking: successful
  EP0 STARTTRANSFER is not rejected solely for a zero resource index. Activity
  and HWO guard DMA reuse; data endpoints still require a nonzero resource.
  The resource result is read separately, with a bounded settle window and
  status/active checks. First control starts and failures log raw command results.
- A mid-control reset with active EP0 and no retirement index stops the controller
  and retains DMA rather than reusing it. This conservative limitation still
  needs device testing; it is not a full USB compliance claim.
- Local UBSan passes 614 actual USB checks. The model completes CDC setup with
  zero control indices, rejects active DMA reuse and missing data resources,
  and covers late result errors/reactivation. Restoring the old zero-index guard
  or removing the new activity guard fails the respective negative control.
  Both ARM64 compiles, exact patch replay and zero LSE/PAN object audits pass.
- Next full candidate v84 retains the v83 ownership diagnostics below. No v84
  image has been flashed. USB Ethernet and console delivery remain unconfirmed.

## Verified v83 RAMFS ownership diagnostic candidate

- Patch 0161 adds default-off `whyred-ramfs-top=1`, requiring the existing trace
  flag, to diagnose the observed 128 MiB quota exhaustion. On sampled quota
  failures it selects the four largest resident file-page owners and snapshots
  inode/pages/logical size/link count/UID/mapped-write state plus one bounded name
  and parent-directory name. It does not identify a last-writer process.
- Selection reuses the coherent locked hash walk; namespace searches share a
  65,536-entry budget. Fixed metadata/name arrays are copied under the mount lock
  and printed after unlocking, with no node pointers escaping. Names are bounded,
  printable and explicitly marked when truncated; a missing alias stays unknown.
  Existing 32-record sampling limits and quota/error/allocation policy are retained.
- Local UBSan passes 1,120,134 checks, including ordering/ties, linked/unlinked
  owners, name truncation/control-byte filtering, snapshot lifetime, the name-walk
  budget and default-off behavior. Disabling collection fails the new cases.
  The retained 66 namespace/readdir/sync checks pass. ARM64 compilation passes
  without a >4 KiB frame warning; object LSE/PAN audit is zero.
- Source `f3c036f56057ea4e162d2aeb975e717c2fa7ba69`, workflow
  [41325125813](https://github.com/timosid/ios-sweetwhyred/actions/runs/41325125813),
  job `124903349713`: all 26 steps succeeded, including 1,120,134 pressure/owner
  ASan+UBSan checks and 601 retained USB checks. ZIP provenance/digests/CRC,
  reviewed sources, linked ISA/cache path, loader and layout guards passed.
- Packaged v83 image SHA256
  `90bb7b1a1871c1d1e91a7da83be0b7f2351509a9631f3063758f3ce88982685d`.
  It has not been flashed and retains the old EP0 check; v84 supersedes it for
  the next combined USB/ownership test. Physical owner output is still pending.

## Verified v82 USB PHY shutdown-resume candidate

- User-supplied v81 UART identifies handoff status 6 before any DWC3 register
  access. Domain/clocks passed, while the legacy PHY POWER_DOWN bit remained set
  and PLL lock was absent. The ID/GCTL zeros were unsampled, not proof of a bad
  controller. Current installed phone remains boot v81 / System v70.
- Patch 0160 adds a narrow exit from the exact major-rev=1 shutdown low-byte
  state 0x23, using MiCode's 0x22 enable value. Mode 2 additionally requires the
  reference gate on and QUSB/AHB2PHY resets deasserted. It waits 150 us and polls
  for at most another 1000 us. Failure restores 0x23 before undoing new clock
  votes; status 8 distinguishes an attempted wake timeout. Mode 1 stays read-only.
- This retains bootloader tuning and requires powered rails. It adds no PMIC,
  PLL tuning, reset-controller or SMMU writes and is not full cold PHY init.
  Expanded diagnostics report before/after, attempt/polls/restore and explicitly
  distinguish an unsampled controller from register values actually read.
- Local UBSan passes 601 checks, including delayed/failed PLL, rejected writes,
  missing reference gate, held resets, wrong PHY states and ordered rollback.
  Disabling the new wake branch is rejected by the strengthened tests. Exact
  patch replay and both ARM64 compiles pass; LSE/PAN object audits find zero.
- Source `c6826995ba8de977d97f4e53e1d908d18f664ca1`, full workflow
  [41323121100](https://github.com/timosid/ios-sweetwhyred/actions/runs/41323121100),
  job `124897356634`: all 26 steps succeeded, including 601 USB ASan+UBSan checks.
  Final ZIP provenance/digests/length/CRC, exact sources, linked ISA/cache path,
  trusted loader/ADT/DTB, layout and size guards passed.
- Boot v82 is 22,315,008 bytes, payload 21,504,080 bytes; SHA256
  `55c6a5471135fd1594d9fcc96a9ca94b6246d8e50f268ba5932aeb06f913fb9c`.
  V82 was subsequently flashed as recorded above; installed state is v82/System v70.
- The user's v81 photograph now identifies the RAMFS failure as `data-quota`:
  8192/8192 pages, matching the hash sum and additions minus frees, i.e. 128 MiB.
  Zero-link pages are only 67. See `WHYRED-RAMFS-PRESSURE.md`; largest file owners
  remain unknown. Preparing bounded ownership diagnostics; quotas are unchanged.
- The user also observes repeated `exec bluetoothd` / `exec fairplayd.H2`.
  Each name already has 11+16 occurrences in the preserved v79 fragments.
  Authenticated System v70 config has fairplayd KeepAlive=true and bluetoothd
  KeepAlive SuccessfulExit=false; neither redirects stdout/stderr to console.
  The kernel's exec line precedes completion and has no PID/result. No target
  termination cause is recorded, so failed exec, self-exit and crash/kill remain
  unresolved. No services or restart policies were changed.

## Verified v81 USB baseline and hardware observation

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
- The user's UART confirms stop at PHY preflight (status 6), powered-down legacy
  PHY and no sampled controller. v82 above addresses this observed shutdown
  state. Physical wake, DMA visibility and host compatibility remain unconfirmed.
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
