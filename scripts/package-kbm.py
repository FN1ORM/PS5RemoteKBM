"""Package exact tracked working source and deployed binaries, excluding secrets/builds."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import zipfile

root = Path(__file__).resolve().parents[1]
out = root / 'dist'
binary = Path(os.environ.get('KBM_DEPLOY_DIR', str(out / 'PS5RemoteKBM-win-x64'))).resolve()
if not binary.is_dir() or out.resolve() not in binary.parents:
    raise ValueError('Deployment must be an existing directory inside dist')
out.mkdir(exist_ok=True)

def git(*args, cwd=root):
    return subprocess.check_output(['git', *args], cwd=cwd)

revision = git('rev-parse', 'HEAD').decode().strip()
submodules = git('submodule', 'status').decode()
manifest = {'source_revision': revision, 'submodules': submodules,
            'upstream_revision': '43728af2e639fecb8a7af1cd68c29820a40de973'}
(binary / 'source-provenance.json').write_text(json.dumps(manifest, indent=2))

dependency_archive = out / 'PS5RemoteKBM-dependency-sources.zip'
with zipfile.ZipFile(dependency_archive, 'w', zipfile.ZIP_STORED) as archive:
    for path in sorted((out / 'dependency-sources').rglob('*')):
        if path.is_file():
            archive.write(path, 'dependency-sources/' + path.relative_to(out / 'dependency-sources').as_posix())

# Keep a corresponding source archive beside the portable application. Include
# every tracked file and the full source of submodules used in this Windows build.
source_archive = out / 'PS5RemoteKBM-source.zip'
with zipfile.ZipFile(source_archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for repo in [root, *[root / p for p in ['third-party/curl', 'third-party/gf-complete',
                 'third-party/jerasure', 'third-party/nanopb', 'test/munit']]]:
        for name in git('ls-files', '-z', cwd=repo).decode().split('\0'):
            if not name:
                continue
            path = repo / name
            if path.is_file():
                archive.write(path, 'PS5RemoteKBM/' + path.relative_to(root).as_posix())
    archive.writestr('PS5RemoteKBM/SOURCE-PROVENANCE.json', json.dumps(manifest, indent=2))

portable = out / 'PS5RemoteKBM-win-x64.zip'
with zipfile.ZipFile(portable, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for path in sorted(binary.rglob('*')):
        if path.is_file():
            archive.write(path, 'PS5RemoteKBM-win-x64/' + path.relative_to(binary).as_posix())

hashes = []
for path in [source_archive, portable, dependency_archive]:
    with path.open('rb') as file:
        digest = hashlib.file_digest(file, 'sha256').hexdigest()
    hashes.append(f'{digest}  {path.name}')
(out / 'SHA256SUMS.txt').write_text('\n'.join(hashes) + '\n')
print('\n'.join(hashes))
