# Automatic CPU display candidate

This is an offline System100 candidate descended from the verified System95
image. The phone remains on boot98/System95 and is disconnected. A visible
capture-to-panel Apple logo was confirmed on98c; continuous new-frame output,
changing scenes and a working SpringBoard interface are not established.

## Image contents

The candidate adds `/usr/local/whyred/bin/whyred-display`, the optional manual
`whyred-display-wake` tool, and
`/System/Library/LaunchDaemons/org.whyred.display.plist`. It adds the same job to
the embedded `/System/Library/xpc/launchd.plist` cache. All531 existing jobs,
including disabled bluetoothd/FairPlay, are retained. Existing SSH files and keys
must compare byte-for-byte with95; no keys or images belong in this repository.

The display job runs `whyred-display live` as mobile in the system bootstrap,
with a30s restart throttle. Every iteration obtains a new real LCD capture into
new IOSurface storage, queues it through the existing guarded AppleCLCD ABI and
releases the userspace reference. The kernel owns queued references. It does not
rewrite a surface still being presented. Each iteration has an autorelease pool;
three consecutive failures exit unsuccessfully. A stuck capture terminates after
45s so launchd can restart the process. Updates are limited to at most1fps;
actual speed must be measured on the phone.

The job writes `/private/var/mobile/Library/Logs/whyred-display.log`, whose parent
is seeded501:501 in RAMFS. The process limits regular files to1MiB and ignores
SIGXFSZ, allowing frame updates to continue after the journal cap. Sparse records
include frame/error/change counts, pixel hash and capture time. This is separate
from the Mac's ongoing UDP console log. No permanent display power override is
added; the wake executable is installed for explicit manual tests only.

## Required device pilot

1. Test the already packaged boot99 storage candidate with System95 first.
   The new bounded FIFO-ready recheck has full CI/host proofs, but no hardware
   result yet. Confirm the exact99 kernel UUID in a fresh SSH session.
2. Upload the exact signed live, wake and diagnostic binaries and read them back
   byte-for-byte. Do not reuse an outstanding98 SSH command/session.
3. Run the finite `live-window` as mobile using the explicit
   `whyreddiag run-mobile-display /absolute/executable live-window` command.
   Its fixed90s parent limit accommodates the40s pilot and final bounded capture.
   General `run-mobile` remains30s; the extended mode rejects continuous `live`.
4. Require normal child exit, at least3 clean fresh captures, no errors, and
   review memory before/after, measured capture latency and USB/storage records.
   A static logo can produce equal hashes; it does not establish changing-frame
   delivery. Verify a real scene change and visible output separately. Do not
   treat the private pilot receipt as an automatic deployment approval.
5. Only after the finite test and continued memory/visible checks pass, deploy
   the enabled System image. Retain the verified95 image and boot98 rollback.

## Offline validation

`tools/prepare-whyred-display-image.py --self-test` checks preservation of the
existing job cache and rejects conflicting jobs or changed service policy. The
full builder authenticates the prior images, overlay sources, signed tool
reports/rights/platform/ISA and embedded bootstrap paths. It writes no device.

New file bytes and metadata are checked through the independent read-only XNU
APFS core. `tools/whyred-display-image-compare.c` compares all old files, symlinks
and metadata, allowing exactly3 additions and a single explicit cache-content
exception. Only the two expected parent directory counts may increase. It rejects
deletions, unexpected additions, changed old bytes/modes and wrong new modes.
The image must fit the existing System partition; no partition layout changes.

Offline checks cannot establish capture timing, bounded surface lifetime on the
device, competition with stock swaps, or boot-time launchd behavior. The enabled
image remains a candidate until those measurements pass.
