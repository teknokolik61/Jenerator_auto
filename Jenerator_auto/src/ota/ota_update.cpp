#include "ota/ota_update.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>

#include "ayarlar.h"
#include "version.h"
#include "net/wifi_mgr.h"
#include "sifre.h"

#if USE_LCD
  #include "ui/ui_lcd.h"
#endif

static uint32_t s_lastCheckMs = 0;

static void uiMsg(const char* a, const char* b) {
  Serial.print("[OTA] "); Serial.print(a);
  if (b && b[0]) { Serial.print(" | "); Serial.print(b); }
  Serial.println();
#if USE_LCD
  uiLcdShowMsg(a, b);
#endif
}

static void addAuthHeaders(HTTPClient& http) {
  http.addHeader("User-Agent", OTA_HTTP_USER_AGENT);
  if (GITHUB_TOKEN && GITHUB_TOKEN[0]) {
    http.addHeader("Authorization", String("token ") + GITHUB_TOKEN);
  }
  http.addHeader("Accept", "application/vnd.github+json");
}

static void netDiagOnce() {
  static bool done = false;
  if (done) return;
  done = true;

  Serial.println("[OTA] === NET DIAG ===");
  Serial.print("[OTA] WiFi: "); Serial.println(wifiStatusText());
  Serial.print("[OTA] SSID: "); Serial.println(WiFi.SSID());
  Serial.print("[OTA] IP  : "); Serial.println(WiFi.localIP());
  Serial.print("[OTA] GW  : "); Serial.println(WiFi.gatewayIP());
  Serial.print("[OTA] DNS : "); Serial.println(WiFi.dnsIP());
  Serial.print("[OTA] RSSI: "); Serial.println(WiFi.RSSI());

  IPAddress ip;
  bool dnsOk = WiFi.hostByName("api.github.com", ip);
  Serial.print("[OTA] DNS api.github.com: ");
  if (dnsOk) Serial.println(ip);
  else       Serial.println("FAIL");

  WiFiClientSecure c;
  c.setInsecure();
  c.setTimeout(8000);
  Serial.print("[OTA] TCP 443 connect: ");
  if (c.connect("api.github.com", 443)) {
    Serial.println("OK");
    c.stop();
  } else {
    Serial.println("FAIL");
  }
  Serial.println("[OTA] ================");
}

static bool parseVer(const char* s, int& major, int& minor) {
  major = 0; minor = 0;
  if (!s || !s[0]) return false;
  int i = 0;
  if (s[i] == 'v' || s[i] == 'V') i++;
  bool any = false;
  while (s[i] >= '0' && s[i] <= '9') { major = major * 10 + (s[i]-'0'); i++; any = true; }
  if (!any) return false;
  if (s[i] == '.') {
    i++;
    while (s[i] >= '0' && s[i] <= '9') { minor = minor * 10 + (s[i]-'0'); i++; }
  }
  return true;
}

static int cmpVer(const char* a, const char* b) {
  int am=0, an=0, bm=0, bn=0;
  if (!parseVer(a, am, an)) return -1;
  if (!parseVer(b, bm, bn)) return  1;
  if (am != bm) return (am > bm) ? 1 : -1;
  if (an != bn) return (an > bn) ? 1 : -1;
  return 0;
}

static bool endsWithBin(const char* name) {
  if (!name) return false;
  size_t n = strlen(name);
  return (n >= 4 && name[n-4]=='.' && name[n-3]=='b' && name[n-2]=='i' && name[n-1]=='n');
}

static bool pickBinFromRelease(JsonObject rel, String& outTag, String& outUrl) {
  const char* tag = rel["tag_name"] | "";
  if (!tag[0]) return false;
  outTag = tag;

  JsonArray assets = rel["assets"].as<JsonArray>();
  String bestUrl = "";
  for (JsonObject a : assets) {
    const char* name = a["name"] | "";
    const char* url  = a["browser_download_url"] | "";
    if (!name[0] || !url[0]) continue;
    if (!endsWithBin(name)) continue;

    if (OTA_ASSET_NAME_HINT && OTA_ASSET_NAME_HINT[0] && strcmp(name, OTA_ASSET_NAME_HINT) == 0) {
      bestUrl = url;
      break;
    }
    if (bestUrl.isEmpty()) bestUrl = url;
  }
  if (bestUrl.isEmpty()) return false;
  outUrl = bestUrl;
  return true;
}

static bool fetchLatest(String& outTag, String& outUrl) {
  outTag = "";
  outUrl = "";

  String api = "https://api.github.com/repos/";
  api += OTA_GH_OWNER; api += "/"; api += OTA_GH_REPO;
  String urlLatest = api + "/releases/latest";

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(15000);

  if (!http.begin(client, urlLatest)) {
    uiMsg("API begin failed", "");
    return false;
  }
  addAuthHeaders(http);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    // -1 gibi negatiflerde “sebep” yazdıralım
    String err = http.errorToString(code);
    uiMsg("API HTTP error", String(code).c_str());
    uiMsg("API error str", err.c_str());
    uiMsg("URL", urlLatest.c_str());
    http.end();
    return false;
  }

  DynamicJsonDocument doc(24576);
  auto err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) {
    uiMsg("JSON parse error", err.c_str());
    return false;
  }

  JsonObject rel = doc.as<JsonObject>();
  if (!pickBinFromRelease(rel, outTag, outUrl)) {
    uiMsg("Release .bin yok", "asset yok/adi farkli");
    return false;
  }
  return true;
}

static bool downloadAndFlash(const char* url) {
  uiMsg("Update indiriliyor", url);

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(20000);

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(20000);

  if (!http.begin(client, url)) {
    uiMsg("HTTP begin failed", "");
    return false;
  }
  addAuthHeaders(http);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    uiMsg("Download HTTP error", String(code).c_str());
    uiMsg("Download err str", http.errorToString(code).c_str());
    http.end();
    return false;
  }

  int len = http.getSize();
  if (!Update.begin(len > 0 ? (size_t)len : UPDATE_SIZE_UNKNOWN)) {
    uiMsg("Update.begin fail", "");
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  Update.writeStream(*stream);

  if (!Update.end()) {
    uiMsg("Update.end fail", Update.errorString());
    http.end();
    return false;
  }
  http.end();

  if (!Update.isFinished()) {
    uiMsg("Update not finished", "");
    return false;
  }

  uiMsg("Update OK", "Restart...");
  delay(500);
  ESP.restart();
  return true;
}

void otaInit() { s_lastCheckMs = 0; }

bool otaCheckNow(bool force) {
  if (!USE_OTA) return false;

  if (!force) {
    uint32_t now = millis();
    if (s_lastCheckMs && (now - s_lastCheckMs) < OTA_CHECK_INTERVAL_MS) return false;
  }
  s_lastCheckMs = millis();

  if (!wifiEnsureConnected()) {
    uiMsg("WiFi yok", wifiStatusText());
    return false;
  }

  // ✅ Sadece bir kere ağ teşhisi basalım
  netDiagOnce();

  uiMsg("Release kontrol", "");

  String tag, url;
  if (!fetchLatest(tag, url)) {
    uiMsg("Latest okunamadi", "");
    return false;
  }

  if (cmpVer(tag.c_str(), FW_VERSION) <= 0) {
    uiMsg("Guncel", tag.c_str());
    return true;
  }

  uiMsg("Yeni surum bulundu", tag.c_str());
  return downloadAndFlash(url.c_str());
}

void otaLoop() {
  if (!USE_OTA) return;
  otaCheckNow(false);
}
