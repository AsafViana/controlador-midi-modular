#pragma once
#include <cstdint>
#include <cstring>

// SSD1306 constants used by the framework
#define SSD1306_SWITCHCAPVCC 0x02
#define SSD1306_WHITE 1
#define SSD1306_BLACK 0

class TwoWire;

/**
 * Minimal mock of Adafruit_SSD1306 for native testing.
 * Tracks method calls so tests can verify display initialization behavior.
 */
class Adafruit_SSD1306 {
public:
  // Global override: when set to false, ALL new instances will fail begin().
  // Reset to true in test teardown.
  static bool globalBeginReturnValue;

  // Track calls for test assertions
  bool beginCalled = false;
  uint8_t beginSwitchvcc = 0;
  uint8_t beginAddr = 0;
  bool beginReturnValue = true;

  int clearDisplayCallCount = 0;
  int displayCallCount = 0;
  int renderCallCount = 0;

  // GFX call tracking for TextComponent tests
  int setCursorCallCount = 0;
  int16_t lastCursorX = 0;
  int16_t lastCursorY = 0;

  int setTextSizeCallCount = 0;
  uint8_t lastTextSize = 0;

  int setTextColorCallCount = 0;
  uint16_t lastTextColor = 0;

  int printCallCount = 0;
  char lastPrintedText[256] = {0};

  // GFX call tracking for drawRect/fillRect (ProgressBarComponent tests)
  int drawRectCallCount = 0;
  int16_t lastDrawRectX = 0;
  int16_t lastDrawRectY = 0;
  int16_t lastDrawRectW = 0;
  int16_t lastDrawRectH = 0;
  uint16_t lastDrawRectColor = 0;

  int fillRectCallCount = 0;
  int16_t lastFillRectX = 0;
  int16_t lastFillRectY = 0;
  int16_t lastFillRectW = 0;
  int16_t lastFillRectH = 0;
  uint16_t lastFillRectColor = 0;

  // GFX call tracking for drawBitmap (IconComponent tests)
  int drawBitmapCallCount = 0;
  int16_t lastDrawBitmapX = 0;
  int16_t lastDrawBitmapY = 0;
  const uint8_t *lastDrawBitmapData = nullptr;
  int16_t lastDrawBitmapW = 0;
  int16_t lastDrawBitmapH = 0;
  uint16_t lastDrawBitmapColor = 0;

  // Internal state for getTextBounds simulation
  uint8_t _currentTextSize = 1;

  // Constructor matching Adafruit_SSD1306(width, height, &Wire, resetPin)
  Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *wire, int8_t rstPin)
      : _width(w), _height(h) {
    // Inherit global override at construction time
    beginReturnValue = globalBeginReturnValue;
  }

  // Default constructor for member initialization
  Adafruit_SSD1306() : _width(0), _height(0) {}

  bool begin(uint8_t switchvcc, uint8_t i2caddr) {
    beginCalled = true;
    beginSwitchvcc = switchvcc;
    beginAddr = i2caddr;
    return beginReturnValue;
  }

  void clearDisplay() { clearDisplayCallCount++; }

  void display() { displayCallCount++; }

  void invertDisplay(bool i) {
    invertDisplayCallCount++;
    lastInvertValue = i;
  }

  int invertDisplayCallCount = 0;
  bool lastInvertValue = false;

  // GFX stubs with tracking for component tests
  void setCursor(int16_t x, int16_t y) {
    setCursorCallCount++;
    lastCursorX = x;
    lastCursorY = y;
  }

  void setTextSize(uint8_t s) {
    setTextSizeCallCount++;
    lastTextSize = s;
    _currentTextSize = s;
  }

  void setTextColor(uint16_t c) {
    setTextColorCallCount++;
    lastTextColor = c;
  }

  void print(const char *text) {
    printCallCount++;
    if (text) {
      strncpy(lastPrintedText, text, sizeof(lastPrintedText) - 1);
      lastPrintedText[sizeof(lastPrintedText) - 1] = '\0';
    } else {
      lastPrintedText[0] = '\0';
    }
  }

  /**
   * Mock getTextBounds: simulates Adafruit_GFX character widths.
   * fontSize 1 = 6px per char, fontSize 2 = 12px per char, fontSize 3 = 18px
   * per char. Height: fontSize 1 = 8px, fontSize 2 = 16px, fontSize 3 = 24px.
   */
  void getTextBounds(const char *str, int16_t x, int16_t y, int16_t *x1,
                     int16_t *y1, uint16_t *w, uint16_t *h) {
    uint16_t charWidth = _currentTextSize * 6;
    uint16_t charHeight = _currentTextSize * 8;
    uint16_t len = str ? (uint16_t)strlen(str) : 0;

    if (x1)
      *x1 = x;
    if (y1)
      *y1 = y;
    if (w)
      *w = len * charWidth;
    if (h)
      *h = (len > 0) ? charHeight : 0;
  }

  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawRectCallCount++;
    lastDrawRectX = x;
    lastDrawRectY = y;
    lastDrawRectW = w;
    lastDrawRectH = h;
    lastDrawRectColor = color;
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    fillRectCallCount++;
    lastFillRectX = x;
    lastFillRectY = y;
    lastFillRectW = w;
    lastFillRectH = h;
    lastFillRectColor = color;
  }
  void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                  int16_t h, uint16_t color) {
    drawBitmapCallCount++;
    lastDrawBitmapX = x;
    lastDrawBitmapY = y;
    lastDrawBitmapData = bitmap;
    lastDrawBitmapW = w;
    lastDrawBitmapH = h;
    lastDrawBitmapColor = color;
  }

  void reset() {
    beginCalled = false;
    beginSwitchvcc = 0;
    beginAddr = 0;
    beginReturnValue = true;
    clearDisplayCallCount = 0;
    displayCallCount = 0;
    invertDisplayCallCount = 0;
    lastInvertValue = false;
    setCursorCallCount = 0;
    lastCursorX = 0;
    lastCursorY = 0;
    setTextSizeCallCount = 0;
    lastTextSize = 0;
    setTextColorCallCount = 0;
    lastTextColor = 0;
    printCallCount = 0;
    lastPrintedText[0] = '\0';
    _currentTextSize = 1;
    drawRectCallCount = 0;
    lastDrawRectX = 0;
    lastDrawRectY = 0;
    lastDrawRectW = 0;
    lastDrawRectH = 0;
    lastDrawRectColor = 0;
    fillRectCallCount = 0;
    lastFillRectX = 0;
    lastFillRectY = 0;
    lastFillRectW = 0;
    lastFillRectH = 0;
    lastFillRectColor = 0;
    drawBitmapCallCount = 0;
    lastDrawBitmapX = 0;
    lastDrawBitmapY = 0;
    lastDrawBitmapData = nullptr;
    lastDrawBitmapW = 0;
    lastDrawBitmapH = 0;
    lastDrawBitmapColor = 0;
  }

private:
  uint8_t _width;
  uint8_t _height;
};
