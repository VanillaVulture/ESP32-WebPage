#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID = "Inland-ESP32-Toolkit";
const char* AP_PASS = "esp32darkmode";

// Optional: set these if you also want the board connected to your normal router.
const char* STA_SSID = "";
const char* STA_PASS = "";

WebServer server(80);

String securityToText(wifi_auth_mode_t auth) {
  switch (auth) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
    default: return "UNKNOWN";
  }
}

String uptimeStamp() {
  unsigned long s = millis() / 1000;
  unsigned long m = s / 60;
  unsigned long h = m / 60;
  char buf[32];
  snprintf(buf, sizeof(buf), "uptime-%02lu:%02lu:%02lu", h, m % 60, s % 60);
  return String(buf);
}

void handleRoot() {
  static const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width,initial-scale=1" />
<title>Inland ESP32 Core Wi-Fi Toolkit</title>
<style>
:root{color-scheme:dark;--bg:#0a0e14;--panel:#131a24;--soft:#1a2230;--txt:#ebeff7;--muted:#9ba8bf;--pri:#57d8ff;--ok:#66e09b;--warn:#ffb347;--bad:#ff5f7a}
*{box-sizing:border-box}body{margin:0;font-family:Inter,Segoe UI,Roboto,sans-serif;background:radial-gradient(circle at top right,#112033,var(--bg) 35%);color:var(--txt)}
.wrap{max-width:1000px;margin:0 auto;padding:16px;display:grid;gap:12px}.card{background:linear-gradient(180deg,var(--panel),var(--soft));border:1px solid #243042;border-radius:14px;padding:14px}
.row{display:flex;flex-wrap:wrap;gap:8px;align-items:center;justify-content:space-between}.btn{background:#223047;color:var(--txt);border:1px solid #31435f;padding:9px 12px;border-radius:8px;font-weight:600;cursor:pointer}
.btn:hover{border-color:var(--pri)}table{width:100%;border-collapse:collapse}th,td{text-align:left;padding:8px;border-bottom:1px solid #243042}.good{color:var(--ok)}.mid{color:var(--warn)}.bad{color:var(--bad)}
pre{margin:0;background:#05080d;border:1px solid #17202f;border-radius:8px;padding:10px;max-height:230px;overflow:auto;color:#9be5b3}
.badge{display:inline-block;border-radius:999px;padding:2px 8px;font-size:12px;font-weight:700}.high{background:#4a1f2b;color:#ffc7d1}.med{background:#4a3620;color:#ffe0b7}.low{background:#1f4030;color:#d4ffe8}
</style>
</head>
<body>
<main class="wrap">
<section class="card"><div class="row"><div><h2 style="margin:0">Inland ESP32 Core Wi-Fi Dashboard</h2><p style="margin:.4rem 0 0;color:var(--muted)">Real scan from ESP32, dark mode UI, deauth button remains safe stub.</p></div><button id="scanBtn" class="btn">Scan Nearby APs</button></div></section>
<section class="card"><div style="overflow:auto"><table><thead><tr><th>SSID</th><th>Signal</th><th>Channel</th><th>Security</th><th>Risk</th><th>Action</th></tr></thead><tbody id="apBody"></tbody></table></div></section>
<section class="card"><h3 style="margin-top:0">Device Log</h3><pre id="log">[boot] Ready.</pre></section>
</main>
<script>
const body = document.getElementById('apBody');
const logOut = document.getElementById('log');
const log = (m)=>{const t=new Date().toLocaleTimeString(); logOut.textContent=`[${t}] ${m}\n`+logOut.textContent;};
const risk=(s)=>{s=String(s).toUpperCase(); if(s==='OPEN')return ['High - No Password','high']; if(s==='WEP')return ['Medium - Deprecated','med']; return ['Low - Strong','low'];};
const sig=(r)=> r>=-55?'good':(r>=-72?'mid':'bad');
async function scan(){
  const btn=document.getElementById('scanBtn'); btn.disabled=true;
  try{
    const r = await fetch('/api/scan',{cache:'no-store'});
    if(!r.ok) throw new Error('HTTP '+r.status);
    const d = await r.json();
    body.innerHTML='';
    const aps=(d.aps||[]).sort((a,b)=>b.rssi-a.rssi);
    for(const ap of aps){
      const [label,cls]=risk(ap.security);
      const tr=document.createElement('tr');
      tr.innerHTML=`<td>${ap.ssid||'(hidden)'}</td><td class="${sig(ap.rssi)}">${ap.rssi} dBm</td><td>${ap.channel}</td><td>${ap.security}</td><td><span class="badge ${cls}">${label}</span></td><td><button class="btn deauth" data-ssid="${ap.ssid||'hidden'}">Deauth</button></td>`;
      body.appendChild(tr);
    }
    [...document.querySelectorAll('.deauth')].forEach((b)=>b.onclick=()=>log(`[SIMULATED] Deauth requested for ${b.dataset.ssid}. Stub only.`));
    log(`Scan complete. Found ${aps.length} AP(s).`);
  } catch(e){
    log('Scan failed: '+e.message);
  } finally { btn.disabled=false; }
}
document.getElementById('scanBtn').onclick=scan;
scan();
</script>
</body></html>
  )HTML";

  server.send_P(200, "text/html", PAGE);
}

void handleHealth() {
  String json = "{";
  json += "\"device\":\"Inland-ESP32-Core\",";
  json += "\"mode\":\"AP+STA\",";
  json += "\"apSsid\":\"" + String(AP_SSID) + "\",";
  json += "\"apIp\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"staConnected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  json += "\"staIp\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleScan() {
  int count = WiFi.scanNetworks(false, true);
  String json = "{";
  json += "\"generatedAt\":\"" + uptimeStamp() + "\",";
  json += "\"aps\":[";

  for (int i = 0; i < count; i++) {
    String ssid = WiFi.SSID(i);
    ssid.replace("\\", "\\\\");
    ssid.replace("\"", "\\\"");

    json += "{";
    json += "\"ssid\":\"" + ssid + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"channel\":" + String(WiFi.channel(i)) + ",";
    json += "\"security\":\"" + securityToText(WiFi.encryptionType(i)) + "\"";
    json += "}";

    if (i < count - 1) json += ",";
  }

  json += "]}";
  server.send(200, "application/json", json);
  WiFi.scanDelete();
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  bool apOk = WiFi.softAP(AP_SSID, AP_PASS);
  if (apOk) {
    Serial.print("SoftAP ready: ");
    Serial.print(AP_SSID);
    Serial.print(" @ ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("SoftAP start failed");
  }

  if (strlen(STA_SSID) > 0) {
    WiFi.begin(STA_SSID, STA_PASS);
    Serial.print("Connecting STA");
    for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
      delay(500);
      Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("STA connected @ ");
      Serial.println(WiFi.localIP());
    }
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/health", HTTP_GET, handleHealth);
  server.on("/api/scan", HTTP_GET, handleScan);
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}
