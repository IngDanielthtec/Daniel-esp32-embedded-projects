#include <esp_now.h>
#include <WiFi.h>
#include "DHT.h"

#define NODE_ID_SENSOR2  2

#define DHTPIN  4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Estructura para enviar datos
typedef struct
{
  uint32_t nodoId;
  float    temperatura;
  float    humedad;
}Message_t;
Message_t messageSend;

//Funcion para informar el estado de la comunicacion
void esp_now_send_cb (const uint8_t *mac_addr, esp_now_send_status_t status);

// Direccion MAC del dispositivo responder (receptor)
uint8_t responder_macAddr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Replace with the MAC address of your receiver board
//Variable para configurar la informacion del dispositivo par
esp_now_peer_info_t peerInfo;
//led
uint8_t ledState = 0;

void setup() 
{
  //Init Pin
  pinMode(2, OUTPUT);

  //Init UART0
  Serial.begin(115200);

  //Init DHT22
  dht.begin();
 
  //Configuracion de wifi modo estacion
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  //Registar la funcion que iforma sobre el estado de la comunicacion
  esp_now_register_send_cb(esp_now_send_cb);
  
  //Registrar un dispositivo par
  memcpy(peerInfo.peer_addr, responder_macAddr, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false; // Demo only: enable ESP-NOW encryption (PMK/LMK) for real deployments
  
  //Agregar dispositivo par    
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Error al agregar dispositivo par");
    return;
  }

  //Configurar nodo de sensor
  messageSend.nodoId = NODE_ID_SENSOR2;
}
 
void loop() 
{
  //Leer valor de sensor
  messageSend.humedad     = dht.readHumidity();
  messageSend.temperatura = dht.readTemperature();
  
  //Enviar mensaje via ESP-NOW
  esp_err_t result = esp_now_send(responder_macAddr, (uint8_t *)&messageSend, sizeof(messageSend));

  if (result == ESP_OK) 
  {
    Serial.printf("El mensaje ha sido enviado !!! \r\n");
    Serial.printf("ID nodo    : %u \r\n", messageSend.nodoId);
    Serial.printf("Temperatura: %.2f \r\n", messageSend.temperatura);
    Serial.printf("Humedad    : %.2f \r\n\r\n", messageSend.humedad);
  }
  else {
    Serial.printf("El mensaje no pudo enviarse");
  }

  ledState = ~ledState;
  if(ledState != 0)
  {
    digitalWrite(2, HIGH);
  }
  else 
  {
    digitalWrite(2, LOW);
  }
  delay(3000);
}

//Desarrollo de funciones
void esp_now_send_cb (const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  if(status == ESP_NOW_SEND_SUCCESS)
	{
    Serial.printf("Estado de mensaje ESP-NOW: ESP_NOW_SEND_SUCCESS\r\n\r\n");
	}
	else
	{
    Serial.printf("Estado de mensaje ESP-NOW: ESP_NOW_SEND_FAIL\r\n\r\n");
	}
}

