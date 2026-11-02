# SSH over the verified USB Ethernet link

The user authorized adding SSH after v89 established USB Ethernet and kernel
UDP logs. Installed is now boot v89 / System v90 after the explicit System
installation approval and successful81-part transfer/reboot.
Neither sshd nor /bin/sh exists in the preserved System filesystem, so this
requires userland programs and a launchd job. The SSH package is separate from
the successful USB kernel changes; it does not claim to fix daemon exit causes.

The candidate builds [Dropbear2026.94](https://github.com/mkj/dropbear/releases/tag/DROPBEAR_2026.94)
and dash0.5.13.5 from pinned upstream sources. The Dropbear release archive is
checked against its published SHA256. Dash uses upstream commit
037bbdfd330017c368caf6242f977974123239b5 through the kernel Gitiles mirror.
Its regenerated archive metadata varies, so every member's path, type, mode,
link target and complete content are authenticated by a canonical tree digest.
No source files or executable modes can vary without failing that digest.

The server is configured for192.168.7.2:2222 with public-key authentication;
password/PAM, forwarding and SFTP support are omitted. Darwin lacks setresgid,
so Dropbear's optional newer early privilege-drop mode is disabled while its
ordinary authenticated session UID handling remains enabled. Normal account,
shell, authorized-key ownership and signature checks remain in place. The local
Mac test runs unprivileged and accepts only its isolated test key. Test keys are
excluded from CI artifacts. Production client/host keys will be local-only.

`whyredctl` provides ps, fs, du, ls, stat, cat, readlink, uname and id. It reports
syscall errors directly. The shell supplies commands, pipes and redirection;
SFTP is outside the first package. The existing USB kernel-log receiver remains
separate and running throughout preparation.

Local native tests pass key acceptance/rejection, command exit status,
1MiB binary round trip, dash expansion/builtins and uname/fs/ps/ls diagnostics.
The ARM64 build uses the official iPhoneOS SDK, minimum iOS16.4 and generic+crypto
without outline atomics. CI verifies its Mach-O platform, code signatures,
dependencies and absence of detected LSE instructions. The final System image is verified and flashed; physical SSH login is pending
restoration of the Mac USB address through OS administrator authentication.

The first iOS build passed CI but its shell imported strchrnul, absent from the
exact installed16.5 cache. The iOS configure cache now selects dash's portable
implementation. Every packaged binary must pass an export-trie comparison with
that installed cache, bound to its binary SHA256, before image construction.

The corrected source b5b9b074c51e3b676928ced77a3b1b7c565ff600 passed all9
steps in CI41350694615/job124979995276. Artifact13101197811 has ZIP SHA256
dec89a1c4bddd17ba2f5204f383a50d5fce230938fe2dcc6fae6a97805fe1b18.
Provenance, complete archive CRC, native authentication tests and the exact16.5
import comparison pass for all five binaries. Offline System image preparation
adds one job to the existing530 and checks the baseline45734 files/16600
directories/671 symlinks. The image passed64 complete file comparisons and17 metadata checks. Device SSH
remains untested while the Mac USB address is link-local only.
