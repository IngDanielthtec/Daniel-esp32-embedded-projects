
#include <string.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

//Pinout DHT22
#define DHTPIN  22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Credenciales red WiFi
static const char *ssid     = "YOUR_WIFI_SSID";
static const char *password = "YOUR_WIFI_PASSWORD";

// MQTT Broker
const char *mqtt_broker   = "broker.emqx.io";
const char *mqtt_username = "emqx"; 
const char *mqtt_password = "public";
const int   mqtt_port     = 1883;
//Topic
const char *topic_pub_temp   = "esp32_demo/temperatura";
const char *topic_pub_hmd    = "esp32_demo/humedad";

//Objetos para menejo de cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

//Variables de APP
float tempAmbiente = 0.0;
float hmdRelativa  = 0.0;
char messagePublish[100] = {0};

void setup() 
{
  //UART 0
  Serial.begin(115200);

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
  client.setServer(mqtt_broker, mqtt_port);
  //client.setCallback(mqtt_callback); //Recibir datos de suscripcion

  while (!client.connected()) 
  {
    String client_id = "ESP32-Client-";  
    client_id += String(WiFi.macAddress());
    Serial.printf("Cliente | %s | conectado a broker EMQX\n", client_id.c_str());

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
}

void loop() 
{
  //Adquisicion de datos
  tempAmbiente = dht.readTemperature();
  hmdRelativa  = dht.readHumidity();

  //Imprimir datos
  Serial.printf("Temperatura ambiente: %.2f \r\n", tempAmbiente);
  Serial.printf("Humedad relativa    : %.2f %%\r\n", hmdRelativa);

  snprintf(messagePublish, sizeof(messagePublish), "{\"temp_ambiente\": %.2f}", tempAmbiente);
  client.publish(topic_pub_temp, messagePublish); //Publicacion
  memset(messagePublish, 0x00, 100);

  snprintf(messagePublish, sizeof(messagePublish), "{\"humedad_relativa\": %.2f}", hmdRelativa);
  client.publish(topic_pub_hmd, messagePublish); //Publicacion
  memset(messagePublish, 0x00, 100);

  delay(3000);
}
