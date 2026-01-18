#include "ui/ui_lcd.h"
#include <Arduino.h>
#include <SPI.h>

#include "pins.h"
#include "ayarlar.h"
#include "version.h"

#include <Adafruit_GFX.h>

#if UI_DRV_ST7789
  #include <Adafruit_ST7789.h>
  static Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);
#elif UI_DRV_ILI9341
  #include <Adafruit_ILI9341.h>
  static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
#else
  #error "LCD driver secilmedi! ayarlar.h icinde UI_DRV_* ayarla."
#endif

static void blOn(bool on) {
  pinMode(TFT_BL, OUTPUT);
  bool level = on ? TFT_BL_ACTIVE_HIGH : !TFT_BL_ACTIVE_HIGH;
  digitalWrite(TFT_BL, level ? HIGH : LOW);
}

bool uiLcdInit() {
  blOn(true);
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);

#if UI_DRV_ST7789
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(TFT_ROTATION);
#elif UI_DRV_ILI9341
  tft.begin();
  tft.setRotation(TFT_ROTATION);
#endif

  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("LCD INIT OK");
  return true;
}

// OTA/WiFi genel mesaj (üst bant)
void uiLcdShowMsg(const char* line1, const char* line2) {
  tft.fillRect(0, 0, TFT_WIDTH, 60, 0x0000);
  tft.setTextSize(2);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.setCursor(10, 8);
  tft.print(line1 ? line1 : "");

  tft.setTextSize(1);
  tft.setCursor(10, 35);
  tft.print(line2 ? line2 : "");
}

// Yeni: WiFi + IP (üst bant)
void uiLcdShowWifi(const char* wifiStatus, const char* ipText) {
  // 2 satır daha çok sığsın diye küçük font
  tft.fillRect(0, 0, TFT_WIDTH, 40, 0x0000);
  tft.setTextSize(1);
  tft.setTextColor(0xFFFF, 0x0000);

  tft.setCursor(6, 6);
  tft.print("WiFi: ");
  tft.print(wifiStatus ? wifiStatus : "-");

  tft.setCursor(6, 22);
  tft.print("IP  : ");
  tft.print(ipText ? ipText : "-");
}

void uiLcdTestDraw() {
  tft.fillScreen(0x0000);

  tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, 0xFFFF);

  tft.setTextSize(2);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(10, 70);
  tft.println("JENERATOR AUTO2222222");

  int h = (TFT_HEIGHT - 120) / 6;
  int y0 = 110;
  tft.fillRect(0, y0 + h * 0, TFT_WIDTH, h, 0xF800);
  tft.fillRect(0, y0 + h * 1, TFT_WIDTH, h, 0x07E0);
  tft.fillRect(0, y0 + h * 2, TFT_WIDTH, h, 0x001F);
  tft.fillRect(0, y0 + h * 3, TFT_WIDTH, h, 0xFFE0);
  tft.fillRect(0, y0 + h * 4, TFT_WIDTH, h, 0x07FF);

  tft.setTextSize(2);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.setCursor(10, TFT_HEIGHT - 30);
  tft.print("FW ");
  tft.print(FW_VERSION);
}
