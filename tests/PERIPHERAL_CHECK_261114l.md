# PacketDeck processor/peripheral check - 14 November 2026

Firmware: 0.1.0_261114l. Observed live over USB for 95 seconds without
rebooting, changing radio settings or sending receiver configuration commands.
Raw logs remain in private-backups/peripheral-check-l.log and are not distributed.

## Radio

SX1262 SPI status repeatedly returned 0x52; its mode bits match RadioLib's
RX constant 0x50. The driver's receive state agreed. Received packets increased
from 5 to 6 and completed transmissions from 6 to 7; receive-error count stayed
zero. This exercises MCU/chip communication and the driver's receive/interrupt
path. Saved profile: 923.125 MHz / BW62.5 / SF8 / CR8 / 22 dBm.
No node advertisements populated Heard during this check. Successful transmitter
completion does not establish remote delivery, range or message acknowledgement.

## GPS

UART stayed at nominal 38400 baud (reported 38401). RX bytes increased from
563791 to 642338, and checksum-valid NMEA sentences from 11874 to 13490.
The newest valid sentence was about 0.7 seconds old at health samples.
GGA and RMC counters continued increasing at approximately one per second.
The receiver reported GGA quality 0 and RMC status V; parser validity correctly
remained false. Last GSV report indicated five satellites in view, none used.
This confirms receiver-to-MCU data flow and NMEA processing. It does not establish
a position fix or validate MCU-to-receiver configuration commands, which the
current application does not need for ordinary reception.

## Result

Both peripheral links show working communication with the processor/program.
Radio packet reception and transmitter completion are observed. GPS positioning
remains unverified because the receiver itself has not supplied a valid fix.
Firmware uptime continued through the samples; internal free memory was stable
at 46448 bytes in the later observations. No firmware change was made for this check.
