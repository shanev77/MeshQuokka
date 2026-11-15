#include "SDArchive.h"
#include "DeckHW.h"
#include <SD.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <ArduinoJson.h>

const char* SDArchive::start(DeckHW& hw,MessageStore& store,uint32_t epoch) {
  if (_busy) return "Archive already running";
  _finished=false;_ok=false;_count=0;_index=0;_offset=0;_epoch=epoch;
  for(int t=0;t<store.numThreads();++t) _count+=store.thread(t)->count;
  if (_count>MD_MAX_THREADS*MD_THREAD_MSGS) return "Invalid history size";
  _records=(Record*)heap_caps_calloc(_count?_count:1,sizeof(Record),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  _buffer=(char*)heap_caps_malloc(2048,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if (!_records || !_buffer) {free(_records);free(_buffer);_records=nullptr;_buffer=nullptr;return "Archive needs free PSRAM";}
  size_t n=0;
  // Called on the same main loop as MessageStore mutations: a consistent copy.
  for(int t=0;t<store.numThreads();++t) {
    DeckThread* thread=store.thread(t);
    for(int m=0;m<thread->count;++m) {
      DeckMsg* msg=store.msgAt(thread,m);if(!msg)continue;
      Record& r=_records[n++];r.time=msg->ts;r.kind=thread->kind;r.channel=thread->channel_idx;
      r.flags=msg->flags;r.hops=msg->hops;r.snr4=msg->snr4;
      strlcpy(r.title,thread->title,sizeof(r.title));strlcpy(r.sender,msg->sender,sizeof(r.sender));
      strlcpy(r.text,msg->text,sizeof(r.text));
    }
  }
  _count=n;_hw=&hw;
  if(!hw.sdBegin()){free(_records);free(_buffer);_records=nullptr;_buffer=nullptr;return "No SD card";}
  _busy=true;
  if(SD.totalBytes()-SD.usedBytes()<uint64_t(_count)*2048+8192){finish(false,"SD full");return "SD full";}
  if(!SD.exists("/meshcore"))SD.mkdir("/meshcore");
  if(!SD.exists("/meshcore/archives"))SD.mkdir("/meshcore/archives");
  bool available=false;
  for(int i=0;i<8;++i){
    snprintf(_path,sizeof(_path),"/meshcore/archives/messages-%lu-%08lx.jsonl",(unsigned long)epoch,(unsigned long)esp_random());
    snprintf(_part,sizeof(_part),"%s.part",_path);
    if(!SD.exists(_path)&&!SD.exists(_part)){available=true;break;}
  }
  if(!available){finish(false,"Archive name collision");return "Archive name collision";}
  _file=SD.open(_part,FILE_WRITE);
  if(!_file){finish(false,"Cannot create SD archive");return "Cannot create SD archive";}
  _length=snprintf(_buffer,2048,"{\"type\":\"header\",\"format\":\"meshcore-enhanced-messages-v1\",\"version\":\"%s\",\"created_epoch\":%lu,\"messages\":%u}\n",
      MESHDECK_VERSION,(unsigned long)epoch,(unsigned)_count);
  strlcpy(_status,"Archiving...",sizeof(_status));return nullptr;
}

bool SDArchive::makeRecord() {
  const Record& r=_records[_index];JsonDocument doc;
  doc["type"]="message";doc["thread"]=r.title;doc["kind"]=r.kind==TK_CHANNEL?"channel":"contact";
  if(r.kind==TK_CHANNEL)doc["channel_slot"]=r.channel;
  doc["epoch"]=r.time;doc["sender"]=r.sender;doc["text"]=r.text;
  doc["outgoing"]=(r.flags&MF_OUT)!=0;doc["delivered"]=(r.flags&MF_DELIVERED)!=0;
  doc["failed"]=(r.flags&MF_FAILED)!=0;doc["snr_db"]=r.snr4/4.0f;doc["hops_raw"]=r.hops;
  if(doc.overflowed() || measureJson(doc)>2046)return false;
  _length=serializeJson(doc,_buffer,2047);_buffer[_length++]='\n';_offset=0;return true;
}
void SDArchive::poll() {
  if(!_busy)return;
  if(_offset<_length){
    size_t size=std::min(size_t(512),_length-_offset);
    if(_file.write((const uint8_t*)_buffer+_offset,size)!=size){finish(false,"SD archive write failed");return;}
    _offset+=size;return;
  }
  if(_index<_count){
    if(!makeRecord()){finish(false,"Cannot encode archive record");return;}
    ++_index;snprintf(_status,sizeof(_status),"Archive %u/%u",(unsigned)_index,(unsigned)_count);return;
  }
  _file.flush();_file.close();
  if(!SD.rename(_part,_path)){finish(false,"Archive commit failed");return;}
  finish(true,"Messages archived to SD");
}
void SDArchive::finish(bool ok,const char* reason) {
  if(_file)_file.close();
  free(_records);free(_buffer);_records=nullptr;_buffer=nullptr;
  if(_busy && _hw)_hw->sdEnd();
  _busy=false;_ok=ok;_finished=true;strlcpy(_status,reason,sizeof(_status));
}
