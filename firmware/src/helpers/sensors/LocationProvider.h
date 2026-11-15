#pragma once

#include "Mesh.h"


class LocationProvider {
protected:
    bool _time_sync_needed = true;

public:
    virtual uint32_t receivedBytes() { return 0; }
    virtual uint32_t validSentences() { return 0; }
    virtual uint32_t lastSentenceMillis() { return 0; }
    virtual void resetParser() {}
    virtual unsigned fixQuality() { return 0; }
    virtual unsigned satellitesInView() { return 0; }
    virtual unsigned strongestSignal() { return 0; }
    virtual char navigationStatus() { return '-'; }
    virtual void syncTime() { _time_sync_needed = true; }
    virtual bool waitingTimeSync() { return _time_sync_needed; }
    virtual long getLatitude() = 0;
    virtual long getLongitude() = 0;
    virtual long getAltitude() = 0;
    virtual long satellitesCount() = 0;
    virtual bool isValid() = 0;
    virtual long getTimestamp() = 0;
    virtual void sendSentence(const char * sentence);
    virtual void reset() = 0;
    virtual void begin() = 0;
    virtual void stop() = 0;
    virtual void loop() = 0;
    virtual bool isEnabled() = 0;
};
