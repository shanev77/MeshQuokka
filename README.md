**Data Packet Radio on the MESHCORE network.**

For LilyGO T-Deck Plus.

**Development firmware, not a production-qualified release.** Real ESP32-S3
source and binaries, based on MeshCore plus the MIT-licensed MeshDeck local UI.
No browser application or simulator substitutes for the firmware.

Target: user-confirmed 915 MHz LilyGO T-Deck Plus. USB interrogation on
14 November 2026 identified ESP32-S3 revision v0.2, 40 MHz crystal, 16 MB quad
flash. Runtime health reports show PSRAM in use; the home/setup screen and trackball
were confirmed by the user. Onboard GPS module revision remains unverified.
See `tests/SOFTWARE_RESULTS.md` and `tests/HARDWARE_ACCEPTANCE.md` for evidence.

## Start here

* `HARDWARE.md`: supplied-document audit and unresolved GPS discrepancy.
* `COMPATIBILITY.md`: actual functionality, restrictions and outstanding work.
* `FLASHING.md`: image layout, backup requirements and recovery.
* `USER_GUIDE.md`: controls and operating instructions.
* `docs/USER_MANUAL.md`: editable complete manual source.
* `tests/HARDWARE_ACCEPTANCE.md`: tests still requiring the device and peers.

## Build

For a fresh Windows checkout, install Python and Git, then create the build
environment from the directory containing `MeshQuokka`:

```powershell
py -m venv .venv
.\.venv\Scripts\python.exe -m pip install platformio==6.2.0
```

The first build needs internet access to fetch the pinned compiler, framework
and libraries. Python 3.14 was used for the reference build. The optional PDF
generator also requires ReportLab; it is not needed to compile the firmware.

From the parent workspace, use:

```powershell
.\MeshQuokka\tools\build.ps1
.\.venv\Scripts\python.exe MeshQuokka/tools/package_release.py
.\MeshQuokka\tools\build.ps1 -Environment MeshCore_Enhanced_TDeck_Plus_GPS9600
.\.venv\Scripts\python.exe MeshQuokka/tools/package_release.py MeshCore_Enhanced_TDeck_Plus_GPS9600
.\.venv\Scripts\python.exe -m unittest discover -s MeshQuokka/tests -v
```

The project virtual environment contains PlatformIO. Its toolchain cache is
the parent's `.platformio` directory. `build.ps1` temporarily maps a free drive
letter to this workspace to avoid GCC 8's Windows command-line length limit,
then removes the alias. It synchronizes and byte-verifies application sources.

The alternative `MeshCore_Enhanced_TDeck_Plus_GPS9600` build starts GPS at
9600 instead of 38400. Both now automatically search UART baud rates while GPS
is enabled and no valid NMEA is received. Separate flashing is normally not
needed merely to try another baud rate.

## Source layout and provenance

`firmware/` preserves the official MeshCore tree and licences. Editable local
application code is `vendor/meshdeck/firmware/`; the compiled copy is
`firmware/examples/meshdeck/`. `tools/sync_application.py` must run after edits.
Root `platformio.local.ini` is authoritative. The one-time `harden_*` and
`integrate_*` scripts record development transformations; do not rerun them
against already-modified files. They intentionally assert their expected input.

MeshCore baseline: `bbb58cceb852a9190b46d5f6984e8e5140e6991e`, the revision
selected by MeshDeck's build script. The ESP32 BLE receive-queue correction
is imported from official MeshCore `d2096bd90e65cd5213e612811eda0bc214e1ab1c`.
The RF packet and cryptographic implementations are retained.

See `DEPENDENCIES.json` for actual checkout and resolved package provenance.
The enhanced environments pin registry dependencies to the reference build
versions. Unused upstream environments retain their original version ranges.
Toolchain/package hashes and dependency metadata are recorded for auditing.

## Private data

`private-backups/` is excluded from version control and release packaging.
Full-flash backups can contain private identity keys, messages and Wi-Fi
passwords. This workspace is under OneDrive: its synchronization settings
apply to those files. Keep recovery copies private. Never distribute that folder.

The current development build does not enable secure boot, flash encryption
or encrypted NVS. UI passwords are masked and are excluded from diagnostics,
but physical flash access can recover stored credentials. Do not represent
this firmware as providing hardware-backed key protection.

## Current release: 0.1.0_261115c

GPS positioning is confirmed outdoors and in live USB diagnostics. Home uses
an enlarged blue MeshQuokka title; the copyright remains in About.
Map now loads OpenStreetMap street tiles for the visible viewport over WiFi,
caches them on SD and overlays positions. See `docs/STREET_MAPS.md` for controls,
cache behaviour, provider terms and offline vector-pack alternatives.

Version u enables the module's DIO2 RF-switch control, matching LilyGO's
factory RadioLib setup. Version v disables periodic receiver restarts by default,
retains USB radio-register diagnostics and adds Heard > Traffic packet history
and advert-processing counters. Range and sparse discovery still need field comparison. Passive reception runs on all screens. RX_DONE polling backs up radio interrupts,
and remote-screen transfers service the mesh loop. A signed nearby room advert
was received without Discover during testing, but sparse long-duration traffic
still requires field comparison; this is not a claim of complete radio qualification.

Private backups, serial logs, device credentials and build caches are excluded from source bundles.

Version x adds a discovery sound for new nodes, including while the screen sleeps. It honors Sound/Volume and queues speaker playback off the radio/UI loop. Both screen-on and screen-asleep previews are user-confirmed.
