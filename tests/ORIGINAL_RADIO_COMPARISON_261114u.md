# Original-firmware and T-Deck radio comparison

14 November 2026; same LilyGO T-Deck Plus 915 MHz device.

## Scope and evidence

The original T-Deck application is closed source. The exact original 16 MiB
flash image was retained and its SHA-256 rechecked. Comparing the public
MeshCore core is not equivalent to inspecting the closed-source T-Deck app.
No claim of identical application internals is made.

Before the on-device comparison, the complete current PacketDeck t flash was
backed up. Each 64 KiB candidate block was matched against the chip's MD5;
mismatches were read from the chip and verified. The resulting complete 16 MiB
image was checked against the device's whole-flash MD5, then saved and SHA-256
hashed. Only 327,680 bytes needed transfer; all 16 MiB were verified.
Both recovery images and raw logs remain private and are excluded from releases.

## Hardware interface audit

| Item | PacketDeck / reference evidence |
|---|---|
| Radio | SX1262 through RadioLib; MeshCore packets and cryptography retained |
| SPI | SCK40, MOSI41, MISO38; chip select9 |
| Other MCU pins | DIO1/IRQ45, reset17, busy13; peripheral power10 |
| Oscillator | DIO3 TCXO control at 1.8 V, matching public T-Deck configurations |
| Current limit | 140 mA, matching manufacturer test and public MeshCore target |
| Receive gain | Boosted, verified register 0x96 |
| Network profile | 923.125 MHz, BW62.5, SF8, CR8; original network record numeric prefix agrees |
| LoRa format | Private sync 0x12 (register 0x1424), explicit header, CRC enabled |
| Path support | One/two/three-byte parsing; saved two-byte transmit paths; no additional auto-add hop cap |
| SPI arbitration | One shared SPI object with per-device transactions; main-loop access for radio/display/SD |

## Discrepancy found: DIO2 RF-switch control

PacketDeck through version t inherited `SX126X_DIO2_AS_RF_SWITCH=false`
from the public MeshCore T-Deck board target. The manufacturer factory test
calls RadioLib begin without disabling its default DIO2 RF-switch control;
the manufacturer's bundled RadioLib begin explicitly enables it. Meshtastic's
T-Deck board definition likewise enables DIO2 RF-switch control and documents
the module's internal TX/RX-switch wiring. This is a hardware reference only;
PacketDeck remains on MeshCore and does not use Meshtastic packets or channels.

Version u overrides the inherited false setting with true. It adds a guarded,
nonpersistent `rfswitch 0/1` diagnostic to compare both settings in the same
application, plus reporting of the driver's DIO2-control state. Frequency,
modulation, identity, keys, protocol and contact preferences are unchanged.
This difference warrants testing; the source comparison alone does not prove
it caused the sparse reception or establish the original closed application's
exact register settings.

The other intentional driver differences are RX_DONE polling as an interrupt
fallback and guarded periodic standby/RX restarts. The full recalibration
variant was rejected because the tested device repeatedly latched oscillator
errors. Current standby/RX recovery preserved calibration and passed error checks.

## Primary references

- https://github.com/meshcore-dev/MeshCore/blob/main/docs/faq.md (source availability)
- https://raw.githubusercontent.com/meshcore-dev/MeshCore/main/variants/lilygo_tdeck/platformio.ini
- https://raw.githubusercontent.com/Xinyuan-LilyGO/T-Deck/master/examples/UnitTest/UnitTest.ino
- https://raw.githubusercontent.com/Xinyuan-LilyGO/T-Deck/master/lib/RadioLib/src/modules/SX126x/SX126x.cpp
- https://raw.githubusercontent.com/meshtastic/firmware/master/variants/esp32s3/t-deck/variant.h
- https://perth.meshcore.au/ (regional MeshCore profile)

## Original-image runtime observation

The exact original full image was restored at 0x0 with esptool hash verification.
The user confirmed the familiar original UI and read the displayed settings:
923.125 MHz, SF8, CR8 and BW62.5. During the short comparison the user reported
no freshly discovered devices, only the previously saved entries. The original
application emitted no useful USB diagnostics during the passive capture.
This does not establish equivalent sensitivity: the window was short and no
controlled peer was available. It does establish that the visible old device
list cannot be counted as newly received traffic during this comparison.

## PacketDeck controlled switch check

The verified current PacketDeck full backup was restored with a verified write,
then the u application was installed at 0x10000 with another verified write.
The current identity, preferences, channels and message history were retained.
GPS continued producing valid fixes; touch and SD were detected.

Startup with DIO2 control enabled received a one-hop returned copy of this
node's MeshCore advert (RSSI -89 dBm, SNR +4.5 dB). The diagnostic then disabled
control and sent one flood advert; a one-hop self-return was also received
(-86 dBm, +6.8 dB). Re-enabling control succeeded, and another flood advert
returned through one hop (-88 dBm, +4.5 dB). Other MeshCore packet types were
also received. These returns demonstrate a functioning transmit/relay/receive
link, not just an internal transmit-complete indication.

Because both settings received returns, this short comparison does not prove
that disabled DIO2 control caused the original sparse-discovery complaint.
The final setting remains enabled, matching the manufacturer's setup. No
MeshCore network, identity, key or modulation change was used for this test.

## Discovery timing

WA's published setup recommends 60-minute local adverts and 12-hour flood
adverts. Official CLI documentation confirms the different units (minutes for
local, hours for flood). Distant repeaters can therefore take many hours to
appear in a fresh table. This is a plausible explanation, not a measurement
of actual nearby repeater configuration. The original firmware's existing
saved entries are not equivalent to fresh adverts during our short comparison.

- https://github.com/meshcore-dev/MeshCore/blob/main/docs/cli_commands.md


## Final release check

At 180 seconds, u reported RX=5, receive errors=0, TX=5, RX mode active,
radio device errors=0x0000, boosted receive gain enabled and DIO2 control=1.
GPS had a valid fix with 11 satellites; touch reported zero errors. These are
short-window observations, not a calibrated sensitivity or maximum-range test.

Code review confirms main.cpp services the_mesh.loop() before sensors and UI
on every iteration, independently of the selected screen. The advert callback
updates Heard independently of Bluetooth connection state. Current preferences
automatically add chat clients, repeaters and room servers, with no additional
auto-add hop cap. Display sleep does not stop the radio loop. Reception still
requires an advert to reach this radio; it cannot enumerate every remote node
without receiving its announcement. No firmware changes followed the tested u
build; final work updated documentation and distribution artifacts only.
