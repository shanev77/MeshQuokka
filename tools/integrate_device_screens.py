from pathlib import Path
r=Path('MeshCore-Enhanced/vendor/meshdeck/firmware')
def edit(p,a,b):
 p=r/p;s=p.read_text(encoding='utf8');assert a in s,(p,a[:50]);p.write_text(s.replace(a,b),encoding='utf8')
edit('ui/UITask.h','SCR_VOICE, SCR_COUNT','SCR_VOICE, SCR_GPS, SCR_BLE, SCR_COUNT')
edit('ui/UITask.cpp','#include "AllScreens.h"','#include "AllScreens.h"\n#include "DeviceScreens.h"')
edit('ui/UITask.cpp','  _screens[SCR_VOICE]     = new VoiceScreen(*this);','  _screens[SCR_VOICE]     = new VoiceScreen(*this);\n  _screens[SCR_GPS] = new GPSStatusScreen(*this);\n  _screens[SCR_BLE] = new BLEStatusScreen(*this);')
edit('ui/HomeScreen.cpp','{ "Noise",     SCR_NOISE,     C_PINK,   \'~\', false }','{ "GPS",       SCR_GPS,       C_PINK,   \'G\', false }')
edit('ui/HomeScreen.cpp','{ "SOS",       SCR_SOS,       C_RED,    \'!\', false }','{ "Bluetooth", SCR_BLE,       C_CYAN,   \'B\', false }')
edit('ui/UITask.cpp','  c.print("MeshDeck");','  c.setTextSize(2);\n  c.setCursor(36,92);\n  c.print("MeshCore Enhanced");')
# Keep map arrays out of precious internal RAM and reject malformed records.
edit('ui/SDMap.cpp','#include <esp_heap_caps.h>','#include <esp_heap_caps.h>\n#include <math.h>')
edit('ui/SDMap.cpp','  if (!p) p = malloc(sz);','  // No fallback to internal heap: preserve BLE/radio headroom.')
edit('ui/SDMap.cpp','scale < 50 || scale > 2000','!isfinite(scale) || scale < 50 || scale > 2000 || f.size() != 28ULL+4ULL*npts+6ULL*nlines+20ULL*ncities')
edit('ui/SDMap.cpp','  f.close();\n  if (!ok) {','''  f.close();
  for (uint32_t i=0;ok && i<nlines;++i)
    if (p->lines[i].start>npts || p->lines[i].count>npts-p->lines[i].start) ok=false;
  for (uint32_t i=0;ok && i<ncities;++i) p->cities[i].name[14]=0;
  if (!ok) {''')
edit('ui/SDMap.cpp','int SDMaps::load(DeckHW& hw) {\n  _n = 0;','''int SDMaps::load(DeckHW& hw) {
  for(int i=0;i<_n;++i){free(_packs[i].pts);free(_packs[i].lines);free(_packs[i].cities);}
  _n = 0;''')
print('Added GPS/BLE screens and SD map validation')
