/*
 * MeshDeck OS - standalone MeshCore firmware for the LilyGo T-Deck
 *
 * Built on the open-source MeshCore stack (MIT licence).
 * Turns the T-Deck into a self-contained mesh communicator:
 * chat UI, offline map, repeater tools, noise monitor, terminal and more.
 * BLE companion mode stays available, so the phone apps still work too.
 */

#include <Arduino.h>
#include <Mesh.h>
#include <SPIFFS.h>
#include <esp_partition.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <SD.h>
#include <mbedtls/platform.h>

static bool mountStorage() {
  if (SPIFFS.begin(false)) return true;
  const esp_partition_t* p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, nullptr);
  if (!p) return false;
  uint8_t chunk[256];
  for (size_t at=0; at<p->size; at+=sizeof(chunk)) {
    if (esp_partition_read(p,at,chunk,sizeof(chunk)) != ESP_OK) return false;
    for (uint8_t b : chunk) if (b != 0xff) return false;
  }
  return SPIFFS.format() && SPIFFS.begin(false);
}

#include "MyMesh.h"
extern RADIO_CLASS radio;

DataStore store(SPIFFS, rtc_clock);

#include <helpers/esp32/SerialBLEInterface.h>
SerialBLEInterface serial_interface;

#include "ui/UITask.h"
UITask ui_task(&board, &serial_interface);

StdRNG fast_rng;
SimpleMeshTables tables;
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store, &ui_task);

void halt() {
  while (1);
}

// ESP-IDF's prebuilt TLS defaults to scarce internal RAM. Install once before
// WiFi/BLE/tasks start; both PSRAM and internal blocks use heap_caps_free.
static void* tlsCalloc(size_t n,size_t size) {
  if(size && n>SIZE_MAX/size)return nullptr;
  if(psramFound())return heap_caps_calloc(n,size,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  return heap_caps_calloc(n,size,MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
}
void setup() {
  mbedtls_platform_set_calloc_free(tlsCalloc,heap_caps_free);
  Serial.begin(115200);

  board.begin();

  // Bring the display + input hardware up first so we can show a boot splash
  ui_task.earlyInit();

  if (!radio_init()) {
    ui_task.fatalError("Radio init failed");
    halt();
  }

  fast_rng.begin(radio_driver.getRngSeed());

  ui_task.bootStatus("loading storage...");
  if (!mountStorage()) {
    ui_task.fatalError("Storage error - back up flash");
    halt();
  }
  store.begin();

  ui_task.bootStatus("starting mesh...");
  the_mesh.begin(true);

  ui_task.bootStatus("starting bluetooth...");
  serial_interface.begin(BLE_NAME_PREFIX, the_mesh.getNodePrefs()->node_name, the_mesh.getBLEPin());
  the_mesh.startInterface(serial_interface);

  // Recover + cleanly (re)start I2C, then probe keyboard/touch, RIGHT BEFORE the
  // sensor scan - so the bus is healthy and the scan (and every later read) is
  // fast, and nothing can undo the timeout in between.
  ui_task.hw.initI2CDevices();

  ui_task.bootStatus("starting sensors...");
  sensors.begin();

#if ENV_INCLUDE_GPS == 1
  the_mesh.applyGpsPrefs();
#endif

  ui_task.bootStatus("starting ui...");
  ui_task.begin(&the_mesh, &sensors, the_mesh.getNodePrefs());

  board.onBootComplete();
  Serial.printf("[rf-startup] device_errors=0x%04x\n",radio.getDeviceErrors());
  Serial.printf("[boot] ready version=%s reset=%d psram=%u free_psram=%u internal_free=%u largest_internal=%u\n",
    MESHDECK_VERSION, int(esp_reset_reason()), ESP.getPsramSize(), ESP.getFreePsram(),
    unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)),
    unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)));
}

void loop() {
  static uint32_t last_loop=0,max_gap=0;
  uint32_t now=millis();
  if(last_loop && uint32_t(now-last_loop)>max_gap)max_gap=now-last_loop;
  last_loop=now;
  the_mesh.loop();
  sensors.loop();
  ui_task.loop();
  rtc_clock.tick();
  // Low-rate health record, without identities, messages, credentials or keys.
  static uint32_t last_health = 0;
  if (Serial && uint32_t(millis() - last_health) >= 30000) {
    last_health = millis();
    Serial.printf("[health] version=%s uptime_ms=%lu internal_free=%u free_psram=%u\n",
      MESHDECK_VERSION, (unsigned long)millis(),
      unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)), ESP.getFreePsram());
    auto* prefs = the_mesh.getNodePrefs();
    Serial.printf("[background] max_loop_gap_ms=%lu rx_irq_recovered=%lu boosted_gain=%u agc_resets=%lu recovery_ms=%d\n",
      (unsigned long)max_gap,(unsigned long)radio_driver.getRecoveredRxIRQs(),radio_driver.getRxBoostedGainMode(),(unsigned long)radio_driver.getAGCResets(),the_mesh.getAGCResetInterval());
    max_gap=0;
    Serial.printf("[input] touch=%u frames=%lu errors=%lu\n",ui_task.hw.hasTouch(),(unsigned long)ui_task.hw.touchFrames(),(unsigned long)ui_task.hw.touchErrors());
    Serial.printf("[radio] contacts=%d freq=%.3f bw=%.3f sf=%u cr=%u power=%d manual_add=%u path_bytes=%u\n",
      the_mesh.getNumContacts(), prefs->freq, prefs->bw, prefs->sf, prefs->cr, prefs->tx_power_dbm, prefs->manual_add_contacts, prefs->path_hash_mode+1);
    Serial.printf("[rf] rx=%lu errors=%lu tx=%lu rx_mode=%u chip_status=0x%02x irq=0x%04x rssi=%.1f floor=%d adverts=%u\n",
      (unsigned long)radio_driver.getPacketsRecv(), (unsigned long)radio_driver.getPacketsRecvErrors(),
      (unsigned long)radio_driver.getPacketsSent(), radio_driver.isInRecvMode(), radio.getStatus(), radio.getIrqFlags(),
      radio_driver.getCurrentRSSI(), radio_driver.getNoiseFloor(), unsigned(ui_task.heardCount()));
    auto* gps = sensors.getLocationProvider();
    if (gps) Serial.printf("[gps] enabled=%u baud=%lu bytes=%lu nmea=%lu age_ms=%lu fix=%u sats=%ld\n",
      the_mesh.getNodePrefs()->gps_enabled, (unsigned long)Serial1.baudRate(),
      (unsigned long)gps->receivedBytes(), (unsigned long)gps->validSentences(),
      (unsigned long)(millis()-gps->lastSentenceMillis()), gps->isValid(), gps->satellitesCount());
    static bool sd_reported=false;
    if(!sd_reported){
      sd_reported=true;bool mounted=ui_task.hw.sdBegin();
      if(mounted){Serial.printf("[sd] present=1 total_mb=%lu used_mb=%lu\n",(unsigned long)(SD.totalBytes()/1048576),(unsigned long)(SD.usedBytes()/1048576));ui_task.hw.sdEnd();}
      else Serial.println("[sd] present=0");
    }
  }
}
