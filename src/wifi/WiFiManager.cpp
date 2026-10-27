#include "WiFiManager.h"

static Preferences prefs;
static WiFiServer server(80);
static bool captivePortalActive = false;

static String urlDecode(const String &str) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = str.length();
  for (unsigned int i = 0; i < len; i++) {
    char c = str.charAt(i);
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < len) {
      temp[2] = str.charAt(++i);
      temp[3] = str.charAt(++i);
      decoded += (char)strtol(temp, nullptr, 16);
    } else {
      decoded += c;
    }
  }
  return decoded;
}

static String getQueryParam(const String &data, const char *key) {
  String k = String(key) + "=";
  int idx = data.indexOf(k);
  while (idx != -1) {
    if (idx == 0 || data.charAt(idx - 1) == '?' || data.charAt(idx - 1) == '&') {
      int endIdx = data.indexOf('&', idx + k.length());
      if (endIdx == -1) endIdx = data.indexOf(' ', idx + k.length());
      if (endIdx == -1) endIdx = data.length();
      return urlDecode(data.substring(idx + k.length(), endIdx));
    }
    idx = data.indexOf(k, idx + 1);
  }
  return "";
}

bool WiFiManager::hasCredentials() {
  prefs.begin("wifi_config", true);
  String ssid = prefs.getString("ssid", "");
  prefs.end();
  return ssid.length() > 0;
}

void WiFiManager::saveCredentials(const String &ssid, const String &password) {
  prefs.begin("wifi_config", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", password);
  prefs.end();
  Serial.println("[WIFI] Credentials safely saved to encrypted NVS storage.");
}

void WiFiManager::clearCredentials() {
  prefs.begin("wifi_config", false);
  prefs.clear();
  prefs.end();
}

bool WiFiManager::connect(uint32_t timeoutMs) {
  return connectStoredWiFi();
}

bool WiFiManager::connectStoredWiFi() {
  prefs.begin("wifi_config", true);
  String ssid = prefs.getString("ssid", "");
  String password = prefs.getString("pass", "");
  prefs.end();

  if (ssid.length() == 0) {
    Serial.println("[WIFI] No stored credentials found in NVS.");
    return false;
  }

  Serial.printf("[WIFI] Connecting to '%s'...\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  // Giảm công suất phát từ 19.5dBm xuống 15dBm để cắt giảm đỉnh dòng LDO ME6211, chống sụt áp brownout
  WiFi.setTxPower(WIFI_TX_POWER_SAFE);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_CONNECT_TIMEOUT_MS) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WIFI] Connected! IP: %s, RSSI: %d dBm (Took %lu ms)\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI(), millis() - startAttempt);
    // Sync SNTP clock with pool.ntp.org (UTC+7 Vietnam)
    configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
    return true;
  }

  Serial.println("[WIFI] Connection timed out!");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  return false;
}

void WiFiManager::startCaptivePortal(const String &apName) {
  WiFi.mode(WIFI_AP_STA);

  // Nếu có Wi-Fi đã lưu thì thử kết nối STA để có thêm địa chỉ IP LAN (192.168.1.x)
  prefs.begin("wifi_config", true);
  String savedSsid = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end();

  if (savedSsid.length() > 0 && WiFi.status() != WL_CONNECTED) {
    WiFi.setTxPower(WIFI_TX_POWER_SAFE);
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 3500) {
      delay(100);
    }
  }

  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName.c_str(), "12345678");

  server.begin();
  captivePortalActive = true;

  Serial.println("\n================================================================");
  Serial.println("🎯 [CẤU HÌNH] ĐÃ GIỮ NÚT BOOT 5 GIÂY -> XUẤT ĐỊA CHỈ CẤU HÌNH:");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("🌐 1. ĐỊA CHỈ LAN: http://%s\n", WiFi.localIP().toString().c_str());
  }
  Serial.printf("🌐 2. ĐỊA CHỈ AP : http://192.168.4.1 (Wi-Fi: %s / Pass: 12345678)\n", apName.c_str());
  Serial.println("🔵 3. BLUETOOTH  : SOB-BTN-8829-WTR");
  Serial.println("================================================================\n");
}

void WiFiManager::handleCaptivePortal() {
  if (!captivePortalActive) return;

  WiFiClient client = server.available();
  if (!client) return;

  unsigned long t0 = millis();
  while (!client.available() && millis() - t0 < 400) delay(2);
  if (!client.available()) {
    client.stop();
    return;
  }

  String reqLine = client.readStringUntil('\n');
  reqLine.trim();
  int contentLen = 0;
  while (client.connected()) {
    String h = client.readStringUntil('\n');
    h.trim();
    if (h.length() == 0) break;
    if (h.startsWith("Content-Length:") || h.startsWith("content-length:")) {
      contentLen = h.substring(15).toInt();
    }
  }

  String body = "";
  if (contentLen > 0 && contentLen < 512) {
    unsigned long bt = millis();
    while ((int)body.length() < contentLen && millis() - bt < 400) {
      if (client.available()) body += (char)client.read();
    }
  }

  if (reqLine.indexOf("/save") != -1) {
    String querySource = (body.length() > 0) ? body : reqLine;
    String ssid = getQueryParam(querySource, "s");
    String pass = getQueryParam(querySource, "p");
    if (ssid.length() > 0) {
      saveCredentials(ssid, pass);
      client.print(
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'></head>"
        "<body style='background:#0f172a;color:#10b981;text-align:center;padding:40px;font-family:sans-serif'>"
        "<h2>🎉 Đã lưu Wi-Fi thành công!</h2><p style='color:#cbd5e1'>Thiết bị đang khởi động lại và kết nối vào <b>" + ssid + "</b>...</p></body></html>"
      );
      client.stop();
      delay(1200);
      ESP.restart();
      return;
    }
  }

  String lanIp = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "Chưa kết nối";
  String curSsid = (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : "Chưa có";
  client.print(
    "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Cấu Hình Wi-Fi Smart Order Button</title><style>"
    "body{font-family:sans-serif;background:#0f172a;color:#f8fafc;display:flex;justify-content:center;align-items:center;min-height:90vh;margin:0;padding:16px}"
    ".c{background:#1e293b;border:1px solid #334155;padding:22px;border-radius:14px;width:100%;max-width:360px}"
    "h2{margin:0 0 10px;color:#38bdf8;font-size:19px;text-align:center}"
    ".i{background:#0f172a;border-radius:8px;padding:10px;font-size:12px;color:#cbd5e1;margin-bottom:14px;line-height:1.5}"
    "input{width:100%;padding:10px;margin:4px 0 12px;border-radius:8px;border:1px solid #475569;background:#0f172a;color:#fff;box-sizing:border-box}"
    "button{width:100%;background:#0284c7;color:#fff;padding:12px;border:none;border-radius:8px;font-weight:bold;cursor:pointer}"
    "</style></head><body><div class='c'><h2>⚙️ Cấu Hình Wi-Fi Nút Bấm</h2>"
    "<div class='i'>• Wi-Fi: <b>" + curSsid + "</b><br>• IP LAN: <b>" + lanIp + "</b><br>• IP AP: <b>192.168.4.1</b></div>"
    "<form action='/save' method='GET'>"
    "Tên Wi-Fi (SSID):<input name='s' required placeholder='Nhập tên Wi-Fi...'>"
    "Mật khẩu Wi-Fi:<input type='password' name='p' placeholder='Nhập mật khẩu Wi-Fi...'>"
    "<button type='submit'>💾 LƯU CẤU HÌNH & KẾT NỐI</button></form></div></body></html>"
  );
  client.stop();
}

void WiFiManager::stopWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  captivePortalActive = false;
}
