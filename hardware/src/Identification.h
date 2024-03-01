// Allow OTA software updates
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <EEPROM.h>


const String projectTitle = "Title";
const String projectDesc = "Desc";


AsyncWebServer server(80);

void otaInit(const char *ssid, const char *password) {

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);

  WiFi.disconnect();
  delay(100);

  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  unsigned long wifiTimeout = 10000;  // 10 seconds

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiTimeout) {
    delay(200);
    Serial.print(".");
    if (millis() - startTime >= wifiTimeout && WiFi.status() != WL_CONNECTED) {
      Serial.println("");
      Serial.print("Failed to connect. WiFi status: ");
      Serial.println(WiFi.status());
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }

  AsyncElegantOTA.begin(&server);
  server.begin();
}