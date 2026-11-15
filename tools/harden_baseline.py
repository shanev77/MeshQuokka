from pathlib import Path
r=Path('MeshCore-Enhanced'); app=r/'vendor/meshdeck/firmware'
def edit(p,a,b):
 s=p.read_text(encoding='utf8'); assert a in s,(p,a[:70]); p.write_text(s.replace(a,b),encoding='utf8')
# One physical SPI bus shared by all clients, used only on the Arduino loop task.
p=r/'firmware/variants/lilygo_tdeck/target.cpp'
edit(p,'static SPIClass spi;','SPIClass& tdeckSharedSPI() { static SPIClass shared; return shared; }\n  static SPIClass& spi = tdeckSharedSPI();')
p=app/'ui/DeckHW.cpp'
edit(p,'_spi = new SPIClass(HSPI);','extern SPIClass& tdeckSharedSPI();\n  _spi = &tdeckSharedSPI();')
edit(p,'  _flip = flip_display;','  _flip = flip_display;\n  for (int cs : {9, 12, 39}) { pinMode(cs, OUTPUT); digitalWrite(cs, HIGH); }')
edit(p,'  _canvas = new GFXcanvas16(SCREEN_W, SCREEN_H);','  _canvas = new GFXcanvas16(SCREEN_W, SCREEN_H);\n  if (!_canvas || !_canvas->getBuffer()) return false;')
# Never auto-format unknown existing data. Format only a demonstrably entirely erased partition.
p=app/'main.cpp'
edit(p,'#include <SPIFFS.h>','#include <SPIFFS.h>\n#include <esp_partition.h>\n\nstatic bool mountStorage() {\n  if (SPIFFS.begin(false)) return true;\n  const esp_partition_t* p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, nullptr);\n  if (!p) return false;\n  uint8_t chunk[256];\n  for (size_t at=0; at<p->size; at+=sizeof(chunk)) {\n    if (esp_partition_read(p,at,chunk,sizeof(chunk)) != ESP_OK) return false;\n    for (uint8_t b : chunk) if (b != 0xff) return false;\n  }\n  return SPIFFS.format() && SPIFFS.begin(false);\n}\n')
edit(p,'  SPIFFS.begin(true);','  if (!mountStorage()) {\n    ui_task.fatalError("Storage error - back up flash");\n    halt();\n  }')
# Actual validity, never infer a fix from nonzero coordinates (manual/stale coordinates are not GPS fixes).
p=app/'ui/UITask.h'
edit(p,'return sensors && (sensors->node_lat != 0 || sensors->node_lon != 0);','return prefs && prefs->gps_enabled && sensors && sensors->getLocationProvider() && sensors->getLocationProvider()->isValid();')
# AU custom first selection; keep published presets available but policy will reject incompatible band.
p=app/'ui/UITask.cpp'
edit(p,'static const RadioPreset PRESETS[] = {','static const RadioPreset PRESETS[] = {\n  { "Australia custom 923.125", 923.125f, 62.5f, 7, 8 },')
# Do not trim meaningful spaces in passwords/SSIDs.
edit(p,"String s = f.readStringUntil('\\n'); s.trim();","String s = f.readStringUntil('\\n');")
edit(p,"String p = f.readStringUntil('\\n'); p.trim();","String p = f.readStringUntil('\\n');")
# Avoid SDK duplicate credential persistence; explicitly enabled reconnect only.
edit(p,'  WiFi.begin(_wifi_ssid, _wifi_pass);','  WiFi.persistent(false);\n  WiFi.setAutoReconnect(true);\n  WiFi.begin(_wifi_ssid, _wifi_pass);')
# Poll asynchronous WiFi scan from UI ticks instead of blocking LoRa for seconds.
p=app/'ui/AllScreens.h'
edit(p,'  void doScan();             // scan nearby networks -> pick list','  void doScan();             // asynchronous scan\n  void tick1s() override;\n  bool _scanning = false;')
p=app/'ui/WifiScreen.cpp'
edit(p,'  int n = WiFi.scanNetworks();','  if (_scanning) return;\n  WiFi.scanDelete();\n  int n = WiFi.scanNetworks(true);\n  _scanning = n == WIFI_SCAN_RUNNING;\n  if (!_scanning) ui.toast("WiFi scan could not start", C_RED);\n}\n\nvoid WifiScreen::tick1s() {\n  if (!_scanning) return;\n  int n = WiFi.scanComplete();\n  if (n == WIFI_SCAN_RUNNING) return;\n  _scanning = false;\n  if (n < 0) { ui.toast("WiFi scan failed", C_RED); return; }')
edit(p,'      ui.saveWifi();\n    }\n    _picking = false;','      ui.wifiPass()[0] = 0; // never carry a different SSID password\n    }\n    _picking = false;')
edit(p,'      c.print(_edit);','      if (_sel == 1) { for (int j=0; j<_elen && j<22; ++j) c.print("*"); }\n      else c.print(_edit);')
# Add backpressure-yield bound to GPS parser.
p=r/'firmware/src/helpers/sensors/MicroNMEALocationProvider.h'
edit(p,'        while (_gps_serial->available()) {','        unsigned budget = 128;\n        while (budget-- && _gps_serial->available()) {')
print('Applied storage, SPI, GPS and WiFi hardening')
