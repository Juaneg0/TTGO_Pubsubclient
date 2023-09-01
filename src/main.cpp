#include <Arduino.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <stdlib.h>

#include "DHT20.h"

// Update these with values suitable for your network.

const char* ssid = "juanes";
const char* password = "juanes1012";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (10)
char msg1[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];

/*** DEFINICIÓN DE VARIABLES***************************************************************/
DHT20 DHT(&Wire);
TFT_eSPI tft = TFT_eSPI();

float Humedad = 0;
float Temperatura = 0;
char WifiCon = 0;
/*****************************************************************************************/
int tam = 0;
char pos = '0';
char val = '0';
int indx = 0;
char LED[2] = {'0', '0'};

const uint8_t LED1 = 27;
const uint8_t LED2 = 26;

void TurnLEDS(char *topic, byte *payload){
  tam = (strlen(topic))-1;
  pos = (char)(topic[tam]);
  indx = (int)(pos - '0') - 1;
  val = (char)payload[0];

  LED[indx] = val;

  if (LED[0] == '1'){
    digitalWrite(LED1, HIGH);
  }else{
    digitalWrite(LED1, LOW);
  }

  if (LED[1] == '1'){
    digitalWrite(LED2, HIGH);
  }else{
    digitalWrite(LED2, LOW);
  }
}

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Conectando a la red: ");
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
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    TurnLEDS(topic, payload);
}

void reconnect() {
    // Loop until we're reconnected
    if (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("TTGO")) {
            Serial.println("connected");
            //Subscribe
            client.subscribe("esp32/led1");
            client.subscribe("esp32/led2");
            
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(500);
        }
    }
}

void setup() {
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Realizado por:", 10, 5, 2);
    tft.drawString("Juan E. Gomez", 10, 23, 4);
    tft.drawString("Humedad:", 10, 70, 2);
    tft.drawString("Temperatura", 140, 70, 2);
    tft.drawFastHLine(10, 50, 170, TFT_GREEN);
    tft.fillRect(110, 65, 3, 80, TFT_GREEN);
    Serial.println("--------------Pantalla Encendida-------");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000) {
        lastMsg = now;

        Humedad += 1;
        Temperatura += 1;

        // Se muestra el dato en la pantalla
        if (WifiCon = 1){
            tft.drawString("WiFi", 200, 5, 2);
        }
        tft.drawString((String(Humedad, 2)) + '%', 10, 100, 4);
        tft.drawString((String(Temperatura, 2)) + 'C', 140, 100, 4);
        Serial.print(Humedad);
        Serial.print(" % \t");
        Serial.print(Temperatura);
        Serial.println(" C");

        snprintf(msg1, MSG_BUFFER_SIZE, "%.2f", Humedad);
        snprintf(msg2, MSG_BUFFER_SIZE, "%.2f", Temperatura);
        client.publish("esp32/Humedad", msg1);
        client.publish("esp32/Temperatura", msg2);
        client.loop();
    }
}
