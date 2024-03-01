#include "Identification.h"
#include "softAP.h"
#include "LCDHandler.h"
#include "BME280Handler.h"
#include "SoilMoistureHandler.h"
#include <MHZ19.h>

/* ESP32 Dependencies */
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <HTTPClient.h>

#define PWM_PIN 18          // pwm HEATER
#define CONTROL_PIN 19      // pwm FAN
#define CONTROL_DELAY 60000 // Delay time in milliseconds (1 minute)

#define SDA_PIN 21
#define SCL_PIN 22

#define BUTTON_PIN 2
#define SOIL_MOISTURE_SENSOR_PIN_1 34

#define START_PWM 0
#define END_PWM 255
#define PWM_RAMP_DURATION 60000

#define MHZ19C_RX_PIN 16 // Connect to TX of MH-Z19C
#define MHZ19C_TX_PIN 17 // Connect to RX of MH-Z19C

MHZ19 myMHZ19;              // Create an instance of the MHZ19 class
HardwareSerial mySerial(2); // Use the second hardware serial port for MHZ19

AsyncWebSocket ws("/ws");

JSONVar readings;

unsigned long startTime = 0;
bool rampingPWM = false;

bool controlActive = false;
unsigned long controlStartTime;

SoilMoistureHandler soilSensor1(SOIL_MOISTURE_SENSOR_PIN_1);
int dryValue = 2500;
int wetValue = 700;

int currentScreen = 0;

bool isBmeScreen = true;

BME280Handler bmeHandler;
LCDHandler lcdHandler;

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void clearEEPROM(int start, int end)
{
  for (int i = start; i < end; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

// Get Sensor Readings and return JSON object
String getSensorReadings()
{
  readings["temperature"] = String(bmeHandler.readTemperature());
  readings["humidity"] = String(bmeHandler.readHumidity());
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void sendDataToServer(float temperature, float humidity)
{
  Serial.println("Sending data to server...");
  HTTPClient http;
  http.begin("http://192.168.1.168:5000/api/readings"); // Replace with your Flask server IP and port
  http.addHeader("Content-Type", "application/json");

  JSONVar readings;
  readings["temperature"] = temperature;
  readings["humidity"] = humidity;
  String requestBody = JSON.stringify(readings);

  int httpResponseCode = http.POST(requestBody);
  if (httpResponseCode > 0)
  {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  }
  else
  {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
  {
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());

    // Get the latest sensor readings
    String sensorData = getSensorReadings();

    // Send the latest sensor data to the newly connected client
    client->text(sensorData);
  }
  case WS_EVT_DISCONNECT:
  {
    Serial.println("WS event disconnect");
  }
  case WS_EVT_DATA:
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      if (info->opcode == WS_TEXT)
      {
        data[len] = 0;
        char *command = (char *)data;
      }
    }
  }
  case WS_EVT_PONG:
  {
  }

  case WS_EVT_ERROR:
  {
    Serial.println("WS event error");
  }
  }
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(201, "text/plain", "Not found bitch");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool isValidCredentials(const char *ssid, const char *password)
{
  Serial.print("Stored SSID: ");
  Serial.println(ssid);
  Serial.print("Stored Password: ");
  Serial.println(password);
  // Check the validity of each character in the SSID and password
  for (int i = 0; i < strlen(ssid); i++)
  {
    if (ssid[i] < 32 || ssid[i] > 126)
      return false;
  }
  for (int i = 0; i < strlen(password); i++)
  {
    if (password[i] < 32 || password[i] > 126)
      return false;
  }

  // Check the length of SSID and password
  return (strlen(ssid) && strlen(password));
}

void setup()
{
  Serial.begin(115200);
  lcdHandler.init();
  Serial.println("LCD Initialized");

  EEPROM.begin(512);

  // clearEEPROM(0, 200);

  char ssid[32] = {0};
  char password[64] = {0};

  EEPROM.get(0, ssid);
  EEPROM.get(100, password);

  soilSensor1.begin();

  // Initialize MH-Z19 sensor
  mySerial.begin(9600, SERIAL_8N1, MHZ19C_RX_PIN, MHZ19C_TX_PIN);
  myMHZ19.begin(mySerial); // Initialize the MHZ19 object

  if (isValidCredentials(ssid, password))
  {
    Serial.println("Valid credentials found. Initializing OTA...");
    otaInit(ssid, password);
  }
  else
  {
    Serial.println("No valid credentials found. Setting up SoftAP...");
    setupSoftAP();
  }

  if (!bmeHandler.begin())
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    lcdHandler.print("BME280 not found!", 0);
  }

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.onNotFound(notFound);

  // Start server
  server.begin();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
int readMoisturePercentage()
{
  int rawValue = soilSensor1.readMoisture();
  int moisturePercentage = map(rawValue, dryValue, wetValue, 0, 100);
  moisturePercentage = constrain(moisturePercentage, 0, 100); // Ensure within bounds
  return moisturePercentage;

  ws.cleanupClients();
}

unsigned long lastSendTime = 0;
const long sendInterval = 5000; // Send data every 5 seconds

unsigned long previousMillis = 0;
const long interval = 60000; // Interval at which to send data (1 minute)

void broadcastSensorReadings()
{
  String sensorData = getSensorReadings(); // Use your existing function
  ws.textAll(sensorData);                  // Send to all connected WebSocket clients
}

void loop()
{
  unsigned long currentMillis = millis();

  AsyncElegantOTA.loop();

  float temperature = bmeHandler.readTemperature();
  float humidity = bmeHandler.readHumidity();

  String tempMessage = "Temp: " + String(temperature) + "c";
  String humidityMessage = "Humidity: " + String(humidity) + "%";
  lcdHandler.print(tempMessage, 0);
  lcdHandler.print(humidityMessage, 1);

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    // Replace these with actual readings from your sensors
    float temperature = bmeHandler.readTemperature();
    float humidity = bmeHandler.readHumidity();

    sendDataToServer(temperature, humidity);
  }

  // If button is pressed
  if (digitalRead(BUTTON_PIN) == HIGH)
  {
    lcdHandler.clear();
    currentScreen = (currentScreen + 1) % 3;
    delay(300);
  }

  if (currentScreen == 0)
  {
    // BME280 screen
    lcdHandler.clear();
    lcdHandler.print(tempMessage, 0);
    lcdHandler.print(humidityMessage, 1);
  }
  else if (currentScreen == 1)
  {
    // Soil Moisture screen
    int moisturePercentage = readMoisturePercentage();
    lcdHandler.clear();
    String displayStr = "Soil Moist: " + String(moisturePercentage) + "%";
    lcdHandler.print(displayStr, 0);
  }
  else if (currentScreen == 2)
  {
    // CO2 screen
    int co2Concentration = myMHZ19.getCO2(); // Read CO2 level using MHZ19 library
    String co2Message = "CO2: " + String(co2Concentration) + " ppm";
    lcdHandler.clear();
    lcdHandler.print(co2Message, 0);
  }

  if (temperature > 27.0 && !controlActive)
  {
    controlStartTime = millis(); // Record the time when temp first goes above 27°C
    controlActive = true;
  }

  if (controlActive)
  {
    if (millis() - controlStartTime < CONTROL_DELAY)
    {
      digitalWrite(CONTROL_PIN, HIGH); // Keep the CONTROL_PIN HIGH during the delay
    }
    else
    {
      digitalWrite(CONTROL_PIN, LOW); // Turn off CONTROL_PIN after the delay
      controlActive = false;          // Reset the flag
    }
  }
  else
  {
    digitalWrite(CONTROL_PIN, LOW); // Default state when temp is <= 27 and not in delay period
  }

  if (temperature < 25.0 && !rampingPWM)
  {
    analogWrite(PWM_PIN, START_PWM); // Start the PWM at its initial value
    startTime = millis();            // Record the start time
    rampingPWM = true;               // Set the flag to indicate we're ramping
  }
  else if (temperature >= 26.0)
  { // If temperature goes above 27°C, turn off PWM
    analogWrite(PWM_PIN, 0);
    rampingPWM = false; // Ensure we stop any ongoing ramping
  }

  if (rampingPWM)
  {
    unsigned long elapsedTime = millis() - startTime;
    if (elapsedTime < PWM_RAMP_DURATION)
    {
      int pwmValue = map(elapsedTime, 0, PWM_RAMP_DURATION, START_PWM, END_PWM);
      analogWrite(PWM_PIN, pwmValue);
    }
    else
    {
      analogWrite(PWM_PIN, END_PWM); // Set to the final value
      rampingPWM = false;            // Stop ramping
    }
  }

  // Non-blocking timer to send sensor readings
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval)
  {
    lastSendTime = currentTime;
    // Get sensor readings and broadcast
    broadcastSensorReadings();
  }

  // Serial.println(getSensorReadings());

  delay(30);
}
