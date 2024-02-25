#include <WiFi.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer softAPServer(80);

void transitionToSTA(char* ssid, char* password) {
  softAPServer.end();
  otaInit(ssid, password);
}

void setupSoftAP() {
  Serial.println("Setting up SoftAP...");
  WiFi.softAP("SoftAP_SSID", "SoftAP_Password");

  softAPServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<form method='post' action='/set'>";
    html += "SSID: <input type='text' name='ssid'><br>";
    html += "Password: <input type='password' name='password'><br>";
    html += "<input type='submit' value='Submit'>";
    html += "</form>";
    request->send(200, "text/html", html);
  });

softAPServer.on("/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid = request->arg("ssid");
    String password = request->arg("password");
    
    char ssid_char[32];
    char password_char[64];

    ssid.toCharArray(ssid_char, 32);
    password.toCharArray(password_char, 64);

    EEPROM.put(0, ssid_char);
    EEPROM.put(100, password_char);
    EEPROM.commit();
    
    request->redirect("/redirect");
});


  softAPServer.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "You will be redirected...");

    char ssid[32];
    char password[64];
    EEPROM.get(0, ssid);
    EEPROM.get(100, password);
    transitionToSTA(ssid, password);
  });

  softAPServer.begin();
  Serial.println("SoftAP setup complete.");
}