"""Inspect a completed recovery backup; never print identities or flash contents."""
from pathlib import Path
import hashlib
import json
import sys
from package_release import partitions, ROOT

backup = Path(sys.argv[1]).resolve()
data = backup.read_bytes()
if len(data) != 16 * 1024 * 1024:
    raise SystemExit('Backup is incomplete: expected exactly 16 MiB')
old = partitions(data[0x8000:0x9000])
new = partitions((ROOT / 'release/partitions.bin').read_bytes())
storage = next(p for p in new if p['type'] == 1 and p['subtype'] == 0x82)
region = data[storage['offset']:storage['offset']+storage['size']]
metadata = dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest(),
                old_partitions=old, new_partitions=new,
                partition_layout_matches=old == new,
                new_spiffs_region_erased=all(b == 255 for b in region),
                scope='Local file integrity and partition inspection. Read-transfer MD5 is enforced by esptool; require successful read_flash exit separately. Recovery write has not been tested.')
backup.with_suffix('.json').write_text(json.dumps(metadata, indent=2)+'\n')
backup.with_suffix('.sha256').write_text(f"{metadata['sha256']}  {backup.name}\n")
print(json.dumps(metadata, indent=2))
