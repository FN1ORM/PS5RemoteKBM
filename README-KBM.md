<!-- Personal client release: 2026.11 -->
# PS5 Remote KBM 0.1.0 — Windows x64

A native keyboard/mouse fork of chiaki-ng, independent of Sony's Windows client.
No virtual controller driver or physical controller is required. Uses the existing
PSN, registration, video, audio and streaming stack with a new Windows Raw Input
mouse-to-stick path. This is an unsigned personal development build.

The owner confirmed cross-city registration and gameplay through the Android VPN
route. Watch only with a local player, Rest Mode wake, and interruption recovery
still need a live console test. Remote-first automatic registration remains
experimental and depends on network compatibility.

## Full manual and November update

Open the [complete user manual](docs/kbm/user-manual.html) offline, or press **F3**
in the app. Its 22 sections cover the Android subnet router, everyday connections,
PSN after registration, all connection methods, home-page labels, quality, aiming,
Rest Mode, local monitor behavior and recovery.

**F4** opens read-only Tailscale peer diagnostics and presets for both PS5 video
profiles. **F2** now offers an optional Precise mouse preset; old input profiles
keep their existing behavior. Popups wrap, scroll and follow the window size.
See [release notes](docs/kbm/release-2026-11-07.md) for tests and limitations.

During a stream, press **Alt+F5** or choose **Watch only** in the **Ctrl+O** stream
menu to keep the picture and sound while blocking laptop controls. Toggle back
with **Alt+F5**, then use **F1** for keyboard and mouse capture.

## Install and run

Extract the entire `PS5RemoteKBM-win-x64.zip` folder. Run `PS5RemoteKBM.exe`; keep
its DLL, `platforms`, `qml` and other subfolders together. No paid software, Sony
Windows application, .NET runtime or ViGEm/vJoy driver is required. The streaming
renderer needs a compatible Intel graphics driver. Start with the default 720p60
H.264 SDR profile. Change quality, codec, resolution/FPS, output device and volume
in chiaki-ng's existing settings. F11 toggles fullscreen and normal aspect-ratio
mode is available. Performance depends on the decoder, renderer and network.

If the Vulkan interface is blank or unstable, launch `PS5RemoteKBM.exe --opengl`
once to persist the alternative renderer. For input-only testing without creating
the video renderer, launch `PS5RemoteKBM.exe --input-test`.

1. Sign into PSN using the app's external-browser flow. Type your password only on
   Sony's browser page. Copy the final redirect URL into the app promptly; it expires.
   Never share the redirect URL, tokens, PIN or configuration backup in chat.
2. Refresh PSN consoles. An unregistered PS5 should now appear with **Automatic
   Registration Available**. Click it to attempt authorized PSN registration.
   Console Remote Play must be enabled and the network/account must permit it.
3. If that fails, use [the routed VPN guide](docs/kbm/vpn-registration.md), then
   **Settings → Consoles → Register New**, PS5 LAN IP (unicast registration),
   and the console's current eight-digit Link Device PIN. The Android phone or another gateway must advertise the PS5 subnet route.
   Simply connecting two devices to Tailscale does not route the PS5 LAN.
4. After registration connect via the PSN entry. Retain manual IP/VPN as an alternate
   path. With live video showing, press **F1** to arm keyboard/mouse control.

## Controls

F1 captures/releases the mouse during a stream. **Esc always releases**.
Alt+Tab, focus loss, minimize, moving/resizing the window, menus and disconnect
release input; press F1 again after returning. Controls remain neutral until armed.
The title bar and stream overlay show capture state. **F2** opens the mapping,
sensitivity and local test panel; it also has a button on the home screen.

| Input | PS5 control |
|---|---|
| W/A/S/D | Left stick, circularly normalized diagonals |
| Relative mouse | Right stick |
| Left/right mouse | R2/L2 |
| Space / C / E / R | Cross / Circle / Square / Triangle |
| Shift / V | L3 / R3 |
| Q / F | L1 / R1 |
| Tab / Enter | Touchpad click / Options |
| Arrows | D-pad |
| P | PS button |
| Ctrl | Walk with partial left-stick tilt |

The mapping panel supports individual keys (including modifiers) and five mouse
buttons; click a key button then press the new key. Use Clear for unbound actions.
Esc/F1/F2/F3/F4 are reserved. Qt treats left/right Shift and Ctrl as the same binding.
Settings save immediately. Save As creates independent named input profiles.
The panel's Test capture mode shows live sticks/buttons/triggers without sending
console reports. Esc releases the test capture. Input profiles are separate from
the upstream video/console profiles.

Mouse settings include separate X/Y sensitivity, invert Y, exponent response curve,
radial deadzone/anti-deadzone, smoothing window, finite recenter ramp, maximum output
and walking tilt. A 125 Hz timer updates input and returns the mouse stick to center;
mouse movement uses counts/second, not accumulated cursor position. The game still
applies its own deadzone, turn-speed limit and aim acceleration. Configure the game's
controller aiming settings as well; this does not turn a PS5 game into native PC aim.

## Security, logs and removal

PSN tokens and console keys/saved PINs use Windows current-user DPAPI, under the
separate `PS5RemoteKBM` settings namespace. Existing Chiaki and KBM2DS settings are
not migrated. Exported settings retain encrypted blobs; another Windows user or PC
cannot decrypt them. Legacy plaintext credential imports are rejected. Sign in and
pair again when moving to a different PC. Settings → PSN logout deletes local tokens;
remove registered consoles separately to delete their local console keys.

Protocol hexdumps are redacted, and token-bearing OAuth account-lookup URLs are
removed from error signals. Logs still can identify networks/consoles; inspect
before sharing. Existing diagnostics provide session progress/errors, decoder,
stream statistics and log access. The F2 panel shows native input state. No telemetry
or custom authentication server was added. Upstream PSN, STUN/network negotiation
and public-account-lookup functions retain their existing external dependencies;
use the authenticated account flow for your own account.

## Build and verify

Source: the adjacent `PS5RemoteKBM-source.zip`, including used submodule source.
Exact third-party source/patch/build-recipe archives are in `dependency-sources`
and `PS5RemoteKBM-dependency-sources.zip`, with SHA-256 manifest. Preserve those
archives, `COPYING`, dependency licences and source when redistributing this build.
See [Windows build instructions](docs/kbm/build.md), [test results](docs/kbm/test-results.md),
[research report](docs/kbm/phase-1-research.md) and [owner acceptance tests](docs/kbm/acceptance.md).

## Troubleshooting and limits

- Blank interface/crash: try `--opengl`, check Intel driver and the full extracted
  folder. Input-only test mode works independently of the video renderer.
- Sony login refuses/redirect expires: use a normal browser, generate a fresh URL,
  paste into the app, and read the error code. Do not paste credentials into chat.
- PS5 not listed: verify PSN account, console's Remote Play setting and refresh.
  Manual IP registration remains available. Sign-in alone does not authorize pairing.
- PSN connection timeout: NAT/CGNAT/UDP policy may prevent negotiation. Retry once,
  then use the verified VPN/manual IP path. Registration success does not guarantee
  connectivity from every ISP. Do not disable Windows Firewall to diagnose this.
- No control response: ensure video is focused, F1 capture is shown, and the F2 test
  behaves correctly. Existing chiaki-ng keyboard bindings do not drive Windows
  streaming input in this fork. Releases intentionally require rearming.
- Choppy stream/high CPU: lower bitrate, try 720p30, try a compatible hardware
  decoder or software fallback, and compare actual statistics. No 720p60 benchmark
  or hardware-decoder compatibility is claimed yet.
- Lost connection: input releases. Existing chiaki-ng network recovery remains;
  some failures need a manual reconnect. Reliable audio/video recovery is unverified.

The binary currently includes the MSYS2 FFmpeg dependency set, so the extracted
folder is larger than a minimal custom codec build. Local startup memory is reported
in the test results; streaming CPU/RAM/GPU use has not been measured. Microphone,
virtual controllers and Sony binary reverse engineering are not required for the
implemented path. See the original upstream README in the full source archive for
unmodified chiaki-ng functionality and authors.
