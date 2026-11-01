# Volatile RAMFS pressure diagnostics

Patch 0158 adds `whyred-ramfs-trace=1` to development/debug kernels. It is off by
default. It diagnoses failed writes to the volatile filesystem; it does not
increase quotas, change allocation flags, alter permissions or substitute syscall
results. Existing container syscall tracing can remain enabled for path context.

`WHYRED RAMFS PRESSURE` has three lines sharing an ID. The cause is the branch
that returned the existing error:

| Cause | Existing result | Meaning |
| --- | --- | --- |
| `data-quota` | ENOSPC | A new file page would exceed the mount page quota. |
| `data-page` | ENOSPC | A new page is needed, the quota has room, and the caller has no spare page. `kr` records its VM allocation result. |
| `slots-nowait` | ENOSPC | The nonwaiting file page-pointer-array allocation failed. |
| `getxattr-nowait` | ENOMEM | The nonwaiting attribute-value snapshot allocation failed. |
| `listxattr-nowait` | ENOMEM | The nonwaiting attribute-name snapshot allocation failed. |

If both quota and spare-page availability fail, `data-quota` takes precedence.
`kr` is meaningful for `data-page` only. For slots, `length` is the requested
pointer-array size and `off` is the start of the indexed page. For other causes
they describe the data operation or attribute-copy size.

The record includes used/limit/peak file pages, nodes, xattr bytes, cumulative
file-page additions/frees and zero-link vnode reclamations. `hash_pages` sums
file pages on all nodes while holding the mount lock. `unlinked_pages` and
`unlinked_nodes` count nodes with zero links. Open-unlinked files legitimately
retain data until reclaim; these counts alone do not establish a leak.
Global free/wired VM page counts are approximate samples, not a coherent VM
snapshot. An allocation failure does not by itself identify global RAM exhaustion.

There are at most 32 records per mount. The first 16 failures are recorded, then
power-of-two failure numbers. Cause counters continue after the output budget is
exhausted. The bounded hash walk happens only when producing a record, under the
existing mount lock. A scalar snapshot is copied before unlocking; printing is
outside that lock and performs no path lookup or file-data copy. Pageout keeps
its nonwaiting allocation behavior. No node/vnode pointer is used by the printer.

`tools/test-whyred-ramfs-pressure.py` extracts the actual declarations and 21
functions into a host fixture. It injects each allocation failure, exercises
quota-limited short writes and residual counts, verifies default-off behavior,
fills the output budget with 1.1 million failures and churns 10,000 file lifetimes.
It checks truncate, open-unlinked retention, named-vnode reclaim, attribute error
propagation and balanced page counters. Host locks/allocators/uio/vnodes are
adapters; kernel scheduling, UBC, memory pressure and VFS reference counts are
not emulated. CI also runs the existing namespace/directory/sync tests and the
full ARM64 kernel build. These checks do not emulate runtime behavior; the
hardware observation below identifies the sampled failure branch.

## Observed v81 quota failure (2026-11-01)

The user's photograph shows `pkd` SIGABRT, a container-manager write returning
ENOSPC, and pressure record 1024 with `cause=data-quota`, `pages=8192`,
`limit=8192` and `hash_pages=8192`. With this kernel's 16 KiB pages, that is the
128 MiB mount quota. The additions/frees in the same record are 9623/1431,
whose difference is also 8192. The failure branch is now observed on hardware.

Zero-link nodes account for 67 pages, approximately 1.05 MiB. Most of the sampled
usage belongs to linked nodes. These coherent counters do not rule out semantic
retention or excessive writes. The largest files, writers and full causal chain
to each daemon's termination remain unknown. Approximate global VM samples do
not establish a safe larger quota. The next measurement is a bounded snapshot
of the largest resident file owners/names; no quota or return-value change has
been made from this observation.

## Bounded resident-owner follow-up

Patch 0161 adds `whyred-ramfs-top=1`, also requiring `whyred-ramfs-trace=1`.
It extends sampled quota failures with the four largest resident file-page
owners, sorted by pages and then inode. Each record contains copied inode,
allocated pages, logical size, link count, UID, writable-mapping flag, one name
and the parent directory's name/inode. The failed caller and last writer are
not inferred from a file's UID. Hard links yield one sampled alias; no full
canonical path is promised. Zero-link owners and unresolved names remain marked
by link count/zero parent, rather than a guessed name.

The snapshot lives under the existing mount lock. The top-four selection shares
the existing hash walk; all name searches share a 65,536-entry budget. Only fixed
metadata and bounded character arrays leave the lock. No file content is read,
no VFS path lookup/allocation is performed and no node/vnode pointer is retained
by the printer. Control, quote and backslash bytes become `?`; name and directory
limits are 47/31 bytes with truncation flags. A name-budget flag discloses an
incomplete search. The event is under 1 KiB in the host ABI check. Each selected
record adds at most five lines to the existing bounded 32-record policy.

Local UBSan passes 1,120,134 checks; the wake-independent namespace suite retains
all 66 passing cases. Ownership tests cover ties, unlinked retention, truncated
and control-byte names, printing after node reclamation, the entry-walk budget
and default-off/other-failure behavior. The disabled-collector negative control
fails. Full [CI 41325125813](https://github.com/timosid/ios-sweetwhyred/actions/runs/41325125813)
passed all 26 steps, including these 1,120,134 checks with ASan+UBSan. Physical
owner output remains pending. No quota, allocation,
error-return, namespace or service-restart policy changes are included.
