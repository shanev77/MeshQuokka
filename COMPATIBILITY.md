# Compatibility and completion status

Version: 0.1.0_261115c. This is a development implementation with a successful
ESP32 build, software checks and a basic device smoke test; it is **not the completed production firmware
described in the project brief**. Hardware acceptance remains open.

| Area | Implemented in source | Remaining evidence or functionality |
|---|---|---|
| MeshCore | Upstream routing/identity/encryption, contacts, adverts, direct/channel messaging, ACK handling | Interoperate with real peers; packet-loss and retransmission tests |
| Standalone UI | Colour display, keyboard/trackball/touch, chats, contacts, repeater tools, settings, diagnostics | User confirmed keyboard, touch, Back and on-screen keyboard; exhaustive UI qualification remains open |
| BLE | Actual MeshCore command parser and GATT UUIDs, encrypted pairing, reconnect, status, name via node-name setting, PIN setting | Official Android/iOS send/reply and reconnect tests |
| Radio | AU profile, 923.125 MHz default, BW62.5/SF8/CR8, editable params, previous-config restore, local confirmation | Local mesh parameters must match; RF/regulatory measurements; other regions intentionally rejected |
| Wi-Fi | Async scan, password entry/masking, four remembered networks, active-network reconnect, disconnect/forget, IP display | Automatic roaming among all saved networks and coexistence stress test |
| GPS | Existing NMEA integration, enable/disable, satellites, valid fix, position, altitude, UTC, advert-sharing switch | Receiver identification; speed/course/HDOP; UART now auto-detects baud using checksummed NMEA; power gating not established |
| Maps | On-demand HTTPS street tiles with SD caching and decoded PSRAM tiles; real Natural Earth vectors, SD .mdm/.mdv packs, pan/zoom, true advertised node positions, map marker details, client/repeater filter, unlocated list | Arbitrary on-device radius/zoom tile planner, favourites/recent filters, continuous follow, region deletion UI |
| Downloads | HTTPS country/region pack catalogue, bounded network queue, pause, ETag-checked resume, SD capacity check, temporary-file commit | On-device area/radius selection and pre-download exact size estimates; device interruption tests |
| Storage | MeshCore identity/contact storage, local message history, versioned Wi-Fi profiles, non-destructive mount failure, SD map validation, manual JSONL message archive to SD | Comprehensive migrations, encrypted credentials, archive import/restore, automatic archive retention, GPS track exports |
| Power/audio | Brightness/sleep controls, voltage-based battery estimate, inherited notification audio | Calibrated battery model, battery-life and audio measurements |
| Concurrency | Network worker cannot touch radio/SPI; bounded main-loop storage drains; shared SPI object; BLE receive queue fix | UI/rendering still shares cooperative mesh loop; worst-case latency and watchdog recovery are NOT established |
| Packaging | Real ESP32 application, bootloader, partition table, OTA initializer, merged image, hashes and manifest | Default build flashed with verified writes; basic boot/UI observed. Full original/current-image restoration was exercised on the same unit during the radio comparison |

## Protocol boundaries

Client is the default and only role in the enhanced image. Upstream repeater,
USB companion and KISS modem environments remain separate. Experimental
MeshDeck voice extensions are not enabled. No replacement mesh packet format
or replacement cryptography was introduced.

MeshCore BLE uses service `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`, RX write
characteristic ending `0002`, and TX read/notify characteristic ending `0003`.
Although these UUIDs come from the UART-service family, application frames are
the actual MeshCore companion binary commands. The transport requires encrypted
MITM-protected access and retains official pairing behaviour.

The local display receives callbacks while the phone retains its own message
queue. Concurrent local/phone sending and duplicate-history behaviour still
require acceptance tests; do not infer they passed from retained source.

## Radio and map constraints

The AU envelope permits centre frequencies only when the selected bandwidth
fits inside 915-928 MHz, SF7-12, CR5-8 and -9 to 22 dBm. This is not equipment
certification. The Australian 2025 LIPD instrument also sets EIRP and radiated
spectral-density limits; antenna gain and actual emissions matter.
Source: https://www.legislation.gov.au/F2025L01047/asmade/text

The default 923.125 MHz is user-requested. BW62.5/SF8/CR8 is a starting choice,
not confirmation of the local mesh's modulation settings.

Natural Earth maps are public domain; the supplied packs are coastlines/place
labels, not guaranteed street-level navigation. OSM-derived `.mdv` packs have
ODbL attribution requirements. No bulk download is made from
`tile.openstreetmap.org`, whose policy prohibits offline prefetching:
https://operations.osmfoundation.org/policies/tiles/

Zero/zero contact coordinates are treated as unspecified by the inherited
contact model. This means a genuine advertised position exactly at (0,0) cannot
be distinguished in this UI. Positions are never inferred from signal strength.

Version i: touch keyboard added; GPS/Bluetooth status refresh each second; Repeaters/Trace hold up to 350 entries in PSRAM. Original /ripple-format records are not imported. Valid NMEA reception and outdoor positioning are confirmed. Versions q-t include working street maps and touch navigation. Version t adds guarded standby/RX restarts every four seconds; it avoids the full calibration sequence that raised oscillator-start flags on the tested device. Sparse reception remains unresolved; see tests/RADIO_CHECK_261114t.md and the current full manual.

Version u enables internal DIO2 RF-switch control after a manufacturer/board-source comparison. See tests/ORIGINAL_RADIO_COMPARISON_261114u.md. Runtime comparisons are needed to establish its effect on discovery.

Version v disables periodic RX restarts by default and adds packet history/adverts counters to Heard. See tests/RECEIVER_ACTIVITY_261114v.md for validation and limits.
