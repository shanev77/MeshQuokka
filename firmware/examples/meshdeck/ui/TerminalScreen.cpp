#include "BatteryGauge.h"
#include "AllScreens.h"
#include "../MyMesh.h"
#include <helpers/TxtDataHelpers.h>
#include <esp_heap_caps.h>
extern RADIO_CLASS radio;

#define T_LINE_H 11
#define T_TOP (STATUS_H + 2)
#define T_BOT (SCREEN_H - 18)
#define T_VIS ((T_BOT - T_TOP) / T_LINE_H)

void TerminalScreen::enter() {
  _scroll = 0;
}

void TerminalScreen::draw() {
  GFXcanvas16& c = ui.cv();
  c.fillScreen(C_BG);
  ui.drawStatusBar("Terminal");
  c.setTextSize(1);

  int total = ui.termCount();
  int newest = total - 1 - _scroll;
  int y = T_BOT - T_LINE_H;
  for (int i = newest; i >= 0 && y >= T_TOP; i--) {
    TermLine* l = ui.termLine(i);
    if (!l) break;
    c.setTextColor(l->color);
    c.setCursor(2, y);
    c.print(l->text);
    y -= T_LINE_H;
  }
  if (_scroll > 0) {
    c.setTextColor(C_YELLOW);
    c.setCursor(SCREEN_W - 40, T_TOP);
    char s[10];
    snprintf(s, sizeof(s), "-%d", _scroll);
    c.print(s);
  }

  // input line
  c.fillRect(0, SCREEN_H - 16, SCREEN_W, 16, C_BG_RAISED);
  c.setCursor(4, SCREEN_H - 12);
  c.setTextColor(C_TERM_IN);
  c.print("> ");
  _line[_llen] = 0;
  c.setTextColor(C_FG);
  // show tail if too long
  if (_llen <= 48) c.print(_line);
  else c.print(_line + _llen - 48);
  int cx = 16 + (_llen > 48 ? 48 : _llen) * 6;
  c.fillRect(cx + 1, SCREEN_H - 13, 2, 10, C_ACCENT);
}

ContactInfo* TerminalScreen::findByName(const char* name) {
  ContactInfo* ct = ui.mesh->searchContactsByPrefix(name);
  return ct;
}

void TerminalScreen::execCommand(const char* line_in) {
  char line[96];
  StrHelper::strncpy(line, line_in, sizeof(line));
  // Never mirror command arguments (may contain passwords or private text).
  ui.termLog(C_TERM_IN, "> command");

  // split cmd + args
  char* args = strchr(line, ' ');
  if (args) { *args++ = 0; while (*args == ' ') args++; }

  MyMesh* m = ui.mesh;

  if (strcmp(line, "help") == 0) {
    ui.termLog(C_TERM_SYS, "advert | advertf | contacts | heard | traffic | noise | mem");
    ui.termLog(C_TERM_SYS, "send <name> <msg>   ch <idx> <msg>");
    ui.termLog(C_TERM_SYS, "login <name> <pwd>  resync <name>  cmd <name> <cli...>");
    ui.termLog(C_TERM_SYS, "freq <MHz> | sf <7-12> | bw <kHz> | cr <5-8> | power <dBm>");
    ui.termLog(C_TERM_SYS, "name <newname> | time <epoch> | clear | reboot");
    ui.termLog(C_TERM_SYS, "rfcheck | rfclear | rxrecover <0|4> | rfswitch <0|1>");
  } else if (strcmp(line, "battery") == 0) {
    int mv=ui.getBattMilliVolts();
    ui.termLog(C_TERM_SYS,"Supply %dmV %s percent=%d",mv,batteryRailExternal(mv)?"USB (cell unknown)":"battery estimate",batteryPercentFromMv(mv));
  } else if (strcmp(line, "batterytest") == 0) {
    bool ok=batteryPercentFromMv(0)==-1 && batteryPercentFromMv(3000)==0 && batteryPercentFromMv(3350)==0 && batteryPercentFromMv(3710)==50 && batteryPercentFromMv(4070)==100 && batteryPercentFromMv(4200)==100 && batteryPercentFromMv(5000)==100;
    for(int mv=3350;mv<5000;mv++) ok &= batteryPercentFromMv(mv)<=batteryPercentFromMv(mv+1);
    ok &= !batteryRailExternal(4200) && batteryRailExternal(4500);
    ui.termLog(C_TERM_SYS,"Battery bounds/monotonicity/USB: %s",ok?"PASS":"FAIL");
  } else if (strcmp(line, "rfcheck") == 0) {
    uint8_t frequency[4]={},sync[2]={},gain=0;
    int f=radio.readRegister(RADIOLIB_SX126X_REG_RF_FREQUENCY_0,frequency,4);
    int s=radio.readRegister(RADIOLIB_SX126X_REG_LORA_SYNC_WORD_MSB,sync,2);
    int g=radio.readRegister(RADIOLIB_SX126X_REG_RX_GAIN,&gain,1);
    uint32_t word=(uint32_t(frequency[0])<<24)|(uint32_t(frequency[1])<<16)|(uint32_t(frequency[2])<<8)|frequency[3];
    Serial.printf("[rf-registers] read=%d,%d,%d frequency_word=0x%08lx sync=0x%02x%02x gain=0x%02x device_errors=0x%04x\n",f,s,g,(unsigned long)word,sync[0],sync[1],gain,radio.getDeviceErrors());
    Serial.printf("[rf-config] driver_freq=%.3f bw=%.3f sf=%u cr=%u crc=%u header=%u autoadd_mask=%u max_hops=%u recovery_ms=%d resets=%lu\n",radio.freqMHz,radio.bandwidthKhz,radio.spreadingFactor,radio.codingRate+4,radio.crcTypeLoRa,radio.headerType,m->getNodePrefs()->autoadd_config,m->getNodePrefs()->autoadd_max_hops,m->getAGCResetInterval(),(unsigned long)radio_driver.getAGCResets());
    ui.termLog(C_TERM_SYS,"RF register check written to USB");
    Serial.printf("[rf-switch] dio2_control=%u\n",radio.dio2RfSwitch);
  } else if (strcmp(line, "rfswitch") == 0) {
    if(args && (strcmp(args,"0")==0 || strcmp(args,"1")==0)) {
      uint32_t resets=radio_driver.getAGCResets();
      radio_driver.resetAGC(); // guarded RX->standby; skips pending packet/TX
      if(resets==radio_driver.getAGCResets()) {
        ui.termLog(C_TERM_SYS,"Radio busy; retry switch check");
        Serial.println("[rf-switch] busy=1 unchanged=1");
      } else {
        int result=radio.setDio2AsRfSwitch(strcmp(args,"1")==0);
        Serial.printf("[rf-switch] result=%d dio2_control=%u\n",result,radio.dio2RfSwitch);
        ui.termLog(C_TERM_SYS,"DIO2 RF switch: %u result %d",radio.dio2RfSwitch,result);
      }
    } else ui.termLog(C_TERM_SYS,"rfswitch 0/1; diagnostic, reboot restores 1");
  } else if (strcmp(line, "rfclear") == 0) {
    uint16_t before=radio.getDeviceErrors();
    int result=radio.clearDeviceErrors();
    Serial.printf("[rf-errors] before=0x%04x clear_result=%d after=0x%04x\n",before,result,radio.getDeviceErrors());
    ui.termLog(C_TERM_SYS,"RF latched error clear: %d",result);
  } else if (strcmp(line, "rxrecover") == 0) {
    if(args && (strcmp(args,"0")==0 || strcmp(args,"4")==0)) m->setRxRecovery(strcmp(args,"4")==0);
    Serial.printf("[rf-recovery] interval_ms=%d resets=%lu\n",m->getAGCResetInterval(),(unsigned long)radio_driver.getAGCResets());
    ui.termLog(C_TERM_SYS,"RX recovery: %d ms",m->getAGCResetInterval());
  } else if (strcmp(line, "mapcheck") == 0) {
    bool ok=ui.transfer.checkTLSFileLifetime();
    Serial.printf("[map-test] repeated_TLS_stop_open_SD=%s\n",ok?"PASS":"FAIL/busy");
    ui.termLog(C_TERM_SYS,ok?"TLS / SD check passed":"TLS / SD check failed or busy");
  } else if (strcmp(line, "map") == 0) {
    ui.go(SCR_MAP);
  } else if (strcmp(line, "home") == 0) {
    ui.go(SCR_HOME);
  } else if (strcmp(line, "advert") == 0) {
    m->advert();
    ui.termLog(C_TERM_TX, "zero-hop advert sent");
  } else if (strcmp(line, "advertf") == 0) {
    m->advertFlood();
    ui.termLog(C_TERM_TX, "flood advert sent");
  } else if (strcmp(line,"rxtest")==0) {
    ui.termLog(C_TERM_SYS,"RX paths/scopes/bounds/dedup: %s",m->checkRxDecode()?"PASS":"FAIL");
  } else if (strcmp(line,"channeltest")==0) {
    ui.termLog(C_TERM_SYS,"Channel decoder (general + named text): %s",m->checkChannelDecode()?"PASS":"FAIL");
  } else if (strcmp(line,"channelstats")==0) {
    ui.termLog(C_TERM_SYS,"Channel rx=%lu decoded=%lu delivered=%lu stored=%lu",
      (unsigned long)m->channelCount(mesh::Mesh::CH_SEEN),(unsigned long)m->channelCount(mesh::Mesh::CH_DECRYPTED),
      (unsigned long)m->channelCount(mesh::Mesh::CH_DELIVERED),(unsigned long)m->channelCount(mesh::Mesh::CH_STORED));
    ui.termLog(C_TERM_SYS,"Channel no_key=%lu bad_mac=%lu duplicate=%lu",
      (unsigned long)m->channelCount(mesh::Mesh::CH_NO_KEY),(unsigned long)m->channelCount(mesh::Mesh::CH_BAD_MAC),(unsigned long)m->channelCount(mesh::Mesh::CH_DUPLICATE));
    ui.termLog(C_TERM_SYS,"Channel short=%lu bad_text=%lu storage_failed=%lu",
      (unsigned long)m->channelCount(mesh::Mesh::CH_SHORT),(unsigned long)m->channelCount(mesh::Mesh::CH_BAD_TEXT),(unsigned long)m->channelCount(mesh::Mesh::CH_STORE_FAILED));
  } else if (strcmp(line, "unreadtest") == 0) {
    // Isolated RAM store: never load/save or modify the user's conversations.
    MessageStore test;
    DeckThread* a=test.forChannel(0,"test A");
    DeckThread* b=test.forChannel(1,"test B");
    bool ok=a && b;
    if(ok) {
      test.addMsg(a,"peer","one",1,0,0,0,0);
      test.addMsg(b,"peer","two",2,0,0,0,0);
      test.addMsg(b,"me","out",3,MF_OUT,0,0,0);
      ok &= a->unread==1 && b->unread==1 && test.totalUnread()==2;
      test.markRead(a);
      ok &= a->unread==0 && b->unread==1 && !(test.msgAt(a,0)->flags & MF_UNREAD);
      ok &= (test.msgAt(b,0)->flags & MF_UNREAD)!=0;
      for(int i=0;i<MD_THREAD_MSGS+2;i++) test.addMsg(b,"peer","fill",4+i,0,0,0,0);
      ok &= b->unread==MD_THREAD_MSGS;
      test.markRead(b); ok &= test.totalUnread()==0;
    }
    if(a) free(a->msgs);
    if(b) free(b->msgs);
    ui.termLog(C_TERM_SYS,"Isolated unread checks: %s",ok?"PASS":"FAIL");
  } else if (strcmp(line, "ding") == 0) {
    if (args && strcmp(args,"dark")==0) ui.hw.displayOff();
    ui.hw.chimeDiscovery();
    ui.termLog(C_TERM_SYS,"Discovery ding queued: sound=%u volume=%u display=%u",ui.hw.soundOn(),ui.hw.volume(),ui.hw.isDisplayOn());
  } else if (strcmp(line, "traffictest") == 0) {
    RxActivity a; bool ok=true;
    const uint8_t flood[]={0x11,0x41,0xdd,0xbb,0x99};
    a.decode(flood,sizeof(flood)); ok &= a.headerValid && a.hops==1 && a.width==2 && strcmp(a.relay,"ddbb")==0;
    for(unsigned n=0;n<4;n++) { a.decode(flood,n); ok &= !a.headerValid; }
    const uint8_t direct[]={0x12,0x41,0xdd,0xbb};
    a.decode(direct,sizeof(direct)); ok &= a.headerValid && !a.relay[0];
    const uint8_t scoped[]={0x10,0,0,0,0,0x81,0xaa,0xbb,0xcc};
    a.decode(scoped,sizeof(scoped)); ok &= a.headerValid && strcmp(a.relay,"aabbcc")==0;
    for(unsigned n=0;n<9;n++) { a.decode(scoped,n); ok &= !a.headerValid; }
    const uint8_t legacy[]={0x11,1,0xab};
    a.decode(legacy,sizeof(legacy)); ok &= a.headerValid && strcmp(a.relay,"ab")==0;
    const uint8_t reserved[]={0x11,0xc0};
    a.decode(reserved,sizeof(reserved)); ok &= !a.headerValid;
    const uint8_t version[]={0x91,0};
    a.decode(version,sizeof(version)); ok &= !a.headerValid;
    LastHeardScreen screen(ui);
    screen.key('t'); // Traffic mode must not swallow global Back.
    ok &= !screen.nav(NAV_BACK);
    TouchEvent tap = {}; tap.kind=TouchEvent::TAP; tap.x=260; tap.y=STATUS_H+17;
    ok &= screen.touch(tap); // Switch back to Nodes via the same touch target.
    ok &= !screen.nav(NAV_BACK);
    screen.touch(tap); // Traffic again, after a touch toggle.
    ok &= !screen.nav(NAV_BACK);
    ui.termLog(C_TERM_SYS,"Traffic parser/navigation tests: %s",ok?"PASS":"FAIL");
  } else if (strcmp(line, "traffic") == 0) {
    ui.termLog(C_TERM_SYS,"Adverts valid=%lu self=%lu dup=%lu old=%lu badsig=%lu baddata=%lu short=%lu",
      (unsigned long)m->advertCount(mesh::Mesh::ADV_VALID),(unsigned long)m->advertCount(mesh::Mesh::ADV_SELF),
      (unsigned long)m->advertCount(mesh::Mesh::ADV_DUPLICATE),(unsigned long)m->advertCount(mesh::Mesh::ADV_OLD),
      (unsigned long)m->advertCount(mesh::Mesh::ADV_BAD_SIGNATURE),(unsigned long)m->advertCount(mesh::Mesh::ADV_BAD_DATA),
      (unsigned long)m->advertCount(mesh::Mesh::ADV_SHORT));
    for(unsigned i=0;i<m->activityCount();i++) {
      const RxActivity* e=m->activityAt(i);
      ui.termLog(C_TERM_SYS,"%lus %s len=%u rssi=%d snr=%.2f hops=%u width=%u relay=%s",
        (unsigned long)((millis()-e->at)/1000),e->label(),e->length,e->rssi,e->snr4/4.0f,e->hops,e->width,e->relay);
      ui.termLog(C_TERM_SYS,"  scope=%u result=%s",e->scoped,e->outcome);
    }
  } else if (strcmp(line, "contacts") == 0) {
    int n = m->getNumContacts();
    for (int i = 0; i < n; i++) {
      ContactInfo ct;
      if (m->getContactByIdx(i, ct)) {
        ui.termLog(C_TERM_SYS, "%2d. %-24s t%d path %d", i, ct.name, ct.type,
                   ct.out_path_len == 0xFF ? -1 : ct.out_path_len);
      }
    }
    if (n == 0) ui.termLog(C_TERM_SYS, "(none)");
  } else if (strcmp(line, "heard") == 0) {
    for (int i = 0; i < ui.heardCount() && i < 10; i++) {
      const HeardEntry* e = ui.heardAt(i);
      char ago[8];
      ui.fmtAgo(ago, sizeof(ago), e->at);
      ui.termLog(C_TERM_SYS, "%-20s %s ago snr %s", e->name, ago, StrHelper::ftoa(e->snr4 / 4.0f));
    }
  } else if (strcmp(line, "noise") == 0) {
    ui.termLog(C_TERM_SYS, "noise floor: %d dBm", m->getNoiseFloorNow());
  } else if (strcmp(line, "mem") == 0) {
    ui.termLog(C_TERM_SYS, "heap %u free / psram %u free",
               (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getFreePsram());
  } else if (strcmp(line, "send") == 0 && args) {
    char* msg = strchr(args, ' ');
    if (msg) {
      *msg++ = 0;
      ContactInfo* ct = findByName(args);
      if (ct) ui.sendDM(ct->id.pub_key, msg);
      else ui.termLog(C_TERM_ERR, "no contact matching '%s'", args);
    }
  } else if (strcmp(line, "ch") == 0 && args) {
    char* msg = strchr(args, ' ');
    if (msg) {
      *msg++ = 0;
      ui.sendChannel(atoi(args), msg);
    }
  } else if (strcmp(line, "login") == 0 && args) {
    char* pwd = strchr(args, ' ');
    if (pwd) {
      *pwd++ = 0;
      ContactInfo* ct = findByName(args);
      if (ct) {
        // Track pending so onLoginResult can save password / toast
        bool ok = ui.beginRoomLogin(*ct, pwd, true);
        ui.termLog(ok ? C_TERM_TX : C_TERM_ERR, "login -> %s", ct->name);
      } else ui.termLog(C_TERM_ERR, "no contact matching '%s'", args);
    }
  } else if (strcmp(line, "resync") == 0 && args) {
    ContactInfo* ct = findByName(args);
    if (ct) {
      bool ok = ui.resyncRoom(*ct, true);
      ui.termLog(ok ? C_TERM_TX : C_TERM_ERR, "resync full backlog -> %s", ct->name);
    } else ui.termLog(C_TERM_ERR, "no contact matching '%s'", args);
  } else if (strcmp(line, "cmd") == 0 && args) {
    char* cli = strchr(args, ' ');
    if (cli) {
      *cli++ = 0;
      ContactInfo* ct = findByName(args);
      if (ct) {
        uint32_t est;
        int res = m->sendCommandData(*ct, ui.epochNow(), 0, cli, est);
        ui.termLog(res == MSG_SEND_FAILED ? C_TERM_ERR : C_TERM_TX, "cmd -> %s: %s", ct->name, cli);
      } else ui.termLog(C_TERM_ERR, "no contact matching '%s'", args);
    }
  } else if (strcmp(line, "trace") == 0 && args) {
    ContactInfo* ct = findByName(args);
    if (ct) ui.startTrace(*ct);
    else ui.termLog(C_TERM_ERR, "no contact matching '%s'", args);
  } else if (strcmp(line, "freq") == 0 && args) {
    float f = atof(args);
    if (f > 100 && f < 1000) {
      ui.prefs->freq = f;
      m->applyRadioPrefs();
      m->savePrefs();
      ui.termLog(C_TERM_SYS, "freq set to %s MHz", StrHelper::ftoa(f));
    } else ui.termLog(C_TERM_ERR, "bad freq");
  } else if (strcmp(line, "sf") == 0 && args) {
    int v = atoi(args);
    if (v >= 7 && v <= 12) {
      ui.prefs->sf = v;
      m->applyRadioPrefs();
      m->savePrefs();
      ui.termLog(C_TERM_SYS, "sf set to %d", v);
    } else ui.termLog(C_TERM_ERR, "sf must be 7-12");
  } else if (strcmp(line, "bw") == 0 && args) {
    float v = atof(args);
    if (v >= 7 && v <= 500) {
      ui.prefs->bw = v;
      m->applyRadioPrefs();
      m->savePrefs();
      ui.termLog(C_TERM_SYS, "bw set to %s kHz", StrHelper::ftoa(v));
    } else ui.termLog(C_TERM_ERR, "bad bw");
  } else if (strcmp(line, "cr") == 0 && args) {
    int v = atoi(args);
    if (v >= 5 && v <= 8) {
      ui.prefs->cr = v;
      m->applyRadioPrefs();
      m->savePrefs();
      ui.termLog(C_TERM_SYS, "cr set to %d", v);
    } else ui.termLog(C_TERM_ERR, "cr must be 5-8");
  } else if (strcmp(line, "power") == 0 && args) {
    int v = atoi(args);
    if (v >= 1 && v <= 22) {
      ui.prefs->tx_power_dbm = v;
      m->applyRadioPrefs();
      m->savePrefs();
      ui.termLog(C_TERM_SYS, "tx power set to %d dBm", v);
    } else ui.termLog(C_TERM_ERR, "power must be 1-22");
  } else if (strcmp(line, "name") == 0 && args) {
    StrHelper::strncpy(ui.prefs->node_name, args, sizeof(ui.prefs->node_name));
    m->savePrefs();
    ui.termLog(C_TERM_SYS, "name set to '%s' (advert to announce)", args);
  } else if (strcmp(line, "time") == 0 && args) {
    uint32_t e = strtoul(args, NULL, 10);
    if (e > 1600000000) {
      m->getRTCClock()->setCurrentTime(e);
      ui.termLog(C_TERM_SYS, "clock set");
    } else ui.termLog(C_TERM_ERR, "bad epoch");
  } else if (strcmp(line, "clear") == 0) {
    ui.termClear();
  } else if (strcmp(line, "reboot") == 0) {
    ESP.restart();
  } else if (line[0]) {
    ui.termLog(C_TERM_ERR, "unknown command - try 'help'");
  }
}

bool TerminalScreen::key(uint8_t k) {
  if (k == 0x0D) {
    if (_llen > 0) {
      _line[_llen] = 0;
      _llen = 0;
      _scroll = 0;
      execCommand(_line);
    }
    return true;
  }
  if (k == 0x08) {
    if (_llen > 0) { _llen--; return true; }
    return false;
  }
  if (k >= 32 && k < 127 && _llen < (int)sizeof(_line) - 2) {
    _line[_llen++] = k;
    return true;
  }
  return false;
}

bool TerminalScreen::nav(NavEvent e) {
  switch (e) {
    case NAV_UP:
      if (_scroll < ui.termCount() - 1) _scroll += 3;
      return true;
    case NAV_DOWN:
      _scroll -= 3;
      if (_scroll < 0) _scroll = 0;
      return true;
    default:
      return false;
  }
}
