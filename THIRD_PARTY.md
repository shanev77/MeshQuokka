# Source and licence notices

MeshCore Enhanced retains the upstream licence notices in its source trees.
The source archive contains both the editable application and the corresponding
compiled application sources; see README.md for the build procedure.

* MeshCore: MIT, `firmware/license.txt`.
* MeshDeck: MIT, `vendor/meshdeck/LICENSE`.
* Additional MeshDeck components: `vendor/meshdeck/THIRD_PARTY.md`.
* Natural Earth map packs: public domain. Australian west/east packs are
  `assets/maps/auw.mdm` and `assets/maps/aue.mdm`.
* Codec2 sources retain their LGPL notices but are not compiled into these
  non-beta enhanced images. No voice capability is advertised for this build.
* Arduino, ESP-IDF and registry dependencies retain their respective licences.
  Exact resolved package versions are in DEPENDENCIES.json and
  platformio.local.ini; installed dependency notices accompany the release.

The release does not contain the device's private flash backup or user keys.
OSM-derived maps are not included in the bundled Natural Earth map set;
their separate ODbL obligations still apply when users obtain such maps.

Root LICENSE applies to enhanced contributions credited to S. van Ingen.
Original component copyright and licence notices remain in their source trees.
