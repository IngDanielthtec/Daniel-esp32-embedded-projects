#include <esp_now.h>
#include <WiFi.h>

#define NODE_ID_SENSOR1  1
#define NODE_ID_SENSOR2  2

#define DATA_SENSOR1  0
#define DATA_SENSOR2  1


// Estructura para recibir datos
typedef struct
{
  uint32_t nodoId;
  float temperatura;
  float humedad;
}Message_t;
Message_t messageRecv;  // idNodo Temperatura Humedad

//Funcion para que se ejecuta cuando se recibe un dato por ESP-NOW de los nodos
void esp_now_recv_cb(const uint8_t *mac, const uint8_t *incomingData, int len);

//Variables de aplicacion
Message_t dataSensores[2] = {0}; // indice = 0 (sensor 1) y indice = 1 (sensor 2)
//led
uint8_t ledState = 0;
 
void setup() 
{
  //Init Pin
  pinMode(2, OUTPUT);

  //Init UART0
  Serial.begin(115200);
  
  //Configuracion de wifi modo estacion
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }
  
  //Registrar la funcion que recibe un dato por ESP-NOW de los nodos
  esp_now_register_recv_cb(esp_now_recv_cb);
}
 
void loop() 
{
  //Manejo de datos recibidos (puede imprimirlos en un HMI o pantalla LCD)
  if(dataSensores[DATA_SENSOR1].nodoId == NODE_ID_SENSOR1)
  {
    dataSensores[DATA_SENSOR1].nodoId = 0; //Resetear nodo para imprimir solo cuando llegan datos nuevos
    Serial.printf("Datos de SENSOR1 -> SALA \r\n");
    Serial.printf("Temperatura: %.2f\r\n", dataSensores[DATA_SENSOR1].temperatura);
    Serial.printf("Humedad    : %.2f\r\n\r\n", dataSensores[DATA_SENSOR1].humedad);
  }
  else if(dataSensores[DATA_SENSOR2].nodoId == NODE_ID_SENSOR2)
  {
    dataSensores[DATA_SENSOR2].nodoId = 0; //Resetear nodo para imprimir solo cuando llegan datos nuevos
    Serial.printf("Datos de SENSOR2 -> HABITACION \r\n");
    Serial.printf("Temperatura: %.2f\r\n", dataSensores[DATA_SENSOR2].temperatura);
    Serial.printf("Humedad    : %.2f\r\n\r\n", dataSensores[DATA_SENSOR2].humedad);
  }
  delay(500);
}

//Desarrollo de funciones
void esp_now_recv_cb(const uint8_t *mac, const uint8_t *incomingData, int len) 
{
  // Ignore malformed packets
  if (len != (int)sizeof(messageRecv))
  {
    Serial.printf("Message ignored: invalid length (%d)\r\n", len);
    return;
  }
  memcpy(&messageRecv, incomingData, sizeof(messageRecv));
  Serial.printf("Data RX: |Len:%d| Nodo:%u | T:%.2f | H:%.2f | \r\n\r\n", len, messageRecv.nodoId, messageRecv.temperatura, messageRecv.humedad);
  
  if(messageRecv.nodoId == NODE_ID_SENSOR1)
  {
    //Copiar datos recibidos
    dataSensores[DATA_SENSOR1].nodoId      = NODE_ID_SENSOR1;
    dataSensores[DATA_SENSOR1].temperatura = messageRecv.temperatura;
    dataSensores[DATA_SENSOR1].humedad     = messageRecv.humedad;
  }
  else if(messageRecv.nodoId == NODE_ID_SENSOR2)
  {
    dataSensores[DATA_SENSOR2].nodoId      = NODE_ID_SENSOR2;
    dataSensores[DATA_SENSOR2].temperatura = messageRecv.temperatura;
    dataSensores[DATA_SENSOR2].humedad     = messageRecv.humedad;
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
}


