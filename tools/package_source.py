"""Package rebuildable project sources and notices, excluding private/device data."""
from pathlib import Path
import hashlib
import os
import zipfile

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'release/MeshQuokka-Source.zip'
EXCLUDED = {'.git', '.pio', '__pycache__', 'private-backups', 'build', 'release',
            'node_modules', '.venv', '.platformio', '.cache', 'Documents'}

def main():
    count = 0
    with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as archive:
        for directory, children, files in os.walk(ROOT):
            children[:] = sorted(d for d in children if d not in EXCLUDED)
            for name in sorted(files):
                path = Path(directory) / name
                if path.suffix in {'.log', '.pyc', '.o', '.elf'} or path.is_symlink() or name == 'compiler_probe.cpp':
                    continue
                archive.write(path, Path('MeshQuokka') / path.relative_to(ROOT))
                count += 1
    with zipfile.ZipFile(OUT) as archive:
        assert archive.testzip() is None
        assert not any('private-backups' in name.split('/') for name in archive.namelist())
    digest = hashlib.sha256(OUT.read_bytes()).hexdigest()
    OUT.with_suffix('.zip.sha256').write_text(f'{digest}  {OUT.name}\n')
    print(f'{count} files, {OUT.stat().st_size} bytes, SHA256 {digest}')

if __name__ == '__main__':
    main()
