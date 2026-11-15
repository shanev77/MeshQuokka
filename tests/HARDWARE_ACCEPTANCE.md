# Hardware acceptance record

No unchecked item is a passed test. Record date, build hash, equipment, settings,
observed result and supporting serial logs for each test. Software checks do
not substitute for over-the-air tests.

## Observed on connected unit

* USB port COM3, VID 303A / PID 1001.
* Esptool identified ESP32-S3 revision v0.2, 40 MHz crystal, 16 MB quad flash.
* User confirmed 915 MHz radio variant.
* Version 0.1.0_261114d installed with four separate images; all four writes
  passed esptool hash verification. NVS and SPIFFS were not erased.
* Complete 16 MiB original flash read passed esptool's MD5 transfer check;
  local SHA-256 and original partition layout saved privately. Layout matches.
* USB health reports at 30 and 60 seconds: 85,276 bytes free internal RAM,
  8,213,043 bytes free PSRAM. No crash report in the 45-second captured window.
* User reported home/setup screen visible and trackball responding.
* These observations are a smoke test, not full device or RF qualification.

## Acceptance checklist

- [x] Verify complete recovery backup and its checksum; compare old partition layout.
- [x] Flash selected development build and verify esptool write checksums.
- [ ] Boot repeatedly on USB and battery; verify version and reset reason.
- [ ] Display, keyboard, trackball directions/click/back, touch calibration.
- [ ] Detect 8 MB PSRAM; measure free internal heap and largest free allocation.
- [ ] Receive and transmit LoRa to a standard MeshCore peer.
- [ ] Direct-message encryption, ACK, timeout and retry semantics.
- [ ] Public/private channel send/reply and no duplicate processing with phone active.
- [ ] Contact, client and repeater discovery via standard adverts.
- [ ] Official Android app pairing, contacts/radio read, send and reply via BLE.
- [ ] BLE reconnect, stale bond/PIN change, enabled/disabled state; compatible iOS test.
- [ ] Wi-Fi scan, correct/wrong password, saved profiles, forget and reconnect.
- [ ] GPS NMEA baud/module identification, acquisition, validity, altitude and UTC.
- [ ] Position sharing disabled/enabled, manual coordinates, no fabricated positions.
- [ ] SD mount, absent card, corruption, card removal and near-full storage.
- [ ] HTTPS map catalogue and region download, display and marker inspection.
- [ ] Pause/resume with matching ETag, changed server object, interruption and reboot.
- [ ] Radio validation, confirmation and previous-configuration restore after restart.
- [ ] Persistence of identity, contacts, channels, messages and settings after reboot.
- [ ] Simultaneous LoRa/BLE/Wi-Fi/map use; count loss, loop latency and stack/heap minima.
- [ ] Battery estimate/calibration, charging, sleep/wake, sound and battery runtime.
- [ ] Watchdog recovery, exported diagnostics, and successful full-backup restoration.

Suggested coexistence run: a fixed known peer sends numbered messages every
10 seconds for one hour while BLE remains connected and maps download. Compare
received sequence numbers with an idle baseline; record missing/duplicate packets,
maximum service-loop gap, heap low-water mark and any resets. Agree an acceptance
threshold before declaring production readiness.

Navigation update 0.1.0_261114e adds a top-left Back button. Verify it on GPS,
Bluetooth, Settings, Chat and Maps, including closing a Wi-Fi editor before
leaving Wi-Fi. User confirmed the touch Back button works.


## Version i USB observations (14 November 2026)

Application write at 0x10000 passed esptool hash verification. Runtime reported
0.1.0_261114i, about 44,036 bytes internal free memory and 8,178,111 bytes free
PSRAM in the sample. GPS: 50,037 bytes, 1,051 checksum-valid NMEA sentences,
117 ms since last valid sentence, baud 38401 (nominal 38400). GGA quality 0,
RMC status V, zero used satellites, last GSV count 5: no position fix established.
Radio: 923.125 MHz / BW62.5 / SF8 / CR8 / 22 dBm; auto-add enabled; one contact.
SD mounted, 30,420 MiB total / 3 MiB used. User previously answered yes to the
SD archive and Home/About check. User confirmed the on-screen keyboard works well and the GPS screen shows
NMEA received, five in view, and no fix. This is not a complete interoperability qualification.

Original backup inspection found /ripple-format files, not /contacts3 or
/new_prefs. No automatic identity/contact migration from that format exists.
The original backup remains unchanged and private. Historical claims of data
preservation mean partition bytes were retained, not that incompatible records
were imported. The displayed 24-repeater limit is removed in i.


## PacketDeck j / k

User confirmed the PacketDeck boot logo looks good and Heard reports listening.
Version k moves the Home brand from y=30 to y=40 to clear the mesh label at y=26.
USB diagnostics in j: driver RX mode and SX1262 chip status 0x52; one packet
received, zero receive errors, one transmit, no node adverts in the Heard list
at the 120-second sample. The saved profile stayed 923.125/BW62.5/SF8/CR8/22 dBm.
This establishes observed packet reception, not range, sender identity or full
protocol interoperability. The user has no second radio for a controlled test.
Neither named adverts nor a position fix were confirmed during that sample.
