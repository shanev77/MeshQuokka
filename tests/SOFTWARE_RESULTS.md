# Reference verification - 14 November 2026

Version 0.1.0_261114d, development build. This record separates software checks
from device testing; see HARDWARE_ACCEPTANCE.md for observed device results.

## Passed

* Both enhanced PlatformIO environments compile and link: default GPS UART
  38400 and alternative 9600. Registry dependency versions are pinned.
* Each application image is 2,017,856 bytes and fits the 6,553,600-byte app slot.
* Four automated release tests pass: binary partition integrity/capacity,
  manifests and merged-image contents for both variants, byte-identical
  editable/compiled application trees, and bounds of supplied map packs.
* Partition parser rejects a deliberately modified table with a bad MD5.
* Desktop HTTPS requests using the firmware's CA bundle retrieve the region
  catalogue and Australian west/east packs. Both match the bundled files.
  Range requests return 206 and the expected bytes with matching ETags.
* Original source and third-party licence notices are retained. The source ZIP
  excludes private backups, Git history, caches, compiled objects and logs.
* Raw keyboard USB logging was removed before installation. The health record
  contains only version, uptime and memory counts.

## Limits

Desktop HTTPS checks do not prove the ESP32 TLS path or SD interruption recovery.
Compile-time/static memory reports do not establish runtime headroom under load.
No claim is made that the complete project brief, long-duration coexistence,
GPS acquisition, BLE app messaging, RF interoperability or recovery restoration
has passed merely because these software checks passed.

## Navigation update 0.1.0_261114e

Added a visible global Back button and kept existing screen-specific NAV_BACK
handling for modal cancellation. Default application is 2,017,968 bytes.
Application-only installation passed esptool hash verification. The device
reported version e at 30 seconds uptime with 85,452 bytes free internal RAM
and 8,213,043 bytes free PSRAM. Touch response confirmation remains pending.


## 0.1.0_261114i

Both MeshCore_Enhanced_TDeck_Plus and GPS9600 environments compile successfully.
Each application is 2,031,840 bytes, below the 6,553,600-byte slot capacity.
Four release tests pass: source synchronization, binary/manifest/merged-image
integrity, partition MD5/capacity and supplied map bounds. Default application
was flashed at 0x10000 with verified write hash, then booted and emitted live
health, GPS and radio diagnostics. See HARDWARE_ACCEPTANCE.md for limits.
Keyboard input is routed directly to the active field, avoids navigation on
empty delete, and masks secret previews. Trace pagination now follows selection.
No end-to-end radio peer test or valid GNSS position is claimed.


## PacketDeck 0.1.0_261114k

Default and GPS9600 builds use the PacketDeck branding, vector boot mark and
live Heard diagnostics. The Home name is placed at y=40, below the mesh label
at y=26 (8-pixel font). Application size: 2,033,056 bytes. Installation uses
only the application at 0x10000 and preserves settings/data partitions.
The installed j predecessor received a packet with no radio receive errors;
see HARDWARE_ACCEPTANCE.md for observation details and remaining limitations.
