# Minimal hardware boot experiment (System102)

This opt-in profile targets unsupported hardware in the current whyred port. It
retains the verified System101 automatic display client and its finite128MiB
native D22 budget. Boot99 and all USB/SSH files stay unchanged. No daemon binaries
are deleted, and no fake Mach/XPC servers are installed.

`tools/prepare-whyred-display-image.py --hardware-profile minimal-hardware
--display-memory-mb 128` adds this profile to the existing verified-image build
arguments. Without the hardware option, System100/101 behavior is unchanged.
The option prepares host files only; it does not flash or modify a running phone.

## Exact scope

`tools/whyred_boot_profile.py` owns the explicit19-job list:

- Wi-Fi: `wifid`, `wifi.hostapd`, `wifi.wapic`, `wifiFirmwareLoader`,
  `wifip2pd`, `wifivelocityd`, `wifianalyticsd`.
- Bluetooth helpers: `bluetoothuserd`, `BluetoothUIService`.
- Radio coexistence: `WirelessRadioManager`.
- NFC: `nfcd`.
- Camera hardware: `applecamerad`, `dietapplecamerad`, `dietappleh13camerad`,
  `ptpcamerad`.
- Other hardware helpers: `gpsd`, `accessibility.motiontrackingd`,
  `DumpBasebandCrash`, and wireless diagnostics `awdd`.

These are exact Apple labels prefixed with `com.apple.`. Native filenames are
explicit: the Bluetooth UI label's filename uses lowercase `bluetoothuiservice`.
The original `bluetoothd` and `fairplayd.H2` remain disabled as in System95/101;
`companioncamerad` also remains in its original disabled state.

For each selected job, only `Disabled=true` is set in BOTH its native plist and
the embedded launchd cache. Every endpoint, launch event, executable, entitlement,
activation condition and other property is preserved independently in each copy.
The builder rejects absent, mislabelled, mismatched or already-disabled baseline
entries and preserves all532 jobs and all other cache fields.

SpringBoard, backboardd, LaunchServices/pkd, MobileGestaltHelper, container and
installation services, UserEventAgent, keychain/security, logging, mediaserverd,
locationd and CommCenter API providers remain unchanged. Disabling a hardware
job does not implement its IPC protocol: callers can still fail or wait. This
is a reversible hardware experiment, not a guarantee that clients are unaffected.

## Observations and limits

The installed101 process snapshot actually contained `bluetoothuserd`, `nfcd`,
`wifianalyticsd`, `WirelessRadioManagerd` and `awdd`. Camera daemons and several
Wi-Fi helpers were absent in that snapshot; their removal cannot be credited
with present CPU savings. A subsequent read-only CPU sample timed out before
its first result, so there is no measured CPU reduction. The phone was not
reconfigured by the profiler.

Boot99 single-block PIO cold reads are approximately1.3MiB/s. Reducing loaded
binaries and dependencies may help, but this profile does not fix that storage
bottleneck or the separate SpringBoard startup wait. No speedup is established
until a comparable cold boot is measured on the phone.

## Verification and acceptance

Run `python3 tools/test_whyred_boot_profile.py` and the display builder's retained
`--self-test`. The independent C image comparator accepts the19 extra content
exceptions only with BOTH `--minimal-hardware` and `--display-memory-policy`.
All old metadata, symlinks and other file bytes remain strict. The builder checks
every replacement's exact bytes and metadata separately. The C/Python lists are
cross-checked by a conservation test.

Before calling an image ready to test, complete the full image comparison and
sparse reconstruction. Retain verified101 raw rollback and existing boot/system
backups. Installation requires a concrete image and the user's Fastboot readiness.
After installation, compare the same cold-boot milestones: first SSH access,
first automatic frame, read bytes/time, RAMFS pressure and service/crash records.
Verify the19 jobs stay absent, SSH/USB logs continue and automatic changing-scene
output survives. Check SpringBoard and its existing startup wait for regression.
Do not claim normal UI readiness or faster boot from an offline plan alone.

## Completed image comparison,2026-11-04

System102 raw image is5386178560 bytes, SHA256
`6e6851e0f9789de382d38bfa3a113030bfb47b05969ce2ce68397295b958a912`.
Independent comparison against the verified95 base passed63022 old nodes,
45744 files,16607 directories,671 symlinks and8613999393 unchanged logical bytes.
There are exactly3 additions and21 explicit content exceptions: the cache,
display budget and19 native hardware jobs. All24 overlay byte/metadata checks
passed separately. Direct comparison against the actual101 cache confirms just
19 Disabled fields changed and all513 other cached jobs remain equal.

The image retains the previously CI-built and physically tested display binary;
no new kernel or iOS executable was needed. Offline image validation establishes
content preservation, not launchd behavior or startup performance on the phone.

All81 sparse packets subsequently passed independent header/range/checksum
validation and exact payload reconstruction of the same image SHA256. The
verified101 raw rollback remains retained. System102 has not yet been installed;
actual service absence, UI dependencies and boot-speed impact remain untested.

The guarded installer's final files-only preflight passed with exit0: exact102
image and81 packets, working101/95 rollback and retained boot99 authenticated.
Its flash mode rejects missing fresh Fastboot readiness. No phone writes occurred.
