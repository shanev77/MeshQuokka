# MeshQuokka quick start

Version 0.1.0_261115c - LilyGO T-Deck Plus 915 MHz.

The complete current operating and technical reference is
[the PDF manual](docs/MeshQuokka-User-Manual.pdf), with editable source in
[docs/USER_MANUAL.md](docs/USER_MANUAL.md).

1. Fit the correct antenna. In Settings, verify the local radio profile:
   Bunbury/WA reference 923.125 MHz, BW62.5, SF8, CR8 and two-byte paths.
   Saved settings survive updates.
2. Set the node name and Perth time zone UTC+8.
3. Reception and advert discovery run continuously on every screen, including
   Home and while the display sleeps. Discover sends your own advert; it is
   optional and does not request a complete directory of the mesh.
4. Use the trackball, physical keyboard or touch. Tap < Back to return.
   Keys opens the on-screen keyboard in supported text editors and Chat.
5. Enable GPS and allow a clear sky view for position acquisition. Incoming
   NMEA confirms the UART link; a valid fix confirms positioning.
6. Connect the Android MeshCore app over Bluetooth using the displayed PIN.
   Forget an old saved pairing if authentication fails after a firmware change.
7. For street maps, insert the SD card and connect Wi-Fi. Map fetches only
   visible tiles, caches them and can revisit cached areas offline. Pan or
   use + / - to zoom; C centres on your position. See docs/STREET_MAPS.md.
8. Settings > Archive messages to SD exports message history without deleting it.

Radio registers match the saved profile, and unrestricted automatic contact
addition is enabled on the tested device. Sparse network reception is still
under investigation; do not interpret a listening indicator as proof of range
or complete network discovery. The manual distinguishes confirmed functions
from outstanding interoperability and endurance tests.
