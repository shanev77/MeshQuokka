# PacketDeck receiver investigation - 14 November 2026

## Verified configuration and code paths

Device: LilyGO T-Deck Plus 915 MHz, ESP32-S3, PacketDeck 0.1.0_261114t.
Application-only installation at 0x10000 passed esptool write/hash verification.

- Actual frequency register: 0x39b20000, corresponding to 923.125 MHz.
- Actual sync word: 0x1424 (LoRa private sync word 0x12).
- Actual receive gain: 0x96 (boosted).
- Driver modulation: BW62.5, SF8, coding-rate denominator 8; CRC enabled,
  explicit header. Saved profile agrees, including two-byte paths and 22 dBm.
- Automatic contact addition is enabled for chat, repeater and room roles;
  mask 15 also enables overwrite of the oldest eligible contact when full.
  Sensor auto-add is not enabled in the saved preferences. Max-hop setting 0
  means no additional auto-add hop restriction.
- Flood receive filtering returns false. The 64-byte path budget permits
  32 two-byte hops. Five-to-seven-hop adverts are not excluded by these limits.
- The main loop services MeshCore before UI work on every screen. Display sleep
  does not stop it. Remote-screen output also services the mesh between chunks.
- Heard lists validated node adverts since restart; arbitrary received packets
  do not establish a discoverable identity. Discover transmits this node's
  advert, not a request for a complete network directory.

## Controlled recovery checks

Turning Wi-Fi off did not immediately remove the elevated instantaneous RSSI
readings (about -76 dBm). No received packets were counted in that short sample.

Versions r/s tried a guarded four-second full warm-sleep/calibration recovery.
The device repeatedly latched XOSC_START_ERR (0x0020) when that recovery ran.
Clearing the flag with recovery disabled kept it clear; re-enabling recovery
made it return. This identifies a problem with the newly tested recovery path,
not the original cause of sparse reception. Those versions are superseded.

Version t uses standby followed by startReceive, preserving calibration and
parameters. The guard skips active reception, pending RX_DONE/interrupt and TX.
Hardware error registers remained 0x0000 during the observed repeated restarts.
Receive mode remained active (status 0x52), boosted gain stayed enabled, and
GPS continued reporting a valid position with checksum-valid NMEA. No radio
settings, identity, contact table or GPS receiver configuration were erased.

Private raw logs: radio-wifi-comparison-q.log, radio-recovery-r.log,
radio-errors-s.log, radio-check-t.log. They are excluded from distribution.

## Interpretation and remaining validation

The tested peripheral links/configuration work, but this does NOT establish
that reception matches the original firmware. No new packets/adverts arrived
in the short controlled r/s windows. Version t subsequently received an
unsolicited CRC-valid 30-byte type-0 packet: two-byte path, four hops in its
header, RSSI -94 dBm and SNR +2.8 dB. TX remained zero. RX advanced to one
with zero receive errors. This was not a node advert, so Heard remained empty. A prior version received a validated
room advert passively, confirming that path can operate, but it does not prove
range, reception rate, multi-hop peer discovery or long-duration reliability.

Next: leave t on Home in the same location without rebooting and record RX,
error and Heard counters after a normal advert interval. If still sparse,
a controlled original-firmware comparison requires first backing up the current
complete flash so identity/settings/messages can be restored afterward. That
comparison has not been performed. No second known-good radio was available.

Upstream reference: https://github.com/meshcore-dev/MeshCore/blob/main/docs/faq.md
