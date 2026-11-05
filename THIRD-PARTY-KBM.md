# Third-party notices — PS5 Remote KBM

This application is a modified chiaki-ng build, not an official Sony product.
PlayStation and related marks belong to their owners. Retain all original notices.

| Component | Licence / source |
|---|---|
| chiaki-ng/Chiaki and custom client changes | AGPL-3.0 with the existing OpenSSL additional permission; full text in `COPYING`, original authors retained in source. Upstream `43728af2e639fecb8a7af1cd68c29820a40de973`. |
| keyboard-ds5 | AGPL-3.0-only, tejasbnshetty. Mouse velocity/window/recenter design informed native engine; source reference pinned in research report. No Python runtime or browser component bundled. |
| Qt 6 dynamically linked modules | Applicable Qt LGPLv3/GPLv3 and bundled third-party licences; full licence texts included under `licenses`. Source archive includes Qt's upstream source and MSYS2 patches/build recipes. |
| FFmpeg 9.0.2 MSYS2 build | GPL-3.0-or-later build, including its enabled dependencies. Not advertised as LGPL. Source, build flags and patches in corresponding MSYS2 source archive. |
| libplacebo | LGPL-2.1-or-later; source/build recipe and notices included. |
| SDL2 | zlib; source/build recipe and notices included. |
| Opus | BSD-3-Clause; source/build recipe and notices included. |
| OpenSSL 3.6.5 | Apache-2.0 and applicable bundled notices; exact source archive included. |
| curl (bundled static source) | curl licence; source submodule and chiaki-ng's Schannel AIA patch are included. |
| nanopb | zlib; used source submodule included. |
| Jerasure / GF-complete | Their BSD-style licences; used source submodules and COPYING files included. |
| json-c, miniupnpc, libevent, SpeexDSP | Their BSD/MIT-style licences and third-party notices, included in `licenses` and source archives. |
| Shaderc, SPIRV-Cross, Vulkan loader/headers, GCC runtimes and remaining runtime libraries | Their respective Apache/BSD/MIT/LGPL/GPL-with-runtime-exception licences; installed package licence texts and exact source archives are included. Windows-supplied system DLLs are not redistributed. |

`dependency-versions.txt` records the build environment. `dependency-sources/manifest.json`
records exact source package URLs, versions, SHA-256 hashes and deployed DLL ownership.
That source collection includes upstream source archives, patches and PKGBUILD, not
merely links to moving repository branches. The complete custom client source archive
also includes build/packaging scripts, the patch applied to curl and used submodules.

The application dynamically links Qt and other DLLs: keep compatible names/ABI when
replacing or debugging those libraries. Build instructions permit rebuilding and
relinking the application. No restriction is added on modification or reverse
engineering for debugging library modifications. No proprietary Sony executable,
firmware, credential database or signing key is bundled.

pyremoteplay (GPLv3), PS5Upload and REA (MIT) were research references only; no code
or binaries from those projects are included. UCR, ViGEmBus and vJoy are not dependencies.
Tailscale is an optional external VPN service/client; its own terms/free-plan limits
apply and it is not bundled or required for PSN-assisted sessions.
