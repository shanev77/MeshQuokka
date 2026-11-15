# Architecture and implementation plan

## Baseline decision

Preserve MeshCore's MIT-licensed mesh, crypto, routing and BLE transport.
The official T-Deck companion has BLE but a limited local interface. The
proprietary standalone T-Deck application is not available to fork here.
Reuse the MIT-licensed MeshDeck standalone application, against its explicitly
pinned MeshCore commit `bbb58cceb852a9190b46d5f6984e8e5140e6991e`.
Latest upstream was also inspected at `d2096bd90e65cd5213e612811eda0bc214e1ab1c`.
Do not silently update protocol APIs underneath the application.

MeshDeck is third-party application code and needs hardening: its current map
download blocks the mesh loop and disables TLS verification; its display uses
a second SPI host on shared pins; its storage mount formats on failure.
Those behaviours are not acceptable as a production-ready baseline.

## Roles and ownership

Default role is client/companion. Repeater and KISS modem are separate upstream
build environments, not concurrent roles. One MyMesh instance owns identity,
contacts, channels, delivery state and radio. The local UI observes its events;
the phone uses the real upstream companion command parser over its BLE GATT
transport. Do not substitute a serial text protocol or change crypto.

## Phased gates

1. Document audit and hardware report before firmware edits (completed).
2. Build pinned standalone/BLE baseline with Australian defaults. Validate
   image headers, generated partition layout, size and dependency versions.
3. Harden shared SPI, storage, radio validation and local/phone messaging.
   BLE Android send/reply and reconnect require the actual device and peer.
4. Add nonblocking Wi-Fi scanning and multiple saved-network management.
5. Bounded GPS processing and SD map cache/download jobs, with interruption
   recovery and licensed provider configuration.
6. Map region selection, bounded tile estimates, pan/zoom and real positions.
7. Integrate device/GPS/BLE/diagnostic controls, retain local input navigation.
8. Measure concurrent radio/UI/BLE/Wi-Fi load, heap high-water marks, watchdog
   behaviour and battery consumption on hardware.
9. Package verified outputs, checksums, manifest, backup/recovery instructions,
   user guide and a rendered/visually verified PDF manual.

## Preliminary memory budget (targets, not measurements)

| Allocation | Placement / budget |
|---|---|
| RGB565 framebuffer | PSRAM, 153600 bytes |
| Map tile decode | PSRAM, one 256x256 RGB565 tile = 131072 bytes |
| Network staging | bounded 4 KB chunks; no full-region allocation |
| BLE frame queues | upstream bounded queues; inspect configured frame counts |
| Mesh contacts/messages | bounded existing structures; report linker sizes |
| Task stacks | internal RAM; measure minimum remaining stack on hardware |
| Wi-Fi/BLE stacks | framework-managed internal memory; runtime measurement required |
| Headroom gate | avoid release if internal free heap < 40 KB under stress |

PSRAM capacity is not evidence that DMA buffers or BLE stacks fit internal RAM.
Compilation alone does not close performance or hardware acceptance gates.
