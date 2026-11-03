# whyred graphics: measured boundary and next experiment

Updated 2026-11-03. Installed baseline: boot v97 / System v95, iOS 20F66.
The user has taken the phone; current work is offline. No new boot or System
image is a verified graphics fix.

## Measured boundary

| Path | Actual phone result | Implication |
| --- | --- | --- |
| CALayer into CPU bitmap | Exact 192 red / 64 green pixels | CPU layer drawing works for this simple tree. |
| Same tree into own IOSurface | Exact pixels survive unlock/relock | CPU surface mapping and preservation work for this test. |
| EAGL API 2 and 3 | Neither context created | No demonstrated GL renderer. |
| CoreImage with software option | NULL indirect callback; reproduced in mediaserverd | This option is not a working fallback on this iOS path. |
| Server display capture | Request accepted, all pixels transparent | Failure precedes panel copying; acceptance does not prove rendering. |
| Temporary remote CAContext marker | Context created, captures remain transparent | Even a simple submitted remote tree has not reached the captured output. The original marker used display ID 0, whereas capture names LCD; their equivalence is unverified. |

The marker context was invalidated. Its result is not a working stock interface.
Empty capture does not distinguish context routing, server state and renderer
initialization by itself. Panel changes alone cannot resolve that uncertainty.

## Offline QuartzCore findings

All addresses below are **canonical cache addresses for the preserved 20F66
arm64 cache**, not addresses to write to a running process. Determine that task's
actual cache identity and slide before interpreting any later read.

`-[CAWindowServer _detectDisplays]` at `0x1884da264` visits exactly three entries.
Count functions at `0x1d5b428d0` and factories at `0x1d5b428b0` are:

| Entry | Count | Factory | Underlying type |
| --- | --- | --- | --- |
| 0 | `0x1883a639c` | `0x1883a60a0` | 0 |
| 1 | `0x1883a6460` | `0x1883a63a4` | 1 |
| 2 | `0x1883a65b0` | `0x1883a6468` | 2 |

The shared count routine filters the display inventory by its type field.
These are display types, **not three proven rendering engines**. The factory
calls the display object's virtual slot `0x5f8` to create a display server.
Types 0 and 1 share factory `0x1883a3320`. It chooses the `LCD` or `TVOut` name
using bit 8 of the display's field at `+0x9f2`. The base display constructor
sets that bit from the display type; this is **not a software-renderer bit**.

The previously identified `beginExternalUpdate:usingSoftwareRenderer:` reaches
virtual slot `0x58`. For the concrete server vtable `0x1d5b35a98`, implementation
`0x1884f7d38` sets/clears bit `0x80` of the underlying display field at `+0x251`.
The base vtable's implementation is a no-op. This explains one state mutation;
it does not establish a supported persistent software-compositor configuration.
Do not call it on the live window server just because of its selector name.

`setRendererFlags:` ORs its argument with 3 and propagates it to display
servers. No additional flags have been decoded as a working software switch.
The CPU software callback table and destination setter exist, but their presence
does not establish that the window server selects them or can present their output.

## The sampled MobileGestalt wait is identified

The saved lsd stack contains the return chain
`0x19c54d6e4 -> 0x19c54d910 -> 0x19c54d320 -> 0x19c547214`.
This identifies the actual caller, rather than attributing a large offset to
the nearest exported symbol (`MGSetAnswer`).

That caller requests an `IONetworkController` match. Its callback accepts
registry names `sdio`, `wlan` or `marconi-wifi`; other names are skipped.
The helper uses `MobileGestaltRunLoopMode`, with exact imported calls
`CFRunLoopGetCurrent`, `CFRunLoopAddCommonMode` and `CFRunLoopRunInMode`.
This caller supplies a **5-second run-loop timeout**. The outer semaphore wait
synchronizes the worker; it is not evidence that this sampled query waits
forever. Repeated callbacks or queries still require measurement.

The ADT lacks `/product` and `/options`, but those omissions are **not the
identified match in this lsd sample**. Do not add a fictitious Wi-Fi service or
copy Apple GPU capabilities into the device tree to satisfy this observation.
No causal link from this wait to the empty compositor capture is established.

## Concrete next check when the phone returns

The signed read-only `whyreddiag` helper from source
`4ead866d66553908e9f02f314441489832c9db48` is prepared locally. CI run
[41531195903](https://github.com/timosid/ios-sweetwhyred/actions/runs/41531195903)
passed every step. Its binary is 72,960 bytes, SHA256
`a9ebd0d26b2f36a34ee28b8b53c4cafe457661a8d9e28e77f9a813fd10993c5c`.
It has not been uploaded or run on the phone.

1. Verify running kernel UUID, current task PIDs and cache slide. Do not reuse
   old PIDs or assume all tasks have an unchanged slide after a reboot.
2. Read the window-server global at canonical `0x1dcf3c160` in backboardd.
   A missing server is a different failure from a server with an unusable backend.
3. Follow the actual object and concrete vtable using bounded read-only regions;
   compare the discovered factory path with the tables above. Treat racing or
   unmapped reads as unavailable evidence; never turn them into writes.
4. Run the prepared `remote-layer-lcd` comparison only on the returned iOS target.
   It uses the same LCD name as capture and reports the context ID. Compare with
   the retained ID-0 command; neither has yet demonstrated visible output.
5. Relate that backend and display identity to an accepted but empty capture
   before designing a renderer change. Require a known marker to reach capture,
   then stock UI content and physical presentation, as separate success criteria.

Private evidence, task snapshots, disassembly and downloaded Apple/cache bytes
remain outside this repository. The current observer ended with SSH exit 255
around the user taking the phone; that ending is not a newly diagnosed kernel fault
or a completed long-duration stability test.
