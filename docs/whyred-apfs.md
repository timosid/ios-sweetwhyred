# Read-only APFS port — patch 0135

The last handset-validated image is v49: read-only GPT/BSD block registration,
then the existing mockfs test PID 1 from md0. v50 is built and not flashed.
No iOS filesystem has been written to system or another data partition.

Patch 0135 adds an APFS format reader, VFS registration, vnode operations,
compressed-file decoders and a bounded BSD strategy transport to
xnu-8796.141.3. The source is adapted from HakiOS commit
`88513176b1e96ecf0b764cd62c6f4e6ac71345e3`; original Git object hashes were
checked against GitHub, including both decoder dependency trees. APSL and MIT
notices are retained. No HakiOS UFS, resident UEFI, platform driver, write/store
module, scheduler hook or thread pool is imported.

The mount is always read only. Root selection requires `whyred-apfs-root=1`;
without it APFS declines mountroot and the current md0/mockfs path continues.
The block reader checks alignment, device/partition capacity, errors and short
I/O. The port uses the existing 512-byte BSD block units and integer-only
compilation. Metadata cache is reduced to 256 blocks; I/O buffers are 64 KiB
with two retained buffers. Vnode creation is serialized to avoid duplicate
vnodes for the same inode during concurrent lookups.

Reader fixes made during this port: container-size multiplication overflow,
bounded checkpoint scanning, revalidation of selected checkpoint geometry and
incompatible features, strict zlib completion, file-size error propagation and
signed-size limits, and pagein offset overflow. Encryption, snapshots and an
authenticated external sealed-root hash are not supported.

## Checks completed locally

- Patch replay reproduces every candidate file. The five APFS C units and
  vfs_conf.c compile with v49 CI headers/flags. Local LLVM needs metadata
  builtin shims for the two VFS units; these local objects are diagnostics,
  not production kernel inputs. Apple-clang CI must confirm the real link.
- The new objects have no unresolved symbols when combined with the v49
  kernel's symbols. All five APFS objects have zero LSE and PAN instructions.
- A native host executable built from the exact C parser reads a synthetic
  APFS image: directory entries, symlink, uncompressed data, inline zlib,
  resource-fork zlib, unaligned reads across compressed-piece boundaries, EOF,
  and injected device failure. Seven corrupt/truncated image variants are
  rejected, including checksummed checkpoints with invalid geometry.
- A 4 MiB diagnostic APFS root image can be built on Windows with the Python
  builder. It contains the project's existing test PID 1 at /sbin/launchd,
  directories for devfs/var, and a marker. It contains no Apple iOS userland.

## CI and hardware boundary

The workflow now runs the APFS source guard before compiling, the host parser
cases after linking, and builds a small diagnostic APFS root beside the kernel.
Full CI and real-kernel mount/exec results must be recorded before claiming a
working whyred APFS root. No phone operation is part of this source work.

This does not yet install iOS launchd, dyld, backboardd or SpringBoard. A full
iOS root still requires local extraction of the IPSW/cryptex inputs, a writable
runtime /private/var solution, and subsequent userspace/platform adaptation.
