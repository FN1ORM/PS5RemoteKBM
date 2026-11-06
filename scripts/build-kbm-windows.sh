#!/usr/bin/env bash
# Run in an MSYS2 MINGW64 shell. Dependencies must already be installed.
set -euo pipefail
cd "$(dirname "$0")/.."
build_dir="${KBM_BUILD_DIR:-build-kbm}"
deploy_dir="${KBM_DEPLOY_DIR:-dist/PS5RemoteKBM-win-x64}"
cmake -S . -B "$build_dir" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCHIAKI_ENABLE_CLI=OFF -DCHIAKI_ENABLE_TESTS=ON \
  -DCHIAKI_ENABLE_STEAMDECK_NATIVE=OFF -DCHIAKI_ENABLE_SETSU=OFF \
  -DCHIAKI_ENABLE_STEAM_SHORTCUT=OFF -DPython_EXECUTABLE="$MINGW_PREFIX/bin/python.exe"
cmake --build "$build_dir" --parallel "${KBM_BUILD_JOBS:-2}"
ctest --test-dir "$build_dir" --output-on-failure
./scripts/deploy-windows-msys2.sh "$deploy_dir" \
  "$build_dir/gui/PS5RemoteKBM.exe" "$build_dir/third-party/cpp-steam-tools" "$MINGW_PREFIX" gui/src/qml
mkdir -p "$deploy_dir"/docs/kbm "$deploy_dir"/licenses
python scripts/build-kbm-manual.py
cp docs/kbm/*.html "$deploy_dir"/docs/kbm/
cp docs/kbm/*.md "$deploy_dir"/docs/kbm/
cp README-KBM.md "$deploy_dir"/README.md
cp COPYING THIRD-PARTY-KBM.md "$deploy_dir"/
cp -r "$MINGW_PREFIX/share/licenses/." "$deploy_dir"/licenses/
for project in curl gf-complete jerasure nanopb; do
  mkdir -p "$deploy_dir/licenses/bundled-$project"
  for notice in "third-party/$project/COPYING" "third-party/$project/LICENSE.txt"; do
    if [[ -f "$notice" ]]; then cp "$notice" "$deploy_dir/licenses/bundled-$project/"; fi
  done
done
pacman -Q > "$deploy_dir"/dependency-versions.txt
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  git rev-parse HEAD > "$deploy_dir"/source-revision.txt
  git submodule status > "$deploy_dir"/submodules.txt
else
  cp SOURCE-PROVENANCE.json "$deploy_dir"/source-provenance.json
fi
cp scripts/build-kbm-windows.sh "$deploy_dir"/
echo 'Deployment complete. Run scripts/package-kbm.py to create archives.'
