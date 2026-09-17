#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

namespace {
constexpr char kLabSsid[] = "ESP8266-Security-Lab";
constexpr char kLabPassword[] = "securitylab";
constexpr unsigned long kScanIntervalMs = 15000;

ESP8266WebServer server(80);
unsigned long lastScanAt = 0;
int networkCount = 0;

String jsonEscape(const String &value) {
  String out;
  out.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value[i];
    if (c == '\\' || c == '"') out += '\\';
    if (c == '\n') out += "\\n";
    else if (c != '\r') out += c;
  }
  return out;
}

const char *encryptionLabel(uint8_t type) {
  switch (type) {
    case ENC_TYPE_NONE: return "OPEN";
    case ENC_TYPE_WEP: return "WEP";
    case ENC_TYPE_TKIP: return "WPA/TKIP";
    case ENC_TYPE_CCMP: return "WPA2/CCMP";
    case ENC_TYPE_AUTO: return "WPA/WPA2";
    default: return "UNKNOWN";
  }
}

void scanNetworks() {
  WiFi.scanDelete();
  networkCount = WiFi.scanNetworks(false, true);
  lastScanAt = millis();
  Serial.printf("[scan] %d networks found\n", networkCount);
}

String buildNetworksJson() {
  String json = "{\"count\":" + String(networkCount) + ",\"networks\":[";
  for (int i = 0; i < networkCount; ++i) {
    if (i) json += ',';
    json += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\"";
    json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
    json += ",\"channel\":" + String(WiFi.channel(i));
    json += ",\"rssi\":" + String(WiFi.RSSI(i));
    json += ",\"security\":\"" + String(encryptionLabel(WiFi.encryptionType(i))) + "\"}";
  }
  json += "]}";
  return json;
}

String dashboardHtml() {
  return F(R"HTML(
<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP8266 Wireless Security Lab</title>
<style>
:root{--bg:#08111e;--card:#101d2d;--line:#22364b;--text:#e8f0f7;--muted:#8ea2b7;--blue:#3b82f6;--green:#22c55e;--amber:#f59e0b}
*{box-sizing:border-box}body{margin:0;background:linear-gradient(160deg,#07101b,#0d1a2a);color:var(--text);font-family:Inter,system-ui,sans-serif}.wrap{max-width:980px;margin:auto;padding:28px 18px 60px}.hero,.panel{background:rgba(16,29,45,.94);border:1px solid var(--line);border-radius:18px}.hero{padding:24px;margin-bottom:16px}.hero h1{margin:0 0 8px;font-size:clamp(1.6rem,4vw,2.4rem)}.hero p{margin:0;color:var(--muted);line-height:1.6}.toolbar{display:flex;gap:10px;flex-wrap:wrap;margin-top:18px}.btn{border:0;border-radius:10px;padding:11px 15px;font-weight:800;cursor:pointer;background:var(--blue);color:white}.status{padding:8px 11px;border:1px solid var(--line);border-radius:999px;color:var(--muted);font-size:.82rem}.panel{overflow:hidden}.panel-head{padding:16px 18px;border-bottom:1px solid var(--line);display:flex;justify-content:space-between;gap:12px;align-items:center}.panel-head h2{font-size:1rem;margin:0}.table-wrap{overflow:auto}table{width:100%;border-collapse:collapse;min-width:720px}th,td{padding:12px 14px;border-bottom:1px solid var(--line);text-align:left;font-size:.82rem}th{color:#a9bad0;background:#0d1826}.open{color:var(--amber);font-weight:800}.secure{color:var(--green);font-weight:800}.empty{padding:30px;color:var(--muted);text-align:center}.note{margin-top:16px;color:var(--muted);font-size:.8rem;line-height:1.6}.dot{display:inline-block;width:8px;height:8px;background:var(--green);border-radius:50%;margin-right:6px}
</style></head><body><main class="wrap"><section class="hero"><h1>ESP8266 Wireless Security Lab</h1><p>Passive nearby-network visibility for authorized labs. This firmware scans Wi-Fi metadata only; it does not disconnect clients, impersonate access points, or collect credentials.</p><div class="toolbar"><button class="btn" onclick="scanNow()">Scan now</button><span class="status"><span class="dot"></span>Monitor online</span><span class="status" id="summary">Loading…</span></div></section><section class="panel"><div class="panel-head"><h2>Nearby 2.4 GHz Networks</h2><span id="updated" class="status">Waiting</span></div><div class="table-wrap"><table><thead><tr><th>SSID</th><th>BSSID</th><th>Channel</th><th>RSSI</th><th>Security</th></tr></thead><tbody id="rows"><tr><td colspan="5" class="empty">Scanning…</td></tr></tbody></table></div></section><p class="note">Use only on networks and radio environments where you are authorized to perform security observation. Open networks are highlighted as a configuration risk.</p></main><script>
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
async function load(){const r=await fetch('/api/networks');const d=await r.json();document.querySelector('#summary').textContent=d.count+' networks';document.querySelector('#updated').textContent='Updated '+new Date().toLocaleTimeString();document.querySelector('#rows').innerHTML=d.networks.length?d.networks.map(n=>`<tr><td>${esc(n.ssid||'(hidden)')}</td><td>${esc(n.bssid)}</td><td>${n.channel}</td><td>${n.rssi} dBm</td><td class="${n.security==='OPEN'?'open':'secure'}">${esc(n.security)}</td></tr>`).join(''):'<tr><td colspan="5" class="empty">No networks found.</td></tr>'}
async function scanNow(){document.querySelector('#updated').textContent='Scanning…';await fetch('/scan',{method:'POST'});await load()}load();setInterval(load,10000);
</script></body></html>)HTML");
}

void handleRoot() { server.send(200, "text/html", dashboardHtml()); }
void handleNetworks() { server.send(200, "application/json", buildNetworksJson()); }
void handleStatus() {
  String json = "{\"device\":\"ESP8266\",\"firmware\":\"1.0.0\",\"uptime_ms\":" + String(millis()) + ",\"free_heap\":" + String(ESP.getFreeHeap()) + "}";
  server.send(200, "application/json", json);
}
void handleScan() { scanNetworks(); server.send(202, "application/json", "{\"status\":\"scan_complete\"}"); }
}

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("\nESP8266 Wireless Security Lab v1.0.0");

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  WiFi.softAP(kLabSsid, kLabPassword);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/networks", HTTP_GET, handleNetworks);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/scan", HTTP_POST, handleScan);
  server.onNotFound([]() { server.sendHeader("Location", "/", true); server.send(302, "text/plain", ""); });
  server.begin();

  scanNetworks();
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
  if (millis() - lastScanAt >= kScanIntervalMs) scanNetworks();
  delay(2);
}
