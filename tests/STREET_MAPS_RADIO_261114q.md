# Hardware follow-up: 14 November 2026, PacketDeck q

## Installed and built

Default and GPS9600 environments compile at 0.1.0_261114q; each application is
2,073,952 bytes. Application-only flash at 0x10000 passed esptool hash verification.
Four release/source-sync/partition/map-pack tests pass. GPS9600 is built but was
not separately installed. No storage erase or GPS configuration reset was used.

## Maps and touch

A single Bunbury sample tile passed HTTPS validation using the bundled CA roots.
On-device testing exposed TLS internal-RAM exhaustion; TLS allocations now use
PSRAM. Subsequent FAT write crashes were traced to repeated WiFiClientSecure
cleanup: the bundled 2.0.17 stop_ssl_socket zeroes socket to 0; another stop can
close a VFS descriptor belonging to SD. MapTLSClient restores -1 on every stop,
including its derived destructor before base destruction. checkTLSFileLifetime
keeps a real SD file open across repeated stops/destruction and verifies both
written bytes afterwards. Device result: PASS.

Responses are buffered in bounded PSRAM before TLS teardown and queued SD writes.
The filesystem remains mounted across operations. ETags read from CRLF metadata
are trimmed; a partial tile successfully resumed with HTTP 206 and decoded.

After a full power cycle the user confirmed touch restored. Firmware reported
touch=1, frames=253, errors=0. Before that cycle the controller was absent at both
I2C addresses. Added periodic rediscovery and a missed-release fallback; those
alone did not recover the physically unresponsive controller during warm resets.

The user then viewed new areas/zoom levels and confirmed new streets load and
Back works without a restart. Logs show repeated HTTP 200 completions and decoded
PNGs at zoom 14/15. Earlier saved tiles loaded again after reboot without fetching.
No automatic pan, region prefetch or bulk public-OSM tile download is implemented.
Public tile attribution and usage restrictions are documented.

During the fresh-tile test the maximum loop gap reached 421 ms; after returning
to the menu the next observation window reported 54 ms. Internal free heap was
45,376 bytes and PSRAM free 7,318,179 bytes. This is not worst-case qualification.

## Radio and GPS

Saved radio settings: 923.125 MHz / BW62.5 / SF8 / CR8, 22 dBm, two-byte paths,
automatic contact addition and boosted gain enabled. Passive mesh processing is
independent of the selected screen and continues when only the display sleeps.
An unsolicited signed room-server advert was received during the version-o
investigation (RSSI -92, SNR 0, two-byte path, one hop). Contacts subsequently
increased from five to six. This does not prove reception parity with the original
firmware. Sparse six-hour reception remains unexplained without a controlled peer
or original-firmware comparison. RX_DONE polling recovery count remained zero in
observed q windows; no claim is made that a missed IRQ caused the prior symptoms.

GPS fixed outdoors per user and subsequently reported quality 1 / RMC A / up to
12 used satellites over USB. Following the full cold power cycle indoors, the
last sample had not reacquired a fix. UART/NMEA reception continued. No claim of
reliable indoor acquisition or a confirmed receiver model is made.

Private serial logs and original backup are excluded from distribution.
