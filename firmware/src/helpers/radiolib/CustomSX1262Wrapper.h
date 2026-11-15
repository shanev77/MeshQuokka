#pragma once

#include "CustomSX1262.h"
#include "RadioLibWrappers.h"
#include "SX126xReset.h"

#ifndef USE_SX1262
#define USE_SX1262
#endif

class CustomSX1262Wrapper : public RadioLibWrapper {
public:
  CustomSX1262Wrapper(CustomSX1262& radio, mesh::MainBoard& board) : RadioLibWrapper(radio, board) { }

  void setParams(float freq, float bw, uint8_t sf, uint8_t cr) override {
    ((CustomSX1262 *)_radio)->setFrequency(freq);
    ((CustomSX1262 *)_radio)->setSpreadingFactor(sf);
    ((CustomSX1262 *)_radio)->setBandwidth(bw);
    ((CustomSX1262 *)_radio)->setCodingRate(cr);
    updatePreamble(sf);
  }

  bool isReceivingPacket() override { 
    return ((CustomSX1262 *)_radio)->isReceiving();
  }
  bool receiveDonePending() override {
#if defined(MESHDECK) && defined(LILYGO_TDECK)
    return (((CustomSX1262*)_radio)->getIrqFlags() & RADIOLIB_SX126X_IRQ_RX_DONE) != 0;
#else
    return false;
#endif
  }
  float getCurrentRSSI() override {
    return ((CustomSX1262 *)_radio)->getRSSI(false);
  }
  float getLastRSSI() const override { return ((CustomSX1262 *)_radio)->getRSSI(); }
  float getLastSNR() const override { return ((CustomSX1262 *)_radio)->getSNR(); }

  float packetScore(float snr, int packet_len) override {
    int sf = ((CustomSX1262 *)_radio)->spreadingFactor;
    return packetScoreInt(snr, sf, packet_len);
  }
  uint8_t getSpreadingFactor() const override { return ((CustomSX1262 *)_radio)->spreadingFactor; }
  virtual void powerOff() override {
    ((CustomSX1262 *)_radio)->sleep(false);
  }

  void doResetAGC() override {
#if defined(MESHDECK) && defined(LILYGO_TDECK)
    // Full warm-sleep/Calibrate(0x7f) recovery repeatedly latches XOSC_START_ERR
    // on the tested T-Deck Plus. Restart RX through standby, retaining the
    // already established oscillator/image calibration and radio parameters.
    // RadioLibWrapper marks IDLE, and the dispatcher restarts RX immediately.
    _radio->standby();
#else
    sx126xResetAGC((SX126x *)_radio);
#endif
  }

  void setRxBoostedGainMode(bool en) override {
    ((CustomSX1262 *)_radio)->setRxBoostedGainMode(en);
  }
  bool getRxBoostedGainMode() const override {
    return ((CustomSX1262 *)_radio)->getRxBoostedGainMode();
  }
};
