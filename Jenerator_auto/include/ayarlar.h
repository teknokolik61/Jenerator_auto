#pragma once
#include <stdint.h>

// =====================
// FEATURES (aç/kapat)
// =====================
#define USE_LCD     1
#define USE_TG      0
#define USE_OTA     1

// =====================
// WiFi davranış ayarları
// =====================
inline constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000;  // 20sn
inline constexpr uint32_t WIFI_RETRY_BACKOFF_MS   = 30000;  // 30sn

// Yeni: WiFi teşhis logu (Serial’e detay basar)
#define WIFI_DEBUG 1

// =====================
// LCD Driver seçimi (MUTLAKA #define)
// =====================
#define UI_DRV_ST7789   0
#define UI_DRV_ILI9341  1

// Panel boyutu / dönüş
inline constexpr int TFT_WIDTH  = 240;
inline constexpr int TFT_HEIGHT = 320;
inline constexpr uint8_t TFT_ROTATION = 1; // 0/1/2/3 denersin

// Backlight polarity
inline constexpr bool TFT_BL_ACTIVE_HIGH = true;

// =====================
// OTA (GitHub Releases) Ayarları
// =====================
inline constexpr bool     OTA_CHECK_ON_BOOT     = true;
inline constexpr uint32_t OTA_CHECK_INTERVAL_MS = 6UL * 60UL * 60UL * 1000UL; // 6 saat

// GitHub repo
inline constexpr const char* OTA_GH_OWNER = "teknokolik61";
inline constexpr const char* OTA_GH_REPO  = "Jenerator_auto";

// Asset seçimi: boş bırakırsan ilk .bin’i bulur
inline constexpr const char* OTA_ASSET_NAME_HINT = "firmware.bin";

// GitHub API bazı durumlarda User-Agent ister
inline constexpr const char* OTA_HTTP_USER_AGENT = "JeneratorAuto-ESP32S3";

// OTA döngü koruması: aynı tag update edildi sanılıp boot sonrası açılmazsa tekrar deneme
#define OTA_BLOCK_REPEAT_SAME_TAG 1