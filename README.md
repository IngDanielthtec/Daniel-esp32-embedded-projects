# ESP32 Embedded Projects

Personal learning projects in Arduino C++ for the ESP32 (Heltec board):
sensor reading, display output, DC motor control, and wireless/cloud IoT
(ESP-NOW, MQTT, HTTP, Arduino IoT Cloud).
Not affiliated with any company or client.

## Sensors and displays

| Folder | Description | Hardware |
|---|---|---|
| LecturaSensorHumedadSuelo | Reads soil moisture via ADC, scaled to 0-100 % | Analog soil moisture sensor |
| LecturaSensorTemperatura | Converts sensor voltage to °C | MCP9700A |
| proyecto_max6675 | Reads a thermocouple over SPI, prints °C and °F | MAX6675 + K-type thermocouple |
| dht22_y_lcd2x16 | Shows temperature and humidity on an I2C LCD | DHT22 + LCD 2x16 |

## Motor control

| Folder | Description | Hardware |
|---|---|---|
| DriverL298N | Controls two DC motors with PWM speed steps | L298N driver |
| ControlMotores_ConUART | Controls direction and speed with serial commands (A, T, S, P) | L298N driver |

## Wireless and cloud IoT

| Folder | Description | Protocol / Service |
|---|---|---|
| nodo_sensor1, nodo_sensor2 | Sensor nodes sending DHT22 data to a central device | ESP-NOW |
| dispositivo_central | Receives and validates data from both nodes | ESP-NOW |
| esp32_broker_emqx | Publishes DHT22 data as JSON to a public test broker | MQTT (EMQX) |
| esp32_http_cliente | HTTPS POST of sensor data and GET of a dashboard button | HTTPS (Ubidots) |
| cliente_mqtt_app | Publishes 4 sensor values and controls a pump and a light by subscription | MQTT over TLS (Ubidots) |
| monitoreo_c1_sep09a | Dashboard monitoring and remote LED control | Arduino IoT Cloud |

## Libraries
MAX6675, DHT sensor library, LiquidCrystal_I2C, PubSubClient, ArduinoIoTCloud.

## Configuration (credentials are NOT included)
Replace the placeholders with your own values:
- `YOUR_WIFI_SSID` / `YOUR_WIFI_PASSWORD`: your WiFi network.
- `YOUR_UBIDOTS_TOKEN`: your own Ubidots token.
- ESP-NOW nodes: set the MAC address of your receiver board.
- Arduino IoT Cloud: copy `arduino_secrets.example.h` to `arduino_secrets.h`
  (ignored by git) and fill in your values.

## Security notes
- No credentials are stored in this repository; keep it that way.
- Cloud traffic uses TLS. The demos call `setInsecure()` (encrypted, but the
  server certificate is not verified); use `setCACert()` in production.
- ESP-NOW demos are unencrypted; enable PMK/LMK encryption for real deployments.
- `esp32_broker_emqx` uses a public test broker: anyone can read its topics.
  Do not publish sensitive data there.

## Usage
Open a sketch in Arduino IDE, select the ESP32 board, upload and open the
Serial Monitor (baud rate is defined in each sketch).

## About
Personal project developed for learning and prototyping.
