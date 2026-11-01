## Current whyred work (2026-11-01, RAMFS pressure candidate)

- Boot v79 and System v70 are the inherited hardware baseline. Filesystem
  tracing reproduced container metadata write ENOSPC; the quota versus
  allocation/reclamation cause is unresolved. A working graphical scene is
  unconfirmed. USB Ethernet/SSH work remains deferred.
- Patch 0158 adds default-off development diagnostics for data quota, missing
  data pages, nonwaiting page-array allocations and attribute snapshot failures.
  It retains the existing quotas, allocation flags, operation results and storage
  write policy. See `WHYRED-RAMFS-PRESSURE.md` for record semantics and limits.
- Local host checks pass with UBSan: five injected failure branches, short-write
  residuals, open-unlinked and named-vnode reclamation, 1.1 million failures for
  the output budget and 10,000 allocation/reclaim lifetimes. Existing 66 namespace,
  readdir64 and sync checks pass on the candidate. The changed C unit compiles to
  ARM64 using the authenticated v79 generated headers. Full CI is pending.
- Local AddressSanitizer cannot run in the current host environment: even a
  standalone hello-world sanitizer binary exits with SIGILL before main. This
  is not a passing ASan result. The full CI check requires ASan and UBSan on the
  supported runner. No candidate image has been flashed or hardware-tested.
- Next: complete full CI and authenticated boot-only packaging, then obtain the
  new RAMFS pressure records during an explicitly authorized hardware test.
  Private artifacts, UART and transfer verification remain outside this repo.
