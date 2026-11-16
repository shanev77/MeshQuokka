#pragma once
// Full point: user-confirmed 4.07V measured rail after charging/unplugging.
// Voltage estimate, not a coulomb-counting fuel gauge. 3.35V is a conservative
// usable-empty indication under load, not a measured hardware cutoff voltage.
inline int batteryPercentFromMv(int mv) {
  if (mv <= 0) return -1;
  if (mv <= 3350) return 0;
  if (mv >= 4070) return 100;
  return (mv-3350)*100/720;
}
inline bool batteryRailExternal(int mv) { return mv > 4350; }
