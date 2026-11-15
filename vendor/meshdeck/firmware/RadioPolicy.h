#pragma once
#include <math.h>
#include <stdint.h>

// AU 915 MHz hardware profile. This is an engineering envelope, not RF certification.
// Antenna gain, spectral-density compliance and spurious emissions need measurement.
namespace enhanced {
struct RadioProfile { float freq, bw; uint8_t sf, cr; int8_t power; };
inline bool validRadio(float freq, float bw, unsigned sf, unsigned cr, int power) {
  if (!isfinite(freq) || !isfinite(bw)) return false;
  // Limit to supported, conservative bandwidths; account for band edges.
  if (!(bw == 62.5f || bw == 125.f || bw == 250.f || bw == 500.f)) return false;
  return freq - bw / 2000.f >= 915.f && freq + bw / 2000.f <= 928.f &&
         sf >= 7 && sf <= 12 && cr >= 5 && cr <= 8 && power >= -9 && power <= 22;
}
inline bool validPosition(double lat, double lon) {
  return isfinite(lat) && isfinite(lon) && lat >= -90 && lat <= 90 && lon >= -180 && lon <= 180;
}
}
