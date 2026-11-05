# Phase 1 research and architecture — 2026-11-05

This report precedes custom implementation. Findings below are from code inspection,
not observations of a PS5 session. No account was authenticated and no Sony binary was analyzed.

## Decision table

| Question | Finding and decision |
|---|---|
| Can chiaki-ng register remotely without prior local registration? | Its **library can attempt it**: `gui/src/qmlbackend.cpp:autoRegister()` constructs a session with empty keys, a PSN device UID and `auto_regist=true`. `lib/src/session.c:session_thread_func` obtains hole-punch registration information, passes PIN 0 and RUDP to `chiaki_regist_start`; the registration callback receives fresh keys. However, `updatePsnHostsThread()` filters unregistered names out of `psn_hosts`, `hosts()` marks every PSN entry registered, and `displayServerAt()` assumes stored credentials. Thus its normal remote-only UI does not expose this path on a clean installation. Expose unregistered PS5 PSN entries and use the existing automatic-registration route. Actual firmware/network acceptance remains untested. |
| Can Sony's first-time flow be reproduced? | Existing authenticated PSN negotiation + RUDP registration is a strong implementation candidate; we do not need to invent an authorization protocol. Exact equivalence to Sony, clean-client interoperability, account restrictions and NAT coverage are **unknown until hardware tests**. No claim that PSN sign-in guarantees authorization. |
| Does REA reveal additional mechanisms? | No binary findings. Inspected REA MIT source/docs at `fe677f6cf9853fb12490dfb8ef23ccae3a551049`; it orchestrates analysis engines, it is not a Remote Play implementation. No REA MCP/engine was configured in this session. Since a concrete open-source path exists, installing a reverse-engineering stack or modifying Sony's app would not resolve the immediate UI/input gaps. Revisit only after reproducible PSN registration failures with sanitized logs. |
| Reuse what? | chiaki-ng `43728af2e639fecb8a7af1cd68c29820a40de973`: `psntoken.cpp`, `psnaccountid.cpp` (browser OAuth), `discoverymanager.cpp`, `regist.c` (PIN registration), `remote/holepunch.c`, `remote/rudp.c`, `session.c` (PSN and streaming), `ffmpegdecoder.c`, `streamsession.cpp` (SDL audio/Opus, input feedback), `qmlmainwindow.cpp` (Qt/libplacebo render/UI), existing settings/mapping UI. |
| VPN fallback? | Laptop → Tailscale → remote PC subnet router → PS5 LAN address. Advertise a PS5 `/32` route, approve it and allow it in tailnet policy. Tailscale's default subnet SNAT handles the return path; merely installing a VPN on both PCs does not. Windows IP forwarding and host firewall/routing must be verified. Register by manual IP + current eight-digit PIN with broadcast disabled. LAN broadcast discovery will not cross this routed path. |
| Implement first? | Build unchanged streaming base; independent mouse/input math tests; direct native input integration; safe capture/release; profiles and live diagnostics; Windows DPAPI storage; expose existing PSN registration; package and verify; real-console acceptance last with the owner. |

## Foundation and alternatives

Keep chiaki-ng's Qt 6 native desktop application and C streaming library. A custom
web wrapper, Sony executable or virtual gamepad driver is unnecessary. The controller
integration point is `StreamSession::SendFeedbackState()` →
`chiaki_session_set_controller_state()`. New input is a separate `ChiakiControllerState`
merged with existing sources, avoiding changes to networking/AV packet processing.

The current upstream Windows MINGW64 workflow uses GCC/CMake/Ninja, Qt 6,
libplacebo, FFmpeg, SDL, OpenSSL, Opus, nanopb, Jerasure/GF-complete, curl,
json-c and miniupnpc. Use an isolated per-user MSYS2 toolchain instead of changing
KBM2DS or requiring Visual Studio. Start at 720p60 H.264; hardware decode must be
selected and measured locally. Software decode is a fallback, not a performance promise.

`keyboard-ds5` at `170993e24d9861ef4569f507adf289fda1f8b8f1` is
AGPL-3.0-only. Its `ps5remote/gameinput.py` uses counts/second averaged across a
time window, radial stick limits, anti-deadzone, normalized diagonals and a finite
recenter ramp. Adopt those principles in a native C++ input engine; retain source
attribution where translating algorithms. Its Python/browser/server architecture
is unnecessary for this native client.

`pyremoteplay` at `a127422c2599034ec4428919bbaf8b4db278cd75` is GPLv3;
`register.py` builds PIN-based registration and `session.py` consumes existing
registered profile keys. Its Python AV/PySide/sounddevice dependencies offer no
advantage over the selected native stack. No code copied.

`ps5upload` at `72d0c0c160cafb9ec4d99b31ab6ae80b71a8d658` includes
client Remote Play pairing diagnostics (`engine/crates/ps5upload-core/src/remoteplay.rs`),
but its helper-mediated console operations target jailbroken systems. Do not reuse
its helper, payloads, pairing automation, upload or privileged console paths.
UCR/ViGEm are not dependencies; direct controller feedback eliminates that layer.

## Security and licence decisions

Upstream is AGPLv3 with its documented OpenSSL exception (see `COPYING` and
`COPYING.OpenSSL` if present). Preserve upstream authors, notices and licence files.
Custom additions use the same exception-aware SPDX identifier as upstream.
Ship complete corresponding modified source, submodule revision manifest and build
scripts with binaries, and dependency notices/licences for the deployed libraries.
Do not describe bundled GPL FFmpeg as LGPL. Qt uses its applicable open-source
licences with dynamically linked libraries; retain replacement/build information.

Upstream stores PSN tokens and console registration keys in QSettings unencrypted.
The Windows fork must wrap token, console-key and saved PIN values in current-user
DPAPI before saving. Use a separate application namespace so it cannot silently
import or change an existing Chiaki installation. Never log full tokens/registration
payloads; inspect upstream debug/hexdump paths before shipping. Profile exports
must not produce plaintext secrets. DPAPI blobs are bound to the Windows user;
moving a profile to another computer requires sign-in and registration again.

Registration strategies remain separate: ordinary manual PIN registration, existing
authenticated PSN automatic registration, and manual PIN registration over a VPN
route. No speculative Sony protocol is added. PSN/NAT errors must retain existing
diagnostics and direct users to the VPN path when appropriate.

## History and practical limits

Reviewed recent history including `cc40aea1` / `e5f4491d` (PSN console wake retries),
`a138ab3d` (bounded STUN/wakeup copies), and the current Windows build/deployment
workflows. Preserve those upstream fixes. Registration failures and NAT connectivity
are independent: a registered console may still be unreachable from a particular ISP.
No latency, memory, FPS, clean-account pairing or reconnection benchmark is claimed.

## Primary sources

- [chiaki-ng source](https://github.com/chiaki-ng/chiaki-ng/tree/43728af2e639fecb8a7af1cd68c29820a40de973)
- [Configuration](https://github.com/chiaki-ng/chiaki-ng/blob/main/docs/setup/configuration.md)
- [Remote connection](https://chiaki-ng.github.io/chiaki-ng/setup/remoteconnection/)
- [keyboard-ds5 input implementation](https://github.com/tejasbnshetty/keyboard-ds5/blob/170993e24d9861ef4569f507adf289fda1f8b8f1/ps5remote/gameinput.py)
- [pyremoteplay](https://github.com/ktnrg45/pyremoteplay)
- [PS5Upload client pairing model](https://github.com/phantomptr/ps5upload/blob/72d0c0c160cafb9ec4d99b31ab6ae80b71a8d658/engine/crates/ps5upload-core/src/remoteplay.rs)
- [REA](https://github.com/morluto/rea)
- [Tailscale subnet router setup and SNAT](https://tailscale.com/docs/features/subnet-routers?tab=windows)
