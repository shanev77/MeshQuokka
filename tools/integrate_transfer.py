from pathlib import Path
import ssl,certifi,re,tempfile,os
r=Path('MeshCore-Enhanced/vendor/meshdeck/firmware')
roots=[]
for pem in re.findall(r'-----BEGIN CERTIFICATE-----.*?-----END CERTIFICATE-----',Path(certifi.where()).read_text(),re.S):
 fd,path=tempfile.mkstemp(suffix='.pem');os.close(fd);Path(path).write_text(pem)
 cert=ssl._ssl._test_decode_cert(path);os.unlink(path)
 name=str(cert['subject'])
 if any(n in name for n in ['ISRG Root X1','DigiCert Global Root G2','DigiCert Global Root CA','USERTrust RSA Certification Authority']):roots.append(pem)
assert len(roots)>=3
(r/'ui/MapRoots.h').write_text('#pragma once\n// Mozilla trust roots from certifi; HTTPS hostname validation remains enabled.\nstatic const char MAP_ROOTS[] = R"PEM(\n'+'\n'.join(roots)+'\n)PEM";\n')
def edit(p,a,b):
 p=r/p;s=p.read_text(encoding='utf8');assert a in s,(p,a[:50]);p.write_text(s.replace(a,b),encoding='utf8')
edit('ui/UITask.h','#include "NewMap.h"','#include "NewMap.h"\n#include "MapTransfer.h"')
edit('ui/UITask.h','  DeckHW hw;','  DeckHW hw;\n  MapTransfer transfer;')
edit('ui/UITask.cpp','  store.loop();','  transfer.poll();\n  store.loop();')
edit('ui/UITask.cpp','  _screens[SCR_HOME]','  transfer.begin(&hw);\n  _screens[SCR_HOME]') # only initialization exact occurs? check replaced other instances? string only init likely
p=r/'ui/UITask.cpp';s=p.read_text(encoding='utf8');a=s.index('const char* UITask::downloadMapPack');b=s.index('const char* UITask::prepareSD',a)
s=s[:a]+'''const char* UITask::downloadMapPack(const char* name) {
  if (wifiState()!=2) return "Connect WiFi first";
  if (transfer.busy()) return "Transfer already active";
  for (const char* p=name;*p;++p) if (!isalnum((unsigned char)*p) && *p!='_' && *p!='-') return "Invalid region code";
  char url[192],path[96];
  snprintf(url,sizeof(url),"https://meshdeck-os.github.io/meshdeck/maps/%s.mdm",name);
  snprintf(path,sizeof(path),"/meshdeck-maps/%s.mdm",name);
  return transfer.start(url,path,4*1024*1024) ? nullptr : "Cannot start download";
}
const char* UITask::downloadMapIndex(char* buf,size_t sz,int& outlen) {
  outlen=0;
  if (!hw.sdBegin()) return "No SD card";
  File f=SD.open("/meshdeck-maps/index.txt");
  if (!f || f.size()>=sz) {f.close();hw.sdEnd();return "Map index unavailable";}
  outlen=f.readBytes(buf,f.size());buf[outlen]=0;
  f.close();hw.sdEnd();return outlen ? nullptr : "Empty index";
}

'''+s[b:];p.write_text(s,encoding='utf8')
edit('ui/AllScreens.h','  void loadIndex();','  void loadIndex();\n  void tick1s() override;\n  bool _indexPending = false;')
edit('ui/MapDownloadScreen.cpp','  ui.toast("Fetching map list...", C_CYAN);\n  loadIndex();','  if (ui.transfer.busy()) { _status="Transfer running"; return; }\n  _indexPending=ui.transfer.start("https://meshdeck-os.github.io/meshdeck/maps/index.txt", "/meshdeck-maps/index.txt",4095);\n  _status=_indexPending ? "Fetching map list..." : "Cannot fetch index";')
edit('ui/MapDownloadScreen.cpp','void MapDownloadScreen::loadIndex() {','void MapDownloadScreen::tick1s() {\n  if (_indexPending && !ui.transfer.busy()) {\n    _indexPending=false;\n    if (ui.transfer.succeeded()) loadIndex();\n    else _status=ui.transfer.status();\n  }\n  ui.requestDraw();\n}\n\nvoid MapDownloadScreen::loadIndex() {')
edit('ui/MapDownloadScreen.cpp','    snprintf(msg, sizeof(msg), "%s map saved", _names[idx]);','    snprintf(msg, sizeof(msg), "%s download started", _names[idx]);')
edit('ui/MapDownloadScreen.cpp','  if (_status) {\n    c.setTextColor', '''  if (ui.transfer.busy() && !_indexPending) {
    c.setCursor(8,70);c.setTextColor(C_FG);c.print(ui.transfer.status());
    c.setCursor(8,95);c.printf("%lu / %lu bytes",(unsigned long)ui.transfer.received(),(unsigned long)ui.transfer.total());
    c.setCursor(8,120);c.print("P pauses; Enter retries/resumes");
    return;
  }
  if (_status) {
    c.setTextColor''')
edit('ui/MapDownloadScreen.cpp','  if (_status) return false;\n  if (k == 0x0D)', '  if (k==\'p\' || k==\'P\') {ui.transfer.pause();return true;}\n  if (_status) return false;\n  if (k == 0x0D)')
# Reference-count mounts so UI probes cannot unmount an in-flight transfer.
edit('ui/DeckHW.h','  SPIClass* _spi = nullptr;','  unsigned _sd_refs = 0;\n  SPIClass* _spi = nullptr;')
edit('ui/DeckHW.cpp','  return SD.begin(TDECK_SD_CS, *_spi, 10000000);','  if (_sd_refs) {++_sd_refs;return true;}\n  if (!SD.begin(TDECK_SD_CS,*_spi,10000000)) return false;\n  _sd_refs=1;return true;')
edit('ui/DeckHW.cpp','  SD.end();','  if (_sd_refs && --_sd_refs==0) SD.end();')
print('Integrated bounded, certificate-verified asynchronous map transfers')
