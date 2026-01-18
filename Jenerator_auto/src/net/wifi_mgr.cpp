#include "net/wifi_mgr.h"
#include <WiFi.h>
#include "ayarlar.h"
#include "sifre.h"

static uint32_t s_lastAttemptMs = 0;
static uint8_t  s_lastDiscReason = 0;

static bool isStaDisconnectedEvent(WiFiEvent_t event) {
#if defined(ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
  return (int)event == (int)ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
#elif defined(WIFI_EVENT_STA_DISCONNECTED)
  return (int)event == (int)WIFI_EVENT_STA_DISCONNECTED;
#elif defined(SYSTEM_EVENT_STA_DISCONNECTED)
  return (int)event == (int)SYSTEM_EVENT_STA_DISCONNECTED;
#else
  (void)event;
  return false;
#endif
}

static void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (isStaDisconnectedEvent(event)) {
    s_lastDiscReason = info.wifi_sta_disconnected.reason;
  }
}

static const char* reasonText(uint8_t r) {
  switch (r) {
    case 1:   return "UNSPECIFIED";
    case 15:  return "4WAY_TIMEOUT";
    case 201: return "NO_AP_FOUND";
    case 202: return "AUTH_FAIL";
    case 203: return "ASSOC_FAIL";
    case 204: return "HANDSHAKE_TIMEOUT";
    default:  return "OTHER";
  }
}

const char* wifiStatusText() {
  static char buf[96];
  const char* st = "UNKNOWN";
  switch (WiFi.status()) {
    case WL_CONNECTED:       st = "CONNECTED"; break;
    case WL_NO_SSID_AVAIL:   st = "NO_SSID"; break;
    case WL_CONNECT_FAILED:  st = "FAILED"; break;
    case WL_CONNECTION_LOST: st = "LOST"; break;
    case WL_DISCONNECTED:    st = "DISCONNECTED"; break;
    default: break;
  }
  snprintf(buf, sizeof(buf), "%s r=%u(%s) wl=%d", st, s_lastDiscReason, reasonText(s_lastDiscReason), (int)WiFi.status());
  return buf;
}

static void scanOnce() {
#if WIFI_DEBUG
  Serial.println("[WIFI] scan...");
  int n = WiFi.scanNetworks(false, true);
  if (n <= 0) {
    Serial.println("[WIFI] scan: none");
    return;
  }
  bool found = false;
  for (int i = 0; i < n; i++) {
    String ss = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);
    if (ss == WIFI_SSID) {
      found = true;
      Serial.print("[WIFI] FOUND SSID: "); Serial.print(ss);
      Serial.print(" RSSI="); Serial.println(rssi);
    }
  }
  if (!found) {
    Serial.println("[WIFI] target SSID NOT FOUND in scan");
  }
  WiFi.scanDelete();
#endif
}

bool wifiEnsureConnected() {
  if (WiFi.status() == WL_CONNECTED) return true;

  uint32_t now = millis();

  // ✅ İlk denemede backoff uygulama
  if (s_lastAttemptMs != 0 && (now - s_lastAttemptMs) < WIFI_RETRY_BACKOFF_MS) {
    return false;
  }
  s_lastAttemptMs = now;

  static bool inited = false;
  if (!inited) {
    inited = true;
    WiFi.onEvent(onWiFiEvent);
  }

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);

#if WIFI_DEBUG
  Serial.print("[WIFI] SSID len="); Serial.println(strlen(WIFI_SSID));
  Serial.print("[WIFI] PASS len="); Serial.println(strlen(WIFI_PASS));
  scanOnce();
#endif

  WiFi.disconnect(true, true);
  delay(150);

  Serial.println("[WIFI] begin()");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(100);
    yield();
#if WIFI_DEBUG
    Serial.print(".");
#endif
  }

#if WIFI_DEBUG
  Serial.println();
  Serial.print("[WIFI] status: "); Serial.println(wifiStatusText());
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WIFI] IP: "); Serial.println(WiFi.localIP());
    Serial.print("[WIFI] RSSI: "); Serial.println(WiFi.RSSI());
  }
#endif

  return (WiFi.status() == WL_CONNECTED);
}
