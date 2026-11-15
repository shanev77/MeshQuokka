from pathlib import Path
r=Path('MeshCore-Enhanced/vendor/meshdeck/firmware')
def edit(p,a,b):
 p=r/p;s=p.read_text(encoding='utf8');assert a in s,(p,a[:50]);p.write_text(s.replace(a,b),encoding='utf8')
edit('ui/UITask.h','#include "MapTransfer.h"','#include "MapTransfer.h"\n#include "WifiProfiles.h"')
edit('ui/UITask.h','  MapTransfer transfer;','  MapTransfer transfer;\n  WifiProfiles wifiProfiles;\n  void selectSavedWifi();\n  void forgetWifi();')
edit('ui/UITask.cpp','void UITask::loadWifi() {','void UITask::loadWifi() {\n  wifiProfiles.load();')
edit('ui/UITask.cpp','  _wifi_want = true;\n  saveWifi();','  _wifi_want = true;\n  wifiProfiles.remember(_wifi_ssid,_wifi_pass);\n  saveWifi();')
edit('ui/UITask.cpp','void UITask::wifiOff() {','''void UITask::selectSavedWifi() {
  int start=wifiProfiles.find(_wifi_ssid);
  for(int j=1;j<=4;++j) {
    int i=(start+j+4)%4;auto& e=wifiProfiles.data.entries[i];
    if(e.ssid[0]) {strlcpy(_wifi_ssid,e.ssid,sizeof(_wifi_ssid));strlcpy(_wifi_pass,e.password,sizeof(_wifi_pass));saveWifi();toast("Saved network selected");return;}
  }
  toast("No saved networks");
}
void UITask::forgetWifi() {
  wifiProfiles.forget(_wifi_ssid);wifiOff();_wifi_ssid[0]=_wifi_pass[0]=0;saveWifi();toast("Network forgotten");
}
void UITask::wifiOff() {''')
edit('ui/WifiScreen.cpp','#define W_ROW_Y0 82','#define W_ROW_Y0 76')
edit('ui/WifiScreen.cpp','#define W_ROW_H  30','#define W_ROW_H  27')
edit('ui/WifiScreen.cpp','const char* labels[4] = { "Network (SSID)", "Password", ws == 0 ? "Connect" : "Disconnect", "Remote screen" };','const char* labels[5] = { "Network (SSID)", "Password", ws == 0 ? "Connect" : "Disconnect", "Next saved network", "Forget selected network" };')
edit('ui/WifiScreen.cpp','for (int i = 0; i < 4; i++)','for (int i = 0; i < 5; i++)')
edit('ui/WifiScreen.cpp','else             strcpy(v, ui.remoteScreenOn() ? "on" : "off");','else             strcpy(v, ">");')
edit('ui/WifiScreen.cpp','  } else {   // remote screen toggle\n    if (ui.remoteScreenOn()) ui.stopRemoteScreen();\n    else ui.startRemoteScreen();\n  }','  } else if(_sel==3) {ui.selectSavedWifi();}\n  else {ui.forgetWifi();}')
edit('ui/WifiScreen.cpp','if (_sel < 3)','if (_sel < 4)')
edit('ui/WifiScreen.cpp','r < 4','r < 5')
# suppress premature save of a freshly selected SSID paired with old password.
edit('ui/WifiScreen.cpp','      ui.wifiPass()[0] = 0; // never carry a different SSID password','      int saved=ui.wifiProfiles.find(ui.wifiSsid());\n      strlcpy(ui.wifiPass(),saved>=0?ui.wifiProfiles.data.entries[saved].password:"",65);')
print('Added four saved WiFi profiles and forget controls')
