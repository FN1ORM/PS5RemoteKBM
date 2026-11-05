# Build Windows x64

The initial unchanged upstream application compiled successfully on Windows before
native input integration. The customized build uses the same source tree and CMake
targets. This development session used MSYS2 MINGW64 GCC 16.2, CMake 4.4.5, Ninja
1.13.2, Qt 6.12.0, FFmpeg 9.0.2, libplacebo 7.387.0 and OpenSSL 3.6.5. Consult the
packaged `dependency-versions.txt` for the complete environment.

Install an ordinary [MSYS2 x64 toolchain](https://www.msys2.org/), or extract its
official self-extracting base to a user-writable directory. Run MINGW64 bash, update
with `pacman -Syu`, restart if the core runtime upgrade requests it, and install:

```bash
pacman -S --needed git make diffutils unzip zip \
  mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-pkgconf mingw-w64-x86_64-python mingw-w64-x86_64-python-protobuf \
  mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-declarative mingw-w64-x86_64-qt6-svg \
  mingw-w64-x86_64-libplacebo mingw-w64-x86_64-ffmpeg mingw-w64-x86_64-SDL2 \
  mingw-w64-x86_64-openssl mingw-w64-x86_64-opus mingw-w64-x86_64-json-c \
  mingw-w64-x86_64-miniupnpc mingw-w64-x86_64-speexdsp mingw-w64-x86_64-fftw \
  mingw-w64-x86_64-curl mingw-w64-x86_64-libevent mingw-w64-x86_64-fast_float
```

From a Git checkout run `git submodule update --init --recursive`. The packaged
source ZIP already contains the sources of the five submodules used by this build.
Steam shortcuts and Steam Deck-only input are disabled, so cpp-steam-tools and
the Android/Switch submodules are not needed. No HID driver or hidapi is needed
with that feature set. The build intentionally excludes QtWebEngine; PSN login
uses the normal external browser.

```bash
cd /path/to/PS5RemoteKBM
bash scripts/build-kbm-windows.sh
```

Default parallelism is 2 to limit build memory; set `KBM_BUILD_JOBS` if appropriate.
The script configures, builds, runs tests and deploys the DLL/QML dependencies.
Use `KBM_BUILD_DIR=build-baseline` to reuse this session's existing build tree.
The historical directory name does not mean its current contents are unchanged;
the final artifact is the custom build.

For a compiler-only check before packaging:

```bash
cmake --build build-kbm --parallel 2
ctest --test-dir build-kbm --output-on-failure
```

The frozen source dependency archives permit rebuilding the exact dependency
versions with MSYS2 `makepkg-mingw` and the included PKGBUILDs. They are distinct
from installing whatever versions currently exist on MSYS2's rolling mirrors.
To refresh packaging source acquisition from the toolchain's package database:

```powershell
python scripts\collect-kbm-dependency-sources.py C:\path\to\msys64
python scripts\package-kbm.py
```

Keep source archives and licence notices beside the portable ZIP when distributing.
`package-kbm.py` packages tracked custom source plus full used submodule source;
commit/add intended source before running it. It excludes user credentials, registry
settings, build trees, research clones and logs. Archives and executables have a
SHA-256 manifest. This session's toolchain lives outside KBM2DS; that project was
not modified. An unsigned personal build does not require disabling Windows security.
