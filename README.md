# IoT Based Smart Irrigation System

This is a small college project I made in Hyderabad. A NodeMCU checks if the soil is dry and if it is raining, then turns a water pump on or off. I can also watch it and switch it by hand from the Blynk app.

The weather request is only for Hyderabad (`Hyderabad,IN`).

## Parts

- NodeMCU ESP8266
- Soil moisture sensor
- Rain sensor
- DHT11
- 5V relay module (active-low)
- Small water pump
- Jumper wires and a 5V supply

## Connections

| Part | Pin |
| --- | --- |
| Soil moisture | A0 |
| Rain sensor | D5 |
| DHT11 | D4 |
| Relay | D2 |

The relay is active-low. HIGH on D2 means the pump is off. LOW means the pump is on.

## What the code does

The file is `code/smart_irrigation.ino`.

`setup()` starts the serial monitor at 9600, keeps the pump off, starts the DHT11, and connects to Wi-Fi and Blynk.

`loop()` does two jobs:

1. Every 1 second it reads the sensors.
2. Every 30 seconds it asks OpenWeatherMap for the current weather in Hyderabad.

Soil moisture is an analog reading. The sketch maps it to a percent from 0 to 100. The rain pin is LOW when rain is there.

There are two modes, chosen from Blynk pin V5:

- Manual (`0`): the pump follows the button on V4. The sensors do not change it.
- Auto (`1`): if the rain sensor sees rain, the pump stays off. If the soil is under 40% and the Hyderabad weather is not `Rain`, the pump turns on. Otherwise it stays off.

Each second it also sends soil percent, temperature, humidity, and rain status to Blynk. The Hyderabad temperature and sky text go to Blynk when the weather call works.

## Blynk pins

| Pin | What it is |
| --- | --- |
| V0 | Soil moisture percent |
| V1 | DHT11 temperature |
| V2 | DHT11 humidity |
| V3 | Rain, 1 or 0 |
| V4 | Pump button in manual mode |
| V5 | Mode switch, 0 manual and 1 auto |
| V6 | Hyderabad temperature from the weather API |
| V7 | Hyderabad weather text, such as Clear or Rain |

## How to run it

Install these from the Arduino Library Manager:

- Blynk
- DHT sensor library
- Adafruit Unified Sensor
- ArduinoJson version 6

Add the ESP8266 board URL in Arduino preferences:

`http://arduino.esp8266.com/stable/package_esp8266com_index.json`

Then install the ESP8266 boards and pick NodeMCU 1.0 (ESP-12E Module).

Before upload, fill in your own Wi-Fi name, Wi-Fi password, Blynk template id, template name, auth token, and OpenWeatherMap key. Leave the city as `Hyderabad,IN`. Do not put real keys in a public GitHub repo.

Open the serial monitor at 9600. It prints the soil, temperature, humidity, rain, and pump state once a second.
