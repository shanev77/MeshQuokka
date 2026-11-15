"""Export build-authoritative flash inputs; never hand-maintain image offsets."""
Import('env')
import json
from pathlib import Path

def capture(source, target, env):
    board = env.BoardConfig()
    images = []
    for offset, path in env.get('FLASH_EXTRA_IMAGES', []):
        actual = Path(env.subst(path))
        images.append({'offset': int(str(offset), 0), 'path': str(actual), 'name': actual.name})
    images.append({'offset': int(env.subst('$ESP32_APP_OFFSET'), 0),
                   'path': str(Path(env.subst('$BUILD_DIR')) / 'firmware.bin'), 'name': 'firmware.bin'})
    data = {'chip': board.get('build.mcu'), 'flash_size': board.get('upload.flash_size'),
            'flash_mode': board.get('build.flash_mode'), 'flash_frequency': board.get('build.f_flash'),
            'environment': env.subst('$PIOENV'), 'images': images}
    (Path(env.subst('$BUILD_DIR')) / 'flash-layout.json').write_text(json.dumps(data, indent=2))

env.AddPostAction('$BUILD_DIR/${PROGNAME}.bin', capture)
