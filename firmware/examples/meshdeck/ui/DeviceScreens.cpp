#include "DeviceScreens.h"
#include "MeshQuokkaLogo.h"
#include "../MyMesh.h"
#include <helpers/sensors/MicroNMEALocationProvider.h>

void AboutScreen::draw() {
  auto& c=ui.cv();c.fillScreen(C_BG);ui.drawStatusBar("About");
  c.setTextSize(2);c.setTextColor(C_ACCENT);c.setCursor(12,32);c.print("MeshQuokka");
  drawMeshQuokkaLogo(c,218,30);
  c.setTextSize(1);c.setTextColor(C_FG);c.setCursor(12,62);c.print("By S. van Ingen");
  c.setCursor(12,82);c.print("(C)2026 S. van Ingen");
  c.setCursor(12,104);c.print("Data Packet Radio");
  c.setCursor(12,122);c.print("on the MESHCORE network");
  c.setCursor(12,140);c.print("WiFi downloads and SD message archives.");
  c.setCursor(12,164);c.printf("Version %s",MESHDECK_VERSION);
  c.setTextColor(C_FG_DIM);c.setCursor(12,190);c.print("Built on MeshCore and MeshDeck.");
  c.setCursor(12,208);c.print("Upstream credits/licences in the manual.");
}

void GPSStatusScreen::draw() {
  auto& c=ui.cv();c.fillScreen(C_BG);ui.drawStatusBar("GPS receiver");
  c.setTextSize(1);c.setTextColor(C_FG);c.setCursor(8,36);
  auto* gps=ui.sensors?ui.sensors->getLocationProvider():nullptr;
  c.printf("Receiver: %s\n",ui.prefs->gps_enabled?"enabled":"disabled");
  bool stream=gps && gps->validSentences() && uint32_t(millis()-gps->lastSentenceMillis())<10000;
  c.setCursor(8,58);c.printf("Link: %s",!ui.prefs->gps_enabled?"disabled":(stream?"NMEA received":"searching baud / no NMEA"));
  if(gps) {
    c.setCursor(8,80);c.printf("Fix: %s  Sats used: %ld",ui.gpsFix()?"valid":"none",gps->satellitesCount());
    if(ui.gpsFix()) {
      c.setCursor(8,102);c.printf("Lat %.6f  Lon %.6f",gps->getLatitude()/1e6,gps->getLongitude()/1e6);
      c.setCursor(8,124);c.printf("Altitude %.1f m",gps->getAltitude()/1000.0);
    }
    else {
      c.setCursor(8,102);c.printf("Receiver GGA quality: %u",gps->fixQuality());
      c.setCursor(8,124);c.printf("RMC: %c  In view (last): %u",gps->navigationStatus(),gps->satellitesInView());
    }
    c.setCursor(8,146);c.printf("RX bytes %lu  NMEA %lu",(unsigned long)gps->receivedBytes(),(unsigned long)gps->validSentences());
  }
  if(gps) {c.setCursor(8,158);c.printf("Signal max (10s): %u dB-Hz",gps->strongestSignal());}
  // Upstream initBasicGPS uses setPins(PIN_GPS_TX, PIN_GPS_RX): MCU RX44/TX43.
  c.setCursor(8,170);c.printf("MCU RX44 TX43 / %lu baud",(unsigned long)Serial1.baudRate());
  c.setCursor(8,192);c.print("E toggle GPS; S toggle position sharing");
  c.setCursor(8,212);c.printf("Advert location: %s",ui.prefs->advert_loc_policy?"enabled":"disabled");
}
bool GPSStatusScreen::key(uint8_t k) {
  if(k=='e'||k=='E') {
    ui.prefs->gps_enabled=!ui.prefs->gps_enabled;
    if(ui.prefs->gps_enabled && !ui.prefs->gps_interval)ui.prefs->gps_interval=5;
    ui.mesh->applyGpsPrefs();ui.mesh->savePrefs();return true;
  }
  if(k=='s'||k=='S') {ui.prefs->advert_loc_policy=!ui.prefs->advert_loc_policy;ui.mesh->savePrefs();return true;}
  return false;
}
void BLEStatusScreen::draw() {
  auto& c=ui.cv();c.fillScreen(C_BG);ui.drawStatusBar("MeshCore Bluetooth");
  c.setTextSize(1);c.setTextColor(C_FG);c.setCursor(8,40);
  c.printf("Service: %s",ui.isSerialEnabled()?"enabled":"disabled");
  c.setCursor(8,65);c.printf("Phone: %s",ui.hasConnection()?"connected":"disconnected");
  c.setCursor(8,90);c.printf("Name: %s%s",BLE_NAME_PREFIX,ui.prefs->node_name);
  c.setCursor(8,115);c.printf("Pairing PIN: %06lu",(unsigned long)ui.mesh->getBLEPin());
  c.setCursor(8,140);c.print("E enable/disable; P change pairing PIN");
  c.setCursor(8,163);c.print("Name: change Node name in Settings.");
  c.setCursor(8,184);c.print("Name / PIN changes require restart.");
  if(_editing) {c.setCursor(8,210);c.printf("New PIN (6 digits): %s",_pin);}
}
bool BLEStatusScreen::key(uint8_t k) {
  if(_editing) {
    if(k==27){_editing=false;return true;}
    if(k==8||k==127){if(_len)_pin[--_len]=0;return true;}
    if(k>= '0' && k<='9' && _len<6){_pin[_len++]=k;_pin[_len]=0;return true;}
    if(k==13 && _len==6) {ui.prefs->ble_pin=strtoul(_pin,nullptr,10);ui.mesh->savePrefs();_editing=false;ui.toast("PIN saved. Restart and pair again.");}
    return true;
  }
  if(k=='e'||k=='E'){if(ui.isSerialEnabled())ui.disableSerial();else ui.enableSerial();return true;}
  if(k=='p'||k=='P'){_editing=true;_len=0;_pin[0]=0;return true;}
  return false;
}
