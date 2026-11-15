#include "StreetMap.h"
#include "UITask.h"
#include <PNGdec.h>
#include <SD.h>
#include <esp_heap_caps.h>
#include <new>
#include <math.h>

namespace {
struct Decode { PNG* png; uint16_t* pixels; };
int line(PNGDRAW* d) {
  auto* ctx=static_cast<Decode*>(d->pUser);
  if(d->y<0 || d->y>=256 || d->iWidth!=256) return 0;
  ctx->png->getLineAsRGB565(d,ctx->pixels+d->y*256,PNG_RGB565_LITTLE_ENDIAN,0xffffffff);
  return 1;
}
void pathFor(char* out,size_t size,int z,int x,int y) {
  snprintf(out,size,"/meshdeck-maps/streets/%d_%d_%d.png",z,x,y);
}
}
double StreetMap::worldX(double lon,int z) { return (lon+180.)/360.*(256UL<<z); }
double StreetMap::worldY(double lat,int z) {
  lat=fmax(-85.05112878,fmin(85.05112878,lat));
  double r=lat*0.017453292519943295;
  return (1.-asinh(tan(r))/M_PI)*.5*(256UL<<z);
}
StreetMap::Tile* StreetMap::load(UITask& ui,int z,int x,int y,bool& loaded) {
  for(auto& t:_tiles) if(t.z==z && t.x==x && t.y==y){t.used=++_serial;return &t;}
  if(loaded) return nullptr; // Decode at most one tile between mesh loop calls.
  char path[96];pathFor(path,sizeof(path),z,x,y);
  if(!ui.hw.sdBegin()){strlcpy(_status,"Insert SD card for street maps",sizeof(_status));return nullptr;}
  if(!SD.exists(path)){ui.hw.sdEnd();return nullptr;}
  File f=SD.open(path);
  if(!f){ui.hw.sdEnd();return nullptr;}
  size_t size=f.size();
  auto* data=size>0 && size<=131072 ? static_cast<uint8_t*>(heap_caps_malloc(size,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT)) : nullptr;
  bool readOK=data && f.read(data,size)==size;
  f.close();ui.hw.sdEnd();loaded=true;
  if(!_png){void* mem=heap_caps_malloc(sizeof(PNG),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);if(mem)_png=new(mem) PNG();}
  Tile* slot=&_tiles[0];for(auto& t:_tiles)if(t.used<slot->used)slot=&t;
  if(!slot->pixels)slot->pixels=static_cast<uint16_t*>(heap_caps_malloc(256*256*2,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  bool ok=false;
  if(readOK && _png && slot->pixels && _png->openRAM(data,size,line)==PNG_SUCCESS){
    if(_png->getWidth()==256 && _png->getHeight()==256 && !_png->isInterlaced()){
      slot->z=-1;Decode ctx{_png,slot->pixels};ok=_png->decode(&ctx,PNG_CHECK_CRC)==PNG_SUCCESS;
    }
    _png->close();
  }
  free(data);
  if(!ok){strlcpy(_status,"Invalid tile or low map memory",sizeof(_status));return nullptr;}
  if(Serial)Serial.printf("[map] decoded=1 bytes=%u zoom=%d\n",unsigned(size),z);
  slot->z=z;slot->x=x;slot->y=y;slot->used=++_serial;return slot;
}
void StreetMap::draw(UITask& ui,double lat,double lon,int z) {
  auto& c=ui.cv();c.fillScreen(0xdefb);
  // Hold one mount reference for the whole frame; nested readers reuse it.
  if(!ui.hw.sdBegin()){strlcpy(_status,"Insert SD card for street maps",sizeof(_status));return;}
  if(!_configured && ui.hw.sdBegin()) {
    SD.mkdir("/meshdeck-maps");SD.mkdir("/meshdeck-maps/streets");
    File cfg=SD.open("/meshdeck-maps/streets/source.txt");
    if(cfg){String s=cfg.readStringUntil('\n');s.trim();
      if(s.startsWith("https://") && s.length()<sizeof(_source) && s.indexOf("{z}")>=0 && s.indexOf("{x}")>=0 && s.indexOf("{y}")>=0)strlcpy(_source,s.c_str(),sizeof(_source));cfg.close();}
    ui.hw.sdEnd();_configured=true;
  }
  if(_pending && !ui.transfer.busy()) {
    _pending=false;
    if(!ui.transfer.succeeded()){
      _nextRequest=millis()+60000;strlcpy(_status,"Download failed; retry in 60 seconds",sizeof(_status));
    }
  }
  double cx=worldX(lon,z),cy=worldY(lat,z);
  int left=int(floor(cx-160)),top=int(floor(cy-120)),n=1<<z;
  bool loaded=false, missing=false;int requestX=-1,requestY=0;
  for(int ty=int(floor(top/256.));ty<=int(floor((top+239)/256.));++ty){
    if(ty<0 || ty>=n)continue;
    for(int tx=int(floor(left/256.));tx<=int(floor((left+319)/256.));++tx){
      int x=((tx%n)+n)%n;Tile* t=load(ui,z,x,ty,loaded);
      int dx=tx*256-left,dy=ty*256-top;
      if(t)c.drawRGBBitmap(dx,dy,t->pixels,256,256);
      else {
        missing=true;c.drawRect(dx,dy,256,256,0xbdf7);
        // Cached files await decoding; never download them again on redraw.
        if(requestX<0 && ui.hw.sdBegin()){
          char path[96];pathFor(path,sizeof(path),z,x,ty);
          if(!SD.exists(path)){requestX=x;requestY=ty;}ui.hw.sdEnd();
        }
      }
    }
  }
  if(requestX>=0 && !_pending && !ui.transfer.busy() && ui.wifiState()==2 && int32_t(millis()-_nextRequest)>=0){
    String url=_source;url.replace("{z}",String(z));url.replace("{x}",String(requestX));url.replace("{y}",String(requestY));
    pathFor(_pendingPath,sizeof(_pendingPath),z,requestX,requestY);
    _pending=ui.transfer.start(url.c_str(),_pendingPath,131072);
    _nextRequest=millis()+(_pending?1000:60000);
  }
  if(_pending)strlcpy(_status,"Loading visible street tiles...",sizeof(_status));
  else if(!missing)strlcpy(_status,"Street maps - SD cache",sizeof(_status));
  else if(ui.wifiState()!=2)strlcpy(_status,"Missing tiles: connect WiFi / hotspot",sizeof(_status));
  else if(int32_t(millis()-_nextRequest)>=0)strlcpy(_status,"Loading SD tiles...",sizeof(_status));
  ui.hw.sdEnd();
}
