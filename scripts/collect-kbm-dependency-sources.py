"""Acquire exact MSYS2 source archives for deployed runtimes, including build recipes."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
import sys
import urllib.request
import time

root = Path(__file__).resolve().parents[1]
msys = Path(sys.argv[1])
deploy = root / 'dist/PS5RemoteKBM-win-x64'
sources = root / 'dist/dependency-sources'
sources.mkdir(exist_ok=True, parents=True)
needed = {p.name.lower() for p in deploy.rglob('*.dll')}
packages = {}
for entry in (msys / 'var/lib/pacman/local').iterdir():
    if not (entry / 'desc').is_file():
        continue
    lines = (entry / 'desc').read_text().split('\n')
    fields = {}
    for n, line in enumerate(lines):
        if line.startswith('%') and n + 1 < len(lines):
            fields[line.strip('%')] = lines[n + 1]
    if not (entry / 'files').is_file():
        continue
    files = (entry / 'files').read_text().splitlines()
    provided = {Path(f).name.lower() for f in files if f.lower().endswith('.dll')}
    if provided & needed:
        base = fields.get('BASE', fields['NAME'])
        previous = set(packages.get(base, {}).get('dlls', []))
        packages[base] = {'base': base, 'version': fields['VERSION'],
                          'dlls': sorted(previous | (provided & needed))}

def acquire(item):
    # MSYS2 source archives include upstream source, patches and PKGBUILD.
    filename = f"{item['base']}-{item['version']}.src.tar.zst"
    url = 'https://repo.msys2.org/mingw/sources/' + filename
    target = sources / filename
    if not target.exists():
        for attempt in range(3):
            try:
                request = urllib.request.Request(url, headers={'User-Agent': 'PS5RemoteKBM-source-packaging/1'})
                with urllib.request.urlopen(request, timeout=90) as response, target.with_suffix('.partial').open('wb') as output:
                    while chunk := response.read(1024*1024):
                        output.write(chunk)
                target.with_suffix('.partial').replace(target)
                break
            except Exception:
                if attempt == 2:
                    raise
                time.sleep(1)
    with target.open('rb') as file:
        digest = hashlib.file_digest(file, 'sha256').hexdigest()
    return {**item, 'url': url, 'file': filename, 'sha256': digest, 'bytes': target.stat().st_size}

records, failures = [], []
with ThreadPoolExecutor(max_workers=4) as pool:
    tasks = {pool.submit(acquire, item): item for item in packages.values()}
    for task in as_completed(tasks):
        try:
            record = task.result()
            records.append(record)
            print(f"OK {record['file']} ({record['bytes']} bytes)", flush=True)
        except Exception as error:
            failures.append({'package': tasks[task], 'error': str(error)})
            print(f"FAILED {tasks[task]['base']}: {error}", flush=True)
covered = {dll for item in records for dll in item['dlls']}
missing = sorted(needed - covered)
manifest = {'archives': sorted(records, key=lambda r: r['base']), 'failures': failures, 'unowned_dlls': missing}
(sources / 'manifest.json').write_text(json.dumps(manifest, indent=2))
print(f'{len(records)} complete source archives; {len(failures)} failures', flush=True)
if failures or missing:
    print('Unowned DLLs:', missing)
    sys.exit(1)
