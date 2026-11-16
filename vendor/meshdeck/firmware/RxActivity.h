#pragma once
#include <stdint.h>
struct RxActivity {
  uint32_t at = 0;
  int16_t rssi = 0;
  int8_t snr4 = 0;
  uint16_t length = 0;
  uint8_t type = 0, hops = 0, width = 0;
  const void* packet = nullptr; // pending pool slot; cleared after processing
  const char* outcome = "Pending";
  uint8_t route = 0;
  bool scoped = false;
  bool headerValid = false;
  char relay[7] = "";
  void decode(const uint8_t* raw, unsigned len) {
    headerValid = false; relay[0] = 0; type = hops = width = 0;
    outcome = "Short header";
    if (!raw || len < 2) return;
    type = (raw[0] >> 2) & 15;
    route = raw[0] & 3; scoped = route == 0 || route == 3;
    unsigned pos = (route == 0 || route == 3) ? 5 : 1;
    outcome = "Unknown version";
    if ((raw[0] >> 6) > 0) return;
    outcome = "Short transport";
    if (pos >= len) return;
    width = (raw[pos] >> 6) + 1; hops = raw[pos] & 63;
    unsigned bytes = width * hops;
    outcome = "Reserved path mode";
    if (width > 3) return;
    outcome = "Path over 64 bytes";
    if (bytes > 64) return;
    outcome = "Truncated path";
    if (pos + 1 + bytes > len) return;
    outcome = "Pending";
    headerValid = true;
    // Only flood paths describe relays already traversed. Direct paths are destinations.
    if ((route == 0 || route == 1) && hops) {
      const char* hex = "0123456789abcdef";
      unsigned last = pos + 1 + bytes - width;
      for (unsigned i=0;i<width;i++) { relay[i*2]=hex[raw[last+i]>>4]; relay[i*2+1]=hex[raw[last+i]&15]; }
      relay[width*2]=0;
    }
  }
  const char* label() const {
    if (!headerValid) return "Bad header";
    static const char* names[] = {"Request","Response","Direct msg","ACK","Advert","Channel msg","Channel data","Anon req","Path","Trace","Multipart","Control"};
    return type < 12 ? names[type] : "Other";
  }
};
