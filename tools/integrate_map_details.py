from pathlib import Path
r=Path('MeshCore-Enhanced/vendor/meshdeck/firmware')
def edit(p,a,b):
 p=r/p;s=p.read_text(encoding='utf8');assert a in s,(p,a[:50]);p.write_text(s.replace(a,b),encoding='utf8')
edit('ui/AllScreens.h','  void drawNodes();','  void drawNodes();\n  void pickNode(int x,int y);\n  bool drawNodePanel();\n  int _selectedNode=-1;\n  bool _unlocated=false;\n  unsigned _nodeFilter=0;')
edit('ui/AllScreens.h','double _clat = 54.5, _clon = -3.0;','double _clat = -25.0, _clon = 134.0;')
edit('ui/MapScreen.cpp','#include "mapdata.h"','#include "mapdata.h"\n#include "../RadioPolicy.h"')
edit('ui/MapScreen.cpp','void MapScreen::draw() {','void MapScreen::draw() {\n  if(drawNodePanel()) return;')
edit('ui/MapScreen.cpp','    if (ct.gps_lat == 0 && ct.gps_lon == 0) continue;','    if (ct.gps_lat == 0 && ct.gps_lon == 0) continue;\n    if(_nodeFilter==1 && ct.type!=ADV_TYPE_REPEATER)continue;\n    if(_nodeFilter==2 && ct.type!=ADV_TYPE_CHAT)continue;\n    if(!enhanced::validPosition(ct.gps_lat/1e6,ct.gps_lon/1e6))continue;')
edit('ui/MapScreen.cpp','bool MapScreen::key(uint8_t k) {','''bool MapScreen::key(uint8_t k) {
  if(_selectedNode>=0 || _unlocated) {_selectedNode=-1;_unlocated=false;return true;}
  if(k=='l'){_unlocated=true;return true;}
  if(k=='f'){_nodeFilter=(_nodeFilter+1)%3;ui.toast(_nodeFilter==0?"All located nodes":_nodeFilter==1?"Repeaters":"Clients");return true;}
  if(k==13){pickNode(SCREEN_W/2,SCREEN_H/2);return true;}''')
edit('ui/MapScreen.cpp','bool MapScreen::nav(NavEvent e) {','''bool MapScreen::nav(NavEvent e) {
  if(_selectedNode>=0 || _unlocated){_selectedNode=-1;_unlocated=false;return true;}
  if(e==NAV_SELECT){pickNode(SCREEN_W/2,SCREEN_H/2);if(_selectedNode>=0)return true;}''')
edit('ui/MapScreen.cpp','  if (e.kind == TouchEvent::TAP) {\n    // recentre','  if (e.kind == TouchEvent::TAP) {\n    if(_selectedNode>=0 || _unlocated){_selectedNode=-1;_unlocated=false;return true;}\n    pickNode(e.x,e.y);if(_selectedNode>=0)return true;\n    // recentre')
edit('ui/MapScreen.cpp','  c.print("+/- zoom  c centre  ball pan");','  c.print("+/- zoom C centre F filter L unlocated");')
p=r/'ui/MapScreen.cpp';s=p.read_text();s+='''
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
      ++count;if(y<205){c.setCursor(8,y);c.print(ct.name);y+=12;}
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
''';p.write_text(s)
print('Added map marker inspection, client/repeater filters, unlocated list')
