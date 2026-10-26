# Whyred CI migration to macOS 15

The workflow uses macos-15 with Xcode 16.4. GitHub's macOS 14 brownout on 2026-10-26 prevented earlier jobs from obtaining a runner; full retirement is scheduled for November 2.

Run 40346688944 obtained a macOS 15 runner, then compilation failed. Its buildtree contains DriverKit-427 IIG output using IORPC.kernelContent, absent from xnu-8796.

The IIG adapter preserves the older RPC ABI by replaying Xcode 15 generated sources from successful run 40334160445. The cache records the artifact provenance and checks 40 source inputs plus every generated output digest. Local validation reproduced 34 output files for 17 interfaces byte-for-byte and rejected changed input. The user authorized publishing this cache and adapter.

The follow-up workflow saves build.log in its artifact. A successful full build is still required before packaging a new boot image.
