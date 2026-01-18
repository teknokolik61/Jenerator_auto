#include "app/hw.h"
#include <Arduino.h>

void hwPrintInfo() {
  Serial.println("=== HW INFO ===");
  Serial.print("Chip model: "); Serial.println(ESP.getChipModel());
  Serial.print("Chip rev  : "); Serial.println(ESP.getChipRevision());
  Serial.print("CPU MHz   : "); Serial.println(ESP.getCpuFreqMHz());

  Serial.print("Flash size: "); Serial.println(ESP.getFlashChipSize());

  bool ps = psramFound();
  Serial.print("PSRAM     : "); Serial.println(ps ? "FOUND" : "NOT FOUND");
  if (ps) {
    Serial.print("PSRAM size: "); Serial.println(ESP.getPsramSize());
    Serial.print("PSRAM free: "); Serial.println(ESP.getFreePsram());
  }

  Serial.print("Heap free : "); Serial.println(ESP.getFreeHeap());
  Serial.println("=============");
}
