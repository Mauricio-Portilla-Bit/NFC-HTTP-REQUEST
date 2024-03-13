#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <MFRC522.h>
#include <ArduinoJson.h>


// MFRC CONFIGURATION
#define RST_PIN 16                 // Configurable, see typical pin layout above
#define SS_PIN 15                  // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);  ///Creamos el objeto para el RC522

// WIFI CONFIG
const char* ssid = "";
const char* password = "";

// WIFI CLIENT
WiFiClientSecure client;

void setup() {
  Serial.begin(9600);  //Iniciamos La comunicacion serial

  // Conexión al mfrc522
  SPI.begin();         //Iniciamos el Bus SPI
  mfrc522.PCD_Init();  // Iniciamos el MFRC522

  //Conexión a internet
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();

  // Inicio del programa
  Serial.println("INICIANDO EL PROGRAMA:");
}



void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    String lectura = "";
    // Revisamos si hay nuevas tarjetas  presentes
    if (mfrc522.PICC_IsNewCardPresent()) {
      //Seleccionamos una tarjeta
      if (mfrc522.PICC_ReadCardSerial()) {
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          lectura += String(mfrc522.uid.uidByte[i]);
        }
        Serial.println(lectura);
        //Enviar la lectura por http
        Serial.println("[HTTP] Iniciando...\n");
        if (http.begin(client, "http://")) {
          http.addHeader("Content-Type", "application/json");
          
          StaticJsonDocument<200> doc;
          doc["id"] = lectura;
          String requestBody;
          serializeJson(doc, requestBody);
          int httpCode = http.POST(requestBody);

          if (httpCode > 0) {
            Serial.println("Mensaje recibido");
          } else {
            Serial.println("Mensaje no recibido ");
          }
        } else {

          Serial.println("Imposible conectarse al servidor");
        }
        http.end();
      }
      delay(5000);

      // Terminamos la lectura de la tarjeta tarjeta  actual
      mfrc522.PICC_HaltA();
    }
  }
}
