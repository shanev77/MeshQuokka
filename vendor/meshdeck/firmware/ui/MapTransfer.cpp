#include "MapTransfer.h"
#include "DeckHW.h"
#include "MapRoots.h"
#include <SD.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <esp_heap_caps.h>

// Arduino-ESP32 2.0.17 stop_ssl_socket() zeroes its context, including socket.
// A later stop/destructor can therefore close VFS descriptor 0 (an SD file).
// Restore the invalid-socket sentinel after EVERY stop, including destruction.
class MapTLSClient : public WiFiClientSecure {
public:
  void stop() override { WiFiClientSecure::stop(); sslclient->socket=-1; }
  ~MapTLSClient() override { stop(); }
};

bool MapTransfer::checkTLSFileLifetime() {
  if(_busy || !_hw || !_hw->sdBegin())return false;
  const char* path="/meshdeck-maps/.tls-file-test";
  SD.mkdir("/meshdeck-maps");
  if(SD.exists(path)){_hw->sdEnd();return false;}
  File f=SD.open(path,FILE_WRITE);
  bool ok=f && f.write(uint8_t(0x51))==1;
  if(ok){
    { MapTLSClient c; c.stop(); c.stop(); } // includes base/derived destruction
    ok=f.write(uint8_t(0xa7))==1;f.flush();
  }
  f.close();
  File check=SD.open(path);
  ok=ok && check && check.size()==2 && check.read()==0x51 && check.read()==0xa7;
  check.close();SD.remove(path);_hw->sdEnd();
  return ok;
}

bool MapTransfer::begin(DeckHW* hw) {
  if (_requests) return true;
  _hw=hw;
  _requests=xQueueCreate(1,sizeof(Request));
  _events=xQueueCreate(4,sizeof(Event));
  if (_requests && _events && xTaskCreatePinnedToCore(task,"map-network",8192,this,1,nullptr,0)==pdPASS) return true;
  if(_requests)vQueueDelete(_requests);if(_events)vQueueDelete(_events);
  _requests=nullptr;_events=nullptr;return false;
}
void MapTransfer::task(void* ptr) { static_cast<MapTransfer*>(ptr)->run(); }
bool MapTransfer::emit(Event& ev) {
  // Even cancellation must deliver a terminal event so main can close storage.
  return xQueueSend(_events,&ev,portMAX_DELAY)==pdTRUE;
}
void MapTransfer::run() {
  Request req;
  for (;;) {
    if (xQueueReceive(_requests,&req,portMAX_DELAY)!=pdTRUE) continue;
    Event ev{};
    MapTLSClient client;
    client.setCACert(MAP_ROOTS);
    client.setHandshakeTimeout(10);
    HTTPClient http;
    http.setUserAgent("MeshQuokka/" MESHDECK_VERSION " (LilyGO T-Deck Plus)");
    http.setConnectTimeout(8000); http.setTimeout(5000);
    const char* headers[]={"ETag","Content-Range"};
    http.collectHeaders(headers,2);
    bool ok=http.begin(client,req.url);
    if (ok && req.offset && req.etag[0]) {
      http.addHeader("Range",String("bytes=")+req.offset+"-");
      http.addHeader("If-Range",req.etag);
    }
    int code=ok ? http.GET() : -1;
    int length=http.getSize();
    uint32_t offset=code==206 ? req.offset : 0;
    String range=http.header("Content-Range");
    String tag=http.header("ETag");
    unsigned first=0,last=0,total=0;
    bool rangeOK=code!=206 || (req.offset>0 &&
      sscanf(range.c_str(),"bytes %u-%u/%u",&first,&last,&total)==3 &&
      first==req.offset && last>=first && last-first+1==(unsigned)length &&
      last+1==total && tag==req.etag);
    ok=(code==200 || code==206) && length>0 && rangeOK &&
       (uint64_t)offset+length<=req.limit;
    // Stage the bounded response in PSRAM. Close TLS before delivering storage
    // events, so network teardown cannot overlap an outstanding FAT writer.
    uint8_t* body=ok ? static_cast<uint8_t*>(heap_caps_malloc(length,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT)) : nullptr;
    ok=ok && body;
    if(ok && !_cancel.load()) {
      WiFiClient* stream=http.getStreamPtr();uint32_t idle=millis();int done=0;
      while(done<length && !_cancel.load()) {
        int available=stream->available();
        if(available>0){
          int got=stream->read(body+done,std::min(length-done,std::min(available,2048)));
          if(got<=0){ok=false;break;}done+=got;idle=millis();
        } else {
          if(!http.connected() || uint32_t(millis()-idle)>10000){ok=false;break;}
          vTaskDelay(pdMS_TO_TICKS(2));
        }
      }
      ok=ok && done==length && !_cancel.load();
    } else ok=false;
    http.end();client.stop();
    if(ok) {
      ev.kind=0;ev.code=code;ev.total=offset+length;
      if(tag.startsWith("\"") && tag.length()<sizeof(ev.etag))strlcpy(ev.etag,tag.c_str(),sizeof(ev.etag));
      emit(ev);
      for(int at=0;at<length && !_cancel.load();) {
        ev.kind=1;ev.count=std::min(length-at,512);memcpy(ev.data,body+at,ev.count);emit(ev);at+=ev.count;
      }
      ok=!_cancel.load();
    }
    free(body);
    if(Serial)Serial.printf("[map-transfer] http=%d complete=%u bytes=%d\n",code,ok,length);
    ev={}; ev.kind=2; ev.code=ok ? 1 : 0;
    emit(ev);
  }
}
bool MapTransfer::start(const char* url,const char* destination,uint32_t limit) {
  if (_busy || !_requests || strncmp(url,"https://",8)!=0 || strlen(url)>=192 ||
      strlen(destination)>=sizeof(_path) || limit>4*1024*1024) return false;
  if (!_hw->sdBegin()) {strlcpy(_status,"No SD card",sizeof(_status)); return false;}
  SD.mkdir("/meshdeck-maps");
  strlcpy(_path,destination,sizeof(_path));
  snprintf(_part,sizeof(_part),"%s.part",_path);
  snprintf(_meta,sizeof(_meta),"%s.etag",_path);
  Request req{}; strlcpy(req.url,url,sizeof(req.url)); req.limit=limit;
  File partial,meta;
  if(SD.exists(_part))partial=SD.open(_part);
  if(SD.exists(_meta))meta=SD.open(_meta);
  if (partial && meta) {
    String tag=meta.readStringUntil('\n');tag.trim();
    if (tag.startsWith("\"") && tag.length()<sizeof(req.etag) && partial.size()<limit) {
      strlcpy(req.etag,tag.c_str(),sizeof(req.etag)); req.offset=partial.size();
    }
  }
  partial.close();meta.close();
  _limit=limit; _received=req.offset; _total=0; _ok=false; _failed=false;
  _cancel.store(false); _busy=true;
  strlcpy(_status,"Connecting securely...",sizeof(_status));
  if (xQueueSend(_requests,&req,0)!=pdTRUE) {_busy=false;_hw->sdEnd();return false;}
  return true;
}
void MapTransfer::poll() {
  if (!_busy) return;
  Event ev;
  // At most 1 KiB written per call; mesh loop executes between drains.
  for (int budget=0;budget<2 && xQueueReceive(_events,&ev,0)==pdTRUE;++budget) {
    if (ev.kind==0) {
      _total=ev.total;
      if (SD.totalBytes()-SD.usedBytes()<uint64_t(_total-_received)+8192) {
        _failed=true;_cancel.store(true);strlcpy(_status,"SD full",sizeof(_status));continue;
      }
      if (ev.code==200) { SD.remove(_part); _received=0; }
      _file=SD.open(_part,FILE_APPEND);
      if (!_file) {_failed=true;_cancel.store(true);continue;}
      SD.remove(_meta);
      if (ev.etag[0]) {File m=SD.open(_meta,FILE_WRITE);if(m){m.println(ev.etag);m.close();}}
      strlcpy(_status,"Downloading (P pauses)",sizeof(_status));
    } else if (ev.kind==1 && !_failed) {
      if (!_file || _received+ev.count>_limit || _file.write(ev.data,ev.count)!=ev.count) {
        _failed=true;_cancel.store(true);strlcpy(_status,"SD write failed",sizeof(_status));
      } else _received+=ev.count;
    } else if (ev.kind==2) {
      if (_file) {_file.flush();_file.close();}
      _ok=ev.code==1 && !_failed && _received==_total && _total>0 && !_cancel.load();
      if (_ok) {
        // Preserve old complete map until the new file is entirely received.
        char backup[104];snprintf(backup,sizeof(backup),"%s.old",_path);
        SD.remove(backup);
        bool existed=SD.exists(_path);
        if (existed && !SD.rename(_path,backup)) _ok=false;
        if (_ok && !SD.rename(_part,_path)) {
          _ok=false;if(existed)SD.rename(backup,_path);
        }
        if (_ok) {SD.remove(backup);SD.remove(_meta);}
      }
      strlcpy(_status,_ok ? "Saved. Reload maps to view." : (_cancel.load()?"Paused; Enter resumes":"Transfer failed; Enter retries"),sizeof(_status));
      _busy=false;_hw->sdEnd();return;
    }
  }
}
