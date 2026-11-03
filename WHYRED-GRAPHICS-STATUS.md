# whyred graphics: measured boundary and next experiment

Updated 2026-11-03 Minsk. Installed baseline: boot v97 / System v95, iOS 20F66.
The phone has returned. After a Mac crash, passive USB logging was restored and
an ordinary phone reboot was requested to recover SSH. No partition was written.
Graphics is still unresolved: the user sees console text and occasional black
clears with the backlight on.

## Actual physical results

| Path | Observed result | What it establishes |
| --- | --- | --- |
| CALayer into CPU bitmap | Exact 192 red / 64 green pixels | Simple CPU layer drawing works. |
| Same tree into own IOSurface | Pixels survive unlock/relock | Own CPU surface mapping works. |
| Two-process IOSurface IPC | All 512 shared bytes pass in both directions | Tested cross-process surface delivery works on whyred. |
| EAGL API 2 and 3 | Neither context created | No demonstrated GL renderer. |
| CoreImage software option | NULL indirect callback, also seen in mediaserverd | This particular fallback is unusable. |
| Entitled named LCD marker | Exactly 1,024 red marker pixels reach capture | Server CPU composition and remote context routing work for this test. |
| Entitled LCD PNG capture | White Apple boot logo on black, 1125 by 2436 | The server has a real boot scene. This is not a working SpringBoard interface. |
| Stock framebuffer swaps | Surface 3 contains zero RGB and alpha and is copied to the panel | Captured server scene and stock scanout contents differ. |

Earlier captures and marker tests lacked the QuartzCore displayable-context and
global-capture entitlements. Their transparent results were superseded by the
entitled test; they do not establish a broken or empty compositor. The temporary
marker was invalidated. No invented GPU capabilities or fake interface were added.

The named LCD marker changed RGB pixel count from 25,430 to 26,454, including
all 1,024 marker pixels. The successful PNG is a private audit artifact and is
not committed. Stock panel source measurements were zero, while a later panel
measurement also contained console text. No exact timestamp correlation between
every user-observed black clear and those swaps has been established.

## Current presentation experiment

`whyred-render-probe present-once` captures the actual LCD scene into its own
IOSurface and submits it once through our existing AppleCLCD user client. It
checks the exact installed v97 kernel UUID, display ID 0 and logical size
1125 by 2436 before calling the custom swap selectors. The driver retains the
queued surface. It changes neither daemon configuration nor renderer flags.

This is a single-frame test of capture-to-panel delivery, not a continuous
rendering solution. A successful submission alone will not establish visible
panel output or a repaired interface. Require driver source/panel measurements
and the user's actual screen observation.

The initial iOS build failed because `mach_task_self` depended on a transitive
header available only in the desktop build. An explicit Mach header fixes it.
Source `b03952a947f529f7f0b009535e221d2efdf76b47` passed every step of CI
[41653213351](https://github.com/timosid/ios-sweetwhyred/actions/runs/41653213351).
Artifact digest, length, ZIP CRC, report source, binary digest, iOS platform,
ISA, strict signature and the exact two-entitlement set were checked locally.
The signed binary is 93,760 bytes, SHA256
`9b0a77d5394de0987aa0313ee3a0d63578b9d1318ebd37e4f00aec4640fb7091`.
Physical execution is pending restored SSH after the requested reboot.

## SpringBoard startup is a separate remaining blocker

The preserved 97c main-thread sample waits through CoreServices while
ControlCenterServices resolves `initWithBundleIdentifier:allowPlaceholder:error:`.
Another thread makes a synchronous NSXPC request with the exact selector
`getServerStoreBlockingWithCompletionHandler:`. The main thread waits on the
corresponding CoreServices store lock. These selectors were decoded from the
exact cache stubs, not inferred from a distant nearest exported symbol.

This identifies a LaunchServices database-store acquisition wait. It does not
yet identify why the server has not supplied that store, prove a permanent
seeding loop, or establish a new lsd crash. Fresh lsd/SpringBoard samples and
store/connection state are the next read-only checks after panel delivery.

## Read-only server inspection

Determine the task's actual cache identity and slide before interpreting a
read. All addresses below are canonical addresses in the preserved 20F66 cache,
not live addresses or instructions to write memory.

In physical run 97c, the actual LCD had two render clients, nonzero root layer
IDs/cached roots and a constructed CPU renderer. Its concrete server vtable was
`0x1d5b45c68`; render-to-target first attempts GPU setup and has a verified CPU
fallback. Thus an empty LCD list is ruled out for that snapshot.

The capture request 40221 reaches display-server event 20 and a renderer with
vtable `0x1d5b3d618`, allocation `0x4fe0`. Capture acceptance alone is insufficient:
the event marks the request handled before checking render-client count, and a
zero-client path can return without drawing. The entitled pixel result, rather
than API acceptance alone, establishes drawing in the successful physical test.

Display inventory entries visited by `_detectDisplays` are display types, not
three established rendering engines. The factory's LCD/TVOut choice uses type
bit 8 at display `+0x9f2`; this is not a software-renderer switch.
`beginExternalUpdate:usingSoftwareRenderer:` changes bit `0x80` at display
`+0x251` through the concrete external-update method. That fact does not by
itself establish a persistent software-compositor configuration.

Stock backboardd initially passes true for
`kCAWindowServerDisableUpdatesOnMainDisplay`, then sets renderer flags to 3.
Overlay handling later enables and disables updates. The actual live gate has
not been read. Do not blindly clear it or call an external-update method solely
because its selector mentions software rendering.

## Earlier MobileGestalt sample

The saved lsd return chain identifies an IONetworkController match whose
callback accepts `sdio`, `wlan` or `marconi-wifi`. That caller supplies a five
second run-loop timeout. The outer semaphore wait synchronizes the worker; it
is not evidence of an infinite query. The missing ADT product/options nodes
were not the target of that sampled match. Do not fabricate a Wi-Fi or Apple GPU
service to satisfy this observation.

Private task snapshots, disassembly, PNGs, logs, keys and downloaded Apple bytes
remain outside Git. Exact source/CI/artifact history is retained in NOTES.md.
