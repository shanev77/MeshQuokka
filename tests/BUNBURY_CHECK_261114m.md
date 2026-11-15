# Bunbury / WA diagnostic update - 14 November 2026

PacketDeck 0.1.0_261114m was built and installed at app offset 0x10000 with
esptool hash verification. Existing settings and GPS configuration were retained.
The four existing release-integrity/source-sync/map-bound checks passed; the
alternate GPS-start binary was not rebuilt as part of this diagnostic update.

## Local network checks

The WA community publishes 923.125 MHz / BW62.5 / SF8 and a two-byte path migration.
Its preset request specifies CR8. Bunbury's mapping region also identifies itself
as two-byte enabled. Live device settings matched 923.125/BW62.5/SF8/CR8/22 dBm,
with two bytes per hop, automatic contact addition enabled and five saved contacts.

Sources consulted:
- https://perth.meshcore.au/
- https://github.com/meshcore-dev/MeshCore/issues/949
- https://buy.meshmapper.net/index.php

## Corrections

- Fresh-install defaults and the named WA/Bunbury preset now use SF8; choosing
  that preset selects two-byte paths. Saved custom preferences still load normally.
- Local flood adverts now honor the selected path-hash width instead of silently
  using the one-byte sendFlood default.
- Heard/Trace hop counts remove the packed path-width bits.
- Local Trace translates packed contact path length to actual byte length and
  flags, preventing a two-byte path from being treated as a much longer buffer.
  For example, encoded 0x43 means three two-byte hops (six bytes), not 67 bytes.
  Unknown paths, more than 64 bytes, and unsupported three-byte trace paths are
  rejected. Two-byte trace identifiers are displayed as four hex digits.
- Header-only RX diagnostics show type/version/path width without logging payloads.

Following the user's Discover action, received type-4 (advert) frames included:
119 bytes, two-byte path, one hop, RSSI -89 dBm / SNR +7 dB;
121 bytes, two-byte path, two hops, RSSI -108 dBm / SNR -11.8 dB.
An advert may be this device's own returned flood, so these observations alone do
not establish a newly discovered peer identity or end-to-end message delivery.

## GPS

Added maximum non-zero GSV C/N0 over ten-second windows, updating on battery as
well as USB. The initial indoor/USB sample reported no non-zero C/N0 values,
GGA quality 0, RMC V, five satellites in the last GSV report, and zero used.
Checksum-valid NMEA continued at nominal 38400 baud. The user confirms the
original firmware obtained real fixes, but has not yet completed an uninterrupted
10-15 minute clear-sky battery test with PacketDeck. Receiver configuration,
antenna reception/interference, and firmware differences remain possible causes;
a receiver hardware fault has not been established. No receiver reset or
configuration-clear commands were sent.

LilyGO troubleshooting references:
- https://github.com/Xinyuan-LilyGO/T-Deck/issues/79
- https://github.com/Xinyuan-LilyGO/T-Deck/issues/109
- https://content.u-blox.com/sites/default/files/products/documents/GNSS-Antennas_AppNote_%28UBX-15030289%29.pdf
