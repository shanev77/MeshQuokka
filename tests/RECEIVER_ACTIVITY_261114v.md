# PacketDeck v receiver and advert audit

## Changes

- Default periodic receiver recovery disabled. The four-second diagnostic can still be enabled explicitly with rxrecover 4; reboot returns to zero. RX_DONE polling remains enabled.
- Heard > Traffic shows a bounded 32-packet metadata history, including raw header type, age, signal, path width/hops and the last flood relay prefix. No payloads are displayed. Prefixes are unverified and direct paths never claim to identify the transmitter.
- Advert pipeline counters distinguish short/self/duplicate/signature-valid/bad-signature/bad-app-data/old-timestamp outcomes. Signature and replay protections remain unchanged. Counters describe stages, not disjoint categories.
- Heard retains up to 64 distinct six-byte identity prefixes, moving a repeat observation to the newest position rather than consuming another slot. Saved contacts remain separate.
- Dispatcher rejects too-short plain or transport-coded headers before accessing the missing bytes.

## On-device validation

Application-only installation passed esptool write-hash verification. Existing full backup integrity was checked and the previous application retained privately. On-device traffictest passed legacy one-byte, two-byte, three-byte transport-coded, truncated, reserved-width and unsupported-version cases; direct paths correctly suppress relay attribution. The test does not transmit synthetic or malformed packets.

Early live observation: two returned one-hop own adverts were received with RSSI -83 dBm and SNR +6.8 / +6.2 dB, RX errors zero, recovery interval zero, recovery count zero, boosted gain enabled. TX count included device interaction; only one advert was requested by the USB test script. GPS remained fixed and touch detected. At 120 seconds RX=2/errors=0/TX=2, recovery count=0. Both returned paths carried prefix ddbb. This matches the label of a saved repeater entry, but a short path prefix is not authenticated identity. The user confirmed the Traffic/Nodes toggle, readable rows and touch Back. The user subsequently reported being unable to exit Traffic. Live USB/GPS/radio diagnostics continued responding: Traffic nav() was consuming NAV_BACK. Version w returns false for unhandled navigation so the global Back handler runs, and changes the toggle text to explicit white. This regression requires a specific Traffic-to-Back check after installation.

## Limits

No controlled peer or calibrated signal generator is available. These checks do not prove equal sensitivity or long-duration discovery to the original closed-source T-Deck application. Old-timestamp adverts remain excluded from named contact updates; their count is now visible. Duplicate/self advert counts do not imply new stations. Packet metadata does not authenticate the transmitter or identify encrypted-message authors. History is RAM-only and resets on reboot.

## Version w follow-up

Installed application write verified. On-device parser/navigation tests PASS, including NAV_BACK after both keyboard and touch mode changes. At 30 seconds: RX mode active, recovery interval/count both zero, device errors zero, GPS valid with 10 satellites. The receiver logic is unchanged from v; the final firmware adds white toggle text and correct Back event propagation. Physical user confirmation is recorded separately if received.
