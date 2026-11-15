#pragma once
#include "UITask.h"
class AboutScreen : public Screen {
public:
  AboutScreen(UITask& u):Screen(u){}
  void draw() override;
};
class GPSStatusScreen : public Screen {
public:
  GPSStatusScreen(UITask& u):Screen(u){}
  void draw() override;
  bool key(uint8_t k) override;
};
class BLEStatusScreen : public Screen {
public:
  BLEStatusScreen(UITask& u):Screen(u){}
  bool textInputActive() const override { return _editing; }
  const char* inputText() const override { return _pin; }
  bool inputSecret() const override { return true; }
  void draw() override;
  bool key(uint8_t k) override;
private:
  char _pin[7]{};
  unsigned _len=0;
  bool _editing=false;
};
