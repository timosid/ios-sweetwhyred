# SSH over the verified USB Ethernet link

The user authorized adding SSH after v89 established USB Ethernet and kernel
UDP logs. Installed remains boot v89 / System v70 while preparation runs.
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
dependencies and absence of detected LSE instructions. Physical SSH login and
the final installation image are not yet verified.
