#pragma once
#include <stdbool.h>

bool uiLcdInit();
void uiLcdTestDraw();

// Var olan (OTA mesajları kullanıyor)
void uiLcdShowMsg(const char* line1, const char* line2);

// Yeni: WiFi durum + IP satırı
void uiLcdShowWifi(const char* wifiStatus, const char* ipText);
