#include "app/app.h"
#include <Arduino.h>

#include "version.h"
#include "ayarlar.h"
#include "app/hw.h"

#if USE_LCD
  #include "ui/ui_lcd.h"
#endif

void appSetup() {
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.print("Boot OK. FW=");
  Serial.println(FW_VERSION);

  hwPrintInfo();

#if USE_LCD
  if (uiLcdInit()) {
    uiLcdTestDraw();
  } else {
    Serial.println("LCD INIT FAILED");
  }
#endif
}

void appLoop() {
  static uint32_t t = 0;
  if (millis() - t > 2000) {
    t = millis();
    Serial.println("tick");

#if USE_LCD
    uiLcdTestDraw();
#endif
  }
}
