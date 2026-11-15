#pragma once

#include "LocationProvider.h"
#include <MicroNMEA.h>
#include <RTClib.h>
#include <helpers/RefCountedDigitalPin.h>

#ifndef GPS_EN
    #ifdef PIN_GPS_EN
        #define GPS_EN PIN_GPS_EN
    #else
        #define GPS_EN (-1)
    #endif
#endif

#ifndef PIN_GPS_EN_ACTIVE
    #define PIN_GPS_EN_ACTIVE HIGH
#endif

#ifndef GPS_RESET
    #ifdef PIN_GPS_RESET
        #define GPS_RESET PIN_GPS_RESET
    #else
        #define GPS_RESET (-1)
    #endif
#endif

#ifndef GPS_RESET_FORCE
    #ifdef PIN_GPS_RESET_ACTIVE
        #define GPS_RESET_FORCE PIN_GPS_RESET_ACTIVE
    #else
        #define GPS_RESET_FORCE LOW
    #endif
#endif

class MicroNMEALocationProvider : public LocationProvider {
    char _nmeaBuffer[240]; // MicroNMEA's buffer length parameter is uint8_t
    static_assert(sizeof(_nmeaBuffer) <= 255, "MicroNMEA buffer length must fit uint8_t");
    MicroNMEA nmea;
    mesh::RTCClock* _clock;
    Stream* _gps_serial;
    RefCountedDigitalPin* _peripher_power;
    int8_t _claims = 0;
    int _pin_reset;
    int _pin_en;
    long next_check = 0;
    long time_valid = 0;
    unsigned long _last_time_sync = 0;
    uint32_t _received_bytes = 0, _valid_sentences = 0, _last_sentence_ms = 0;
    uint32_t _gga_count = 0, _rmc_count = 0, _last_diag = 0;
    unsigned _quality = 0, _view = 0, _cn0_max = 0, _last_cn0_max = 0;
    char _navigation_status = '-';
    static const char* field(const char* s, unsigned index) {
        while (index-- && *s) {
            while (*s && *s != ',' && *s != '*') ++s;
            if (*s != ',') return "";
            ++s;
        }
        return s;
    }
    static const unsigned long TIME_SYNC_INTERVAL = 1800000; // Re-sync every 30 minutes

public :
    uint32_t receivedBytes() override { return _received_bytes; }
    uint32_t validSentences() override { return _valid_sentences; }
    uint32_t lastSentenceMillis() override { return _last_sentence_ms; }
    unsigned fixQuality() override { return _quality; }
    unsigned satellitesInView() override { return _view; }
    unsigned strongestSignal() override { return _cn0_max > _last_cn0_max ? _cn0_max : _last_cn0_max; }
    char navigationStatus() override { return _navigation_status; }
    void resetParser() override { nmea.clear(); nmea.process('\n'); }
    MicroNMEALocationProvider(Stream& ser, mesh::RTCClock* clock = NULL, int pin_reset = GPS_RESET, int pin_en = GPS_EN,RefCountedDigitalPin* peripher_power=NULL) :
    _gps_serial(&ser), nmea(_nmeaBuffer, sizeof(_nmeaBuffer)), _pin_reset(pin_reset), _pin_en(pin_en), _clock(clock), _peripher_power(peripher_power) {
        if (_pin_reset != -1) {
            pinMode(_pin_reset, OUTPUT);
            digitalWrite(_pin_reset, GPS_RESET_FORCE);
        }
        if (_pin_en != -1) {
            pinMode(_pin_en, OUTPUT);
            digitalWrite(_pin_en, LOW);
        }
    }

    void claim() {
        _claims++;
        if (_claims > 0) {
            if (_peripher_power) _peripher_power->claim();
        }
    }

    void release() {
        if (_claims == 0) return; // avoid negative _claims
        _claims--;
        if (_peripher_power) _peripher_power->release();
    }

    void begin() override {
        claim();
        if (_pin_en != -1) {
            digitalWrite(_pin_en, PIN_GPS_EN_ACTIVE);
        }
        if (_pin_reset != -1) {
            digitalWrite(_pin_reset, !GPS_RESET_FORCE);
        }
    }

    void reset() override {
        if (_pin_reset != -1) {
            digitalWrite(_pin_reset, GPS_RESET_FORCE);
            delay(10);
            digitalWrite(_pin_reset, !GPS_RESET_FORCE);
        }
    }

    void stop() override {
        if (_pin_en != -1) {
            digitalWrite(_pin_en, !PIN_GPS_EN_ACTIVE);
        }
        if (_pin_reset != -1) {
            digitalWrite(_pin_reset, GPS_RESET_FORCE);
        }
        release();
    }

    bool isEnabled() override {
        // directly read the enable pin if present as gps can be
        // activated/deactivated outside of here ...
        if (_pin_en != -1) {
            return digitalRead(_pin_en) == PIN_GPS_EN_ACTIVE;
        } else {
            return true; // no enable so must be active
        }
    }

    void syncTime() override { nmea.clear(); LocationProvider::syncTime(); }
    long getLatitude() override { return nmea.getLatitude(); }
    long getLongitude() override { return nmea.getLongitude(); }
    long getAltitude() override { 
        long alt = 0;
        nmea.getAltitude(alt);
        return alt;
    }
    long satellitesCount() override { return nmea.getNumSatellites(); }
    bool isValid() override { return nmea.isValid(); }

    long getTimestamp() override { 
        DateTime dt(nmea.getYear(), nmea.getMonth(),nmea.getDay(),nmea.getHour(),nmea.getMinute(),nmea.getSecond());
        return dt.unixtime();
    } 

    void sendSentence(const char *sentence) override {
        nmea.sendSentence(*_gps_serial, sentence);
    }

    void loop() override {

        unsigned budget = 128;
        while (budget-- && _gps_serial->available()) {
            char c = _gps_serial->read();
            ++_received_bytes;
            #ifdef GPS_NMEA_DEBUG
            Serial.print(c);
            #endif
            if (nmea.process(c)) {
                const char* sentence = nmea.getSentence();
                if (sentence[0] == '$' && MicroNMEA::testChecksum(sentence)) {
                    ++_valid_sentences;
                    _last_sentence_ms = millis();
                    if (strlen(sentence) >= 7) {
                        if (strncmp(sentence+3,"GGA,",4)==0) {
                            ++_gga_count; _quality=atoi(field(sentence,6));
                        } else if (strncmp(sentence+3,"RMC,",4)==0) {
                            ++_rmc_count; const char* status=field(sentence,2);
                            _navigation_status=(*status=='A'||*status=='V')?*status:'-';
                        } else if (strncmp(sentence+3,"GSV,",4)==0) {
                            _view=atoi(field(sentence,3));
                            for (unsigned i=7; i<=19; i+=4) {
                                unsigned cn0=atoi(field(sentence,i));
                                if (cn0<=99 && cn0>_cn0_max) _cn0_max=cn0;
                            }
                        }
                    }
                }
            }
        }

        if (!isValid()) time_valid = 0;
#if defined(MESHDECK) && defined(LILYGO_TDECK)
        if (uint32_t(millis()-_last_diag)>=10000) {
            _last_diag=millis();
            if (Serial) Serial.printf("[gnss] gga=%lu quality=%u rmc=%lu status=%c view_last=%u parser_fix=%u cn0_max_10s=%u\n",
                (unsigned long)_gga_count,_quality,(unsigned long)_rmc_count,
                _navigation_status,_view,isValid(),_cn0_max);
            _last_cn0_max=_cn0_max; _cn0_max=0;
        }
#endif

        if (millis() > next_check) {
            next_check = millis() + 1000;
            // Re-enable time sync periodically when GPS has valid fix
            if (!_time_sync_needed && _clock != NULL && (millis() - _last_time_sync) > TIME_SYNC_INTERVAL) {
                _time_sync_needed = true;
            }
            if (_time_sync_needed && time_valid > 2) {
                if (_clock != NULL) {
                    _clock->setCurrentTime(getTimestamp());
                    _time_sync_needed = false;
                    _last_time_sync = millis();
                }
            }
            if (isValid()) {
                time_valid ++;
            }
        }
    }
};
