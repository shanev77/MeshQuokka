# MeshQuokka receiver audit, 16 November 2026

## Findings

- Dispatcher::tryParsePacket already separates path width from hop count, supporting 1/2/3-byte entries within 64 path bytes; no global multihop filter found.
- MyMesh::filterRecvFloodPacket returns false, accepting scoped and unscoped incoming floods. Outbound scope preferences affect reach through repeaters, not this local receive filter.
- SimpleMeshTables uses the first eight SHA-256 bytes of payload type plus payload (TRACE additionally includes path length). Relay path is deliberately excluded so retransmissions do not create duplicate chat messages. No evidence of distinct messages colliding was found.
- Channel lookup, authentication/decryption and UI storage do not filter by mentioned name or active screen. Existing counters measured zero group packets before the update, so they provide no evidence of a Public-key or MAC failure in that session.
- Auto-add max_hops was zero; even a configured auto-add limit still calls onDiscoveredContact for valid unknown adverts, allowing Heard to show them.
- Radio register checks: 923.125 MHz, BW62.5, SF8, CR4/8, CRC enabled, sync0x1424, boosted gain0x96, DIO2 switch enabled, device errors 0. Recovery interval0 and resets 0. ISR notification and a 50 ms latched RX_DONE fallback remain enabled.
- Firmware 261116a passed on-device actual decoder/scope/bounds/dedup, Traffic navigation, channel decoder and unread tests. User pressed Discover during observation; RX returned a one-hop two-byte own advert (-111 dBm, -12 dB SNR), correctly ignored as a self-return. No automated test transmissions were sent.

## Changes

Added bounded per-frame outcome correlation using pending packet-pool slots (cleared after processing), radio error events, and detailed channel/advert outcomes. Corrected Traffic's version validation to match the real decoder (payload version field0 only). Hardened Packet::readFrom to check short headers/transport/path lengths before reading/copying. These bounds checks address malformed input; they are not an established cause of the missing valid traffic.

Raw events are metadata, not full retained packet payloads. The latest32 ring is not a full capture, and generic non-chat handling is not a complete reason taxonomy. No acceptance, signature or key checks were disabled. Passive reception remains independent of the UI screen.

## Sources checked

- https://raw.githubusercontent.com/meshcore-dev/MeshCore/main/src/Dispatcher.cpp
- https://raw.githubusercontent.com/meshcore-dev/MeshCore/main/src/Mesh.cpp
- Local firmware/src/Packet.cpp, helpers/SimpleMeshTables.h, helpers/BaseChatMesh.cpp, helpers/radiolib/RadioLibWrappers.cpp, and vendor/meshdeck/firmware/MyMesh.cpp.

Upstream main has later table APIs and other changes; this audit compared relevant receive behavior rather than replacing the firmware core wholesale. Monitor reports provided by the user show outbound propagation to multiple observers, not a controlled inbound coverage test.


## Battery follow-up in 261116b

LilyGO schematic RD2/RD3 form a 1:2 divider at BAT_ADC on the switched BAT rail downstream of Q4, not the battery connector VBAT. The prior formula assumed ADC full scale was 3.3V and converted uncalibrated raw counts. Replaced with averaged analogReadMilliVolts at 11 dB attenuation and a one-second cache; retained bounded 0-100% voltage mapping. Home now distinguishes supply voltage above 4.35 V as USB, without claiming the cell is full. Physical unplug validation is tracked separately from formula tests.

- https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/schematic/schematic.pdf
- https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html

Final 261116b device checks: battery bounds/monotonicity/USB, RX decoder/scopes/bounds/dedup, Traffic navigation, channel decoding and unread storage all PASS. USB supply measured 4520 mV; RF register reads and chip errors 0, recovery 0, resets 0, TX 0 during the final isolated checks.

User confirmed Home after unplugging:84%,4.07V; reconnected:USB,4.51V. This validates the UI power-source distinction and displayed calculation, not independent ADC accuracy or measured remaining capacity.
