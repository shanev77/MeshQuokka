#pragma once

#include <Wire.h>
#include <Arduino.h>
#include "helpers/ESP32Board.h"
#include <driver/rtc_io.h>

#define PIN_VBAT_READ 4
#define BATTERY_SAMPLES 8
#define ADC_MULTIPLIER (2.0f * 3.3f * 1000)

class TDeckBoard : public ESP32Board {
public:
  void begin();

  #ifdef P_LORA_TX_LED
    void onBeforeTransmit() override{
      digitalWrite(P_LORA_TX_LED, LOW); // turn TX LED on - invert pin for SX1276
    }

    void onAfterTransmit() override{
      digitalWrite(P_LORA_TX_LED, HIGH); // turn TX LED off - invert pin for SX1276
    }
  #endif

  void enterDeepSleep(uint32_t secs, int pin_wake_btn) {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // Make sure the DIO1 and NSS GPIOs are hold on required levels during deep sleep
    rtc_gpio_set_direction((gpio_num_t)P_LORA_DIO_1, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_en((gpio_num_t)P_LORA_DIO_1);

    rtc_gpio_hold_en((gpio_num_t)P_LORA_NSS);

    if (pin_wake_btn < 0) {
      esp_sleep_enable_ext1_wakeup( (1L << P_LORA_DIO_1), ESP_EXT1_WAKEUP_ANY_HIGH); // wake up on: recv LoRa packet
    } else {
      esp_sleep_enable_ext1_wakeup( (1L << P_LORA_DIO_1) | (1L << pin_wake_btn), ESP_EXT1_WAKEUP_ANY_HIGH); // wake up on: recv LoRa packet OR wake btn
    }

    if (secs > 0) {
      esp_sleep_enable_timer_wakeup(secs * 1000000);
    }

    // Finally set ESP32 into sleep
    esp_deep_sleep_start(); // CPU halts here and never returns!
  }

  uint16_t getBattMilliVolts() override {
    // BAT_ADC measures the switched supply rail through RD2/RD3 (100k/100k).
    // On USB this rail is above cell voltage: it cannot reveal state of charge.
    // Use the ESP32 calibration, not an assumed 3.3V full-scale ADC conversion.
    static uint32_t sampledAt = 0;
    static uint16_t cached = 0;
    if (cached && uint32_t(millis()-sampledAt) < 1000) return cached;
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_VBAT_READ, ADC_11db);
    uint32_t sum = 0;
    for (int i=0;i<BATTERY_SAMPLES;i++) sum += analogReadMilliVolts(PIN_VBAT_READ);
    cached = (sum * 2 + BATTERY_SAMPLES/2) / BATTERY_SAMPLES;
    sampledAt = millis();
    return cached;
  }

  const char* getManufacturerName() const{
    return "LilyGo T-Deck";
  }
};