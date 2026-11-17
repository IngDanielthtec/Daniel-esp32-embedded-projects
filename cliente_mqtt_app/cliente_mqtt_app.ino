/*

{"var_name": val, "var_name": val, "var_name": val}

*/

#include <string.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "max6675.h"
#include "DHT.h"

//Pinout DHT22
#define DHTPIN  22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Pinout MAX6675
#define MAX_SO  19
#define MAX_SCK 18
#define MAX_CS  5
MAX6675 termopar(MAX_SCK, MAX_CS, MAX_SO);

//Canal ADC
#define ADC1_CH7_SENSOR_HMD 36
//Pinout Bomba
#define BOMBA_PIN           27
//Pinout Luz artificial
#define LED_PIN             14

// MQTT Broker
const char *mqtt_broker   = "industrial.api.ubidots.com";
const char *mqtt_username = "YOUR_UBIDOTS_TOKEN"; // Configurar tu propio token (cuenta ubidots)
const char *mqtt_password = NULL;
const int   mqtt_port     = 8883; // TLS
//Topic
const char *topic_publish   = "/v1.6/devices/esp32_demo_curso";
const char *topic_bomba_sub = "/v1.6/devices/esp32_demo_curso/ctrl_bomba/lv"; // "/v1.6/devices/device_name/varible_name/lv"
const char *topic_led_sub   = "/v1.6/devices/esp32_demo_curso/ctrl_led/lv";

//Credenciales red WiFi
static const char *ssid     = "YOUR_WIFI_SSID";
static const char *password = "YOUR_WIFI_PASSWORD";

//Objetos para menejo de cliente MQTT
WiFiClientSecure espClient; // MQTT over TLS (encrypted)
PubSubClient client(espClient);

//Variables de APP
float tempAmbiente = 0.0;
float tempSuelo    = 0.0;
float hmdRelativa  = 0.0;
float hmdSuelo     = 0.0;
uint16_t valorSensor = 0; //0...4095
char messagePublish[200] = {0};
char dataSubscriber[50]  = {0};

void mqtt_callback(char *topic, byte *payload, unsigned int length);


void setup() 
{
  //UART 0
  Serial.begin(115200);

  //Init GPIO
  pinMode(BOMBA_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BOMBA_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);

  //Set ADC
  analogSetWidth(12); //Resolucion 12 bits

  //DHT22 init
  dht.begin();

  //Set WIFI_STA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //Compruebe que el ESP32 está conectado a la red wi-fi
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Conectando WiFi..");
  }

  //Conexion exitosa
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel     : ");
  Serial.println(WiFi.channel());

  //Conectar a  broker MQTT
  espClient.setInsecure(); // Encrypts traffic; server certificate is not verified (demo). Use setCACert() in production.
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(mqtt_callback); //Recibir datos de suscripcion

  while (!client.connected()) 
  {
    String client_id = "ESP32-Client-";  
    client_id += String(WiFi.macAddress());
    Serial.printf("Cliente | %s | conectado a broker ubidots\n", client_id.c_str());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Cliente conectado OK");
    } 
    else 
    {
      Serial.println("Cliente falla la conexion");
      Serial.println(client.state());
      delay(2000);
    }
  }

  //Suscribirse a temas (topic)
  client.subscribe(topic_bomba_sub);
  client.subscribe(topic_led_sub);
}

void loop() 
{
  //Adquisicion de datos
  tempAmbiente = dht.readTemperature();
  hmdRelativa  = dht.readHumidity();
  tempSuelo    = termopar.readCelsius();
  valorSensor  = analogRead(ADC1_CH7_SENSOR_HMD); 
  hmdSuelo     = (float)map(valorSensor, 0, 4095, 100, 0); 

  //Imprimir datos
  Serial.printf("Temperatura ambiente: %.2f \r\n", tempAmbiente);
  Serial.printf("Temperatura de suelo: %.2f \r\n", tempSuelo);
  Serial.printf("Humedad relativa    : %.2f %%\r\n", hmdRelativa);
  Serial.printf("Humedad de suelo    : %.2f %%\r\n", hmdSuelo);

  snprintf(messagePublish, sizeof(messagePublish), "{\"temp_ambiente\": %.2f,\"hmd_relativa\": %.2f,\"temp_suelo\": %.2f,\"hmd_suelo\": %.2f}", 
          tempAmbiente, hmdRelativa, tempSuelo, hmdSuelo);

  Serial.printf("Enviando Datos al Broker MQTT \n");
  // Publicacion en ubidots
  client.publish(topic_publish, messagePublish);
  Serial.printf("Data: %s \r\n\r\n", messagePublish);

  //Actualizar la conexion
  client.loop();

  delay(1000);
}

//Implementacion de funcion para procesar los datos recibos de topics suscritos
void mqtt_callback(char *topic, byte *payload, unsigned int length) 
{
  int indice = 0;

  Serial.printf("\r\nEvento de suscripcion\r\n");

  // Bounded copy: never write beyond the buffer, whatever the message size
  unsigned int maxLen = sizeof(dataSubscriber) - 1;
  unsigned int n = (length < maxLen) ? length : maxLen;
  for (indice = 0; indice < (int)n; indice++) 
  {
    Serial.print((char) payload[indice]);
    dataSubscriber[indice] = payload[indice];
  }
  dataSubscriber[n] = '\0';
  Serial.printf("Llego topic: %s -> msg: %s \r\n", topic, dataSubscriber);

  //"/v1.6/devices/esp32_demo_curso/ctrl_bomba/lv"
  if(strstr(topic, "ctrl_bomba"))
  {
    if(0 == strcmp(dataSubscriber, "0.0"))
    {
      digitalWrite(BOMBA_PIN, HIGH);
      Serial.printf("Bomba = OFF \r\n\r\n");
    }
    else if(0 == strcmp(dataSubscriber, "1.0"))
    {
      digitalWrite(BOMBA_PIN, LOW);
      Serial.printf("Bomba = ON \r\n\r\n");
    }
  }
  else if(strstr(topic, "ctrl_led"))  //"/v1.6/devices/esp32_demo_curso/ctrl_led/lv"
  {
    if(0 == strcmp(dataSubscriber, "0.0"))
    {
      digitalWrite(LED_PIN, HIGH);
      Serial.printf("LUZ artificial = OFF \r\n\r\n");
    }
    else if(0 == strcmp(dataSubscriber, "1.0"))
    {
      digitalWrite(LED_PIN, LOW);
      Serial.printf("LUZ artificial = ON \r\n\r\n");
    }
  }

  Serial.println();
  memset(dataSubscriber, 0x00, 50); //Limpiar buffer de datos
}




