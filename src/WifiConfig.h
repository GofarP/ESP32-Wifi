#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <SPIFFS.h>

Preferences preferences;
WebServer server(80);
IPAddress ipAddr;

String ssid;
String password;
String apName = "Wifi ESP32";
String ip = "192.168.100.1";
String subnetmask = "255.255.255.0";
bool isConnected = false;
bool reconnecting = false;

String getNavbar(String navbarTitle)
{
    String html = R"rawliteral(
    <nav class="navbar navbar-expand-lg navbar-dark bg-dark mb-4">
      <div class="container-fluid">
        <a class="navbar-brand" href="/">NAVBAR_TITLE</a>
        <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav">
          <span class="navbar-toggler-icon"></span>
        </button>
        <div class="collapse navbar-collapse" id="navbarNav">
          <ul class="navbar-nav ms-auto">
            <li class="nav-item">
              <a class="nav-link active" aria-current="page" href="/">Home</a>
              <a class="nav-link active" aria-current="page" href="/attendance">Attendance</a>
              <a class="nav-link active" aria-current="page" href="/wifibackup">Wifi Backup</a>
            </li>
          </ul>
        </div>
      </div>
    </nav>
  )rawliteral";
    html.replace("NAVBAR_TITLE", navbarTitle);

    return html;
}

void handleLayoutMainPage()
{
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>WiFi Config</title>
      <link rel="stylesheet" href="/bootstrap.min.css">
      <style>
        body {
          background-color: #f8f9fa;
        }
      </style>
    </head>
    <body>
  )rawliteral";

    // Navbar penuh
    html += getNavbar("WiFi Config"); // sisipkan navbar di sini

    // Isi halaman utama
    html += R"rawliteral(
    <div class="container py-4">
      <div class="card shadow mx-auto" style="max-width: 500px;">
        <div class="card-body">
          <h4 class="card-title text-center mb-4">WiFi Configuration</h4>
          <p class="card-text text-center">AP_NAME</p>
          <form action="/save" method="POST">
            <div class="mb-3">
              <label for="ssid" class="form-label">SSID</label>
              <input type="text" class="form-control" id="ssid" name="ssid" value="REPLACE_SSID" required>
            </div>
            <div class="mb-3">
              <label for="password" class="form-label">Password</label>
              <input type="password" class="form-control" id="password" name="password" required>
            </div>
            <button type="submit" class="btn btn-success w-100">Save WiFi</button>
          </form>
        </div>
      </div>
    </div>
    <script src="/bootstrap.min.js"></script>
  </body>
  </html>
  )rawliteral";

    html.replace("REPLACE_SSID", ssid);
    html.replace("AP_NAME", apName);
    server.send(200, "text/html", html);
}

void handleSave()
{
    ssid = server.arg("ssid");
    password = server.arg("password");

    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();

    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <script src="/sweetalert.all.min.js"></script>
    </head>
    <body>
      <script>
        Swal.fire({
          icon: 'success',
          title: 'SSID dan password baru disimpan!',
          text: 'SSID dan password utama baru berhasil disimpan',
          confirmButtonText: 'OK'
        }).then(() => {
          window.location.href = "/";
        });
      </script>
    </body>
    </html>
  )rawliteral";

    server.send(200, "text/html", html);
    ESP.restart();
}

void setupWiFiAP()
{
    WiFi.softAP(apName, "", 1, true);
    ipAddr.fromString(ip);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(ipAddr, ipAddr, NMask);
    WiFi.softAP(apName, "", 1, false); // false = AP is visible (not hidden)
    Serial.println("AP Mode: ESP32_Config, IP: " + WiFi.softAPIP().toString());
}

bool connectToWiFi()
{
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    preferences.end();

    if (ssid == "")
    {
        Serial.println("SSID kosong");
        return false;
    }

    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Menghubungkan ke " + ssid);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nTersambung! IP: " + WiFi.localIP().toString());
        // singleBuzzPattern();
        // lcd.init();
        // lcd.backlight();
        // lcdShowIp(apName, WiFi.localIP().toString(), WiFi.macAddress());

        delay(1500);
        isConnected = true;
        return true;
    }

    // multipleBuzzPattern();
    // lcdShowFailedConnect(apName);
    delay(1000);
    Serial.println("\nKoneksi gagal.");
    delay(1500);
    isConnected = false;
    return false;
}

void setupWiFiManager()
{
    setupWiFiAP();
    connectToWiFi();
    server.on("/", HTTP_GET, handleLayoutMainPage);
    server.on("/save", HTTP_POST, handleSave);

    if (!SPIFFS.begin(true))
    {
        Serial.println("Gagal mounting SPIFFS");
        return;
    }
    server.serveStatic("/", SPIFFS, "/"); // Melayani file SPIFFS
    server.begin();
    Serial.println("Web server aktif di " + ip);
}

void handleWiFiManagerLoop()
{
  server.handleClient();
}


#endif