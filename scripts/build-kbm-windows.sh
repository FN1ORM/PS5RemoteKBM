#!/usr/bin/env bash
# Run in an MSYS2 MINGW64 shell. Dependencies must already be installed.
set -euo pipefail
cd "$(dirname "$0")/.."
build_dir="${KBM_BUILD_DIR:-build-kbm}"
cmake -S . -B "$build_dir" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCHIAKI_ENABLE_CLI=OFF -DCHIAKI_ENABLE_TESTS=ON \
  -DCHIAKI_ENABLE_STEAMDECK_NATIVE=OFF -DCHIAKI_ENABLE_SETSU=OFF \
  -DCHIAKI_ENABLE_STEAM_SHORTCUT=OFF -DPython_EXECUTABLE="$MINGW_PREFIX/bin/python.exe"
cmake --build "$build_dir" --parallel "${KBM_BUILD_JOBS:-2}"
ctest --test-dir "$build_dir" --output-on-failure
./scripts/deploy-windows-msys2.sh dist/PS5RemoteKBM-win-x64 \
  "$build_dir/gui/PS5RemoteKBM.exe" "$build_dir/third-party/cpp-steam-tools" "$MINGW_PREFIX" gui/src/qml
mkdir -p dist/PS5RemoteKBM-win-x64/docs/kbm dist/PS5RemoteKBM-win-x64/licenses
cp docs/kbm/*.md dist/PS5RemoteKBM-win-x64/docs/kbm/
cp README-KBM.md dist/PS5RemoteKBM-win-x64/README.md
cp COPYING THIRD-PARTY-KBM.md dist/PS5RemoteKBM-win-x64/
cp -r "$MINGW_PREFIX/share/licenses/." dist/PS5RemoteKBM-win-x64/licenses/
for project in curl gf-complete jerasure nanopb; do
  mkdir -p "dist/PS5RemoteKBM-win-x64/licenses/bundled-$project"
  for notice in "third-party/$project/COPYING" "third-party/$project/LICENSE.txt"; do
    if [[ -f "$notice" ]]; then cp "$notice" "dist/PS5RemoteKBM-win-x64/licenses/bundled-$project/"; fi
  done
done
pacman -Q > dist/PS5RemoteKBM-win-x64/dependency-versions.txt
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  git rev-parse HEAD > dist/PS5RemoteKBM-win-x64/source-revision.txt
  git submodule status > dist/PS5RemoteKBM-win-x64/submodules.txt
else
  cp SOURCE-PROVENANCE.json dist/PS5RemoteKBM-win-x64/source-provenance.json
fi
cp scripts/build-kbm-windows.sh dist/PS5RemoteKBM-win-x64/
echo 'Deployment complete. Run scripts/package-kbm.py to create archives.'
