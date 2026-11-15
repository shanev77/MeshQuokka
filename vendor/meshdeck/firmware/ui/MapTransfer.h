#pragma once
#include <Arduino.h>
#include <FS.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
class DeckHW;

// Network task never touches SPI, SD, UI or MyMesh. The main loop drains a
// bounded queue into storage. Pause/resume uses strong ETag + HTTP If-Range.
class MapTransfer {
public:
  bool begin(DeckHW* hw);
  bool start(const char* url, const char* destination, uint32_t limit);
  void poll();
  bool checkTLSFileLifetime(); // local diagnostic; no network traffic
  void pause() { if (_busy) _cancel.store(true); }
  bool busy() const { return _busy; }
  bool succeeded() const { return _ok; }
  uint32_t received() const { return _received; }
  uint32_t total() const { return _total; }
  const char* status() const { return _status; }
private:
  struct Request { char url[192]; char etag[96]; uint32_t offset, limit; };
  struct Event { int kind, code; uint32_t total; uint16_t count; char etag[96]; uint8_t data[512]; };
  static void task(void* ptr);
  void run();
  bool emit(Event& ev);
  DeckHW* _hw = nullptr;
  QueueHandle_t _requests = nullptr, _events = nullptr;
  std::atomic<bool> _cancel{false};
  bool _busy = false, _ok = false, _failed = false;
  File _file;
  char _path[96]{}, _part[104]{}, _meta[108]{}, _status[64]{};
  uint32_t _received = 0, _total = 0, _limit = 0;
};
