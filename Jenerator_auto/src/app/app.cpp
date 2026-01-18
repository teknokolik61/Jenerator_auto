#include "app.h"
#include <Arduino.h>
#include "version.h"

void appSetup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.print("Boot OK. FW=");
  Serial.println(FW_VERSION);

  // PSRAM test (varsa bilgi basar)
  #if defined(BOARD_HAS_PSRAM)
    Serial.print("PSRAM: ");
    Serial.println(psramFound() ? "FOUND" : "NOT FOUND");
    if (psramFound()) {
      Serial.print("PSRAM size: ");
      Serial.println(ESP.getPsramSize());
    }
  #endif

  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
}

void appLoop() {
  static uint32_t t = 0;
  if (millis() - t > 1000) {
    t = millis();
    Serial.println("tick");
  }
}
