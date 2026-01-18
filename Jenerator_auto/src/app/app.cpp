#include "app/app.h"
#include <Arduino.h>
#include <WiFi.h>

#include "version.h"
#include "ayarlar.h"
#include "app/hw.h"

#if USE_LCD
  #include "ui/ui_lcd.h"
#endif

#if USE_OTA
  #include "net/wifi_mgr.h"
  #include "ota/ota_update.h"
#endif

static void lcdShowWifiNow() {
#if USE_LCD
  char ipbuf[24] = "-";
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    snprintf(ipbuf, sizeof(ipbuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  }
  uiLcdShowWifi(wifiStatusText(), ipbuf);
#endif
}

void appSetup() {
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.print("Boot OK. FW=");
  Serial.println(FW_VERSION);

  hwPrintInfo();

#if USE_LCD
  uiLcdInit();
  uiLcdTestDraw();
#endif

#if USE_OTA
  otaInit();

  // WiFi dene ve LCD’ye bas
  wifiEnsureConnected();
  lcdShowWifiNow();

  if (OTA_CHECK_ON_BOOT) {
    otaCheckNow(true);
    lcdShowWifiNow();
  }
#endif
}

void appLoop() {
  static uint32_t t = 0;
  if (millis() - t > 2000) {
    t = millis();
    Serial.println("tick");

#if USE_OTA
    wifiEnsureConnected();   // backoff var, sık çağrı sorun değil
#endif
    lcdShowWifiNow();
  }

#if USE_OTA
  otaLoop();
#endif
}
