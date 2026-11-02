## Full CI test SDK correction (2026-11-02)

- Initial kernel run41470349414/job125361960778 stopped BEFORE kernel compile:
  the new host test inherited build.sh SDKROOT and could not find assert.h.
  No failed-run boot artifact is eligible. Patch0169 applied and APFS source
  read-only guards passed; this is not a device/kernel result.
- Host test now removes inherited SDKROOT and explicitly chooses macOS SDK,
  matching the existing eMMC contract-test pattern. A deliberately invalid
  SDKROOT local run passes. CI also retains its report in the kernel artifact.
- New raw F_NOCACHE probe source78fde09d / CI41470349071 all6 passed, binary
  3996342f804590682d77e2bb4da2dbfabaa9666ebd9449a907a03a8ab7edea43,
  ZIP8d237575107f8c98a0cbd83b7274eb0d5cf931fd7c65368e3ef6eeed68d4ddd7.
  Complete volatile self digest passed. Current boot89 ignores IO_NOCACHE in
  VNOP_READ: pread still EIO with fcntl success, as expected from exact source.
  Signed probe remains usable despite broken dyld. No new boot/System flash.

## Read/mmap failure confirmed; bounded APFS diagnostics preparing (2026-11-02)

- Corrected raw helper cc0ab5c / CI41468481553/job125355507394 passes all6
  steps. ZIP49e05ae3ee299910feb47b63c22aa56d9cbe339437e6bc3f82d6d8cb2a9bfac3,
  binary97db040f25f6faaef038229892af6b6dc3a3aa5bddd1934f05ba74cf3c87b490.
  Signature/platform/no imports validated, volatile complete self digest passes.
  It now starts on every tested argument layout. Shells/passwd/.27 fault-page
  pread EIO, then mmap succeeds but actual mapped read SIGBUS138. Cache .17
  pread/mmap still give the exact expected512-byte digest. These are genuine
  memory-error faults after entering main, distinct from the fixed SP bug.
- Patch0169 adds default-off whyred-apfs-errors=1 tracing at transport, cached
  read/fill and page-in failures. Preserves original errno/UPL abort behavior,
  records original mapping KR, file inode/offset/size and free/wired counters.
  At most32 records, no allocation or pathname lookup. It also honors existing
  IO_NOCACHE read requests through the already implemented direct path; this
  enables a controlled cache-vs-parser comparison, not a claim of root repair.
- Both changed C units compile locally for ARM64 with authenticated v79
  generated headers. Exact no-fuzz patch replay on authenticated v89 passes.
  Host fault injection extracts the actual kernel functions and checks17
  read/page-in/transport fault cases, cleanup, EOF zeroing, direct dispatch and
  default-off/32-record logging budget. Undefined-behavior sanitizer passes.
  Local ASan runtime itself traps in libpthread initialization before main;
  that attempted run is not counted as a kernel/test result.
- Raw helper adds optional bounded F_NOCACHE toggle; native syscall test passes.
  Full kernel CI/new signed raw artifact pending. Boot89/System91 is still
  installed, verified System92 reserve unused, held SSH shell/UDP log alive.
  No UART or partition access added. User's ordinary reboot is not a new flash
  readiness answer. Prepare a verified boot-only candidate before next test.

## First common SIGBUS captured; read-only APFS files return EIO (2026-11-02)

- The ordinary reboot reproduced the common failure at14:46:27Z (~33 minutes
  into boot). VM pressure/compression began first: free about2094 pages,
  compressor753/pageouts31 at the last complete sample; a later partial sample
  shows free2345/wired18089/compressor2374/pageouts39. RAMFS still had2838 free
  pages (~44MiB), so this failure is not its128MiB data quota. Earlier
  mediaserverd NULL-call SIGSEGV is distinct from subsequent common SIGBUS.
- Automatic first-failure collector completed. Freshly executing dynamic probes,
  whyredctl and whyreddiag all die before main in dyld at the same writable
  shared-cache address0x2120e4d80 (unslid0x1e1eb0d80/.27.dylddata offset0xa59cd80).
  Held raw SSH shell PID40269 survives. New server2223 rejects valid auth because
  User root has invalid shell, rejected; auth/key checks stay unchanged.
- Critically, built-in shell reads of /etc/shells and passwd produce no bytes.
  A signed freestanding ARM64 probe (no dyld/imports) was transferred using only
  shell printf into the no-longer-running volatile probe's0755 inode. It starts
  in the broken state, and complete self-file digest matches the reviewed Mac
  binary. This modifies only /var/tmp, not System or any partition.
- Raw syscalls prove: /etc/shells open/fstat succeed, UID0/mode100644/size8,
  but pread=-5(EIO). Passwd has correct2795-byte size and also pread EIO. The
  critical .27.dylddata file has correct189300736-byte size but that offset
  returns EIO. The old .17 literal's512 bytes still read correctly with expected
  FNV d58f5b0f9afadca0. Mounted root block-device open is EBUSY, not a transport
  test. eMMC counters last report0 errors; failures before hardware accounting
  and UBC/page-in failures remain possible. APFS pread uses its custom UBC path;
  these results do NOT prove whether parser, UBC, mapping or transport is root.
- Raw first version CI41466390673/job125348296582 passed all6 steps; zero imports,
  LC_UNIXTHREAD/no dylinker, iOS platform, code signature and ISA checked. Binary
  bf775bbf07bc8023c9b5a83342a954275e4685d65bce6b42ce4983bd8520722e,
  ZIP2f3472292b39b3e9b79387d086524014b1c16b9464f6005995d685bfb23850e9.
  Earlier CI41465971269 failed because static ld omits platform metadata; the
  build now inserts the correct iOS16.4 declaration in reserved padding BEFORE
  signing. No wrong-platform artifact was deployed.
- Some first raw runs hit our own initial-SP alignment bug (exception0x103,
  not the common memory-error0xa). Confirmed via stack address/PC; argument
  padding allowed immediate EIO measurements. Source9e897dea85c8345a8650ae09a47cc02257b3b133
  aligns SP while retaining argv pointer. Its new CI and improved mmap-after-
  pread-error diagnostic are pending. Do not confuse probe alignment with root.
- Next: finish signed raw probe update/read-vs-mmap comparison, then prepare
  narrowly bounded kernel error diagnostics/repair based on actual failure.
  No boot/System flash since v91; v92 is still an unused verified reserve.
  Native collector37864 failed at the common SIGBUS; cold watcher45132 completed.
  Old watcher42713 was superseded for a5-minute pause in new auth requests.
  Kernel UDP receiver47443 and held shell40269 remain; no UART opened.

## Volatile diagnostics verified after user reboot (2026-11-02)

- User confirmed ordinary reboot. Existing USB/host address and UDP receiver
  survived. Installed stays boot89/System91; no partition write or UART access.
  Early pinned root SSH succeeded, and one persistent raw shell remains alive.
- Reviewed v92 helperc1a356f4/server5f1f16a4 were uploaded only into /var/tmp via
  SCP0755 and complete readback equality. On-phone helper self-test, cross-task
  registers/frame reads and VM statistics succeed. A diagnostic server on2223
  uses the existing pinned System host/authorized keys; actual root login passed
  and server messages arrive in the existing USB UDP log. Original2222 job stays.
- Manual mobile-user Bluetooth child prints missing marconi-bt/bluetooth service,
  invalid/missing device address and No bluetooth on this device, then exit0.
  This confirms its missing-device exit branch in that run, consistent with
  stock clean exits; manual execution does not reproduce all launchd context.
  FairPlay mobile child also exits0 but supplies no stderr cause. No DRM or
  anti-debug behavior was bypassed; Bluetooth hardware functionality is unported.
- Current samples show pkd305/suggestd314/lsd96 alive, with task frames captured;
  no new target crash yet in this boot. Initial VM free37944 pages fell to about
  7700, inactive about25500, wired15400; compressor/pageouts/swap stay0. RAMFS
  free about4700/8192 pages. This is not a long-run stability or repair claim.
- Added bounded read-only file/pread/mmap/task-memory probe source
  c99b6ccf444e2c82f600135fc981fa81cb3ab34f. Full CI41462202753/job125333835496
  all7 steps pass. Artifact13149202188 ZIP16978 bytes, SHA256
  48bfefb53548365ace93ca7e85a8405b142f280e0a43ea7add23b5e8f6ba1bc4;
  binaryd47c8b9e1206ae4b4c99f69ec3536fd4e968200047a8a1899ecd732ae34e9b4a.
  Signature/platform/ISA and31 exact16.5 imports pass. Volatile SCP/readback and
  physical baseline pass: root lookup succeeds, the old fault's cache .17
  literal returns identical512 bytes by pread/mmap/remote suggestd VM read, and
  the old writable cache-page address is readable in launchd. No target writes.
- Read-only baseline VM slide0x30234000 is inferred from mach_msg2_trap return
  and validated by identical literal bytes. Local symbol helper's old100-load-
  command cap was raised to1000 (same64KiB bound) for legitimate SpringBoard
  224 commands; Apple/cache bytes and analysis remain private.
- Native collectors: PID37864 samples VM/fs/PIDs/new target stacks until14:47Z;
  persistent shell PID40269/FIFO is retained afterward. PID42713 watches first
  SIGBUS or fresh2223 auth failure until about15:01Z, then reads account/file/
  mapped-page/VM/server evidence through that held shell. Do not duplicate them.
  Private audit/services-volatile-v91/ and monitor JSON hold current status.
  The earlier v91 common SIGBUS and MCM lack of Data Protection remain unresolved.

## Ready v92 reserve; preferred next test is volatile upload after reboot (2026-11-02)

- Detached v92 build completed all offline checks at14:09:00Z. Image size
  5,386,178,560 bytes, SHA256
  0357bf46dc72f5c957ce7725d18952f7d31063e564681734c5d75bd44131bc28.
  All65 exact file comparisons,18 metadata checks and81 sparse full-payload
  reconstruction checks pass. Prior531 launchd jobs/keys are unchanged; only
  the server changes and a diagnostic helper is added relative to v91.
  Packager SHA4850cf670c08d38c5f28654ee3b074d38fcc1135184c62766fec02efb3ed32df.
  No new flash occurred; installed remains boot89/System91. The first,
  interrupted candidate remains quarantined and is not eligible.
- Faster next action: user was asked to reboot normally, or enter Fastboot for
  host reboot only. Once early SSH returns, use the reviewed v92 binaries in
  volatile /private/var/tmp through the existing v91 server. This does not
  require another System write. Do not treat this reboot-readiness answer as
  authorization to flash the reserve v92 image.
- Private detached monitor PID37864/script ssh-v90/observe_volatile_diag.py
  started14:07:15Z, waits for TCP disconnect/reappearance, then opens a single
  pinned persistent raw shell. It uploads/checks complete helper/server bytes,
  starts the diagnostic server on2223 with existing System keys, and samples
  VM/fs/process/thread state. It expires40 minutes after startup. Check status
  audit/services-volatile-v91-monitor.json before starting anything else. It
  was still waiting for physical reboot; no device tool upload or runtime
  helper test has happened. No UART was opened; existing UDP capture continues.
- Additional host-only investigation executes the exact authenticated boot89
  APFS reader core against System91:4096 namespace probes,512 partial shared-
  cache reads,133 cfile resets,33 metadata flushes pass with exact source bytes.
  This does not test device eMMC, kernel UPL or VM pressure behavior and does
  not establish a fix for the common SIGBUS/KERN_MEMORY_ERROR failure.

## V92 userland verified; image write restarted after interruption (2026-11-02)

- Full v92 userland source838c644feea10ac38578c14b8aedadb0e22bab4b,
  CI41454271748/job125307206157: all10 steps success. ZIP SHA256
  f4f12a56ebe68c0ab18970972807c68b7a979aef11777e2b4e31fbe8bbb23adb,
  artifact13144949733/573129 bytes, exact file set/CRC/provenance and every
  binary digest pass. All six binaries match installed16.5 cache imports.
  Dropbear5f1f16a4d23d3019a8cd3a799514f13cc5db7b838bd0be59c3ea7184e31c9ab1;
  diagnostic helperc1a356f4fae9537a0336e2aedef465ed9cf248d2a99c369a3bfa210c18f10fa9.
- Native authentication/transport5 checks and USB mirror512-budget/1023-byte/
  sanitization/errno-preservation checks pass. Separate final sampler workflow
  41454271609 also passes, including VM counter self-test. First full v92 build
  failed because -g created a dSYM directory in the binary inventory; fixed by
  omitting that debug bundle. No failed-build artifact was used.
- First local v92 container write was interrupted before completed file checks.
  /bin/sh comparison reports resource-fork EIO. It is quarantined privately in
  system-v92-interrupted-ineligible with DO-NOT-FLASH marker; no successful
  manifest exists and it must never be flashed. A separate detached build/
  complete verification pipeline restarted; image results remain pending.
- Later v91 USB capture reveals a system-wide SIGBUS/KERN_MEMORY_ERROR storm.
  The first captured SIGBUS is SpringBoard25, followed by xpcproxy737; suggestd
  later faults in strlen/strdup/xpc string creation reading a valid original
  cache string. Shared-cache and private-data ranges both appear. Transport
  counters last showed zero eMMC errors, which does not prove VM/page data
  correctness. This supersedes the earlier short-run absence-of-crash sample.
  Do not claim filesystem encryption or any daemon repair. Root cause remains
  unproven; distinguish VM/resource, pager and file-read failure paths.
- V92 retains boot89/kernel policy and every existing531 launchd job/key. It
  changes the SSH server only to mirror bounded diagnostics and add failure
  errno, plus installs the read-only task/VM helper. A persistent remote shell
  collector is prepared so later diagnostics reuse one authenticated session;
  loopback-only FIFO framing/exit7/next-command tests pass. No new phone write
  or UART access happened. New candidate needs complete offline verification
  before requesting fresh physical Fastboot readiness.

## SSH long-run authentication failure; v92 diagnostics preparing (2026-11-02)

- After several successful read-only sessions, SSH began rejecting the same
  pinned production public key before the signed-key step. Private/public/
  authorized local key correspondence still passes. TCP/host identity and live
  kernel UDP continue. No cause is proven: account lookup, shell checks, file
  lookup/permissions and resource pressure must remain distinct possibilities.
- The standalone sampler passed full CI41452473934/job125300803642 on source
  231eafc514f11e9f71d3bfc6b3309e30c0eb53fc. ZIP SHA256
  0658c018c635099c69e7e20ce4b93f55e83748da5f9e0ed3cb9860284cf65c1f,
  binary d2193f8a6d79b5ac416744bb4d5d832edcba298039ca1495f862509f6c2335fb;
  signature/platform/ISA and37 exact installed-cache imports passed. Upload
  stopped at SSH authentication, before any file payload; no sampler executed
  on the phone. Initial sampler CI failed on unsupported SDK mach_vm.h; the
  reviewed correction uses the supported64-bit vm_read_overwrite interface.
- Latest linked allocations are distributed: /var/mobile43.3MiB,
  /var/db24.9MiB (uuidtext21.8MiB), /var/containers16.8MiB. No new quota-pressure
  or pkd/lsd/suggestd crash was in the captured v91 segment at this checkpoint.
- V92 candidate adds bounded nonblocking SSH-server log mirroring to the same
  fixed USB destination192.168.7.1:5140 and includes the live sampler in System.
  It also records errno on failed account/authorized-key metadata/open lookup.
  Account/shell/ownership/key/signature decisions and job activation are intact.
  Native UDP delivery/budget512/max1023/sanitization/errno tests and all exact
  upstream patch contexts pass. New full userland CI/package are still pending.
  No new flash was performed. UART stays unopened; existing receiver stays alive.

## Live SSH service diagnostics in progress (2026-11-02)

- Boot v89 / System v91 stays installed; no additional partition writes. Root
  SSH and the existing single UDP receiver are working; UART is not opened.
- Across the first roughly half-hour, pkd280/suggestd283/lsd95 retain their PIDs
  and no pkd/lsd crash appears in the v91 capture. This is an observation, not a
  long-run stability claim. Bluetooth/fairplay continue clean status0 exits.
- RAMFS free pages fell from7198 shortly after SSH to2214/8192. The root DSC
  temporary file disappeared; the remaining DSC file is about1.2MB. Thus the
  original bulk DSC footprint is absent at the sample, but other growth still
  needs attribution; global quota128MiB remains unchanged.
- The private MCM log repeatedly reports unsupported class2 Data Protection;
  exact local cache code checks statfs MNT_CPROTECT. RAMFS does not implement
  per-file keys. Do not blindly advertise encryption to hide this error.
- A bounded live thread sampler and mobile-user child runner are being built
  as a standalone signed iOS helper. It reads registers/frames without target
  suspension or memory writes; child diagnostics have a30s timeout. Upload
  through SSH into volatile /var/tmp, not a new System flash. CI/device results
  are pending. Device logs/cache bytes and keys remain outside Git.

## System v91 SSH verified on the phone (2026-11-02)

- Installed is boot v89 / System v91. The user returned the intentionally powered
  off phone to Fastboot and requested continuing the repair. Fresh identity,
  whyred/unlocked/System geometry/download guards passed. All81 sparse sends/
  writes and final reboot returned OKAY; only System was written.
- V91 image5,386,170,368 bytes, SHA256
  fa356fa4a02ae74fff9dbc674336f52f9754560767ee3c88ca3579629905ebe0.
  All64 complete XNU-reader file comparisons,17 metadata checks, obsolete job
  ENOENT and81 sparse coverage/payload checks passed. The v90/v91 caches differ
  only by the SSH job key. Programs/keys and original530 jobs are unchanged.
  Packaging source73572a42a155bc3a8b26911a7d7fd430f95c3065. Existing authenticated
  userland CI41350694615/b5b9b074 source is retained; no new kernel/SDK build.
- V90's /Library/LaunchDaemons job was outside this exact launchd's embedded
  Bootstrap/Paths. V91 uses /System/Library/LaunchDaemons for the standalone job
  and cache key, and the builder validates the actual embedded search paths.
  The old v90 mistake is rejected by a functional negative check.
- The Mac OS administrator confirmation persisted only the exact USB service's
  Manual192.168.7.1/24 without a router. This returned automatically after reboot.
  Actual new kernel UDP logs resumed in the same detached receiver. UART was
  not opened and no second5140 listener was started.
- TCP2222/Dropbear2026.94 was first observed331s after the boot observer started.
  The first20s SSH attempt timed out during startup. Retrying with90s confirmed
  the pinned Ed25519 host and authenticated root, then returned actual Darwin
  22.6.0/iPhone10,3 and uid/gid/euid/egid0. Subsequent full device checks passed
  ps/fs, shell exit7, exact65536-byte binary round trip, unrelated-key rejection
  and reading the server log. Production keys remain local; client timeout90s.
- Read-only SSH health snapshot: RAMFS free6581/8192 pages (~102.8MiB of128MiB).
  DSC files occupy6471680 allocated bytes at this snapshot; a5.25MB temporary
  main-cache file is still being materialized. This does not yet establish the
  physical8MiB cap or long-run quota behavior. PluginKitPlugin data directory
  is absent at this point. Bluetoothd restarted between snapshots; backboardd
  nil URL exception and container ENOENT/EPERM appear in the new UDP log.
  Original pkd/lsd/other daemon and container/RAMFS goals remain open.

## System v90 flashed; initial host address pending (2026-11-02)

- User explicitly authorized adding SSH, then said to flash the concrete System
  v90 candidate. Fresh private-baseline identity, whyred/unlocked, exact System
  geometry6,700,000,256 bytes and512MiB download limit passed. All81 sparse
  sends/writes and final reboot returned OKAY. Installed is boot v89 / System
  v90. Only System was written; no separate format/GPT command was used.
- System image5,386,170,368 bytes, SHA256
  60b6411a89b72d3452e6bcde01e2f6589c48bfa106aa3ad82cb2778dc46a5464.
  XNU APFS reader passed64 complete byte comparisons, including all43 active
  cache members;17 inode/owner/mode checks and all81 sparse payload coverage/
  digest checks passed. Original System v70 and preserved boot v89 hashes pass.
- The baseline catalog45734 files/16600 directories/671 links is reproduced
  without host metadata. Added Dropbear, dash, scp, whyredctl, private local
  host/authorized keys and one job to the530 preserved decoded launchd jobs.
  Keys were generated on the Mac and the host identity pinned; no private keys
  or Apple bytes were uploaded. See WHYRED-SSH.md for exact source/options.
- Corrected userland source b5b9b074c51e3b676928ced77a3b1b7c565ff600,
  CI41350694615/job124979995276 passed all9 steps. Artifact13101197811 ZIP
  SHA256dec89a1c4bddd17ba2f5204f383a50d5fce230938fe2dcc6fae6a97805fe1b18
  passed provenance/digest/length/CRC and all binary digests. All five binaries
  match exact installed16.5 cache exports, with proofs bound to binary SHA256.
  Native key acceptance/rejection, command status,1MiB transport, shell builtins
  and diagnostic commands passed. The earlier41349937032 shell imported absent
  strchrnul; it was rejected and is not installed.
- After the new reboot, the exact-MAC USB Ethernet interface returned. macOS
  DHCP had removed the temporary192.168.7.1 alias; the host now has link-local
  only. A guarded SystemConfiguration helper is ready to persist only this USB
  service's192.168.7.1/24 without a router. OS administrator authentication timed
  out without confirmation. A user question about the OS window is pending.
  Restore this host address before testing SSH or inferring a phone crash.
- The detached USB UDP receiver remains alive and retains its prior private log;
  no second listener or UART was opened. Actual device SSH authentication and
  commands remain UNVERIFIED. pkd/lsd, container/RAMFS behavior and other daemon
  exit causes remain open; installing SSH does not establish their repair.

## V89 USB Ethernet and live kernel logs verified on device (2026-11-01)

- User confirmed Fastboot with UART open. Fresh single private-baseline device,
  whyred/unlocked/boot64MiB and image/kernel/source/version/v87 rollback checks
  passed. Boot send/write and reboot returned OKAY at2026-11-01T20:38:05Z.
  Installed is now boot v89 / System v70; only boot was written.
- UART confirms UTMI TESTMUX release: before35353535/40404040, after00000000,
  release1/changed1. DWC3 receives connect/setup traffic and configures ECM.
  Mac found the exact host MAC02:00:5e:77:ee:01 on en1 after10.078 seconds;
  its USB inventory identifies the1209:0001 whyred Ethernet gadget.
- After OS administrator authentication, the guarded helper added192.168.7.1/24
  as an alias only on that exact interface, retaining its link-local address.
  Ping to192.168.7.2 passed5/5, zero loss,2.118..10.564ms (average5.154ms).
  The Mac receives actual phone kernel WHYRED IO/exec bytes on UDP5140. Thus
  enumeration, bidirectional IP and live kernel-log transport are established.
  SSH is outside this image. Reconnect/long-run throughput remain untested.
- Continued live capture is checking root DSC limits, container creation and
  service exits. Those outcomes are still pending; USB success does not establish
  pkd/lsd/daemon stability. UART remains manual; device dumps/serials/logs private.

## Verified v89 build and package (2026-11-01)

- Final build source `f375a2f8cc2b9b29b08f19357dfdd5606969fcca`, full workflow
  [41346137446](https://github.com/timosid/ios-sweetwhyred/actions/runs/41346137446),
  job124966434206: all27 steps passed. RAMFS1,120,194/28 functions, USB1299/28
  driver functions plus4 board helpers, idle/service799 passed ASan+UBSan.
  Current namespace suite passed75 checks. Legacy0145 regression41346137417/
  job124966245310 passed66 checks/all8 steps after test-generator compatibility
  was retained. Earlier side41346036895 failed before compile on a missing new
  helper in its pinned old source; this was corrected without kernel changes.
- Both final ZIP provenance/SHA256/length/CRC, exact reviewed sources/fixtures/
  board helper bodies, linked ISA/cache/idle instructions and trusted loader/
  ADT/stock-DTB/layout/size guards passed. Combined0167+0168 replay also matches
  the authenticated v87 buildtree. Superseded v88 run41344572473 was cancelled;
  earlier v89 source036f6ad/run41346036804 supplies no candidate artifacts.
- V89 boot:22,331,392 bytes, payload21,520,992; image SHA256
  `668b16beddb2be2765b01ae6137b6bfc6b907072bca66d83a07ab65cf1becea1`.
  It retains the8MiB opt-in, USB snapshots and idle/service diagnostics and was
  flashed as recorded above.
- Next: dsc=1 file-limit/pressure/container/service evidence from continued live
  capture and manual UART. Logd/cache/container behavior remains unverified.

## V89 root DSC cache correction plus USB (2026-11-01)

- New v87 UART proves patch0166's UID-only bound is insufficient at early boot:
  DSC main6199pages plus temporary1970pages are owned by root, total8169 of8192.
  Pressure is data-quota, uid0 and file_limit_hits0. Later UID272 ownership had
  hidden this initialization phase. No pkd/lsd crash is present in this excerpt;
  its early cutoff does not establish service stability. The repeated attachment
  has identical bytes and adds no further outcome evidence.
- Patch0168 classifies /db/uuidtext/dsc entries inside the /var RAMFS, independent
  of owner, at insertion and replacement/swap rename. Classification remains with
  inodes and cache directory identities across moves/aliases/UID changes. Existing
  data is not evicted; the opt-in8MiB per-file bound and UID272 bound both apply.
  Unrelated root files and the global128MiB quota retain their limits.
- Local1,120,194 UBSan checks/28 actual functions plus75 namespace/rename checks
  pass, including root-created cache/temp, unrelated files, owner/namespace races,
  prepopulated directory insertion and actual replacement/swap renames. ARM64,
  noLSE/PAN and exact replay pass. Removing root or swap classification compiles
  then fails functional checks. See WHYRED-RAMFS-PRESSURE.md.
- V89 combines0168 with v88 USB0167, retained idle repair and lifecycle trace.
  Full CI/package/device results pending. Do not flash the USB-only v88 as a
  combined resource fix. Installed remains boot v87 / System v70, UART manual.

## V88 USB override release implementation (2026-11-01)

- V87 boot/System v70 is installed. New manual snapshots show UTMI1 low byte35,
  UTMI2 low byte40: MiCode's FS/non-drive signature plus TEST_MUX. PLL low-byte
  lock remains set; one event processed, raw value not supplied. No exact-MAC
  Ethernet interface appeared in90 seconds. New RAMFS/daemon outcomes pending.
- Patch0167 releases only TEST_MUX after controller halt and before DMA. It
  preserves CTRL1/interface-select/other CTRL2 bits, verifies the byte and restores
  the owned bit on failure. Unknown active override signatures stop safely.
  No new PLL/tune/power/reset/PMIC/TCSR/SMMU writes. See WHYRED-USB.md.
- Local1299 UBSan checks (28 driver/ring functions plus4 board helpers), two ARM64
  units, noLSE/PAN and exact replay pass. Both no-release/interface-clobber negative
  controls compile then fail functional assertions. Full CI/package/device test
  next. Keep the v87 cache bound, idle repair and service trace in the candidate.
- UART remains manual. Boot-only workflow authorized; obtain fresh physical
  Fastboot readiness once the next image is verified. No other partition writes.

## V87 boot flashed; hardware results pending (2026-11-01)

- The user recovered the complete v85 pressure record: data-quota, 8192/8192
  pages, hash8192 and added10657-freed2465=8192; only4 pages are unlinked.
  Rank0 confirms the UID272 DSC file uses6199 pages. This supports the v87
  resource policy; it does not establish a daemon or USB fix.
- After the user said they were rebooting into Fastboot, the exact private
  baseline device appeared. Fresh whyred/unlocked/boot64MiB/image/kernel/source/
  version and v85 rollback checks passed. Boot send/write and reboot returned
  OKAY at2026-11-01T19:54:28Z. Installed is now boot v87 / System v70.
- Read-only Mac Ethernet observation and new manual UART are pending. No other
  partition or host network configuration was changed. New USB snapshots,
  logd-file-limit/pressure and pkd/lsd records are needed. UART stays manual.
- Exact bluetoothd static analysis finds an exit0 path when its UART device
  and both Bluetooth IOKit matches are absent; the runtime branch remains
  unconfirmed. Do not equate clean exits with crashes or claim Bluetooth works.

## V87 cache-bound implementation (2026-11-01)

- New v85 UART recovers the damaged largest TOP row: logd's DSC cache is6199
  pages/96.86MiB,75.7% of128MiB. Its filename matches the main dyld cache UUID.
  TOP ranks are zero-based; the earlier rank1-largest interpretation was wrong.
  Containermanagerd then fails556-byte container metadata writes with ENOSPC.
- Patch0166 adds default-off whyred-logd-file-mb=N (1..64MiB per UID272 regular
  file). Next v87 tries8MiB. Oversized writes/truncate return EFBIG before page
  allocation; locked stores and final owner/size checks cover races/pageout and
  owner transfer. No global quota increase, file deletion or job policy change.
  Physical logd behavior and unified-log symbol availability need testing.
- Local UBSan1120166/25 actual functions,66 namespace checks, ARM64/noLSE/PAN,
  exact replay and functional limit/owner negative controls pass.
  See WHYRED-RAMFS-PRESSURE.md. This is a candidate resource fix; daemon exit
  causes/lsd faults and successful container creation remain open.
- V86 USB snapshot source1ebb9427e50bf5ff0cbd5e641849b7baa38beea7, CI41338911774
  job124944527693: all27 steps passed (USB637/RAMFS1120134/idle799 ASan+UBSan).
  Provenance/digests/CRC, exact sources/fixtures, linked ISA/cache/idle and loader/
  layout guards passed. Image SHAa3f3bd6839ac59ad8c86f8e0707253ba1aa08a46f0d8e4e186042cf7422cd260,
  22,331,392 bytes; unflashed. V87 combines those snapshots with the cache bound.
- Actual service traces: fairplayd.H2/bluetoothd spawn succeeds then exit0;
  suggestd exit2/reason0; pkd SIGABRT same stack; lsd SIGSEGV still reproduces.
  Services remain enabled. No more lost old UART can be supplied. Installed
  installed state is superseded by the v87 flash milestone above.

## Verified v87 image (2026-11-01)

- Build source `0e0895bd4709eb645dde32806d3876a969c86172`, workflow
  [41342210494](https://github.com/timosid/ios-sweetwhyred/actions/runs/41342210494),
  job `124954436881`: all 27 steps passed. RAMFS 1,120,166 checks across 25
  actual functions, USB 637 and idle/service 799 passed ASan+UBSan.
- Both final ZIPs passed source/run provenance, SHA256, byte length and CRC.
  Exact reviewed runtime sources/fixtures, linked ISA/cache/idle instructions,
  trusted loader/ADT/stock DTB, reserved layout and size checks passed.
- Packaged v87 boot is 22,331,392 bytes, payload 21,520,912; SHA256
  `60a583a1c4a2f2d0f89cf436333406f9e970c30c38b5d6d2ab6c660060c66f9e`.
  It enables the 8 MiB per-file UID272 bound and retains USB snapshots, owner
  tracing and idle/service changes. It was flashed as recorded above.
- Physical USB enumeration, logd behavior, container creation and daemon/lsd
  stability remain unverified. No further older UART is required.

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
