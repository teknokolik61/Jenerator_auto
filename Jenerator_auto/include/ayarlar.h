#pragma once
#include <stdint.h>

// =====================
// FEATURES (aç/kapat)
// =====================
#define USE_LCD     1
#define USE_TG      0
#define USE_OTA     0

// =====================
// LCD Driver seçimi (MUTLAKA #define olmalı)
// =====================
// Varsayılan: ST7789 (çok yaygın)
// Ekranın ILI9341 ise: ST7789=0, ILI9341=1 yap
#define UI_DRV_ST7789   1
#define UI_DRV_ILI9341  0

// =====================
// Panel boyutu / dönüş
// =====================
inline constexpr int TFT_WIDTH  = 240;
inline constexpr int TFT_HEIGHT = 320;
inline constexpr uint8_t TFT_ROTATION = 1; // 0/1/2/3 denersin

// =====================
// Backlight polarity
// =====================
inline constexpr bool TFT_BL_ACTIVE_HIGH = true;
