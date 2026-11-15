from pathlib import Path
r=Path('MeshCore-Enhanced/vendor/meshdeck/firmware')
def edit(p,a,b):
 p=r/p;s=p.read_text(encoding='utf8');assert a in s,(p,a[:50]);p.write_text(s.replace(a,b),encoding='utf8')
edit('ui/UITask.h','  void finishOnboarding();','  bool confirmRadioChange(int action);\n  void backupRadio();\n  void restoreRadio();\n  void finishOnboarding();')
edit('ui/UITask.cpp','void UITask::applyPreset(float freq, float bw, uint8_t sf, uint8_t cr) {','''bool UITask::confirmRadioChange(int action) {
  static int pending=-1; static uint32_t at=0;
  if (pending==action && uint32_t(millis()-at)<8000) {pending=-1;return true;}
  pending=action;at=millis();toast("Mesh may disconnect. Repeat to confirm",C_YELLOW);return false;
}
void UITask::backupRadio() {
  if (!prefs) return;
  enhanced::RadioProfile p{prefs->freq,prefs->bw,prefs->sf,prefs->cr,prefs->tx_power_dbm};
  File f=SPIFFS.open("/radio-prev-v1","w");if(f){f.write((uint8_t*)&p,sizeof(p));f.close();}
}
void UITask::restoreRadio() {
  enhanced::RadioProfile p{};File f=SPIFFS.open("/radio-prev-v1","r");
  bool ok=f && f.size()==sizeof(p) && f.read((uint8_t*)&p,sizeof(p))==sizeof(p);f.close();
  if(!ok || !enhanced::validRadio(p.freq,p.bw,p.sf,p.cr,p.power)) {toast("No valid previous radio config",C_RED);return;}
  if(!confirmRadioChange(999)) return;
  prefs->freq=p.freq;prefs->bw=p.bw;prefs->sf=p.sf;prefs->cr=p.cr;prefs->tx_power_dbm=p.power;
  mesh->applyRadioPrefs();mesh->savePrefs();toast("Previous radio config restored");
}
void UITask::applyPreset(float freq, float bw, uint8_t sf, uint8_t cr) {''')
edit('ui/UITask.cpp','  prefs->freq = freq;','  backupRadio();\n  prefs->freq = freq;')
edit('ui/UITask.cpp','void OnboardScreen::choose(int i) {','''void OnboardScreen::choose(int i) {
  if (i>=0 && i<N_PRESETS) {
    const RadioPreset& p=PRESETS[i];
    if (!enhanced::validRadio(p.freq,p.bw,p.sf,p.cr,ui.prefs->tx_power_dbm)) {ui.toast("Preset outside AU 915 profile",C_RED);return;}
    if (ui.set.configured && !ui.confirmRadioChange(500+i)) return;
  }''')
edit('ui/SettingsScreen.cpp','  SI_ROOMTRIES,','  SI_ROOMTRIES, SI_RADIORESTORE,')
edit('ui/SettingsScreen.cpp','  "Room auto-login tries",','  "Room auto-login tries", "Restore previous radio",')
edit('ui/SettingsScreen.cpp','  bool radio = false;','''  bool radio = false;
  if(item==SI_SF || item==SI_CR || item==SI_POWER || item==SI_BW) {
    if(!ui.confirmRadioChange(item*3+dir))return;
    ui.backupRadio();
  }''')
# select handles switch plus early restore after realItem call (occurs adjust/select etc use unique function)
p=r/'ui/SettingsScreen.cpp';s=p.read_text();start=s.index('void SettingsScreen::select()');tail=s[start:];needle='  int item = realItem();';assert needle in tail;tail=tail.replace(needle,needle+'\n  if(item==SI_RADIORESTORE){ui.restoreRadio();return;}',1);s=s[:start]+tail;p.write_text(s)
edit('ui/SettingsScreen.cpp','        p->freq = f;','        if(!ui.confirmRadioChange(SI_FREQ)) return;\n        ui.backupRadio();\n        p->freq = f;')
edit('MyMesh.cpp','  _store->loadPrefs(_prefs, sensors.node_lat, sensors.node_lon);','''  _store->loadPrefs(_prefs, sensors.node_lat, sensors.node_lon);
  if (!enhanced::validRadio(_prefs.freq,_prefs.bw,_prefs.sf,_prefs.cr,_prefs.tx_power_dbm)) {
    _prefs.freq=LORA_FREQ;_prefs.bw=LORA_BW;_prefs.sf=LORA_SF;_prefs.cr=LORA_CR;_prefs.tx_power_dbm=LORA_TX_POWER;
  }''')
print('Added radio confirmation, rollback, and boot-time profile validation')
