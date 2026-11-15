#pragma once
#include <Arduino.h>
#include <FS.h>
#include "MessageStore.h"
class DeckHW;

// Snapshot in bounded PSRAM; main-loop writes never exceed 512 bytes per poll.
class SDArchive {
public:
  const char* start(DeckHW& hw, MessageStore& store, uint32_t epoch);
  void poll();
  bool busy() const { return _busy; }
  bool succeeded() const { return _ok; }
  bool takeFinished() { bool v=_finished;_finished=false;return v; }
  const char* status() const { return _status; }
  const char* path() const { return _path; }
private:
  struct Record {
    uint32_t time;
    uint8_t kind,channel,flags,hops;
    int8_t snr4;
    char title[28],sender[MD_SENDER_LEN],text[MD_TEXT_LEN];
  };
  void finish(bool ok, const char* reason);
  bool makeRecord();
  DeckHW* _hw=nullptr;
  Record* _records=nullptr;
  char* _buffer=nullptr;
  File _file;
  size_t _count=0,_index=0,_length=0,_offset=0;
  uint32_t _epoch=0;
  bool _busy=false,_ok=false,_finished=false;
  char _path[100]{},_part[108]{},_status[48]{"Not archived"};
};
