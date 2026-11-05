# Local test results — 2026-11-05

**End-to-end PS5 acceptance remains pending.** The owner deferred PSN sign-in and
console testing. No account credentials were requested in chat and no live PS5
session was observed. A successful build does not establish the project's final
cross-city gameplay criterion.

## Completed checks

| Check | Observed result |
|---|---|
| Unchanged chiaki-ng Windows x64 baseline | Compiled and linked before custom integration. Baseline CTest passed. |
| Customized Windows x64 client | GCC Release build succeeded; real PE application plus deployed dependencies. |
| Upstream tests | 122 successful, 1 skipped by upstream (`takion/send_after_receive_error`). This includes protocol, registration payload, FEC and FFmpeg timestamp tests; not PSN/PS5 interoperability. |
| Native input engine | 1,035 checks passed: buttons/triggers, normalized diagonals, opposites, walking, duplicate/shared bindings, invert Y, radial limits, deadzone/anti-deadzone, recentering, reset, invalid config, polling cadence and output sweep. |
| Windows credentials | DPAPI encryption/decryption, plaintext rejection, tamper rejection, on-disk save/reload and deletion passed with synthetic data. |
| Input profiles | All mouse settings and bindings saved/reloaded; separate profiles, invalid input clamping and reserved key protection passed in temporary files. |
| Network diagnostics/privacy | A local loopback server returned HTTP 403 from a synthetic token-bearing URL. The error retained status/code and excluded the secret path/query. Passed. No PSN service was contacted in this test. |
| AV initialization | H.264 and HEVC software decoders opened; Opus decoded packet-loss concealment; SDL opened the actual default output at 48 kHz/stereo. No audio was played. |
| Packaged controls UI | Launched without the build toolchain on PATH; settings, mappings and stick panel displayed. Local mouse capture activated and Esc returned the sticks/state to neutral. |
| Packaged main UI | Home/PSN reminder rendered with `--opengl`; F2 opened the native controls panel. No console was connected. |
| Dependency provenance | 82 exact MSYS2 source archives downloaded, with no failures and no unowned deployed DLLs. Source archives contain upstream sources plus patches/build recipes. |
| Package binary closure | Statically parsed 195 deployed PE executables/DLLs: all x64, no missing non-system imports. This does not replace testing dynamic decoder/graphics behavior. |

Final CTest run: **6/6 suites passed** in approximately one second on this host.
That duration is test execution time, not streaming latency. Build warnings include
upstream FFmpeg Vulkan deprecated APIs and Qt QFile result warnings; no final build
failure remains. `git diff --check` passed.

## Resource observations

Process working set from local snapshots: the input-only panel was approximately
60–75 MiB. The main Vulkan UI was approximately 550–820 MiB during startup/idle
snapshots; the OpenGL UI snapshot was approximately 325 MiB. These are varying
Windows working-set observations, not controlled benchmarks. Other desktop apps
were running; graphics/driver allocations and cache residency affect the results.
The existing local profile was switched to OpenGL for the successful visual check.
No streaming CPU percentage, GPU decoding use, FPS, latency or network bitrate was
measured. Do not infer stable 720p60 from these results.

## Still requires owner/hardware verification

PSN OAuth completion; device listing with the real account; clean-client remote-first
automatic registration; manual eight-digit pairing PIN; VPN route to the remote PS5;
cross-city PSN/VPN connection; real video/audio; hardware decoding compatibility;
keyboard console navigation; in-game mouse aim/triggers; physical mouse behavior;
Alt+Tab/minimize/disconnect release during a stream; Rest Mode wake; reconnect/audio
recovery; 720p30/60 and 1080p quality/performance. Follow `acceptance.md` and record
observed results. No Sony binary/REA protocol findings or gameplay benchmarks exist.
