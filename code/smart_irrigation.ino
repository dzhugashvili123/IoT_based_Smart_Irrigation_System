#define BLYNK_TEMPLATE_ID   "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

char wifiName[] = "YOUR_WIFI_SSID";
char wifiPassword[] = "YOUR_WIFI_PASSWORD";

const int soilPin = A0;
const int rainPin = D5;
const int relayPin = D2;
const int dhtPin = D4;

const int dryPercent = 40;
const unsigned long sensorEveryMs = 1000;
const unsigned long weatherEveryMs = 30000;

const char* weatherKey = "YOUR_OPENWEATHER_API_KEY";
const char* weatherCity = "Hyderabad,IN";

DHT dht(dhtPin, DHT11);

int pumpState = 0;
int mode = 0;
float outsideTemp = 0;
String outsideSky = "Clear";
unsigned long lastSensorAt = 0;
unsigned long lastWeatherAt = 0;

void startBoard();
void sampleAndControl();
void decidePump(int soilPercent, int raining);
void printSample(int soilPercent, float temp, float hum, int raining);
void pullHyderabadWeather();
int soilPercentFromAdc(int raw);
int rainFlag();

BLYNK_WRITE(V4) {
  pumpState = param.asInt();
}

BLYNK_WRITE(V5) {
  mode = param.asInt();
  Serial.print("Mode: ");
  Serial.println(mode == 1 ? "AUTO" : "MANUAL");
}

void setup() {
  startBoard();
}

void loop() {
  Blynk.run();

  if (millis() - lastSensorAt >= sensorEveryMs) {
    lastSensorAt = millis();
    sampleAndControl();
  }

  if (millis() - lastWeatherAt >= weatherEveryMs) {
    lastWeatherAt = millis();
    pullHyderabadWeather();
  }
}

void startBoard() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  pinMode(rainPin, INPUT);
  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, wifiName, wifiPassword);
  Serial.println("=== Smart Irrigation System Started ===");
}

int soilPercentFromAdc(int raw) {
  int percent = map(raw, 1024, 370, 0, 100);
  return constrain(percent, 0, 100);
}

int rainFlag() {
  return digitalRead(rainPin) == LOW ? 1 : 0;
}

void decidePump(int soilPercent, int raining) {
  if (mode != 1) {
    return;
  }
  if (raining == 1) {
    pumpState = 0;
    return;
  }
  if (soilPercent < dryPercent && outsideSky != "Rain") {
    pumpState = 1;
  } else {
    pumpState = 0;
  }
}

void sampleAndControl() {
  int soilPercent = soilPercentFromAdc(analogRead(soilPin));
  int raining = rainFlag();
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  decidePump(soilPercent, raining);
  digitalWrite(relayPin, pumpState ? LOW : HIGH);

  Blynk.virtualWrite(V0, soilPercent);
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, hum);
  Blynk.virtualWrite(V3, raining);
  printSample(soilPercent, temp, hum, raining);
}

void printSample(int soilPercent, float temp, float hum, int raining) {
  Serial.print("[SENSOR] Soil: ");
  Serial.print(soilPercent);
  Serial.print("% | Temp: ");
  Serial.print(temp);
  Serial.print("°C | Hum: ");
  Serial.print(hum);
  Serial.print("% | Rain: ");
  Serial.print(raining ? "YES" : "NO");
  Serial.print(" | Pump: ");
  Serial.println(pumpState ? "ON" : "OFF");
}

void pullHyderabadWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WEATHER] WiFi not connected, skipping.");
    return;
  }

  WiFiClient client;
  HTTPClient http;
  String url = String("http://api.openweathermap.org/data/2.5/weather?q=") +
               weatherCity + "&appid=" + weatherKey + "&units=metric";

  Serial.println("[WEATHER] Fetching data...");
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String body = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      outsideTemp = doc["main"]["temp"].as<float>();
      outsideSky = doc["weather"][0]["main"].as<String>();
      Blynk.virtualWrite(V6, outsideTemp);
      Blynk.virtualWrite(V7, outsideSky);
      Serial.print("[WEATHER] Temp: ");
      Serial.print(outsideTemp);
      Serial.print("°C | Condition: ");
      Serial.println(outsideSky);
    } else {
      Serial.println("[WEATHER] JSON parse error.");
    }
  } else {
    Serial.print("[WEATHER] HTTP Error: ");
    Serial.println(httpCode);
  }

  http.end();
}
