#include "AllScreens.h"
#include "../MyMesh.h"
#include "mapdata.h"
#include "../RadioPolicy.h"
#include <helpers/AdvertDataHelpers.h>
#include <math.h>

// px per degree of longitude at each zoom step
static const float ZOOMS[] = { 256.f * 4 / 360.f, 256.f * 8 / 360.f, 256.f * 16 / 360.f, 256.f * 32 / 360.f, 256.f * 64 / 360.f, 256.f * 128 / 360.f, 256.f * 256 / 360.f, 256.f * 512 / 360.f, 256.f * 1024 / 360.f, 256.f * 2048 / 360.f, 256.f * 4096 / 360.f, 256.f * 8192 / 360.f, 256.f * 16384 / 360.f, 256.f * 32768 / 360.f, 256.f * 65536 / 360.f, 256.f * 131072 / 360.f, 256.f * 262144 / 360.f };
#define N_ZOOMS 17

int MapScreen::streetZoom() const {
  return constrain(int(lround(log2(_scale*360./256.))),2,18);
}

void MapScreen::enter() {
  if (!_centered_once) {
    double lat, lon;
    if (ui.ownPos(lat, lon)) {
      _clat = lat; _clon = lon;
      _scale = ZOOMS[13];
    } else {
      // centre on first contact with a position, else UK default
      int n = ui.mesh->getNumContacts();
      for (int i = 0; i < n; i++) {
        ContactInfo ct;
        if (ui.mesh->getContactByIdx(i, ct) && (ct.gps_lat || ct.gps_lon)) {
          _clat = ct.gps_lat / 1000000.0;
          _clon = ct.gps_lon / 1000000.0;
          _scale = ZOOMS[11];
          break;
        }
      }
    }
    _centered_once = true;
  }
}

void MapScreen::project(double lat, double lon, int& x, int& y) const {
  if(_streetMode){
    int z=streetZoom();double width=256UL<<z;
    double dx=StreetMap::worldX(lon,z)-StreetMap::worldX(_clon,z);
    if(dx>width/2)dx-=width;if(dx<-width/2)dx+=width;
    x=int(dx)+SCREEN_W/2;y=int(StreetMap::worldY(lat,z)-StreetMap::worldY(_clat,z))+SCREEN_H/2;return;
  }
  double ys = _scale / cos(_clat * 0.0174533);
  x = (int)((lon - _clon) * _scale) + SCREEN_W / 2;
  y = SCREEN_H / 2 - (int)((lat - _clat) * ys);
}

static bool onscreenish(int x, int y) {
  return x > -1500 && x < SCREEN_W + 1500 && y > -1500 && y < SCREEN_H + 1500;
}

void MapScreen::draw() {
  if(drawNodePanel()) return;
  GFXcanvas16& c = ui.cv();
  if(_streetMode){
    _clat=fmax(-85.,fmin(85.,_clat));_clon=fmod(_clon+540.,360.)-180.;
    _streets.draw(ui,_clat,_clon,streetZoom());drawNodes();
    c.drawFastHLine(155,120,11,0);c.drawFastVLine(160,115,11,0);
    ui.drawStatusBar("Street map");
    for(int i=0;i<3;++i){c.fillRoundRect(230+i*30,40,28,24,3,C_BG);c.setTextColor(C_FG);c.setTextSize(2);c.setCursor(238+i*30,44);c.print(i==0?"C":i==1?"-":"+");}
    c.fillRect(0,STATUS_H,SCREEN_W,12,C_BG);c.setTextSize(1);c.setTextColor(C_FG);
    c.setCursor(4,STATUS_H+2);c.print(_streets.status());
    c.fillRect(0,210,SCREEN_W,30,C_BG);c.setCursor(4,212);c.print("+/- zoom C centre V outline  Z");c.print(streetZoom());
    c.setCursor(4,227);c.print("(C) OpenStreetMap contributors");return;
  }
  c.fillScreen(C_MAP_BG);

  double ys = _scale / cos(_clat * 0.0174533);
  double half_lon = (SCREEN_W / 2) / _scale;
  double half_lat = (SCREEN_H / 2) / ys;
  double lat0 = _clat - half_lat, lat1 = _clat + half_lat;
  double lon0 = _clon - half_lon, lon1 = _clon + half_lon;

  // graticule
  float step = _scale < 4 ? 30 : _scale < 16 ? 10 : _scale < 64 ? 5 : 1;
  for (float g = -180; g <= 180; g += step) {
    if (g >= lon0 && g <= lon1) {
      int x, y;
      project(0, g, x, y);
      c.drawFastVLine(x, 0, SCREEN_H, C_MAP_GRID);
    }
  }
  for (float g = -80; g <= 84; g += step) {
    if (g >= lat0 && g <= lat1) {
      int x, y;
      project(g, 0, x, y);
      c.drawFastHLine(0, y, SCREEN_W, C_MAP_GRID);
    }
  }

  // does an SD pack fully cover the current view at high zoom?
  bool sd_covers = false;
  if (_scale >= 48) {
    for (int pi = 0; pi < ui.sdmaps.count(); pi++) {
      const SDMapPack* pk = ui.sdmaps.pack(pi);
      if (pk->loaded && lon0 >= pk->lon_min && lon1 <= pk->lon_max &&
          lat0 >= pk->lat_min && lat1 <= pk->lat_max) { sd_covers = true; break; }
    }
  }

  bool in_eu = lon1 > EU_LON_MIN && lon0 < EU_LON_MAX && lat1 > EU_LAT_MIN && lat0 < EU_LAT_MAX;
  bool use_eu = in_eu && _scale >= 24 && !sd_covers;

  // coastlines
  if (!sd_covers && (!use_eu || lon0 < EU_LON_MIN || lon1 > EU_LON_MAX || lat0 < EU_LAT_MIN || lat1 > EU_LAT_MAX)) {
    for (int li = 0; li < WORLD_NLINES; li++) {
      const MapLine& L = WORLD_LINES[li];
      int px = 0, py = 0;
      bool have = false;
      for (int i = 0; i < L.count; i++) {
        const MapPt& p = WORLD_PTS[L.start + i];
        double lat = p.lat / (double)WORLD_SCALE, lon = p.lon / (double)WORLD_SCALE;
        if (lat < lat0 - 8 || lat > lat1 + 8 || lon < lon0 - 8 || lon > lon1 + 8) { have = false; continue; }
        int x, y;
        project(lat, lon, x, y);
        if (!onscreenish(x, y)) { have = false; continue; }
        if (have) c.drawLine(px, py, x, y, C_MAP_LAND);
        px = x; py = y; have = true;
      }
    }
  }
  if (use_eu) {
    for (int li = 0; li < EU_NLINES; li++) {
      const MapLine& L = EU_LINES[li];
      int px = 0, py = 0;
      bool have = false;
      for (int i = 0; i < L.count; i++) {
        const MapPt& p = EU_PTS[L.start + i];
        double lat = p.lat / (double)EU_SCALE, lon = p.lon / (double)EU_SCALE;
        if (lat < lat0 - 2 || lat > lat1 + 2 || lon < lon0 - 2 || lon > lon1 + 2) { have = false; continue; }
        int x, y;
        project(lat, lon, x, y);
        if (!onscreenish(x, y)) { have = false; continue; }
        if (have) c.drawLine(px, py, x, y, C_MAP_LAND);
        px = x; py = y; have = true;
      }
    }
  }

  // SD map packs: high-detail overlays, drawn on top when zoomed into their region
  for (int pi = 0; pi < ui.sdmaps.count(); pi++) {
    const SDMapPack* pk = ui.sdmaps.pack(pi);
    if (!pk->loaded || _scale < 48) continue;
    if (lon1 < pk->lon_min || lon0 > pk->lon_max || lat1 < pk->lat_min || lat0 > pk->lat_max) continue;
    for (uint32_t li = 0; li < pk->nlines; li++) {
      const SDMapLine& L = pk->lines[li];
      int px = 0, py = 0;
      bool have = false;
      for (uint16_t i = 0; i < L.count; i++) {
        const SDMapPt& p = pk->pts[L.start + i];
        double lat = p.lat / (double)pk->scale, lon = p.lon / (double)pk->scale;
        if (lat < lat0 - 1 || lat > lat1 + 1 || lon < lon0 - 1 || lon > lon1 + 1) { have = false; continue; }
        int x, y;
        project(lat, lon, x, y);
        if (!onscreenish(x, y)) { have = false; continue; }
        if (have) c.drawLine(px, py, x, y, C_MAP_LAND);
        px = x; py = y; have = true;
      }
    }
    // extra place labels from the pack
    if (_scale >= 64) {
      c.setTextSize(1);
      for (uint32_t i = 0; i < pk->ncities; i++) {
        const SDMapCity& ct = pk->cities[i];
        double lat = ct.lat100 / 100.0, lon = ct.lon100 / 100.0;
        if (lat < lat0 || lat > lat1 || lon < lon0 || lon > lon1) continue;
        int x, y;
        project(lat, lon, x, y);
        c.fillCircle(x, y, 1, C_MAP_CITY);
        c.setTextColor(C_MAP_CITY);
        c.setCursor(x + 4, y - 3);
        c.print(ct.name);
      }
    }
  }

  // cities: which ranks show at this zoom
  int max_rank = _scale < 3 ? 0 : _scale < 8 ? 1 : _scale < 24 ? 2 : _scale < 90 ? 3 : 5;
  bool labels = _scale >= 8;
  c.setTextSize(1);
  for (int i = 0; i < N_CITIES; i++) {
    const MapCity& ct = CITIES[i];
    if (ct.rank > max_rank) continue;
    double lat = ct.lat100 / 100.0, lon = ct.lon100 / 100.0;
    if (lat < lat0 || lat > lat1 || lon < lon0 || lon > lon1) continue;
    int x, y;
    project(lat, lon, x, y);
    c.fillCircle(x, y, 1, C_MAP_CITY);
    if (labels) {
      c.setTextColor(C_MAP_CITY);
      c.setCursor(x + 4, y - 3);
      c.print(ct.name);
    }
  }

  drawNodes();

  // crosshair
  c.drawFastHLine(SCREEN_W / 2 - 5, SCREEN_H / 2, 11, C_FG_DIM);
  c.drawFastVLine(SCREEN_W / 2, SCREEN_H / 2 - 5, 11, C_FG_DIM);

  // scale bar (approx km per 60px)
  double km = 60.0 / _scale * 111.0 * cos(_clat * 0.0174533);
  char sb[24];
  if (km >= 1) snprintf(sb, sizeof(sb), "%d km", (int)km);
  else snprintf(sb, sizeof(sb), "%d m", (int)(km * 1000));
  c.drawFastHLine(8, SCREEN_H - 10, 60, C_FG);
  c.drawFastVLine(8, SCREEN_H - 13, 6, C_FG);
  c.drawFastVLine(68, SCREEN_H - 13, 6, C_FG);
  c.setTextColor(C_FG);
  c.setCursor(74, SCREEN_H - 14);
  c.print(sb);

  // header strip
  ui.drawStatusBar("Map");
  c.setTextColor(C_FG_FAINT);
  c.setCursor(6, SCREEN_H - 26);
  c.print("+/- zoom C centre F filter L unlocated");
}

void MapScreen::drawNodes() {
  GFXcanvas16& c = ui.cv();
  bool labels = _scale >= 16;

  int n = ui.mesh->getNumContacts();
  c.setTextSize(1);
  for (int i = 0; i < n; i++) {
    ContactInfo ct;
    if (!ui.mesh->getContactByIdx(i, ct)) continue;
    if (ct.gps_lat == 0 && ct.gps_lon == 0) continue;
    if(_nodeFilter==1 && ct.type!=ADV_TYPE_REPEATER)continue;
    if(_nodeFilter==2 && ct.type!=ADV_TYPE_CHAT)continue;
    if(!enhanced::validPosition(ct.gps_lat/1e6,ct.gps_lon/1e6))continue;
    double lat = ct.gps_lat / 1000000.0, lon = ct.gps_lon / 1000000.0;
    int x, y;
    project(lat, lon, x, y);
    if (x < -20 || x > SCREEN_W + 20 || y < -20 || y > SCREEN_H + 20) continue;
    if (ct.type == ADV_TYPE_REPEATER) {
      // orange diamond
      c.fillTriangle(x, y - 5, x - 5, y, x + 5, y, C_MAP_RPT);
      c.fillTriangle(x - 5, y, x + 5, y, x, y + 5, C_MAP_RPT);
    } else {
      c.fillCircle(x, y, 3, C_MAP_NODE);
      c.drawCircle(x, y, 5, C_MAP_NODE);
    }
    if (labels) {
      char nm[14];
      ellipsize(nm, sizeof(nm), ct.name);
      c.setTextColor(ct.type == ADV_TYPE_REPEATER ? C_MAP_RPT : C_MAP_NODE);
      c.setCursor(x + 7, y - 3);
      c.print(nm);
    }
  }

  // self
  double slat, slon;
  if (ui.ownPos(slat, slon)) {
    int x, y;
    project(slat, slon, x, y);
    if (x > -20 && x < SCREEN_W + 20 && y > -20 && y < SCREEN_H + 20) {
      c.fillCircle(x, y, 4, C_MAP_SELF);
      c.drawCircle(x, y, 7, C_MAP_SELF);
      if (labels) {
        c.setTextColor(C_MAP_SELF);
        c.setCursor(x + 9, y - 3);
        c.print("me");
      }
    }
  }
}

bool MapScreen::key(uint8_t k) {
  if(k=='v' || k=='V'){_streetMode=!_streetMode;return true;}
  if(_selectedNode>=0 || _unlocated) {_selectedNode=-1;_unlocated=false;return true;}
  if(k=='l'){_unlocated=true;return true;}
  if(k=='f'){_nodeFilter=(_nodeFilter+1)%3;ui.toast(_nodeFilter==0?"All located nodes":_nodeFilter==1?"Repeaters":"Clients");return true;}
  if(k==13){pickNode(SCREEN_W/2,SCREEN_H/2);return true;}
  int zi = 0;
  for (int i = 0; i < N_ZOOMS; i++) if (fabsf(_scale - ZOOMS[i]) < 0.01f) { zi = i; break; }
  if (k == '+' || k == '=' || k == 'q' || k == 'o' || k == 'O') { if (zi < N_ZOOMS - 1) _scale = ZOOMS[zi + 1]; return true; }
  if (k == '-' || k == '_' || k == 'a' || k == 'i' || k == 'I') { if (zi > 0) _scale = ZOOMS[zi - 1]; return true; }
  if (k == 'c' || k == 'C') {
    double lat, lon;
    if (ui.ownPos(lat, lon)) { _clat = lat; _clon = lon; }
    return true;
  }
  if (k == 'w') { _scale = ZOOMS[0]; _clat = 30; _clon = 0; return true; }   // world view
  if (k == 'u') { _scale = 16; _clat = 54.5; _clon = -3.5; return true; }    // UK view
  return false;
}

bool MapScreen::nav(NavEvent e) {
  if(_unlocated && e==NAV_DOWN){_unlocatedTop++;return true;}
  if(_unlocated && e==NAV_UP){if(_unlocatedTop)_unlocatedTop--;return true;}
  if(_selectedNode>=0 || _unlocated){_selectedNode=-1;_unlocated=false;return true;}
  if(e==NAV_SELECT){pickNode(SCREEN_W/2,SCREEN_H/2);if(_selectedNode>=0)return true;}
  double ys = _scale / cos(_clat * 0.0174533);
  double dlon = 30.0 / _scale, dlat = 30.0 / ys;
  switch (e) {
    case NAV_UP:    _clat += dlat; if (_clat > 84) _clat = 84; return true;
    case NAV_DOWN:  _clat -= dlat; if (_clat < -84) _clat = -84; return true;
    case NAV_LEFT:  _clon -= dlon; if (_clon < -180) _clon = -180; return true;
    case NAV_RIGHT: _clon += dlon; if (_clon > 180) _clon = 180; return true;
    case NAV_SELECT: {   // cycle zoom
      int zi = 0;
      for (int i = 0; i < N_ZOOMS; i++) if (fabsf(_scale - ZOOMS[i]) < 0.01f) { zi = i; break; }
      _scale = ZOOMS[(zi + 1) % N_ZOOMS];
      return true;
    }
    default: return false;
  }
}

bool MapScreen::touch(const TouchEvent& e) {
  if(_streetMode && e.kind==TouchEvent::TAP && e.y>=40 && e.y<64 && e.x>=230){
    return key(e.x<260?'c':e.x<290?'-':'+');
  }
  if (e.kind == TouchEvent::DRAG) {
    double ys = _scale / cos(_clat * 0.0174533);
    _clon -= e.dx / _scale;
    _clat += e.dy / ys;
    if (_clat > 84) _clat = 84;
    if (_clat < -84) _clat = -84;
    return true;
  }
  if (e.kind == TouchEvent::TAP) {
    if(_selectedNode>=0 || _unlocated){_selectedNode=-1;_unlocated=false;return true;}
    pickNode(e.x,e.y);if(_selectedNode>=0)return true;
    // recentre on tapped point
    double ys = _scale / cos(_clat * 0.0174533);
    _clon += (e.x - SCREEN_W / 2) / _scale;
    _clat -= (e.y - SCREEN_H / 2) / ys;
    return true;
  }
  // Consume the finger-lift too, so a horizontal pan isn't caught by the
  // global "swipe right = back" and bounce us out to the menu (#9).
  if (e.kind == TouchEvent::RELEASE) return true;
  return false;
}

void MapScreen::pickNode(int sx,int sy) {
  int best=225;_selectedNode=-1;
  for(int i=0;i<ui.mesh->getNumContacts();++i){
    ContactInfo ct;if(!ui.mesh->getContactByIdx(i,ct))continue;
    if(!ct.gps_lat&&!ct.gps_lon)continue;
    if(!enhanced::validPosition(ct.gps_lat/1e6,ct.gps_lon/1e6))continue;
    if(_nodeFilter==1 && ct.type!=ADV_TYPE_REPEATER)continue;
    if(_nodeFilter==2 && ct.type!=ADV_TYPE_CHAT)continue;
    int x,y;project(ct.gps_lat/1e6,ct.gps_lon/1e6,x,y);
    if(abs(x-sx)>15 || abs(y-sy)>15)continue;
    int d=(x-sx)*(x-sx)+(y-sy)*(y-sy);if(d<best){best=d;_selectedNode=i;}
  }
}
bool MapScreen::drawNodePanel() {
  if(_selectedNode<0 && !_unlocated)return false;
  auto& c=ui.cv();c.fillScreen(C_BG);c.setTextSize(1);c.setTextColor(C_FG);
  if(_unlocated){
    ui.drawStatusBar("Unlocated devices");int y=36,count=0;
    for(int i=0;i<ui.mesh->getNumContacts();++i){ContactInfo ct;
      if(!ui.mesh->getContactByIdx(i,ct)||ct.gps_lat||ct.gps_lon)continue;
      ++count;if(count>_unlocatedTop && y<205){c.setCursor(8,y);c.print(ct.name);y+=12;}
    }
    c.setCursor(8,220);c.printf("%d without location; key returns",count);return true;
  }
  ContactInfo ct;if(!ui.mesh->getContactByIdx(_selectedNode,ct)){_selectedNode=-1;return false;}
  ui.drawStatusBar("Advertised node position");
  c.setCursor(8,36);c.print(ct.name);c.setCursor(8,56);
  c.print(ct.type==ADV_TYPE_REPEATER?"Repeater":ct.type==ADV_TYPE_CHAT?"Client":"Other node");
  double lat=ct.gps_lat/1e6,lon=ct.gps_lon/1e6;
  c.setCursor(8,78);c.printf("%.6f, %.6f",lat,lon);
  c.setCursor(8,100);c.print("Advert time (sender clock):");c.setCursor(8,114);c.printf("%lu UTC epoch",(unsigned long)ct.last_advert_timestamp);
  double a,b;
  if(ui.ownPos(a,b)){
    const double rad=0.0174532925199433;
    double dlat=(lat-a)*rad,dlon=(lon-b)*rad;
    double h=sin(dlat/2)*sin(dlat/2)+cos(a*rad)*cos(lat*rad)*sin(dlon/2)*sin(dlon/2);
    h=fmax(0.,fmin(1.,h));double distance=12742.*asin(sqrt(h));
    double bearing=fmod(atan2(sin(dlon)*cos(lat*rad),cos(a*rad)*sin(lat*rad)-sin(a*rad)*cos(lat*rad)*cos(dlon))/rad+360.,360.);
    c.setCursor(8,136);c.printf("%.2f km / %.0f deg",distance,bearing);
  }
  c.setCursor(8,158);c.print("Public key:");
  for(int row=0;row<2;++row){c.setCursor(8,172+row*12);for(int j=0;j<16;++j)c.printf("%02x",ct.id.pub_key[row*16+j]);}
  c.setCursor(8,216);c.print("No RSSI location estimates. Key returns.");return true;
}
