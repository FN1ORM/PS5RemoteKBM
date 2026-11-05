"""Read PE import tables without loading code; verify x64 and runtime closure."""
from pathlib import Path
import json
import os
import struct

root = Path(__file__).resolve().parents[1]
deploy = root / 'dist/PS5RemoteKBM-win-x64'
system = Path(os.environ.get('WINDIR', 'C:/Windows')) / 'System32'
root_dlls = {p.name.lower() for p in deploy.glob('*.dll')}
system_dlls = {p.name.lower() for p in system.glob('*.dll')}
errors, modules = [], []

def pe_imports(path):
    data = path.read_bytes()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    if data[pe:pe+4] != b'PE\0\0':
        raise ValueError('invalid PE signature')
    machine, count = struct.unpack_from('<HH', data, pe+4)
    optional_size = struct.unpack_from('<H', data, pe+20)[0]
    optional = pe+24
    if machine != 0x8664 or struct.unpack_from('<H', data, optional)[0] != 0x20b:
        raise ValueError('not a Windows x64 PE image')
    sections = []
    for n in range(count):
        at = optional+optional_size+n*40
        virtual_size, rva, size, raw = struct.unpack_from('<IIII', data, at+8)
        sections.append((rva, max(virtual_size,size), raw))
    def offset(rva):
        for begin,size,raw in sections:
            if begin <= rva < begin+size:
                return raw+rva-begin
        raise ValueError('unmapped RVA')
    import_rva = struct.unpack_from('<I', data, optional+112+8)[0]
    if not import_rva:
        return []
    at = offset(import_rva)
    imports = []
    while data[at:at+20] != bytes(20):
        name = struct.unpack_from('<I', data, at+12)[0]
        begin = offset(name)
        imports.append(data[begin:data.index(b'\0',begin)].decode('ascii').lower())
        at += 20
    return imports

for path in sorted(deploy.rglob('*')):
    if path.suffix.lower() not in {'.dll','.exe'}:
        continue
    try:
        dependencies = pe_imports(path)
        local = {p.name.lower() for p in path.parent.glob('*.dll')}
        missing = [name for name in dependencies if name not in root_dlls | local | system_dlls
                   and not name.startswith(('api-ms-win-','ext-ms-win-'))]
        if missing:
            errors.append({'module':str(path.relative_to(deploy)), 'missing':missing})
        modules.append({'module':str(path.relative_to(deploy)), 'imports':dependencies})
    except Exception as error:
        errors.append({'module':str(path.relative_to(deploy)), 'error':str(error)})

result = {'modules':len(modules), 'errors':errors, 'details':modules}
(root / 'artifacts/package-verification.json').write_text(json.dumps(result,indent=2))
print(f"Checked {len(modules)} x64 PE modules; {len(errors)} dependency/format errors")
if errors:
    print(json.dumps(errors,indent=2))
    raise SystemExit(1)
