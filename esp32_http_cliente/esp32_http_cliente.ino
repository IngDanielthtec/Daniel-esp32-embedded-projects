
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

//Pinout DHT22
#define DHTPIN  22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Credenciales red WiFi
static const char *ssid     = "YOUR_WIFI_SSID";
static const char *password = "YOUR_WIFI_PASSWORD";

//Credenciales de servidor HTTP Ubidots
#define SERVER_HTTP            "industrial.api.ubidots.com"
#define REMOTE_PORT_HTTP        443 // HTTPS
#define UBT_TOKEN               "YOUR_UBIDOTS_TOKEN"

WiFiClientSecure espClient; // HTTPS (encrypted)
HTTPClient http;

//Variables de APP
float tempAmbiente = 0.0;
float hmdRelativa  = 0.0;
char dataJSON[200];
int respuestaHTTP = 0;

void setup() 
{
  //UART 0
  Serial.begin(115200);

  //DHT22 init
  dht.begin();

  //Set WIFI_STA
  WiFi.mode(WIFI_STA);
  espClient.setInsecure(); // Encrypts traffic; server certificate is not verified (demo). Use setCACert() in production.
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

}

void loop() 
{
  if(WiFi.status() == WL_CONNECTED)
  {
    //Adquisicion de datos
    tempAmbiente = dht.readTemperature();
    hmdRelativa  = dht.readHumidity();

    //Imprimir datos
    Serial.printf("Temperatura ambiente: %.2f \r\n", tempAmbiente);
    Serial.printf("Humedad relativa    : %.2f %%\r\n", hmdRelativa);

    snprintf(dataJSON, sizeof(dataJSON), "{\"temperatura\": %.2f,\"humedad\": %.2f}", tempAmbiente, hmdRelativa);
    Serial.printf("\r\nData JSON: %s \r\n\r\n", dataJSON);


    // METODO POST
    http.begin(espClient, SERVER_HTTP , REMOTE_PORT_HTTP, "/api/v1.6/devices/esp32_http_demo/", true);
    http.addHeader("X-Auth-Token", UBT_TOKEN);
    http.addHeader("Content-Type", "application/json");
    //Realizar la solicitud HTTP con POST
    respuestaHTTP = http.POST(dataJSON);
    http.end();
    if(respuestaHTTP == 200)
    {
      Serial.printf("codigo de respuesta: %d \r\n", respuestaHTTP);
    }
    else
    {
      Serial.printf("Error POST HTTP \r\n");
    }

    //METODO GET
    http.begin(espClient, SERVER_HTTP , REMOTE_PORT_HTTP, "/api/v1.6/devices/esp32_http_demo/button/lv", true);
    http.addHeader("X-Auth-Token", UBT_TOKEN);
    //Realizar la solicitud HTTP GET
    respuestaHTTP = http.GET();
    if(respuestaHTTP == 200)
    {
      Serial.printf("codigo de respuesta: %d \r\n", respuestaHTTP);
      //Leer datos de respuesta HTTP
      Serial.printf("Data recibada: \r\n");
      String stringData = http.getString();
      Serial.println(stringData);
      Serial.printf("\r\n\r\n");
    }
    else
    {
      Serial.printf("Error GET HTTP \r\n");
    }
    http.end();

  }
  else
  {
    Serial.printf("WiFi desconectado\r\n");
  }

  delay(6000);
}
