"""Copy the editable application into MeshCore's compile tree, then verify bytes."""
from pathlib import Path
import shutil
root = Path(__file__).resolve().parents[1]
source = root / 'vendor/meshdeck/firmware'
dest = root / 'firmware/examples/meshdeck'
count = 0
for path in source.rglob('*'):
    if not path.is_file():
        continue
    target = dest / path.relative_to(source)
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.exists() or target.read_bytes() != path.read_bytes():
        shutil.copy2(path, target)
    assert target.read_bytes() == path.read_bytes(), target
    count += 1
shutil.copy2(root / 'platformio.local.ini', root / 'firmware/platformio.local.ini')
assert (root / 'platformio.local.ini').read_bytes() == (root / 'firmware/platformio.local.ini').read_bytes()
print(f'Verified {count} application files and build configuration')
