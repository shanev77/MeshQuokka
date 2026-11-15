"""Create an isolated, reviewable GitHub source folder from the audited source ZIP."""
from pathlib import Path
import hashlib
import json
import re
import zipfile
from package_source import main as package_source, OUT, ROOT

package_source()
version = re.search(r'0\.1\.0_\d{6}[a-z]', (ROOT/'platformio.local.ini').read_text()).group()
target = ROOT.parent / ('MeshQuokka-GitHub-' + version)
if target.exists():
    raise SystemExit('Destination already exists; preserve it and choose a new staging name.')
with zipfile.ZipFile(OUT) as z:
    for member in z.infolist():
        path = Path(member.filename)
        if path.is_absolute() or '..' in path.parts:
            raise ValueError('Unsafe archive path')
        if any(p in {'.git', '.pio', 'private-backups', '.venv', '.platformio', 'Documents'} for p in path.parts):
            raise ValueError('Private, cache or reference-document path in source archive')
    z.extractall(target)
repo = target / 'MeshQuokka'
files = [p for p in repo.rglob('*') if p.is_file()]
manifest = {p.relative_to(repo).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(target/'SOURCE-FILES-SHA256.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'GitHub repository root: {repo}\nFiles: {len(files)}')
