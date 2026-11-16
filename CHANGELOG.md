# Changelog

## 0.1.0_261114g - diagnostics, SD archives and About

* Added receiver-reported GGA/RMC/GSV diagnostics and extended NMEA buffer.
* Home distinguishes GPS data without a fix from absent data.
* Added bounded SD JSONL snapshots of current message history.
* Added (C)2026 S. van Ingen and Home/About entry, retaining upstream credits.
* Rewrote the complete user/technical manual and prepared clean source staging.


## 0.1.0_261114f - GPS diagnostics and recovery

* Removed the one-second startup-data gate that could prevent enabling GPS.
* Added checksummed NMEA and RX-byte counters, freshness and live baud display.
* Added bounded automatic UART baud search without configuring the receiver.
* Corrected documented MCU UART pins from actual setPins call order.
* Added non-secret Bluetooth authentication result/reason logs.


## 0.1.0_261114e - navigation fix

* Added a visible top-left < Back touch button on every non-home screen.
* Touch handling runs before screen handlers, including maps; editors receive
  their existing cancel/back action before leaving the screen.
* Kept the trackball hold shortcut and shifted/truncated header titles to fit.

## 0.1.0_261114d - development, 14 November 2026

* Removed inherited raw-key USB logging; added non-secret boot/health diagnostics.
* Audited supplied hardware documents; recorded GPS/revision discrepancies.
* Integrated pinned official MeshCore and MIT MeshDeck standalone application.
* Added 923.125 MHz AU defaults, local confirmation and radio rollback.
* Preserved actual BLE companion protocol; imported official receive-queue fix.
* Added GPS and Bluetooth status/control screens and four saved Wi-Fi profiles.
* Replaced blocking Wi-Fi scanning with asynchronous scans.
* Replaced blocking/insecure map fetch with certificate-verified bounded network
  task, SD main-loop writes, partial files, pause and ETag-validated resume.
* Unified display/SD/radio SPI ownership; refused destructive storage auto-repair.
* Hardened SD pack bounds/allocation and corrected stale-coordinate GPS status.
* Added located-node details, client/repeater filters and unlocated list.
* Added reproducible source synchronization, captured flash-layout packaging,
  manifest/checksum tests, hardware test checklist and development user manual.

Not a declaration of full feature completion or hardware qualification.
