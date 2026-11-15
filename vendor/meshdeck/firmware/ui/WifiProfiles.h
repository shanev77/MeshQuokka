#pragma once
#include <Preferences.h>
#include <string.h>
// Versioned bounded NVS blob. Passwords never go into diagnostics or SD exports.
// NVS is not encrypted in this development build; physical flash access exposes it.
class WifiProfiles {
public:
  struct Entry {char ssid[33]; char password[65];};
  struct Data {uint32_t version; Entry entries[4];};
  Data data{};
  void load() {
    Preferences p;
    if(p.begin("mce-wifi",true)) {
      if(p.getBytesLength("profiles")==sizeof(data))p.getBytes("profiles",&data,sizeof(data));
      p.end();
    }
    if(data.version!=1){memset(&data,0,sizeof(data));data.version=1;}
    for(auto& e:data.entries){e.ssid[32]=0;e.password[64]=0;}
  }
  void save() {Preferences p;if(p.begin("mce-wifi",false)){p.putBytes("profiles",&data,sizeof(data));p.end();}}
  int find(const char* ssid) {for(int i=0;i<4;++i)if(!strcmp(ssid,data.entries[i].ssid))return i;return -1;}
  void remember(const char* ssid,const char* pass) {
    if(!ssid[0])return;
    int i=find(ssid);
    if(i<0)for(int j=0;j<4;++j)if(!data.entries[j].ssid[0]){i=j;break;}
    if(i<0)i=3; // bounded list: replace last slot, retain first three
    if(!strcmp(data.entries[i].ssid,ssid)&&!strcmp(data.entries[i].password,pass))return;
    strlcpy(data.entries[i].ssid,ssid,33);strlcpy(data.entries[i].password,pass,65);save();
  }
  void forget(const char* ssid){int i=find(ssid);if(i>=0){memset(&data.entries[i],0,sizeof(Entry));save();}}
};
