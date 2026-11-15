import hashlib, importlib.util, json, struct, unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('package_release',ROOT/'tools/package_release.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

class ReleaseTests(unittest.TestCase):
    def test_partition_integrity_and_capacity(self):
        data=(ROOT/'release/partitions.bin').read_bytes()
        p=module.partitions(data)
        app=next(x for x in p if x['type']==0 and x['subtype']==16)
        self.assertLessEqual((ROOT/'release/firmware.bin').stat().st_size,app['size'])
        damaged=bytearray(data);damaged[9]^=1
        with self.assertRaises(ValueError):module.partitions(damaged)
    def test_manifest_matches_every_binary(self):
        for folder in [ROOT/'release', ROOT/'release/gps9600']:
            with self.subTest(environment=folder.name):
                m=json.loads((folder/'flash-manifest.json').read_text())
                merged=(folder/m['merged']['path']).read_bytes()
                for i in m['images']+[m['merged']]:
                    data=(folder/i['path']).read_bytes()
                    self.assertEqual(len(data),i['size'])
                    self.assertEqual(hashlib.sha256(data).hexdigest(),i['sha256'])
                for i in m['images']:
                    offset=i['offset']-m['merged']['offset']
                    self.assertEqual(merged[offset:offset+i['size']],(folder/i['path']).read_bytes())
    def test_application_sync(self):
        for source in (ROOT/'vendor/meshdeck/firmware').rglob('*'):
            if source.is_file():
                self.assertEqual(source.read_bytes(),(ROOT/'firmware/examples/meshdeck'/source.relative_to(ROOT/'vendor/meshdeck/firmware')).read_bytes(),str(source))
    def test_supplied_map_pack_bounds(self):
        for path in (ROOT/'vendor/meshdeck/sdcard/meshdeck-maps').glob('*.mdm'):
            data=path.read_bytes()
            magic,scale,*_=struct.unpack_from('<IfhhhhIII',data)
            npts,nlines,ncities=struct.unpack_from('<III',data,16)
            self.assertEqual(magic,0x314D444D)
            self.assertTrue(50<=scale<=2000)
            self.assertEqual(len(data),28+4*npts+6*nlines+20*ncities)
            for j in range(nlines):
                start,count=struct.unpack_from('<IH',data,28+4*npts+6*j)
                self.assertLessEqual(start+count,npts)

if __name__=='__main__':unittest.main()
