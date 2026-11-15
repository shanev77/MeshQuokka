# Hardware compatibility report

Investigation date: 14 November 2026. Target: LilyGO T-Deck Plus with the
868/915 MHz SX1262 module. Actual unit revision and RF variant are not yet
confirmed. This is a source/document audit, not a physical inspection.

## Evidence

All 74 supplied files were inventoried and hashed in
`Documents/audit/inventory.json`. HTML and PDF text extractions and a rendered
schematic are retained there. Saved JavaScript, stylesheets, advertising/video
resources and decorative images are page dependencies, not hardware authority.
Original documents remain untouched in `../Documents`.

Primary evidence: supplied LilyGO documentation, supplied schematic dated
2023-06-01, supplied Espressif ESP32-S3 datasheet; corroborating official
MeshCore `variants/lilygo_tdeck` and `boards/t-deck.json`.

## Supported configuration

| Component | Documented configuration |
|---|---|
| MCU | ESP32-S3, dual-core Xtensa LX7, 240 MHz; 512 KB internal SRAM |
| Memory | 16 MB flash, 8 MB OPI PSRAM; runtime detection still required |
| RF | SX1262; shared SPI SCK 40, MOSI 41, MISO 38; CS 9, IRQ 45, reset 17, busy 13 |
| Display | ST7789, landscape 320 x 240; CS 12, DC 11, backlight 42; shared SPI |
| Keyboard | ESP32-C3 I2C controller, address 0x55; SDA 18, SCL 8, interrupt 46 |
| Trackball | Four pulse inputs 3, 2, 15, 1; centre/BOOT GPIO0; direction mapping needs physical check |
| Touch | GT911, shared I2C, IRQ16; probe 0x5D and 0x14 |
| GPS | Runtime MCU RX44 / TX43; UART pins replace Grove; receiver and baud discrepancy below |
| Wi-Fi | ESP32-S3 integrated 2.4 GHz 802.11 b/g/n station capability |
| Bluetooth | Bluetooth 5 LE, shared 2.4 GHz coexistence; no Bluetooth Classic |
| SD | SPI, CS39, shared bus with display and LoRa |
| Power | GPIO10 peripheral enable; battery ADC4, 100K/100K divider in schematic |
| Charging | TP4065B in supplied base schematic; blue charging indicator; no software fuel gauge established |
| Audio | Schematic I2S amplifier, WS5/BCK7/data6; ES7210 microphone ADC MCLK48/LRCK21/SCK47/data14 |

## Discrepancies and decisions

* LilyGO's saved page identifies MIA-M10Q and gives 9600 baud; the third-party
  LocalMesh guide identifies L76K; MeshCore sets 38400 baud. Neither a GPS part
  marking nor a Plus-specific GPS schematic was supplied. Do not claim a
  resolved receiver revision. Keep explicit 38400/9600 build options pending
  observation of valid NMEA sentences on the real unit.
* The supplied schematic identifies ESP32-S3-WROOM-1 and predates the Plus
  documentation, which says ESP32-S3FN16R8. Use the verified common pin map,
  not package naming as evidence of the actual installed chip.
* Correction from call-site inspection: Arduino setPins takes RX then TX.
  Upstream calls setPins(PIN_GPS_TX, PIN_GPS_RX), so the running firmware uses
  MCU RX44/TX43 despite the macro names. Earlier reports incorrectly inferred
  RX43/TX44 from those names. Verify reception through checksummed NMEA.
* The guide's DFU-screen and MBTiles instructions describe other firmware.
  ROM download mode does not require a functioning display; map formats must
  be documented for the actual implementation.
* Battery percentage is an estimate derived from voltage, not a measured
  state of charge. Charging state is not exposed by an established GPIO.
* 433 MHz units must not use the planned 915 MHz configuration. Installed
  antenna/module matching cannot be inferred from the SX1262 chip alone.
* Microphone functionality is not enabled: preserve centre-button operation.

## Implementation constraints

Assert all three SPI chip selects high before initialization. Use one shared
SPI host with serialized access; two independently routed hosts on the same
pins are unsafe. Keep large display buffers in PSRAM. Do not enable unverified
power-control pins. Do not format an existing filesystem on mount failure.

Flash configuration inherited from the official board: QIO, 80 MHz, 16 MB,
OPI PSRAM. Partition addresses must come from the installed build framework
and generated partition binary, not this report. Peripheral acceptance tests
remain separate from the USB identification recorded below.

## Connected-unit observation

On 14 November 2026, esptool on COM3 identified ESP32-S3 revision v0.2,
40 MHz crystal, 16 MB flash and quad flash eFuse configuration. The user
confirmed the 915 MHz variant. This resolves the RF variant selection but
not the GPS-module discrepancy or physical PCB revision. The installed
development build subsequently reported 8,213,043 bytes of free PSRAM and
85,276 bytes of free internal RAM at 30 and 60 seconds uptime. This is an
initial runtime observation, not a stress or memory-integrity test.
