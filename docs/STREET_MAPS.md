# MeshQuokka street maps (0.1.0_261114q)

Open Map on Home. With GPS fixed, the first view centres on your position at
street zoom. Touch C to centre, -/+ to zoom, or drag/roll the trackball to pan.
Physical keys C, O/I (zoom in/out), +/- and Q/A also work. V switches between
street tiles and the original coastline outline. Contacts with advertised
coordinates and your own position remain overlaid. Tap a contact for details.

Connect Settings > WiFi to a router or phone hotspot, and insert a working SD
card. Only tiles intersecting the current viewport at the selected zoom are
requested, one transfer at a time. There is no region prefetch or bulk download.
Completed PNG tiles are retained in /meshdeck-maps/streets/ and reused without
another network request, including when WiFi is disconnected. Areas and zooms
not yet cached require Internet access. Moving the map sends the requested map
area to the tile provider; LoRa does not carry map downloads.

The default source is https://tile.openstreetmap.org/{z}/{x}/{y}.png. Map data:
(C) OpenStreetMap contributors, https://www.openstreetmap.org/copyright.
Policy: https://operations.osmfoundation.org/policies/tiles/ . This service
permits normal interactive viewing and local caching, not bulk offline area
packs. For large offline regions, use the existing .mdv pack generator from
OpenStreetMap extracts, with NewMaps, or a provider explicitly allowing offline
packs. Google Street View photographic panoramas are not included.

Tiles are 256x256 PNGs, CRC-checked on decode, with a maximum transfer size of
128 KiB. Six decoded RGB565 tiles and the PNG decoder live in PSRAM. At most one
cached tile is decoded per screen update. HTTPS runs in a separate task; the
main loop drains a bounded storage queue so radio processing continues between
writes. The response is first buffered in PSRAM and TLS is closed before SD
writing begins. A guarded TLS client prevents repeated socket cleanup from
closing an unrelated open SD file. Downloads pause/retry after failure rather than request in a tight loop.
A clock set via WiFi NTP is needed for TLS certificate validation.

An advanced HTTPS XYZ source may be specified as one line in
/meshdeck-maps/streets/source.txt using {z}, {x}, {y}. Restart after editing.
Use 256px non-interlaced PNG tiles and a CA trusted by MapRoots.h. The on-screen
attribution is for OpenStreetMap: a source requiring different attribution needs
an appropriate source-code adjustment. Keep a provider's cache separate: move
old cached PNGs off the card before changing source. No API key is supplied.

A missing-tile message means WiFi/Internet or cached coverage is missing.
A failed transfer retries after 60 seconds. Invalid PNGs remain on SD for
inspection; remove only the affected tile to request a replacement. Storage
is retained indefinitely; no automatic eviction of SD files is performed.

# Background radio reception

The mesh loop runs on Home, menus and other screens, with display sleep affecting
only the panel. Discover transmits your own advert; it is not required for
passive discovery. Heard records validated station adverts since boot, not all
radio packets or all saved contacts. Original Ripple contacts were not migrated.
Frequency/BW/SF/CR must match the nearby network; the saved WA profile checked on
the device was 923.125 MHz / 62.5 kHz / SF8 / CR8, two-byte paths.

This version adds a 50 ms RX_DONE polling fallback for a missed GPIO interrupt,
initialises the receive-error counter explicitly, and records maximum main-loop
gaps, recovered receive interrupts, actual boosted gain and advert rejection
reasons on USB. Signature and replay checks remain enabled. Remote-screen image
transfers now service the mesh loop and have a two-second deadline. These changes
improve robustness but do not prove the cause of sparse traffic or guarantee a
particular number of nearby repeaters.

GPS position was confirmed outdoors by the user and subsequently on USB with
12 satellites used. No receiver reconfiguration was required for this update.

The SD filesystem stays mounted after first use; power down before removing
or replacing the card, and restart after reinserting it. HTTPS allocations use
PSRAM, with ordinary internal-memory fallback only on devices lacking PSRAM.
