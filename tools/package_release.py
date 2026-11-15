"""Validate actual PlatformIO outputs, merge via esptool, and write a release manifest."""
from pathlib import Path
import hashlib, json, shutil, struct, subprocess, sys, re

ROOT = Path(__file__).resolve().parents[1]
WORKSPACE = ROOT.parent

def partitions(data):
    result=[]
    for at in range(0,len(data),32):
        record=data[at:at+32]
        if len(record)<32: raise ValueError('Truncated partition record')
        magic=struct.unpack_from('<H',record)[0]
        if magic==0xFFFF: break
        if magic==0xEBEB:
            if record[16:] != hashlib.md5(data[:at]).digest():
                raise ValueError('Partition table MD5 mismatch')
            continue
        if magic!=0x50AA: raise ValueError('Invalid partition table magic')
        _,kind,subtype,offset,size,label,flags=struct.unpack('<HBBII16sI',record)
        if not size or offset+size>16*1024*1024: raise ValueError('Partition outside flash')
        if any(offset<p['offset']+p['size'] and p['offset']<offset+size for p in result):
            raise ValueError('Overlapping partitions')
        result.append(dict(type=kind,subtype=subtype,offset=offset,size=size,
                           label=label.split(b'\0')[0].decode('ascii'),flags=flags))
    if not result: raise ValueError('No partitions found')
    return result

def resolve_build_path(path):
    # build.ps1's temporary drive maps precisely to WORKSPACE.
    p=Path(path)
    if p.exists(): return p
    if len(path)>3 and path[1]==':': return WORKSPACE/path[3:]
    raise ValueError(f'Build input missing: {path}')

def package(environment='MeshCore_Enhanced_TDeck_Plus'):
    version=re.search(r'0\.1\.0_\d{6}[a-z]',(ROOT/'platformio.local.ini').read_text()).group()
    build=ROOT/'firmware/.pio/build'/environment
    if version.encode() not in (build/'firmware.bin').read_bytes():
        raise ValueError('Build version does not match current source; rebuild before packaging')
    layout=json.loads((build/'flash-layout.json').read_text())
    assert layout['chip']=='esp32s3' and layout['flash_size']=='16MB'
    parts=partitions((build/'partitions.bin').read_bytes())
    app=next(p for p in parts if p['type']==0 and p['subtype']==0x10)
    images=sorted(layout['images'],key=lambda i:i['offset'])
    release=ROOT/'release'/('gps9600' if environment.endswith('GPS9600') else '')
    release.mkdir(parents=True,exist_ok=True)
    for index,item in enumerate(images):
        src=resolve_build_path(item['path']);data=src.read_bytes()
        if not data: raise ValueError(f'Empty image: {src}')
        if index and images[index-1]['offset']+images[index-1]['size']>item['offset']:
            raise ValueError('Overlapping firmware images')
        if item['name'] in ('bootloader.bin','firmware.bin') and data[0]!=0xE9:
            raise ValueError('Not an ESP image')
        if item['name']=='firmware.bin' and (item['offset']!=app['offset'] or len(data)>app['size']):
            raise ValueError('Application does not fit selected partition')
        target=release/item['name'];shutil.copy2(src,target)
        item.update(path=item['name'],size=len(data),sha256=hashlib.sha256(data).hexdigest())
    esp=WORKSPACE/'.platformio/packages/tool-esptoolpy/esptool.py'
    command=[sys.executable,str(esp),'--chip','esp32s3','merge_bin','-o',str(release/'merged-firmware.bin'),
             '--flash_mode','keep','--flash_freq','keep','--flash_size','keep']
    for item in images:command.extend([hex(item['offset']),str(release/item['name'])])
    subprocess.run(command,check=True)
    merged=(release/'merged-firmware.bin').read_bytes()
    manifest=dict(name='MeshQuokka',version=version,status='development-hardware-unvalidated',
        chipFamily='ESP32-S3',environment=environment,flash_mode=layout['flash_mode'],flash_frequency='80m',
        flash_size='16MB',images=images,partitions=parts,
        merged=dict(path='merged-firmware.bin',offset=images[0]['offset'],size=len(merged),sha256=hashlib.sha256(merged).hexdigest()),
        builds=[dict(chipFamily='ESP32-S3',parts=[dict(path=i['name'],offset=i['offset']) for i in images])],
        warning='Back up ALL original flash before installation. Merged image overwrites intervening NVS. Hardware acceptance tests are not complete.')
    (release/'flash-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    (release/'SHA256SUMS.txt').write_text(''.join(f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n" for p in sorted(release.glob('*.bin'))))
    print(json.dumps({'release':str(release),'app_bytes':(release/'firmware.bin').stat().st_size,
                      'application_capacity':app['size'],'images':len(images)},indent=2))

if __name__=='__main__':package(sys.argv[1] if len(sys.argv)>1 else 'MeshCore_Enhanced_TDeck_Plus')
