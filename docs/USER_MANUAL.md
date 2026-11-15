# 1. This version and what has been verified

This manual covers MeshQuokka **0.1.0_261115c** for the **915 MHz LilyGO T-Deck Plus**, dated 14 November 2026. It describes the installed firmware, not the original MeshCore phone app or another T-Deck graphical firmware. MeshQuokka means Data Packet Radio on the MESHCORE network. The boot screen has a packet-and-radio-wave mark, the Home screen carries the MeshQuokka name, and About credits S. van Ingen. MeshCore and MeshDeck remain credited as upstream projects. Internal build environment names, BLE service/name compatibility, map directories and settings paths remain unchanged for compatibility.

The handheld runs the mesh client locally. Its keyboard, trackball and touchscreen provide messaging and settings without a phone. Bluetooth exposes the MeshCore companion interface to the phone app. Wi-Fi supports network time and map downloads; it is not required for ordinary LoRa messaging.

## Evidence and limits

| Area | Current evidence |
|---|---|
| Installation | A complete original 16 MiB flash backup was verified before installation. Firmware writes passed esptool hash verification. Subsequent fixes updated the application partition only. |
| Display and navigation | User confirmed the Home/setup screen, trackball, Back button and version-i on-screen keyboard work. |
| Bluetooth | User confirmed the Android MeshCore app can now connect after troubleshooting saved pairing. End-to-end phone message exchange has not been independently recorded. |
| GPS | User confirmed an outdoor position fix. Subsequent live USB diagnostics showed GGA quality 1, RMC A and 12 satellites used, with valid NMEA at approximately 38400 baud. |
| Street maps | User confirmed fresh street tiles load and touch Back works on version q. USB logs recorded successful HTTPS responses and PNG decoding without a restart during the follow-up test. |
| SD and About | The card mounted (30,420 MiB reported capacity); the user reported success for the archive/Home/About check. The copyright is (C)2026 S. van Ingen. |
| Radio | Version u received five packets with zero receive errors during the three-minute comparison, including one-hop relayed returns of its adverts and other MeshCore traffic. Radio device-error flags were zero. The profile is 923.125 MHz / BW62.5 / SF8 / CR8, two-byte paths. Normal long-range discovery remains unqualified. |
| Build | The default environment compiles and passes release-integrity, source-sync and supplied-map checks. Check each flash manifest for its exact build version and size. |
| Qualification | Long-duration LoRa/BLE/Wi-Fi coexistence, complete peer interoperability, battery runtime and SD updates remain unqualified. Full-flash restore was exercised on this unit during the original-firmware comparison. |

This is a development release with useful working features. It is not completion of every feature in the original project brief. In particular, the on-device arbitrary-radius raster tile planner is not implemented. Interactive street maps download visible raster tiles; optional vector packs remain available for offline areas.

## Changes that matter in this version

Version u enabled the SX1262 module's DIO2 RF-switch control, matching LilyGO's factory RadioLib setup. MeshQuokka remains on MeshCore with unchanged network settings. Periodic receiver restarts are disabled by default in v; the missed-interrupt polling fallback remains enabled. USB diagnostics can compare switch control off/on without saving a preference. Sparse discovery still requires field validation; hardware communication alone does not establish normal range.

Versions l-q enlarge/reposition the Home title, retain copyright only in About, align fresh settings with WA/Bunbury SF8 and two-byte paths, correct two-byte Trace handling, add ten-second GPS signal reporting, and add WiFi street maps with an SD cache. GPS positioning is now confirmed. A valid room-server advert was received passively during testing; the reason for sparse long-duration traffic is not yet established.


Version j renames the app MeshQuokka, adds its native vector boot logo and live Heard reception diagnostics. Heard shows RX packet count, receive errors and listening/busy status. It continues to list validated node adverts; generic encrypted packets do not provide a trustworthy sender identity for a device entry. The counters and Heard list reset at restart. Version k moves the Home name down 10 pixels to clear the mesh label. The user confirmed the boot logo looks good and Heard reports listening. During the USB observation, the SX1262 reported RX mode (chip status 0x52); one packet was received with zero receive errors. No validated node adverts had populated Heard at that point. This verifies observed packet reception, not a complete peer/range/coexistence test.

Version e added the visible **< Back** control. Version f removed the startup GPS detection gate that could prevent later activation, adds UART baud searching and exposes receive counters. Bluetooth authentication results are now logged without printing the pairing PIN. Version g increases the sentence buffer for extended NMEA and distinguishes GPS data / no fix from waiting for data on Home. Version h corrects the sentence buffer to 240 bytes, within MicroNMEA's 8-bit size limit. Version i refreshes GPS/Bluetooth status every second, adds a touch keyboard, masks the Settings Wi-Fi password editor, expands Repeaters and Trace to the 350-contact capacity using PSRAM, and scrolls Trace targets. Updates between enhanced versions preserve their settings and identity. The original firmware's /ripple storage format is different: preserving its files does not automatically import its identity, contacts, settings or messages.

# 2. Getting started and normal operation

## First-use checklist

1. Attach the correct 915 MHz antenna and use a charged battery or stable USB power.
2. Open Settings and set **Node name**. Set **Time zone (UTC+)** to **8** for Perth.
3. Check the Radio screen against your local mesh. This build starts with **923.125 MHz, SF8, BW62.5, CR8** on fresh settings. All four parameters must match the people you want to reach.
4. Open GPS and ensure Receiver says enabled if you want GPS. Position sharing is a separate setting; enable it only when wanted.
5. Use Discover to send your advert, then watch Heard and Contacts. A nearby known working peer is the best way to test messaging.
6. Connect the phone from the MeshCore app if desired. Use the PIN displayed on the T-Deck Bluetooth screen.

Existing settings survive an application update, so a device already configured may not show fresh-install defaults or onboarding. Do not repeatedly change radio settings just because no peers appear immediately.

## Power, sleep and waking

Screen timeout controls display sleep. Always-on clock changes the idle display behaviour; it does not make the device a deep-sleep node. A first key, trackball movement or touch can be consumed solely to wake the screen. Perform the intended action again after it wakes.

Battery percentage is an estimate from voltage, not a calibrated fuel gauge. The charging LED is a hardware indication; the software has no verified charge-status input. Runtime depends strongly on display brightness, radio activity, GPS, Bluetooth and Wi-Fi. No battery-life figure has been measured for this build.

## What connectivity is required?

| Task | Required connection |
|---|---|
| Local messaging, contacts and radio settings | LoRa for communication with peers; no internet or phone required |
| Phone companion | Bluetooth between phone and handheld |
| Obtain map packs on the handheld | Wi-Fi internet, valid clock for TLS and usable SD card |
| View downloaded maps | SD card; no internet required |
| GPS position | Satellite reception; Wi-Fi and cellular service are not required |
| USB flashing and diagnostics | USB data cable and a computer |

# 3. Navigation, keyboard and Home apps

## Reliable ways to go back

Tap **< Back** at the top left of any non-Home screen. Alternatively, hold the trackball centre for more than about half a second. A short press selects; a hold goes back. The user has verified the touch Back button.

Back normally closes the current editor, picker or submenu before leaving the screen. In Settings, it clears an active filter before returning Home. You may therefore need two taps to exit a filtered or nested screen. Back does not undo changes that were already saved.

| Input | Meaning |
|---|---|
| Trackball roll | Move a selection, scroll content or pan a map, depending on the screen. |
| Short centre click / Enter | Select, open or commit. In chat, Enter sends composed text. |
| Hold centre / < Back | Leave a screen or close its current submenu. |
| Touch | Tap items; drag lists/maps. NewMaps also supports pinch and long-touch inspection. |
| Backspace/Delete | Delete a character. With an empty editor or filter, some screens use it to go back. Use < Back if unsure. |
| Number/symbol layer | Alt+C toggles the software layer on supported keyboard output. In Settings/Wi-Fi editors, roll up for numbers/symbols and down for letters. Look for the 123 indication. |
| Escape / backtick | Fallback Back keys only when a screen has not consumed them. They are not universal text-editor shortcuts. |

The display font is printable ASCII. Unsupported Unicode can be replaced or displayed incorrectly. Keep names concise. The keyboard fallback i/k/j/l navigation is also screen-dependent because text entry takes priority.

## Home: all 15 tiles

| Tile | Purpose |
|---|---|
| Chat | Conversations, composing, delivery indicators and sharing actions. |
| Contacts | Saved identities and contact actions. |
| Discover | Send a flood advert and open Heard. This is not a Wi-Fi-style scan. |
| Heard | Recent received stations, signal information and saving contacts. |
| Repeaters | Repeater and room-server contacts, login and remote console. |
| Map | Downloadable street tiles, SD cache, outline fallback and node positions. |
| NewMaps | More detailed OSM-derived .mdv vector packs. |
| Radio | Radio profile, packet activity, memory and storage diagnostics. |
| Trace | Trace a selected contact with a known path. |
| GPS | Enable, link/fix diagnostics and position-sharing control. |
| Terminal | Local command console and operational log. |
| Bluetooth | Enable/disable, phone connection state and pairing PIN. |
| Settings | Full configuration and maintenance actions. |
| WiFi | Network scan, credentials, connect and remembered networks. |
| Channels | Group channel creation, keys and management. |

Home has a large blue MeshQuokka title, a smaller clock and an About touch button. The copyright (C)2026 S. van Ingen appears in About, not on Home. Press ? on Home or use Settings > About as alternatives. About credits S. van Ingen, describes this app, shows the version and acknowledges MeshCore/MeshDeck.

Home shows time/date, node name, unread count, a battery estimate and recent mesh activity. The status bar on other screens adds G for GPS, W for Wi-Fi and B for a connected phone when applicable. Bars and indicators reflect recent/local state, not proof that a distant destination is reachable.

# 4. On-screen keyboard

When a screen accepts text, tap **Keys** at the top right. In Settings it edits the search filter until you select an editable option; after selecting an option it edits that value. In WiFi, Channels, Bluetooth and repeater login, first activate the field. It is available for chat composition, Contacts search, Settings search/values, Wi-Fi SSID/password, channel names/keys, repeater/room login and console, Terminal, Bluetooth PIN editing and initial node naming.

The full-screen keyboard shows the current field and edits it immediately; it is not a separate clipboard. The preview shows the last 100 characters in two lines. Passwords, channel secrets and the new PIN are masked. Physical keys can still edit the same field while it is open.

| Control | Meaning |
|---|---|
| Letter/number keys | Insert a printable ASCII character, subject to the field's existing length and validation rules. |
| Case | Switch between lower-case and upper-case letters. The setting stays active until changed. |
| 123 | Switch between the letters/number layout and the symbols layout. Symbols include punctuation, brackets, slash, backslash and equals. |
| Space | Insert one space. Numeric/PIN fields may reject it. |
| Del | Backspace in the current field. An empty field does not navigate away. |
| Enter | Close the keyboard and perform the field's normal Enter action: send a chat/console command, save a setting or advance an editor. This is an action, not simply dismiss. |
| Hide | Close the keyboard without submitting. Typed edits remain in the field. Trackball Back also hides the keyboard first. Use the screen's normal Back to cancel an editor afterward. |

The keyboard covers the underlying screen while open; Hide reveals it for review. For sending, review the recipient/thread before opening Keys. The user confirmed the installed on-screen keyboard works well. Exhaustive testing of every field and all touch hitboxes remains a hardware acceptance item.

# 5. Chat and direct messages

Open a contact and choose Message to start a direct conversation. For group chat, open the desired channel from Channels. Type in the compose area and press Enter to send. Use trackball up/down to scroll messages and left/right or Tab to change conversation tabs where available. Incoming messages increment unread counts; displaying the latest messages in a thread marks it read locally.

## Delivery indications

An outgoing message may show pending dots, a delivered indication or a failure mark. A MeshCore acknowledgement establishes protocol-level receipt, not that a human read the message. Broadcast/channel delivery does not provide the same recipient-specific acknowledgement as a direct message. A timeout can mean no route, congestion, mismatched radio settings, interference or an unavailable destination.

Keep messages short. The local text buffer is 168 bytes including termination, and the editor leaves additional room; protocol overhead and message type can reduce the useful payload. This is a short-text radio system, not an attachment or large-file service.

## Chat actions

Press **/** with an empty compose field to open Actions. The menu offers Share contact, Quick messages, Jump to latest, and Share this channel when the current thread is a channel. Sharing a channel sends a link containing the channel secret; only do this where recipients should know that secret.

The quick-message picker contains: On my way; OK; Received; Yes; No; At location; Call me; SOS - need help. Select with the trackball/Enter or use the shown number. These are ordinary text messages. **SOS - need help is not an emergency-service connection or guaranteed alert.**

Received supported contact/channel sharing text can open an import confirmation. Check the displayed identity or channel before accepting. A contact public key identifies a node; a channel key is a shared secret. They should not be treated as the same kind of data.

## History and capacity

The local store supports up to 32 threads and 48 messages per thread, using bounded buffers. The contact table and channel table have separate limits. RAM retains up to 48 messages per thread, while the inherited flash saver persists only the newest 16 per thread after an eight-second debounce. A reboot can therefore retain fewer messages than were visible before it. Use Archive messages to SD to preserve the current in-memory history before clearing or rebooting. Old history is not an unlimited archive. Clear message history removes local messages; it does not recall messages already sent or remove the phone's separate history.

Phone and handheld each maintain their own UI state/history. Concurrent local and phone sending and all duplicate-history cases have not been fully qualified. If diagnosing a message problem, test one sender path at a time.

# 6. Contacts, Discover and Heard

Contacts represent saved public identities and their known routes. Type to filter the list by name. Roll to a match and select it to open its action menu. This build reserves up to 350 contacts, but the UI may impose smaller limits in particular views.

| Contact action | Meaning |
|---|---|
| Message | Open a direct conversation with this identity. |
| Show path | Inspect the stored outbound route. An unknown route is different from a direct route with no repeaters. |
| Trace route | Attempt a diagnostic trace using a known path. It does not guarantee a new route will be discovered. |
| Reset path | Discard the learned outbound path so future protocol activity can relearn one. |
| Share (0-hop) | Share the contact using the inherited zero-hop advert mechanism with nearby nodes. |
| Remove | Delete the saved contact locally. It does not delete that remote node or necessarily erase its old chat history. |

Discover sends your advertisement using flooding, then opens Heard. Wait for peers to advertise or respond. A silent node may not appear merely because it is nearby. Repeated Discover presses use airtime and do not overcome incompatible radio settings.

Heard shows recent station name, age, SNR, RSSI, hops and distance when coordinates permit it. Enter/Select saves the highlighted entry as a contact when sufficient identity information is available. A record in Heard is evidence of a received packet, not a permanent online-status promise.

Auto-add contacts in Settings controls automatic admission versus manual adding. Use manual mode if you prefer to manage a crowded contact table. Name matching is not identity verification: different nodes can use the same name, so use their public identities when certainty matters.

# 7. Channels and shared keys

Channels are shared-key group conversations. The Public channel is seeded by the application. It is shared with others who know the public configuration and must not be regarded as private.

Choose **+ Add**, enter a name, then provide or create a key. A blank key with a name beginning **#** produces a deterministic hashtag channel. A blank key with another name generates a new random private channel key. Pasting a compatible base64 key joins the group that uses that secret. The name alone is not sufficient for a randomly keyed private channel.

| Channel action | What it does |
|---|---|
| Open chat | Opens that channel's conversation. |
| Rename | Changes the locally stored label. It does not automatically rename the channel on other devices. |
| Change key | Replaces the shared secret. Other participants must use the same new key to communicate. |
| Show key | Displays the key on the handheld. Keep it out of screenshots intended for public sharing. |
| Share key (QR) | Displays a shareable QR/link for a compatible app. Anyone receiving it can obtain the channel secret. |
| Remove | Clears the local channel slot. It does not erase the channel from anyone else's device. |

The protocol configuration allows up to 40 group-channel slots. Local chat history remains limited to 32 threads across channels and direct conversations. Hashtag channels are convenient for public communities but their keys are derived from the hashtag; a hard-to-guess name is not a replacement for a random secret.

Channel encryption protects traffic using the shared secret, but every participant possessing that secret is inside the group trust boundary. Do not publish a private key as part of a bug report, repository, terminal screenshot or manual example.

# 8. Why the old firmware showed more devices

A decline in newly received adverts is still being investigated; old saved history alone does not establish why reception has changed. These are different lists. Contacts is the enhanced firmware's saved table (maximum 350). Repeaters filters that table to repeater/room roles. Heard is a bounded list of 64 recently heard senders and starts fresh after reboot. Neither is a complete map of all powered-on devices. A radio must hear adverts/traffic before a previously unknown peer appears.

The verified original flash contains a /ripple filesystem layout, including 141 blob files. These files are not equivalent to 141 verified live contacts, and this firmware has no validated importer for that format. The original backup has been retained privately; it was not erased. Preserving a filesystem partition does not make incompatible old records appear in a new application's contact table. Do not assume the original identity or private channel keys were migrated either.

USB diagnostics for version i reported one saved contact and automatic contact addition enabled. The device's actual profile was 923.125 MHz, BW62.5, SF8, CR8, 22 dBm, consistent with the original network record's numeric prefix. The fresh-install default and WA/Bunbury preset now also use SF8; updates preserve saved settings. The old record format has not been fully decoded. No speculative contact/private-key conversion has been attempted.

Passive reception does not require Discover or an open Radio screen. Two-byte paths support up to 32 hops; this device has automatic addition enabled for chat devices, repeaters and room servers with no extra auto-add hop limit. Optionally use Discover and allow time to hear adverts, with the same exact modulation settings as a known peer. Import supported contact/channel shares through the phone or QR/link workflow if available. Discovery sends your advert; it cannot force every remote node to reply. The version-i Repeaters fix removes the previous 24-entry display cap, but does not reconstruct the old history.

WA's published repeater setup recommends local adverts every 60 minutes and flood adverts every 12 hours. Local adverts do not travel through multiple repeaters. Distant nodes may therefore take much longer to populate a fresh contact table. Individual operators can choose different intervals; this is a possible explanation, not a measurement of the repeaters around you. Sources: https://perth.meshcore.au/ and https://github.com/meshcore-dev/MeshCore/blob/main/docs/cli_commands.md .

# 9. Repeaters, rooms and remote console

The Repeaters tile lists saved contacts advertising repeater or room-server roles; this view can list up to 350 saved entries in PSRAM, matching the contact-table capacity. The handheld remains a client. Selecting a repeater here does not convert it into a repeater.

| List key/control | Meaning |
|---|---|
| Enter / centre / Space | Open the selected session, use a saved login when available, or open the password prompt. |
| L | Open the login prompt explicitly. |
| C | Open the selected peer's console. |
| R | Request room re-login/full backlog resynchronization using saved credentials. Rooms only. |
| A | Toggle automatic room login after credentials have been saved. Repeater auto-login is intentionally not enabled. |
| F | Forget the saved password for the selected peer. |

The login screen labels the required password as Room password or Repeater password. Enter submits. Trackball up/down toggles whether the password is shown. For rooms, A/Tab toggles Remember & auto-login when the prompt allows it. Repeater credentials can be remembered for one-tap login without automatic login at boot.

IN indicates an active session. AUTO indicates saved room auto-login. KEY indicates saved credentials. A sync countdown means the room is sending backlog. Wait for synchronization to finish before posting; the radio is half duplex and transmitting during backlog reception can cause missed packets.

## Remote console shortcuts

| Command | Expansion or action |
|---|---|
| /l or /login | Opens login. |
| /r or /resync | Requests room resynchronization. |
| /s or /stats | Sends stats to the selected remote peer. |
| /a or /advert | Sends advert as a remote CLI command. |
| /c or /clock | Sends clock sync. |
| /n or /neighbors | Requests neighbors. |

Other typed text is sent as a remote command. Available commands and permissions depend on the remote firmware and your login rights. These are not the same as local Terminal commands. Remote command text and replies can appear in logs; do not assume every possible command argument is redacted.

# 10. Understanding the radio settings

The hardware is the user-confirmed 915 MHz variant. The enhanced AU policy accepts settings whose channel bandwidth fits inside 915-928 MHz. This software range check is not RF certification or a guarantee that any antenna/power combination complies with local rules.

| Term | Meaning and practical effect |
|---|---|
| Frequency, MHz | Centre of the LoRa channel. Peers need compatible frequency and modulation. The default is 923.125 MHz. |
| Spreading factor, SF | LoRa symbol spreading, 7-12 here. Higher SF generally increases airtime and can improve sensitivity; changing only your device can make it unable to decode the local mesh. |
| Bandwidth, BW | Width of the modulation in kHz: 62.5, 125, 250 or 500. Wider bandwidth generally increases data rate while changing sensitivity and channel occupancy. |
| Coding rate, CR | Error-correction denominator: CR5 through CR8 represent rates 4/5 through 4/8. More redundancy increases airtime. Match the network. |
| TX power, dBm | Radio output setting. 20 dBm is 100 mW; 22 dBm is about 158 mW before antenna/cable effects. More power is not a substitute for matching settings or good placement. |
| Path mode, bytes/hop | Number of identifier bytes used per route hop: 1, 2 or 3. More bytes reduce identifier ambiguity but consume more packet space. It is not a hop-limit control. |
| RSSI, dBm | Received signal strength for a packet. A less negative value generally means stronger reception. |
| SNR, dB | Signal-to-noise ratio. LoRa can decode some packets with negative SNR; do not read it as a distance estimate. |
| 0-hop advert | Advert intended for direct nearby reception without flood propagation. |
| Flood advert | Advert that can propagate through the mesh under MeshCore routing rules. Uses shared airtime. |

Use the community's exact profile. A regional hardware label such as 915 MHz does not tell you the local frequency, SF, BW or CR. This build's default is a starting configuration, not proof of a nearby network's configuration.

Local radio changes require repeating the same action within about eight seconds. The first action prompts confirmation; the second commits. A previous-radio snapshot allows Restore previous radio. Terminal/phone changes do not all use the same local confirmation/rollback path.

# 11. Settings reference: identity and radio

In Settings, typing filters labels. Roll up/down to select. Left/right changes a numeric or toggled value; Enter invokes an action or opens a text editor. In an editor, Enter saves and Back cancels the current edit. Back may first clear a filter. Most committed changes are saved immediately.

| Exact option | Values and meaning |
|---|---|
| Node name | Your display/advert name. Enter edits. Send an advert to announce it; restart for the Bluetooth advertised name to refresh. It does not replace the cryptographic identity. |
| Frequency (MHz) | Text entry; validated against the AU band and current bandwidth. Repeat confirmation to save. |
| Spreading factor | 7-12. Left/right changes by one after confirmation. |
| Bandwidth (kHz) | 62.5, 125, 250 or 500. Left/right steps through choices after confirmation. |
| Coding rate | 5-8. Match the local mesh. Confirmation applies. |
| Path mode (bytes/hop) | 1-3. Changes route encoding width; this is a protocol compatibility/overhead choice. |
| TX power (dBm) | Local Settings editor offers 1-22 dBm. The lower-level policy permits -9 through 22 where other interfaces support it. Confirmation applies. |
| Radio preset setup | Reopens setup/name/preset flow. Choose an AU-compatible profile; visible international presets outside the build's policy are rejected. |
| Restore previous radio | Loads the last stored local radio snapshot when available. It is not a full settings backup and does not restore contacts or identity. |
| Send advert (0-hop) | Immediately announces through the direct advert mechanism. |
| Send advert (flood) | Immediately sends a flood advertisement. |
| Auto-advert (min) | Off, 5, 15, 30 or 60 minutes. Periodically sends a flood advert; more frequent adverts consume more airtime. |
| Auto-add contacts | Auto admits discovered contacts; manual requires deliberate adding. Existing contacts are not deleted merely by changing this switch. |

If an invalid radio combination reaches the underlying application, its policy can reset the profile to valid defaults rather than use an invalid configuration. Inspect Radio afterward. Prefer the Settings editor over Terminal when changing radio parameters because the Settings flow provides explicit validation feedback and rollback.

# 12. Settings reference: display and input

| Exact option | Values and meaning |
|---|---|
| Brightness | Backlight level clamped to 30-255, displayed as a percentage. Left/right changes by 25 units. Lower brightness reduces display power consumption. |
| Screen timeout (s) | Never/0, 15, 30, 60, 120 or 300 seconds of inactivity. This controls the screen, not a guaranteed radio deep-sleep mode. |
| Time zone (UTC+) | Integer offset -12 to +14 hours. Perth is +8. No automatic daylight-saving or fractional-zone selection is provided. |
| Always-on clock | Keeps the idle clock presentation instead of normal screen-off behaviour. Uses more display energy. |
| Sounds | Enables or disables local notification audio. |
| Volume | 0-10. Changing upward plays a short preview tone. Audio quality/loudness is hardware-dependent. |
| Flip display | Changes display orientation. Check touch alignment after altering it. |
| Touch mapping | A-D coordinate transformations. Default C is the intended landscape mapping. If touch targets are wrong, use the trackball to restore a usable mapping. It is not a multi-point calibration wizard. |
| Trackball speed | 1-5. Higher settings require fewer movement pulses for each navigation step. |
| SOS beacon | Inherited enable/disable flag. This build replaced the Home SOS tile with Bluetooth; the switch does not create a reachable emergency workflow. Do not rely on it as a distress feature. |

The on-screen Back button is part of this enhanced build rather than a setting. There is no need to enable it. It is deliberately drawn above normal screen overlays and receives taps before map touch handling.

The display is 320 x 240 in landscape. Long titles and names may be abbreviated. A cropped name is a presentation issue, not necessarily corrupted stored data. Use short printable-ASCII names for easiest operation.

# 13. Settings reference: connectivity and maintenance

| Exact option | Behaviour |
|---|---|
| GPS module | Enables/disables GPS processing. If enabling with a zero update interval, the application sets a five-second interval. It does not guarantee electrical power-off because no separate GPS enable pin is established. |
| WiFi network (SSID) | Edits the selected network name manually. The WiFi tile also offers scanning. |
| WiFi password | Edits the selected password. Both the Settings and WiFi editors mask the password; the touch keyboard also masks its preview. |
| WiFi connect | Connects when off; otherwise disconnects. Uses the currently selected saved credentials. |
| Share location in advert | Controls inclusion of coordinates in supported adverts. GPS can be enabled without sharing your position. |
| Manual latitude | Signed decimal degrees, -90 to +90. Southern latitude is negative. Used when a valid GPS position is unavailable. |
| Manual longitude | Signed decimal degrees, -180 to +180. Western longitude is negative. Manual 0,0 is treated as unset by the inherited model. |
| Room auto-login tries | 1-20 attempts. Applies to saved room auto-login, not repeater auto-login. |
| Clear message history | Immediately clears local message history. There is no extra confirmation in this Settings action. It does not clear the phone's history. |
| Delete all contacts | Immediately deletes all saved contacts locally. No extra confirmation is implemented here. It does not erase the remote nodes. |
| Download maps (WiFi) | Opens the region catalogue downloader. Requires connected Wi-Fi and writable SD storage. |
| Prepare SD (maps folder) | Mounts the card and creates /meshdeck-maps. It is not a disk formatter. |
| Reload SD map packs | Reloads available map packs after copying or downloading files. Releases old classic map allocations before reloading. |
| Update firmware from SD | Looks for /firmware.bin and invokes the inherited application updater. Use an application image, never a merged image. This path has not been hardware-qualified. |
| About | Opens the dedicated About screen credited to S. van Ingen; also logs upstream version/licence information to Terminal. |
| Archive messages to SD | Writes a snapshot of current local messages to /meshcore/archives as readable JSONL. It does not delete history or create a full-device backup. |

Destructive actions above are immediate in this release. Read the label before pressing Enter. Back cannot recover deleted history or contacts; restoration requires an appropriate earlier backup.

# 14. Bluetooth and the Android MeshCore app

Open Bluetooth on the handheld. The screen shows service enabled/disabled, phone connected/disconnected, advertised name and the **active pairing PIN**. Use that PIN, not an assumed default. The displayed PIN was 123456 on the tested device.

## Pairing procedure

1. Enable Bluetooth on the phone and grant the MeshCore app its required nearby-device/Bluetooth permissions.
2. Ensure the handheld Bluetooth service is enabled. E toggles it.
3. Select the node through the MeshCore app. Enter the displayed six-digit PIN when Android prompts for pairing.
4. Confirm that the app completes connection and the handheld reports the phone connected. A discoverable device alone is not an authenticated connection.

If pairing fails after a firmware change, forget the device in Android Bluetooth settings and remove/reselect any stale saved connection in the app, then try again. This resolved the observed connection problem during testing; the user subsequently confirmed Bluetooth connection. No iPhone validation is claimed.

## Controls and PIN changes

E enables/disables the companion service. P opens six-digit PIN entry; Enter saves it. Restart the handheld before pairing with the new PIN. Until restart, the active displayed PIN remains the one used by the running Bluetooth service. After a PIN change, forget the old Android pairing and pair again. Node-name changes also require restart to update the advertised name.

The companion protocol is the real MeshCore binary protocol, not a replacement text service. Although the GATT identifiers are from the Nordic-UART UUID family, a generic terminal is not a MeshCore phone app. Encrypted, authenticated access remains enabled; troubleshooting did not disable pairing security.

This build logs connection/disconnection and authentication success/failure reason codes over USB. Those diagnostic lines do not print the PIN. An app connection test does not by itself prove messaging to another radio peer; test a direct message and reply separately.

# 15. GPS: link, fix and location sharing

Open GPS. E toggles processing; S toggles advert position sharing. Enabling the receiver and broadcasting your location are separate choices. The serial data link and outdoor position fixes are confirmed; live USB diagnostics subsequently showed a valid fix using 12 satellites.

| Field | What it means |
|---|---|
| Receiver | Whether the saved preference enables GPS processing. Disabled means the application is not processing fixes. |
| Link: NMEA received | Checksummed NMEA was received within the last ten seconds. The serial link is working even if Fix still says none. |
| Searching baud / no NMEA | No recent valid NMEA. Automatic UART searching is active while GPS is enabled. |
| Fix | Whether the parser currently has a valid position. A data link alone is not a valid position. |
| Sats used | Satellite count reported for the solution. It is not a complete sky-view list of all visible satellites. |
| Latitude / longitude | Position in decimal degrees, shown when a valid fix is available. |
| Altitude | Parsed altitude in metres. Do not use this unqualified handheld as a precision altitude instrument. |
| RX bytes | Bytes processed from the UART since boot. Increasing bytes prove activity, not that the data is readable NMEA. |
| NMEA | Count of received sentences accepted by the checksum test. Increasing values distinguish data reception from an RF satellite-fix issue. |
| MCU RX44 TX43 / baud | Actual UART wiring used by the upstream initialization and current baud. A displayed 38401 can be normal clock-divider rounding of 38400. |

When no fix is available, g also shows Receiver GGA quality, RMC status and In view (last). GGA quality 0 means the receiver reports no valid solution; RMC V means invalid navigation data and A means valid. In view is the last GSV report, not a combined constellation total. These fields help distinguish receiver acquisition from the higher-level parser.

## Automatic baud searching

When enabled without recent valid NMEA, the firmware tries 9600, 38400, 115200 and 57600 baud at six-second intervals. It changes only the MCU UART rate and sends no receiver-configuration commands. Valid NMEA locks the rate even before a fix. A ten-second absence of valid sentences permits searching again.

On this device, the diagnostic capture showed NMEA at approximately 38400 baud. Thus the earlier failure was not evidence that the module needed 9600. The startup gate was also preventing recovery when no bytes had been observed during its initial detection window; version f removes that gate.

If NMEA increases but Fix remains none, give the receiver a clear sky view and time to acquire satellites. If RX bytes remain zero, investigate the link or enable state. If bytes increase but NMEA stays zero across a full search cycle, investigate baud/wiring/output format. The exact receiver model remains unverified because supplied documents conflict. Speed, course, HDOP and a manual baud-setting UI are not exposed in this version.

GPS-derived time is used internally when the existing time-sync conditions are met. The GPS screen in f prioritizes receive diagnostics rather than displaying a UTC epoch field. The Home clock uses the configured time-zone offset.

# 16. Wi-Fi and saved networks

Wi-Fi is for downloads and time synchronization. It is not a LoRa backhaul or automatic internet bridge for messages. Use a compatible 2.4 GHz network; a 5 GHz-only SSID will not provide an ESP32-S3 Wi-Fi connection.

| WiFi screen item | Meaning |
|---|---|
| Network (SSID) | Starts an asynchronous scan and opens the network picker. The picker also offers Enter name manually. |
| Password | Edits the selected network password with masked characters. Enter saves. Roll up/down for number/letter mode. |
| Connect / Disconnect | Starts or stops the selected connection. Connected state shows its local IP address. |
| Next saved network | Cycles among remembered network profiles for selection. It does not mean automatic roaming is active. |
| Forget selected network | Removes the selected remembered profile. Check the SSID before using it. |

The scan list retains up to 16 named networks. Hidden networks can require manual SSID entry. Signal bars describe the scan's received strength. Selecting another SSID does not intentionally reuse a previous network's password; saved matching credentials can be loaded.

There are four remembered profiles. An existing SSID is updated; when all slots are occupied, the implemented replacement policy uses the last slot for a new network. Connection retry concerns the selected active network. Automatic roaming among all remembered networks is not implemented.

An NTP time-sync attempt runs after Wi-Fi connection. An incorrect clock can cause HTTPS certificate validation failure. Check network access, time, DNS and storage if map downloads fail. Captive portals and enterprise authentication are not implemented as guided login workflows.

Passwords are stored locally without flash/NVS encryption. The WiFi screen masks them; the legacy Settings password editor and remote command consoles are not uniformly masked. Protect physical access and private backups.

# 17. Street maps, outline maps and node positions

Map opens a street-map layer using OpenStreetMap raster tiles. With a position available on first entry it centres at street zoom; otherwise it uses a located contact or the Australian overview. Connect WiFi and insert an SD card. Visible tiles download automatically as you pan or zoom; completed tiles stay on SD for repeat viewing without Internet. Missing areas/zoom levels still need connectivity. This is a labelled street map, not photographic Google Street View or turn-by-turn navigation.

Touch the C, - and + buttons to centre or zoom. The trackball and drag pan the view. Press V to switch to the original Natural Earth coastline outline. Live own position and advertised contact markers are overlaid on both modes. A crosshair marks the viewport centre, not a fabricated GPS fix.

Only the current viewport is requested, with one network transfer at a time. The cache is /meshdeck-maps/streets/; files use zoom_x_y.png names. Six decoded 256x256 RGB565 tiles live in PSRAM. Cache files are retained indefinitely; there is no automatic SD eviction. The card stays mounted to preserve open transfer files; power down before card removal and restart after reinserting it. Internet downloads use HTTPS, an identifying MeshQuokka User-Agent and certificate validation. TLS memory is allocated in PSRAM before network tasks start. The bounded response is buffered in PSRAM and the HTTPS connection is closed before its data is queued for SD writes. A guarded TLS client prevents repeated cleanup from closing an unrelated filesystem descriptor. A failed request backs off for 60 seconds.

OpenStreetMap contributors are credited visibly on the map. The public tile service permits interactive viewing and caching but not bulk offline-region downloads. For larger offline regions, use the separate NewMaps .mdv workflow or a service explicitly allowing offline packs. See https://operations.osmfoundation.org/policies/tiles/ and https://www.openstreetmap.org/copyright . Requested tile areas are disclosed to the map provider.

Advanced users can put an HTTPS XYZ template in /meshdeck-maps/streets/source.txt using {z}, {x}, {y}, then restart. The provider must supply non-interlaced 256px PNG tiles, have a trusted CA, and permit this use. Change attribution in source if required by that provider. Move the previous provider's cached PNGs off the card before switching. This is not an API-key registration service.

| Control | Action |
|---|---|
| Trackball / touch drag | Pan the map. |
| + / -, O / I, Q / A | Zoom. Centre selection can cycle zoom when it does not select a nearby node. |
| C | Centre on an available own GPS/manual position. Continuous follow is not implemented here. |
| Tap marker / select near centre | Inspect a located contact. |
| F | Cycle all located nodes, repeater nodes or client nodes. |
| L | Open the unlocated-node list; roll to scroll. Close it with Back or another supported key. |
| < Back | Close node/list inspection first, then return to the previous screen. |

Node details include name, public identity, advertised coordinates, timestamp, and distance/bearing when your own position is available. Distance is straight-line distance, not a driving route. Advert timestamps and positions are supplied by remote nodes and can be stale or incorrect. Nothing is inferred from RSSI to invent coordinates.

## Outline-map SD packs

Use a compatible FAT32 microSD card. Create **/meshdeck-maps/** and copy .mdm files there, then choose Settings > Reload SD map packs. Preparing the folder does not format the card. Do not format media containing wanted data just to create the directory.

Bundled files include **auw.mdm** for Australia West, including Perth, and **aue.mdm** for Australia East. The catalogue has no single au.mdm file. Classic maps load at most four packs. Keep only the desired classic packs in the directory to avoid relying on filesystem enumeration order.

These packs use public-domain Natural Earth data and are intentionally coarse. A small file or missing local street does not necessarily mean the download failed. For richer vector detail, see NewMaps.

# 18. NewMaps and detailed vector packs

NewMaps displays separately generated **.mdv** packs containing OSM-derived layers such as roads, trails, rail, water, land use and optional buildings. It is a separate renderer from the classic .mdm Map screen. The firmware supports legacy MDV1 and tiled multi-level MDV2, with up to 16 region packs and three level-of-detail records in the format.

Copy compatible .mdv files to **/meshdeck-maps/**. Reload/restart as needed to detect newly copied packs. If the screen says No NewMaps pack on SD, a classic .mdm file is not a substitute for a .mdv pack.

| Control | Action |
|---|---|
| Trackball / touch drag | Pan. |
| +, =, Q or O | Zoom in; bracket aliases are also supported. |
| -, _, A or I | Zoom out. |
| Centre click | Zoom in, or close the current inspection panel. |
| C | Re-centre on own position when available; otherwise use the first pack's centre. |
| 0 / 9 | Jump to the widest / tightest configured zoom level. |
| Pinch | Step zoom in/out. |
| Long touch | Inspect a nearby feature; tap/Back closes the details. |

Labels and layers change with zoom to limit rendering work. A road, building or place absent from a particular view can be outside the pack, filtered at that level of detail or absent from the source data. No turn-by-turn navigation or route calculation is provided.

The source includes **vendor/meshdeck/tools/gen_newmap.py**. Run it with --help before generating a pack; it supports a bounding box and local PBF/GeoJSON inputs as well as its other acquisition options. PBF processing needs its optional dependencies. Pack generation is a desktop developer workflow, not the promised on-device radius planner. Preserve OpenStreetMap attribution and applicable ODbL obligations for derived data. Do not bulk-prefetch the public OSM raster tile server.

# 19. Downloading and maintaining maps

For street maps, connect WiFi and open Map: visible tiles load automatically. The following catalogue workflow applies only to classic coastline packs. Connect Wi-Fi, insert the SD card and open Settings > Download maps (WiFi). The firmware retrieves a catalogue over HTTPS, lists named regions and lets you filter by typing. Select a region and press Enter or tap it to start a classic .mdm download.

## Transfer behaviour

P pauses an active transfer. Enter on the selected region retries/resumes when possible. Progress shows received and total bytes once the server provides the length. The temporary file remains separate from the old complete map until transfer completion. After success, choose Reload SD map packs.

Resume uses a partial file, a strong ETag and HTTP Range/If-Range. A matching server object can resume; a server response containing the full object restarts the partial download. The implementation validates the content range rather than blindly appending. A changed or unavailable object may require retrying from the beginning.

The network worker receives bounded chunks and never directly accesses SPI, SD, the UI or radio. The main loop drains at most two 512-byte data events per poll. A pack is limited to 4 MiB in this downloader, and the catalogue to 4095 bytes. SD free space is checked before writing.

## Common problems

| Symptom | Action |
|---|---|
| Connect WiFi first | Return to WiFi, connect and reopen the downloader. |
| Cannot fetch index / transfer failed | Check internet, DNS, clock, TLS trust and SD mount; reopen/retry. Do not disable certificate verification. |
| SD full / write failed | Preserve existing files, free space on a computer, check the card and retry. |
| Download complete but map unchanged | Reload SD map packs and confirm the file format matches Map versus NewMaps. |
| Interrupted transfer | Reopen and retry the same region. Retain the .part and .etag pair if a resumable transfer is desired. |

Region deletion has no dedicated UI in g. Power down or stop card use, manage files on a computer, reinsert and reload. Avoid removing the card during writes. Street maps support viewport-based pan/zoom downloads and caching. Bulk area/radius downloads, an exact pre-download planner and a cache-management UI are not implemented.

# 20. Radio diagnostics and Trace

Passive reception and advert discovery continue on the Home menu, all other screens and with the display asleep. Discover is optional: it sends your own advert. Heard contains validated adverts since restart, not every received packet. A received packet can be encrypted traffic, a duplicate or a returned copy of your own advert. Version q also polls the latched RX_DONE flag every 50 ms as a fallback for a missed GPIO interrupt and logs loop delays and actual boosted gain. Remote-screen image transfers service mesh processing between chunks. These safeguards do not establish why local traffic has been sparse.

Radio opens a diagnostics page. It shows frequency, SF/BW/CR, TX power, time since the last received packet, RSSI/SNR, RX packet count, contacts/heard counts, free memory and uptime. Press A to send a flood advert.

Roll right for Storage & Performance. It shows SPIFFS used/total space, SD free/total space when detected, CPU clock and a history graph of free RAM. Roll left returns to radio diagnostics. A healthy-looking graph is not a substitute for a sustained load test.

| Diagnostic | Interpretation |
|---|---|
| Never heard | No receive event recorded since startup; check a known peer and radio settings. |
| Packets RX increasing | Radio receive activity exists. It does not mean every packet was a message addressed to you. |
| RSSI/SNR | Measurements for recent reception, not geographic location or delivery guarantees. |
| Free internal RAM | Important for radio/BLE/Wi-Fi stacks and allocations that cannot use PSRAM. |
| Free PSRAM | External memory available for large buffers; it does not replace all internal-memory requirements. |
| Uptime resetting | May indicate reboot, power interruption or a crash. Capture a USB log before diagnosing the cause. |

Trace lists contacts with known paths. Select a target to request a trace. The result can show hop information and signal data returned by the network. **r** retries the current target; **n** selects another. If there is no known path, exchange a message or allow normal discovery/routing first. A failed trace is not proof that every part of the route is offline.

Noise monitoring remains in the inherited code, and Terminal exposes the current noise reading. There is no Noise tile on this enhanced Home grid; GPS occupies that position. Similarly, experimental voice and a dedicated SOS Home workflow are not enabled in this build.

# 21. Local Terminal command reference

Terminal is the local console on the device, not a general shell. Enter executes a line; roll up/down to view older/newer log entries. Commands are case-sensitive as implemented. Contact arguments are matched by name prefix, so use an unambiguous name without spaces for command-line targeting.

| Command | Meaning |
|---|---|
| help | Print the built-in command summary. |
| advert | Send a zero-hop advert. |
| advertf | Send a flood advert. |
| contacts | List contact index, name, role and path information. |
| heard | List up to ten recent heard records. |
| noise | Print the current radio noise-floor value. |
| mem | Print free heap and free PSRAM. |
| send <name> <message> | Send a direct message to a matching contact. |
| ch <index> <message> | Send to a channel slot; the index is a slot number, not necessarily its visible list position. |
| login <name> <password> | Attempt the selected room/repeater login. |
| resync <name> | Request a room's full backlog using the stored login workflow. |
| cmd <name> <remote-command> | Send a CLI command to a remote peer. Rights and syntax depend on that peer. |
| trace <name> | Start a trace for the matching contact; this exists even though the short help list may omit it. |
| freq <MHz> | Change frequency. Underlying AU validation still applies. |
| sf <7-12> | Change spreading factor. |
| bw <kHz> | Change bandwidth. Only supported enhanced profile values remain valid. |
| cr <5-8> | Change coding-rate denominator. |
| power <dBm> | Local terminal accepts 1-22 dBm. |
| name <newname> | Save a node name. Advertise to announce it; restart to refresh the BLE name. |
| time <epoch> | Set UTC Unix time in seconds; the implementation rejects values before its accepted epoch threshold. |

## Screen and receiver diagnostics

| Command | Meaning |
|---|---|
| map / home | Open the Map or Home screen; also accepted over USB serial. |
| rfcheck | Write actual radio frequency/sync/gain registers, driver settings, auto-add limits and recovery counters to USB. |
| rfswitch 0 / rfswitch 1 | Diagnostic only: disable / enable the radio module's DIO2 RF-switch control while idle. Reboot restores enabled. This is not ESP32 GPIO2. |
| rfclear | Report and clear latched radio hardware errors; use rfcheck afterward to see whether they recur. Does not change radio settings. |
| rxrecover 0 / rxrecover 4 | Temporarily disable / enable four-second receiver recovery. Reboot restores disabled (continuous RX). Does not send a packet or alter the saved radio profile. |
| mapcheck | Run the local repeated-TLS-close / open-SD-file regression check when no transfer is active. |
| clear | Clear the Terminal display/log buffer, not chat history. |
| reboot | Restart the device immediately. |

Prefer Settings for radio edits; verify resulting values on Radio. Terminal input and operational logs can contain private messages or commands. Review screenshots and USB logs before sharing.

# 22. Storage, security and privacy

Identity, preferences, contacts and channels are retained through the upstream storage mechanisms. The local UI also stores settings, chat history and room login state in flash. Four Wi-Fi profiles use a versioned NVS record. SD primarily holds map packs and optional application-update files.

The firmware attempts to mount SPIFFS without formatting. It formats automatically only when the entire selected filesystem partition is erased to 0xff. A nonempty partition that cannot mount produces a storage error instead of silently destroying keys. Do not use erase_flash merely to dismiss that error.

| Data | Handling in this release |
|---|---|
| Node identity/private keys | Stored on the device. Treat full-flash backups as secret. An identity is not the same as the editable node name. |
| Wi-Fi / room credentials | Persisted without hardware-backed encryption. UI masking does not encrypt flash. |
| Chat history | Bounded local flash persistence; separate from the phone's history. A manual SD JSONL message export is now available; it is not automatic archival or a restore/import workflow. |
| Map data | SD files; classic downloads use temporary files and commit after completion. |
| Logs | Can contain operational details and messages. New GPS diagnostics report counters/fix state without coordinates. |
| Secure boot / flash encryption | Not enabled by this project. Do not claim protection against someone reading physical flash. |

MeshCore packet/identity/cryptographic code is retained rather than replaced with a proprietary incompatible protocol. This does not make a shared public channel confidential, validate remote users' real-world identities, or protect credentials in an unencrypted backup.

The development workspace resides under OneDrive on the tested computer. Its synchronization policy applies to the private backup directory. That directory is excluded from Git and distribution packages, but exclusion from Git does not disable cloud synchronization.

# 23. Archiving messages to the SD card

Open **Settings > Archive messages to SD**. The operation snapshots the current local message history in PSRAM, then writes small chunks on the main loop. It continues after leaving Settings. Wait for **Messages archived to SD** before removing the card or power. Terminal reports the resulting path. The action rejects another archive while one is already running.

Archives are stored under **/meshcore/archives/**. A filename such as **messages-<epoch>-<random>.jsonl** uses the creation time and a random suffix to avoid overwriting previous exports. If the device clock is not set, the numeric time is not a reliable date; the random suffix still distinguishes the file. In-progress files end in .part and are renamed only after all records are written.

| SD path | Contents |
|---|---|
| /meshdeck-maps/*.mdm | Classic offline coastline/place packs. |
| /meshdeck-maps/*.mdv | Detailed NewMaps vector packs. |
| /meshdeck-maps/streets/*.png | Cached street tiles, loaded for visible map areas. |
| /meshcore/archives/messages-*.jsonl | Completed message snapshots readable on a computer. |
| /meshcore/archives/*.part | Incomplete/interrupted archive; not a completed export. |
| /firmware.bin | Optional application update file, unrelated to map or message storage. |

## Archive format and boundaries

JSONL means one JSON object per line. The first line is a header with format **meshcore-enhanced-messages-v1**, firmware version, creation epoch and message count. Subsequent lines contain thread name/type, channel slot when relevant, timestamp, sender, message text, outgoing/delivered/failed flags, SNR and raw hop value. Open it in a text editor or process it with a JSONL-capable tool. A flag is the recorded state at export time, not a guarantee about later delivery.

The export includes only currently retained messages: at most 32 threads with 48 messages each in RAM. History already overwritten or lost before a reboot cannot be recovered by exporting later. It is a consistent in-memory snapshot, so new messages arriving after the action starts belong in a later export.

The archive does not automatically export private identity keys, Wi-Fi passwords, channel configuration secrets, or full device settings. However, message text itself may contain sensitive content or a channel-share link you previously sent or received. Keep the card and copies private. No archive encryption is provided.

Exporting does not clear history. It is manual, not a continuous logger, and this version has no archive import/restore or automatic retention/deletion policy. GPS tracks and general filesystem backups are not included. Keep the separate full-flash recovery backup for restoration of the original device state.

## Errors and capacity

No SD card means mounting failed or no usable card was found. SD full means the conservative free-space check failed. Write/commit failures leave the operation unsuccessful; do not treat a .part file as complete. Retry on a healthy card after checking free space. Preserve completed archives before formatting or replacing media. Clean up abandoned .part files on a computer while the card is not in use.

The implementation allocates a bounded PSRAM snapshot and writes at most 512 bytes per poll. Final filesystem flush/rename and card latency still require hardware qualification under radio load. SD firmware updating is blocked while an archive or map transfer is active.

# 24. USB flashing, backup and recovery

## Choose the correct image

Use the **915 MHz T-Deck Plus** environment. The default and GPS9600 folders differ in initial GPS baud; both now auto-search when necessary. Do not use an unrelated board image. The application image is **firmware.bin**; **merged-firmware.bin** additionally contains boot/partition data and padded gaps.

| File | Flash offset / purpose |
|---|---|
| bootloader.bin | 0x000000; bootloader |
| partitions.bin | 0x008000; verified partition table |
| boot_app0.bin | 0x00e000; OTA initialization data |
| firmware.bin | 0x010000 for this known app0 layout |
| merged-firmware.bin | 0x000000; combined installation image |
| flash-manifest.json | Exact offsets, sizes, hashes and layout from the build |
| SHA256SUMS.txt | Checksums of the released binary files |

## Before installation

Use a data cable, stable power and the correct antenna. Close other serial monitors/flashers. Identify the current COM port; COM3 was used during development, but it can change. Back up all 16 MiB before changing unknown firmware. Esptool verifies the read transfer's MD5; then record the local SHA-256. Compare the old and new partition tables. A matching table does not by itself establish compatibility of every stored setting.

Commands below run from the directory containing MeshQuokka. The project build environment supplies Python/esptool after dependencies are installed:

```powershell
$py = '.\.venv\Scripts\python.exe'
$esp = '.\.platformio\packages\tool-esptoolpy\esptool.py'
& $py $esp --chip esp32s3 --port COM3 flash_id
New-Item -ItemType Directory -Force .\MeshQuokka\private-backups
& $py $esp --chip esp32s3 --port COM3 --baud 460800 `
  read_flash 0 0x1000000 .\MeshQuokka\private-backups\original.bin
Get-FileHash .\MeshQuokka\private-backups\original.bin -Algorithm SHA256
```

The verified development backup is retained privately as tdeck-before-enhanced-20261114.bin. Do not rename another file to that name and assume it is a valid recovery image. The backup is deliberately absent from the public source package.

## Install using separate images

Separate images preserve the gaps, including NVS, unlike the padded merged image. They still change the application and boot configuration. Use them only after backup and layout checks:

```powershell
& $py $esp --chip esp32s3 --port COM3 --baud 460800 `
  write_flash --flash_mode keep --flash_freq keep --flash_size keep `
  0x0 .\MeshQuokka\release\bootloader.bin `
  0x8000 .\MeshQuokka\release\partitions.bin `
  0xe000 .\MeshQuokka\release\boot_app0.bin `
  0x10000 .\MeshQuokka\release\firmware.bin
```

Preserve image headers with keep. The board uses a QIO configuration, while generated bootstrap/application image headers can say DIO. Do not force-patch a digest-protected header because the board configuration says QIO.

## Application-only update and recovery

For the established layout with app0 selected, later USB updates can write only firmware.bin at 0x10000. After an OTA/SD update, app1 may be selected; do not assume app0 is active. Use the supported updater or inspect OTA selection first.

```powershell
& $py $esp --chip esp32s3 --port COM3 --baud 460800 `
  write_flash --flash_mode keep --flash_freq keep --flash_size keep `
  0x10000 .\MeshQuokka\release\firmware.bin
```

If auto-reset fails, hold the trackball centre/BOOT while connecting USB, then release it; or hold BOOT while pressing reset. ROM download mode may have a blank display. Choose the enumerated port. To restore the same device, write its verified full backup at 0x0 with keep options. This restores flash-held firmware and data together, not eFuses. No eFuses were changed during this project.

```powershell
& $py $esp --chip esp32s3 --port COM3 --baud 460800 `
  write_flash --flash_mode keep --flash_freq keep --flash_size keep `
  0x0 .\MeshQuokka\private-backups\original.bin
```

Do not run erase_flash as a routine step. Stop after a failed backup or write verification. After updating, check version, display, Back, radio profile, Bluetooth and GPS. The original full image and the verified current MeshQuokka backup were both restored with successful write verification during the radio comparison on this unit.

# 25. SD firmware update

The inherited updater looks for **/firmware.bin** at the SD card root. This must be the compatible application image, not partitions.bin, a ZIP archive or merged-firmware.bin renamed to firmware.bin. Verify its hash against the intended release before copying it.

With a stable power source, use Settings > Update firmware from SD. The updater reads the file, invokes the ESP32 Update API and restarts after a successful commit. It reports errors such as No SD card found, firmware.bin not on card, Not enough space or Update failed.

This route has not been hardware-qualified in this project. The verified deployment route used USB. Keep a full recovery backup and USB recovery access before attempting SD updates. The updater does not constitute a signed release-distribution system, and the project has not enabled secure boot.

The framework may select a different OTA application slot after updating. Consequently, a later raw USB application-only write to app0 is not automatically the correct way to update whichever slot is running. Do not mix update methods without understanding slot selection.

Ordinary map use never requires a firmware.bin on the card root. Remove or archive update files once no longer needed to avoid selecting an old image accidentally.

# 26. Hardware and memory reference

| Component | Configuration or observed detail |
|---|---|
| MCU | ESP32-S3, dual-core Xtensa, nominal 240 MHz target; USB identified revision v0.2 and 40 MHz crystal. |
| Flash | 16 MiB quad flash observed by esptool. |
| PSRAM | 8 MiB class OPI configuration; runtime reports more than 8.19 million bytes free under observed conditions. No dedicated memory stress test performed. |
| Display | ST7789, 320 x 240 landscape. Backlight GPIO42; CS12; DC11. |
| Radio | SX1262 target; user-confirmed 915 MHz system. CS9; IRQ45; reset17; busy13. Module DIO2 controls the RF switch; DIO3 supplies 1.8 V TCXO control. |
| Shared SPI | SCK40, MOSI41, MISO38; shared by radio, display and SD with separate chip selects. |
| SD | CS39 on the shared SPI bus. |
| Keyboard | I2C address 0x55, SDA18/SCL8; interrupt46. Separate keyboard controller. |
| Touch | GT911, probed at 0x5D/0x14; interrupt16. |
| Trackball | Direction GPIOs 3, 2, 15, 1; centre GPIO0, also BOOT. |
| GPS UART | Runtime MCU RX44/TX43 based on the actual setPins call. Default begins at 38400; automatic search handles alternative rates. Exact receiver model unresolved. |
| Power/battery | Peripheral power GPIO10; battery ADC4 with documented divider. Voltage-derived percentage is uncalibrated. |
| Audio | Existing I2S speaker path uses GPIO5/7/6. Experimental voice/microphone functionality is not enabled. |

Earlier documentation incorrectly inferred MCU RX43/TX44 from macro names. Arduino setPins takes RX first, while upstream passes PIN_GPS_TX then PIN_GPS_RX. The actual runtime configuration and displayed diagnostics now reflect RX44/TX43. Manufacturer documents and supplied material disagree about GPS module revision; the working data link does not prove its part number.

## Partition table

| Partition | Offset | Size |
|---|---|---|
| NVS | 0x009000 | 0x005000 |
| OTA data | 0x00e000 | 0x002000 |
| app0 | 0x010000 | 0x640000 |
| app1 | 0x650000 | 0x640000 |
| SPIFFS | 0xc90000 | 0x360000 |
| Core dump | 0xff0000 | 0x010000 |

Each app slot is 6,553,600 bytes. See flash-manifest.json for the current application size. A full RGB565 framebuffer is 153,600 bytes. Initial e smoke tests reported about 85 KB internal memory free; the later f diagnostic window with GPS enabled reported roughly 42.7 KB. Different active services change heap use. These observations do not establish worst-case headroom under simultaneous traffic.

# 27. Software architecture and developer workflow

The project combines pinned official MeshCore with the MIT-licensed MeshDeck local UI and the enhanced changes. MeshCore retains routing, identity and packet/cryptographic handling. The UI provides screens, local history and peripheral interaction. The production client image does not enable the experimental Codec2 voice build.

| Layer | Source and responsibility |
|---|---|
| Protocol/core | firmware/src and upstream helpers: mesh packets, identities, contacts, routing and radio abstractions. |
| Board | firmware/variants/lilygo_tdeck and board configuration: pins, radio and peripheral startup. |
| Editable application | vendor/meshdeck/firmware: MyMesh, main, UI screens, stores and transfer worker. |
| Compiled application copy | firmware/examples/meshdeck: synchronized from the editable tree before compilation. Do not edit only this copy. |
| Build configuration | Root platformio.local.ini, copied into firmware by sync_application.py. |
| Packaging | tools/package_release.py validates actual build offsets/partition data, merges images and writes manifests/hashes. |
| Documentation | docs/USER_MANUAL.md plus root guides; tools/build_manual.py renders this manual. |

The main loop services mesh, sensors, UI and the clock cooperatively. GPS parsing has a 128-byte per-loop budget. The map network task runs separately and queues bounded events; SD writes remain on the main loop. The BLE receive transport uses a FreeRTOS queue imported from newer official MeshCore. Rendering still shares execution time with mesh service, so hard real-time receive latency is not claimed.

## Provenance

MeshCore baseline: bbb58cceb852a9190b46d5f6984e8e5140e6991e. MeshDeck baseline: 150f25e03e44fddc3c41b8ec08864e8073e62060. The BLE receive-queue backport comes from MeshCore d2096bd90e65cd5213e612811eda0bc214e1ab1c. Later local diagnostic changes are included in this source distribution.

Reference tools include PlatformIO 6.2.0, Espressif32 platform 6.11.0, Arduino-ESP32 framework 3.20017.241212+sha.dcc1105b and GCC 8.4. Resolved libraries, including RadioLib 7.8.1 and MicroNMEA 2.0.6, are pinned in platformio.local.ini and recorded in DEPENDENCIES.json. Rebuilding requires downloaded toolchains/libraries; they are not bundled as caches in the source package.

# 28. Building the firmware from source

## Windows setup and compilation

Place the repository in a normal writable folder. Run initial setup from its parent directory. Python 3.14 was used in development. Internet access is needed for first-time package installation.

```powershell
py -m venv .venv
.\.venv\Scripts\python.exe -m pip install platformio==6.2.0
.\MeshQuokka\tools\build.ps1
.\.venv\Scripts\python.exe MeshQuokka/tools/package_release.py
.\MeshQuokka\tools\build.ps1 -Environment MeshCore_Enhanced_TDeck_Plus_GPS9600
.\.venv\Scripts\python.exe MeshQuokka/tools/package_release.py MeshCore_Enhanced_TDeck_Plus_GPS9600
.\MeshQuokka\tools\build.ps1 -Environment MeshCore_Enhanced_TDeck_Plus_GPS9600
.\.venv\Scripts\python.exe MeshQuokka/tools/package_release.py MeshCore_Enhanced_TDeck_Plus_GPS9600
.\.venv\Scripts\python.exe -m unittest discover -s MeshQuokka/tests -v
```

The wrapper temporarily maps a free drive letter to shorten GCC command lines on Windows, uses the parent's .platformio cache and removes the alias afterward. It now derives the project directory name rather than requiring one hard-coded clone name. Paths in the example commands still assume the repository is named MeshQuokka.

After editing application code, run sync_application.py, verify byte identity and bump MESHDECK_VERSION for each flashed change set, as required by the vendored AGENTS.md. Do not rerun one-time harden/integrate transformation scripts on an already modified tree. For Linux/macOS, use the normal PlatformIO CLI after synchronization; the Windows wrapper itself is platform-specific.

# 29. Publishing the source on GitHub

Use the prepared **MeshQuokka-GitHub-0.1.0_261115c/MeshQuokka** folder as the repository root. Upload or commit its **contents**, so README.md, platformio.local.ini, firmware, vendor, tools and docs appear at the top level of the GitHub repository. Do not upload the original working workspace wholesale.

Alternatively, extract **MeshQuokka-Source.zip** and use the contents of its MeshQuokka directory. This is the source distribution archive. A ZIP attached to a GitHub Release is convenient for downloading, but committing the extracted source files makes the repository browsable and usable for changes.

| Include in source repository | Keep out of source repository |
|---|---|
| Editable and synchronized firmware sources | private-backups and any device flash dumps |
| Build configuration and scripts | .venv, .platformio, .pio and compiler caches |
| README, manual source/PDF and technical guides | USB/install logs and screenshots with personal data |
| Licences, notices and provenance | Nested .git directories from vendor clones |
| Tests and bundled licensed map assets | Downloaded reference Documents, datasheets and browser page caches |

The prepared package excludes those private/cache/reference paths. A SHA-256 file inventory accompanies the staging folder. Keep .gitignore and upstream licence notices. The editable UI lives under vendor; its presence is deliberate, not an accidentally nested Git repository.

## Releases versus source

For a GitHub Release, attach the versioned Development ZIP, the firmware images/manifests/checksums and the PDF manual as desired. The Development ZIP is a user/developer download bundle. It is not the folder to unpack over the source repository root. Never attach the private recovery backup.

The source repository can be created under your preferred account and name. This task prepares files only; it does not create a repository, publish a release or push to GitHub. Use a release description that says this is development firmware with user-confirmed Back and Android connection, plus verified GPS NMEA reception, and outdoor GPS positioning, while full radio/long-duration qualification remains outstanding.

Root LICENSE covers the enhanced contributions under MIT. Preserve the separate MeshCore/MeshDeck licences and third-party notices. Codec2 source retains LGPL notices but is not linked into this non-beta image. Natural Earth packs are public domain; separately generated OSM-derived packs have their own attribution and data-licence requirements.

# 30. Troubleshooting and remaining tests

| Problem | Checks and next step |
|---|---|
| Touch unresponsive after development restarts | If keyboard/trackball still work but touch is absent, unplug USB and fully power off for 10 seconds, then restart. This restored the tested GT911 controller. |
| Cannot go back | Tap top-left < Back; close the editor/filter first. Or hold trackball centre over half a second. If touch is misaligned, use trackball and restore Touch mapping C. |
| Bluetooth visible, PIN rejected | Use the active screen PIN. Forget stale Android pairing, reconnect from MeshCore, and restart after a PIN change. Capture [ble] failure reason if still unsuccessful. |
| GPS enabled but no position | Read Link/RX/NMEA. Recent NMEA with no fix means the UART works; allow clear-sky acquisition. No bytes or no checksummed NMEA points to a link problem. |
| No nodes or messages | Match frequency/SF/BW/CR with a known working peer; inspect Radio packet count. Send an advert, then wait. Check antenna and peer availability. |
| Message timeout | Recheck route/peer/settings and channel key where applicable. An app connection does not prove an RF path. |
| Wrong map position | Confirm valid own GPS or correct manual signed coordinates. Check whether the displayed location is only the viewport centre or a stale remote advert. |
| No map pack | Match .mdm to Map and .mdv to NewMaps; check /meshdeck-maps and reload. |
| No COM port / busy port | Use a data cable, close monitors, check Device Manager and enter ROM download mode if necessary. Never run two serial clients together. |
| Storage error | Preserve flash. Do not format/erase to bypass it; inspect the backup and partition compatibility. |
| Reset under load | Capture reset/crash output, active services and heap readings. Check power, then reproduce with fewer services. |

## Useful bug report

Include firmware version, hardware variant, exact steps, expected and observed behaviour, selected radio profile and whether Wi-Fi/BLE/GPS/SD were active. For GPS, provide counters, link status and baud rather than publishing coordinates. For Bluetooth, provide Android/app version and authentication reason if present. Review logs for message content, keys and credentials before sharing.

## Still to qualify or implement

Over-the-air encrypted DM/ACK/retry tests with known peers; channel send/reply; phone messaging and reconnect under traffic; iPhone compatibility; SD removal/full-card/resume recovery; battery and audio measurements; and long-duration coexistence remain separate acceptance work. The arbitrary-radius raster downloader, favourite/recent node filters, comprehensive data migrations, encrypted credentials and SD track export are not complete features in this release. SD message snapshot export is implemented separately.

A suggested coexistence test uses a known peer sending numbered messages every ten seconds for an hour while BLE stays connected and a map download runs. Compare losses with an idle baseline, record minimum internal heap and reset events, and agree an acceptance threshold before calling the build production-ready.

# 31. Glossary and source references

| Term | Definition |
|---|---|
| Advert | A MeshCore identity/presence announcement, optionally carrying position. |
| ACK | Protocol acknowledgement, distinct from a human read receipt. |
| BLE bond | Saved Bluetooth security association; stale bonds can survive firmware changes. |
| NMEA | Text sentences emitted by a GNSS receiver, usually with checksums. |
| GNSS / GPS | GNSS is the broader satellite-navigation category; GPS is one constellation and the UI's familiar label. |
| Fix | A receiver-reported valid position solution. |
| ETag / Range | HTTP object identifier and byte-range request used for safe resumable downloads. |
| NVS | ESP32 nonvolatile key/value storage. |
| SPIFFS | Flash filesystem used by this application. |
| PSRAM | External pseudo-static RAM for large allocations; not a substitute for every internal/DMA allocation. |
| OTA slot | An application partition selected by the boot/update mechanism. OTA here describes the slot architecture, not proof of an internet updater. |
| Hash | A fingerprint used to compare file integrity; a checksum alone does not authenticate a trusted publisher. |
| ODbL | Open Database Licence used for OpenStreetMap data. |

The implementation was reviewed directly in the pinned source trees. Exact package metadata is in DEPENDENCIES.json. The most useful upstream references are:

* MeshCore receiver recovery and path-length FAQ: https://github.com/meshcore-dev/MeshCore/blob/main/docs/faq.md
* MeshCore source: https://github.com/meshcore-dev/MeshCore
* MeshDeck source and third-party notices: https://github.com/meshdeck-os/meshdeck
* LilyGO T-Deck hardware/examples: https://github.com/Xinyuan-LilyGO/T-Deck
* LilyGO T-Deck Plus documentation: https://github.com/Xinyuan-LilyGO/documentation/blob/master/en/products/t-deck-series/t-deck-plus/index.md
* Natural Earth data: https://www.naturalearthdata.com/
* OpenStreetMap attribution: https://www.openstreetmap.org/copyright
* OSM public raster tile policy: https://operations.osmfoundation.org/policies/tiles/

Online projects and documentation can change. This manual describes version u and its recorded source revisions. Use the supplied manifest and version string when comparing this release with future firmware.

# 32. Version v: receiver and Traffic view

Normal operation now leaves periodic receiver recovery disabled (interval 0), avoiding the four-second standby/RX interruptions used during troubleshooting. Transmission still temporarily interrupts reception on this half-duplex radio. The 50 ms missed-interrupt polling fallback remains active. Terminal command `rxrecover 4` temporarily enables the old diagnostic behavior; `rxrecover 0` disables it. Reboot restores disabled. This change does not prove equal range to the original firmware.

Open Heard and tap **Traffic[t]**, or press **t**, to switch between named adverts and recent packet activity. The Traffic view retains the latest 32 radio packets in RAM. Up/down scrolls history; tap **Nodes [t]** to return. Each row shows age, packet type, byte length, RSSI in dBm, SNR in dB, hop count and bytes per path identifier. Flood packets can show the last relay prefix; it is an unverified short identifier, not an authenticated repeater name. Direct packet paths describe destinations and do not identify the transmitter. Packet contents and encrypted messages are not exposed by this view.

Advert counters: **Valid** means a non-self, non-duplicate advert passed signature verification; **Self** is a returned own-identity advert; **Duplicate** is an already-seen advert; **Old** means a signature-valid advert failed the contact timestamp freshness check; **Bad sig** means failed signature verification; **Bad data** combines short adverts and invalid/missing app names. Valid and Old/Bad data may overlap because they describe separate processing stages. Raw header labels are observations, not proof of authenticated identities. Signature, timestamp and duplicate protections remain enforced.

The Nodes view retains the latest observation for up to 64 distinct six-byte identity prefixes. Repeated adverts update their existing entry instead of filling the list with the same station. Both views reset on reboot; saved Contacts remain separate.

Terminal **traffic** prints the counters and packet history; **traffictest** runs the metadata-parser checks without transmitting. No additional automatic adverts are sent by this feature.

Version w makes the Traffic/Nodes toggle text white and fixes Traffic mode consuming the global Back event. The on-device parser/navigation regression checks pass.

## Version x: new-node discovery ding

A short high-pitched ding sounds when a valid advert first introduces an unknown node, including clients, repeaters and room servers. It also sounds with the screen asleep, without waking the display. Enable Settings > Sound and set Volume above zero. Existing saved contacts, ordinary updates, duplicate adverts, invalid adverts and returned copies of your own advert do not trigger a discovery sound. Unsaved discoveries are suppressed while their identity remains in the 64-entry Heard history; if evicted or after reboot, an unsaved node may be announced again.

UI tones are queued to a dedicated speaker worker; the radio/UI loop does not wait for sound playback. The bounded queue holds 64 tones; muted/zero-volume events are silent, and sounds may be skipped if audio resources are busy or the queue is full.

Terminal `ding` previews the discovery sound without sending radio traffic or creating a contact. `ding dark` first sleeps the display and then previews the same sound. These commands run only when explicitly entered and do not enable any repeating test. Wake the display normally afterward.

# 33. Background channel messages and unread notifications

Version 0.1.0_261115c receives and attempts to decrypt messages for configured channel keys independently of the selected screen or Bluetooth connection. Home, Map, other channels and display sleep do not disable channel reception. Incoming messages are stored in their own channel conversation. Reception still requires the radio packet to reach this device and the channel key to match; an online observer's history is not automatically downloaded.

Unread channels display a numeric count on their chat tab, and the status bar shows the combined unread count. A channel notification appears when you are elsewhere, including another channel, a chat overlay or scrolled away from the latest messages. Incoming message sounds respect Sound/Volume and wake the display as before. Counts clear only when that conversation's newest messages are drawn, not merely because some Chat screen is open.

Unread state is now stored in each retained message's flags without changing the existing history file layout. The normal eight-second save debounce still applies, and only the newest 16 messages per thread survive restart (48 are held in RAM). The restored unread count covers retained messages; older firmware history starts as read. A sudden power loss before a save can lose recent changes. The isolated `unreadtest` diagnostic creates temporary RAM-only conversations, exercises per-channel counting and ring overwrite, frees them, and neither transmits nor modifies real chats.

# 34. Channel reception diagnostics

Public channel messages are broadcasts; mentioning a name in the message does not direct delivery to that person. MeshQuokka does not filter channel messages by its node name. An online observer may receive traffic from locations or paths your radio cannot hear; MeshQuokka does not download that observer's channel history.

In Heard > Traffic, **Ch RX** counts group text/data packets reaching Mesh processing, **decoded** counts successful channel authentication/decryption, and **saved** counts text messages inserted into local RAM chat history. They cover all configured/observed channels, not Public alone. Raw radio receive errors appear separately at the top. The history now displays five packet rows to leave room for these counters.

**No key** means no configured channel hash matched; **MAC** means matching hashes existed but authentication/decryption failed; **dup** means already-seen channel packets; **drop** combines short packets, unsupported/malformed text and inability to allocate/store a conversation. Successful decode and saved counts can differ for data packets rather than text. Saved means accepted into the in-memory history; normal delayed flash persistence still applies.

Terminal `channelstats` prints all stage counters separately. `channeltest` performs a one-shot RAM-only encryption/decryption check using channel slot 0 and both general and named text. It does not transmit, update the duplicate table, change real chats or inflate these counters. It verifies local decoder behavior, not radio coverage. Counters reset on reboot.

# 35. MeshQuokka branding

Version 0.1.0_261115c renames the application to MeshQuokka. Home, About, boot, remote-screen UI and application diagnostics use the new name. A native 96 x 96 quokka/network vector mark appears at boot and in About. The logo is compiled into the firmware and needs no SD card or internet access. This is a branding update: network protocol, channel keys, radio settings, message storage paths and the MeshCore Bluetooth compatibility prefix remain unchanged. Editable logo sources and SVG/PNG exports are included under assets.
