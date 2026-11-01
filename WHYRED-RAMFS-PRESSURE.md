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
full ARM64 kernel build. A hardware trace is still required to identify the
observed failure branch and validate runtime behavior.
