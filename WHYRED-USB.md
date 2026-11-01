# whyred USB Ethernet and console candidate

Patch 0159 ports the existing polled DWC3 CDC-ECM gadget to whyred. The first
candidate includes Ethernet and a best-effort kernel console stream. It does
not install SSH or require changes to the installed System v70 image.

The driver is off by default. `whyred-usb=1` takes a read-only handoff snapshot;
`whyred-usb=2` attempts the gadget. Startup runs once after `net_init_run`, before
root discovery, independently of UFS initialization. It never starts the UFS
driver or persistent log writer. eMMC/APFS read-only policy is retained.

## Hardware basis and limits

The active board device tree and Xiaomi's
[whyred-o-oss source](https://github.com/MiCode/Xiaomi_Kernel_OpenSource/tree/efbbb9cce47475c49f28d227b2a52eca6f0d02a7)
agree on the controller and legacy PHY. Clock offsets come from
[gcc-sdm660.c](https://github.com/MiCode/Xiaomi_Kernel_OpenSource/blob/efbbb9cce47475c49f28d227b2a52eca6f0d02a7/drivers/clk/qcom/gcc-sdm660.c),
PHY power/lock from
[phy-msm-qusb.c](https://github.com/MiCode/Xiaomi_Kernel_OpenSource/blob/efbbb9cce47475c49f28d227b2a52eca6f0d02a7/drivers/usb/phy/phy-msm-qusb.c),
and the USB2 PIPE mux/VBUS fields from
[dwc3-msm.c](https://github.com/MiCode/Xiaomi_Kernel_OpenSource/blob/efbbb9cce47475c49f28d227b2a52eca6f0d02a7/drivers/usb/dwc3/dwc3-msm.c).

| Component | Whyred value |
| --- | --- |
| DWC3 | `0x0a800000` |
| QSCRATCH | `0x0a8f8800` |
| QUSB2 major 1 | `0x0c012000`, size `0x180` |
| PHY power / PLL status | `0xb4` / `0x38`, lock bit 5 |
| GCC USB30 GDSCR / reset | `0xf004` / `0xf000` from GCC `0x00100000` |
| Master / sleep / UTMI clocks | `0xf008` / `0xf00c` / `0xf010` |
| Config-NoC / aggre2 / AHB2PHY | `0x5018` / `0xf03c` / `0x6a004` |

This initial port takes over the bootloader's powered, calibrated PHY. It
requires GDSC on, reset deasserted and a locked, unclamped PHY. Cold PHY power-up,
PMIC supplies and clock-source/PLL programming are not implemented. Mode 2 can
vote the six confirmed branch enable bits with bounded polls; preflight failure
undoes only votes introduced by that attempt. It does not read gated DWC3/PHY
blocks or copy another SoC's SMMU SID/configuration. DMA below 4 GiB assumes the
bootloader's direct USB DMA path; physical DMA visibility still needs testing.

`WHYRED USB: handoff=N` reports 0 ready, 2 map failure, 3 power domain off,
4 reset asserted, 5 clock unavailable, 6 PHY not ready, or 7 invalid/held-reset
core. The snapshot includes the clock and PHY values. A failed warm handoff
stops before gadget startup; setting the boot flag alone does not prove a link.

The gadget halts the bootloader controller before installing its own DMA area.
TRBs and buffers use separate 128-byte cache-maintenance granules. Transfer
ownership, command timeouts/status, resource indices, RX lengths and event-ring
counts are checked. Failure stops the controller; DMA memory remains pinned
for the boot, including failed startup, rather than being freed while a device
could still access it. There is no automatic retry/reinitialization loop.

USB2 full/high-speed configuration descriptors and packet sizes are selected
from the negotiated speed. Data endpoints run only on data-interface alternate
setting 1. Reset/disconnect/alternate-setting changes retire transfers and free
queued packets. Host requests outside the implemented standard/ECM subset stall;
this is an experimental gadget, not a complete USB class implementation.

## Host connection

The phone interface is `ue0`, `192.168.7.2/24`. The shared development host MAC is
`02:00:5e:77:ee:01`; the phone uses the same prefix ending in `02`. These are test
identities, not handset serials. VID/PID remain the inherited `1209:0001` test ID.

After booting an explicitly enabled candidate on macOS:

```sh
python3 tools/whyred-usbnet.py --connect --log whyred-usb.log
```

Without `--connect` the helper only lists matching interfaces. The connection
command waits up to 30 seconds for one exact MAC match, assigns only that
interface `192.168.7.1/24` with `ifconfig` (sudo may be needed), then receives
UDP port 5140 bound to the USB address. Ambiguous matches or an already-used
USB subnet stop the helper. It changes no DNS/default-route settings. Received
data must come from `192.168.7.2`; the optional local log is created mode 0600.
Ctrl-C stops the receiver. Keep raw logs private.

The memory console ring is replayed from up to 64 KiB before the log thread's
start, subject to its 4 MiB retention. At most eight 1400-byte chunks are sent
per 100 ms interval, with nonblocking socket calls. Failed/short sends keep the
cursor for retry. UDP, queue drops, ring overwrite and concurrent console writes
can still lose bytes; this is not a lossless or panic-safe transport. An earlier
failure before the gadget starts still needs another capture path.

## Validation status

Local checks execute 27 actual kernel functions plus the board helper, with
modeled MMIO, mbufs, sockets and thread waits. They cover default-off/read-only
behavior, every clock and rollback, PHY/core rejection, cache-granule separation,
descriptors/control stages, FS/HS, data/filter gating, reset/reconnect, queue
cleanup, stale DMA ownership, timeouts, bounded event counts and UDP cursor retry.
The macOS interface parser has separate selection/ambiguity checks. Local UBSan
passes 565 checks. Both changed ARM64 compilation units pass with zero LSE/PAN
instructions. Exact patch replay matches all three reviewed files. Full CI requires ASan+UBSan and
all retained kernel checks. No physical enumeration, ping, USB console delivery,
cache coherence, SMMU behavior or host-driver compatibility is established yet.
