/*!
 * @file Wippersnapper_StatusLED.cpp
 *
 * Interfaces for the Wippersnapper status indicator LED/NeoPixel/Dotstar/RGB
 * LED.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Copyright (c) Brent Rubell 2020-2021 for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */
#include "Wippersnapper.h"

#ifdef USE_STATUS_NEOPIXEL
Adafruit_NeoPixel *statusPixel = new Adafruit_NeoPixel(
    STATUS_NEOPIXEL_NUM, STATUS_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

#ifdef USE_STATUS_DOTSTAR
Adafruit_DotStar *statusPixelDotStar =
    new Adafruit_DotStar(STATUS_DOTSTAR_NUM, STATUS_DOTSTAR_PIN_DATA,
                         STATUS_DOTSTAR_PIN_CLK, DOTSTAR_BRG);
#endif

extern Wippersnapper WS;
/****************************************************************************/
/*!
    @brief    Initializes board-specific status LED.
    @returns  True if initialized, False if status LED hardware is already
                in-use.
*/
/****************************************************************************/
bool Wippersnapper::statusLEDInit() {
  bool is_success = false;

#ifdef USE_STATUS_NEOPIXEL
  if (WS.lockStatusNeoPixel == false) {
    statusPixel->begin();
    statusPixel->show(); // turn all pixels off
    statusPixel->setBrightness(10);
    WS.lockStatusNeoPixel = true;
    is_success = true;
  }
#endif

// some hardware requires the NEOPIXEL_POWER pin to be enabled.
#ifdef NEEDS_STATUS_NEOPIXEL_POWER
  pinMode(NEOPIXEL_POWER, OUTPUT);
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) ||                               \
    defined(ARDUINO_ADAFRUIT_QTPY_ESP32S2) ||                                  \
    defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2_TFT)
  digitalWrite(NEOPIXEL_POWER, HIGH);
#else
  digitalWrite(NEOPIXEL_POWER, LOW);
#endif
#endif

#ifdef USE_STATUS_DOTSTAR
  if (WS.lockStatusDotStar == false) {
    statusPixelDotStar->begin();
    statusPixelDotStar->show(); // turn all pixels off
    statusPixelDotStar->setBrightness(5);
    WS.lockStatusDotStar = true;
    is_success = true;
  }
#endif

#ifdef USE_STATUS_LED
  pinMode(STATUS_LED_PIN, OUTPUT); // Initialize LED
  digitalWrite(STATUS_LED_PIN, 0); // Turn OFF LED
  WS.lockStatusLED = true;         // set global pin "lock" flag
  is_success = true;
#endif
  return is_success;
}

/****************************************************************************/
/*!
    @brief    De-initializes status LED. The usingStatus flag is also reset.
*/
/****************************************************************************/
void Wippersnapper::statusLEDDeinit() {
#ifdef USE_STATUS_NEOPIXEL
  statusPixel->clear();
  statusPixel->show(); // turn off
  WS.lockStatusNeoPixel = false;
#endif

#ifdef USE_STATUS_DOTSTAR
  statusPixelDotStar->clear();
  statusPixelDotStar->show(); // turn off
  WS.lockStatusDotStar = false;
#endif

#ifdef USE_STATUS_LED
  digitalWrite(STATUS_LED_PIN, 0); // turn off
  pinMode(STATUS_LED_PIN,
          INPUT);           // "release" for use by setting to input (hi-z)
  WS.lockStatusLED = false; // un-set global pin "lock" flag
#endif
}

/****************************************************************************/
/*!
    @brief    Sets a status RGB LED's color
    @param    color
              Desired RGB color.
*/
/****************************************************************************/
void Wippersnapper::setStatusLEDColor(uint32_t color) {
#ifdef USE_STATUS_NEOPIXEL
  uint8_t red = (color >> 16) & 0xff;  // red
  uint8_t green = (color >> 8) & 0xff; // green
  uint8_t blue = color & 0xff;         // blue
  // flood all neopixels
  for (int i = 0; i < STATUS_NEOPIXEL_NUM; i++) {
    statusPixel->setPixelColor(i, red, green, blue);
  }
  statusPixel->show();
#endif

#ifdef USE_STATUS_DOTSTAR
  uint8_t red = (color >> 16) & 0xff;  // red
  uint8_t green = (color >> 8) & 0xff; // green
  uint8_t blue = color & 0xff;         // blue
  // flood all dotstar pixels
  for (int i = 0; i < STATUS_DOTSTAR_NUM; i++) {
    statusPixelDotStar->setPixelColor(i, green, red, blue);
  }
  statusPixelDotStar->show();
#endif

#ifdef USE_STATUS_LED
  // via
  // https://github.com/adafruit/circuitpython/blob/main/supervisor/shared/status_leds.c
  digitalWrite(STATUS_LED_PIN, color > 0);
#endif
}

/****************************************************************************/
/*!
    @brief    Blinks a status LED a specific color depending on
              the hardware's state.
    @param    statusState
              Hardware's status state.
*/
/****************************************************************************/
void Wippersnapper::statusLEDBlink(ws_led_status_t statusState) {
#ifdef USE_STATUS_LED
  if (!WS.lockStatusLED)
    return;
#endif

  int blinkNum = 0;
  uint32_t ledBlinkColor;
  if (statusState == WS_LED_STATUS_KAT) {
    blinkNum = 1;
    ledBlinkColor = LED_CONNECTED;
  } else if (statusState == WS_LED_STATUS_ERROR) {
    blinkNum = 2;
    ledBlinkColor = LED_ERROR;
  } else if (statusState == WS_LED_STATUS_CONNECTED) {
    blinkNum = 3;
    ledBlinkColor = LED_CONNECTED;
  } else if (statusState == WS_LED_STATUS_FS_WRITE) {
    blinkNum = 4;
    ledBlinkColor = YELLOW;
  } else {
    blinkNum = 0;
    ledBlinkColor = BLACK;
  }

  while (blinkNum > 0) {
    setStatusLEDColor(ledBlinkColor);
    delay(250);
    setStatusLEDColor(BLACK);
    delay(250);
    blinkNum--;
  }
}