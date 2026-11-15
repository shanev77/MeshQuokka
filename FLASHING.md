# Flashing and recovery

**Development build. Do not treat compilation as proof of safe operation.**
No installation is required to inspect the source, binaries or PDF manual.

## Verified image layout

Generated from PlatformIO's `FLASH_EXTRA_IMAGES`, `ESP32_APP_OFFSET` and the
actual binary partition table, not guessed addresses:

| Image | Offset |
|---|---:|
| bootloader.bin | 0x000000 |
| partitions.bin | 0x008000 |
| boot_app0.bin | 0x00e000 |
| firmware.bin | 0x010000 |
| merged-firmware.bin | 0x000000 |

Board configuration: ESP32-S3, 16 MB quad flash, QIO build configuration,
80 MHz, OPI PSRAM. Preserve generated image header values using
`--flash_mode keep --flash_freq keep --flash_size keep`; the bootloader image
may have a DIO header for bootstrap even when the board uses QIO at runtime.
Do not force-patch a digest-protected bootloader header.

| Partition | Offset | Size |
|---|---:|---:|
| NVS | 0x009000 | 0x005000 |
| OTA data | 0x00e000 | 0x002000 |
| app0 | 0x010000 | 0x640000 |
| app1 | 0x650000 | 0x640000 |
| SPIFFS | 0xc90000 | 0x360000 |
| core dump | 0xff0000 | 0x010000 |

## Before writing anything

1. Confirm the device is the 915 MHz T-Deck Plus. Attach the matching antenna.
2. Use a USB data cable and stable power. Close serial monitors and other flashers.
3. Determine the current COM port; it can change after entering download mode.
4. Read the flash ID, then back up the **entire 16 MB** flash. Do not proceed
   after an interrupted or failed backup. Keep the existing firmware version
   and a trusted original firmware image as additional recovery information.
5. Compute and save the backup's SHA-256. Esptool checks the read transfer's
   MD5 against the device. A 16 MB file alone does not prove recoverability.
6. For the first installation, compare old and new partition layouts. Existing
   filesystems are not assumed compatible. The merged image overwrites NVS
   in the gap between bootloader and application; separate images avoid that
   gap but do not guarantee compatibility with the previous application.

Commands below run from the parent workspace. Replace COM3 if it changes:

```powershell
$py = '.\.venv\Scripts\python.exe'
$esp = '.\.platformio\packages\tool-esptoolpy\esptool.py'
& $py $esp --chip esp32s3 --port COM3 flash_id
& $py $esp --chip esp32s3 --port COM3 --baud 460800 read_flash 0 0x1000000 .\MeshQuokka\private-backups\original.bin
Get-FileHash .\MeshQuokka\private-backups\original.bin -Algorithm SHA256
```

## Install only after backup and explicit acceptance of the development build

The following command installs a merged image and overwrites the intervening
NVS area. During the authorized device test, separate images were used instead
to preserve NVS and the existing filesystem:

```powershell
& $py $esp --chip esp32s3 --port COM3 --baud 460800 write_flash --flash_mode keep --flash_freq keep --flash_size keep 0x0 .\MeshQuokka\release\merged-firmware.bin
```

To preserve the gaps when the partition layout has been checked, use the
separate images (the method used for this device):

```powershell
& $py $esp --chip esp32s3 --port COM3 --baud 460800 write_flash --flash_mode keep --flash_freq keep --flash_size keep 0x0 .\MeshQuokka\release\bootloader.bin 0x8000 .\MeshQuokka\release\partitions.bin 0xe000 .\MeshQuokka\release\boot_app0.bin 0x10000 .\MeshQuokka\release\firmware.bin
```

Do **not** issue `erase_flash` as a routine installation step. On a known
matching enhanced layout, later application-only updates can write
`release/firmware.bin` at `0x10000`, provided app0 is the intended boot slot.
Do not assume this for arbitrary existing firmware or after an OTA update.

The application formats SPIFFS automatically only if the whole selected
SPIFFS partition is erased (all 0xff). On an unknown/corrupt nonempty partition
it stops with a storage error. Back up and inspect before deliberately clearing
that partition; there is no automatic destructive repair.

## Download mode and recovery

If automatic reset fails, disconnect USB, hold the trackball centre/BOOT button,
reconnect USB, then release BOOT. Alternatively hold BOOT while pressing RST.
Select the newly enumerated port. The screen may stay blank; a displayed
"DFU" label is not required for ESP32 ROM download mode. Press RST after flashing.

To restore the same device from its verified full backup:

```powershell
& $py $esp --chip esp32s3 --port COM3 --baud 460800 write_flash --flash_mode keep --flash_freq keep --flash_size keep 0x0 .\MeshQuokka\private-backups\original.bin
```

This restores firmware, partition table and flash-held data together. It does
not restore eFuses; this project does not change secure-boot/encryption eFuses.
Never publish the backup, which may contain private identity keys.

After boot, verify version `0.1.0_261114h`, display/input, storage, radio settings,
then perform `tests/HARDWARE_ACCEPTANCE.md`. A working splash screen does not
establish BLE or mesh interoperability.
