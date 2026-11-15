#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
class UITask;
class PNG;
// Only the visible viewport is requested. Completed PNGs remain on SD.
class StreetMap {
public:
  void draw(UITask& ui, double lat, double lon, int zoom);
  const char* status() const { return _status; }
  static double worldX(double lon, int z);
  static double worldY(double lat, int z);
private:
  struct Tile { int z=-1,x=0,y=0; uint32_t used=0; uint16_t* pixels=nullptr; };
  Tile _tiles[6];
  PNG* _png=nullptr;
  uint32_t _serial=0, _nextRequest=0;
  bool _pending=false, _configured=false;
  char _pendingPath[96]{}, _status[64]{};
  char _source[160]="https://tile.openstreetmap.org/{z}/{x}/{y}.png";
  Tile* load(UITask& ui, int z,int x,int y, bool& loaded);
};
