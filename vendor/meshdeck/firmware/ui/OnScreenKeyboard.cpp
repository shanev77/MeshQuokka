#include "UITask.h"
#include <ctype.h>

// Four rows of ten keys. Page 2 covers every printable ASCII symbol.
static const char* keyboardRows(uint8_t page, int row) {
  static const char* letters[] = {"1234567890", "qwertyuiop", "asdfghjkl;", "zxcvbnm,.?"};
  static const char* symbols[] = {"!@#$%^&*()", "-_=+[]{}\\|", ";:'\"<>/?`~", "1234567890"};
  return page == 2 ? symbols[row] : letters[row];
}

void UITask::drawKeyboard() {
  auto& c = cv();
  Screen* s = _screens[_cur];
  c.fillScreen(C_BG);
  c.setTextSize(1); c.setTextColor(C_ACCENT);
  c.setCursor(8, 7); c.print("On-screen keyboard");
  c.setTextColor(C_FG_DIM); c.setCursor(8, 23);
  c.print("Enter sends/saves. Hide keeps editing.");
  const char* value = s->inputText();
  size_t length = strlen(value), start = length > 100 ? length - 100 : 0;
  c.setTextColor(C_FG);
  for (size_t i = start; i < length; ++i) {
    int cell = i - start;
    c.setCursor(8 + (cell % 50) * 6, 43 + (cell / 50) * 15);
    c.write(s->inputSecret() ? '*' : value[i]);
  }
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 10; ++col) {
      char key = keyboardRows(_keyboard_page, row)[col];
      if (_keyboard_page == 1) key = toupper((unsigned char)key);
      c.fillRoundRect(col * 32 + 1, 82 + row * 29, 30, 27, 3, C_BG_RAISED);
      c.setTextColor(C_FG); c.setTextSize(2);
      c.setCursor(col * 32 + 10, 88 + row * 29); c.write(key);
    }
  }
  static const char* labels[] = {"Case", "123", "Space", "Del", "Enter", "Hide"};
  c.setTextSize(1);
  for (int i = 0; i < 6; ++i) {
    c.fillRoundRect(i * 53 + 1, 200, 51, 38, 3, i == 4 ? C_ACCENT : C_BG_RAISED);
    c.setTextColor(i == 4 ? C_BG : C_FG);
    c.setCursor(i * 53 + 6, 215); c.print(labels[i]);
  }
}

bool UITask::keyboardTouch(const TouchEvent& e) {
  if (e.kind != TouchEvent::TAP) return true;
  Screen* s = _screens[_cur];
  if (!s->textInputActive()) { _keyboard_open = false; return true; }
  if (e.x < 0 || e.x >= 320) return true;
  uint8_t key = 0;
  if (e.y >= 82 && e.y < 198) {
    key = keyboardRows(_keyboard_page, (e.y - 82) / 29)[e.x / 32];
    if (_keyboard_page == 1) key = toupper(key);
  } else if (e.y >= 200 && e.y < 240 && e.x < 318) {
    switch (e.x / 53) {
      case 0: _keyboard_page = _keyboard_page == 1 ? 0 : 1; break;
      case 1: _keyboard_page = _keyboard_page == 2 ? 0 : 2; break;
      case 2: key = ' '; break;
      case 3: key = 8; break;
      case 4: _keyboard_open = false; key = 13; break;
      case 5: _keyboard_open = false; break;
    }
  }
  // Call the field directly: an empty backspace must never navigate away.
  if (key) s->textKey(key);
  return true;
}
