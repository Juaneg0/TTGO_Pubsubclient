#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <stdlib.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>

#include "DHT20.h"

#include "data.h"
#include "Settings.h"

#include "oled.h"
#include "AuxFun.h"

#define BUTTON_LEFT 0        // btn activo en bajo
#define LONG_PRESS_TIME 3000 // 3000 milis = 3s

const char* mqtt_server = "192.168.1.3"; // MQTT Broker server

WiFiClient espClient;
PubSubClient client(espClient);
#define MSG_BUFFER_SIZE (10)
char msg1[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];

/*** DEFINICIONES DE LAS FUNCIONES *******************************************************/
//*** VARIABLES DE LEDS Y COMUNICACION ***/
char LED[2] = {'0', '0'};

const uint8_t LED1 = 27; // Pin used to write data based on 1's and 0's coming from Ubidots
const uint8_t LED2 = 26; // Pin used to write data based on 1's and 0's coming from Ubidots

void callback(char *topic, byte *payload, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();

  TurnLEDS(LED, topic, payload, LED1, LED2);
}
/*****************************************************************************************/

/*** DEFINICIÓN DE VARIABLES *************************************************************/
DHT20 DHT(&Wire);

TFT_eSPI tft = TFT_eSPI();
float Humedad = 0;
float Temperatura = 0;
char WifiCon = 0;

//*** VARIABLES DE TIEMPO ***/
unsigned long TS_ant, TS_act;  // Tiempos para el muestreo del sensor
const long TS_fin = 1000 / 40; // Tiempo en milisegundos para un tiempo de muestreo de 40 Hz
unsigned long TP_ant, TP_act;  // Tiempos para la muestra del dato en el OLED
const long TP_fin = 500;       // Tiempo en milisegundos para una actualizacion de 2 Hz
unsigned long Tx_ant, Tx_act;  // Tiempos para la muestra del dato en el OLED
const long Tx_fin = 200;      // Tiempo en milisegundos para una actualizacion de 0.2 Hz
unsigned long Rx_ant, Rx_act;  // Tiempos para la muestra del dato en el OLED
const long Rx_fin = 100;       // Tiempo en milisegundos para una actualizacion de 40 Hz

/*****************************************************************************************/

WebServer server(80);

Settings settings;
int lastState = LOW; // para el btn
int currentState;    // the current reading from the input pin
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

void load404();
void loadIndex();
void loadFunctionsJS();
void restartESP();
void saveSettings();
bool is_STA_mode();
void AP_mode_onRst();
void STA_mode_onRst();
void detect_long_press();

// Rutina para iniciar en modo AP (Access Point) "Servidor"
void startAP(){
  WiFi.disconnect();
  delay(19);
  Serial.println("Starting WiFi Access Point (AP)");
  WiFi.softAP("TTGO", "12345678");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

// Rutina para iniciar en modo STA (Station) "Cliente"
void start_STA_client(){
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();
  delay(100);
  Serial.println("Starting WiFi Station Mode");
  WiFi.begin((const char *)settings.ssid.c_str(), (const char *)settings.password.c_str());
  WiFi.mode(WIFI_STA);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    // Serial.print(".");
    if (cnt == 30) // Si después de 100 intentos no se conecta, vuelve a modo AP
      AP_mode_onRst();
    cnt++;
    Serial.println("attempt # " + (String)cnt);
  }

  WiFi.setAutoReconnect(true);
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  pressedTime = millis();
}

void reconnect() {
    // Loop until we're reconnected
    if (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("TTGO")) {
            Serial.println("connected");
            //Subscribe
            client.subscribe("esp32/Led1");
            client.subscribe("esp32/Led2");
            
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(500);
        }
    }
}



void setup(){
  Serial.begin(115200);
  delay(2000);

  EEPROM.begin(4096);                 // Se inicializa la EEPROM con su tamaño max 4KB
  pinMode(BUTTON_LEFT, INPUT_PULLUP); // btn activo en bajo

  // settings.reset();
  settings.load(); // se carga SSID y PWD guardados en EEPROM
  settings.info(); // ... y se visualizan

  Serial.println("");
  Serial.println("starting...");

  if (is_STA_mode()){
    start_STA_client();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    DHT.begin(); //  ESP32 default pins 21 22
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    
    Oled_Ini(tft);

    Serial.println("Humidity, Temperature");

    TS_ant = millis();
    TP_ant = millis();
    Tx_ant = millis();
    Rx_ant = millis();
  } else{// Modo Access Point & WebServer
    startAP();

    /* ========== Modo Web Server ========== */

    /* HTML sites */
    server.onNotFound(load404);

    server.on("/", loadIndex);
    server.on("/index.html", loadIndex);
    server.on("/functions.js", loadFunctionsJS);

    /* JSON */
    server.on("/settingsSave.json", saveSettings);
    server.on("/restartESP.json", restartESP);

    server.begin();
    Serial.println("HTTP server started");
  }
}

void loop(){
  if (is_STA_mode()){// Rutina para modo Station (cliente)
    TS_act = millis();
    TP_act = millis();
    Tx_act = millis();
    Rx_act = millis();
    
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Lectura del sensor
    if (TS_act - TS_ant >= TS_fin){
      TS_ant = TS_act;
      // Se toma el dato
      DHT.read();
      Humedad = DHT.getHumidity();
      Temperatura = DHT.getTemperature();
    }

    // Pantalla
    if (TP_act - TP_ant >= TP_fin){
      TP_ant = TP_act;
      // Se muestra el dato en la pantalla
      Show_Data(tft, WifiCon, Humedad, Temperatura);
    }

    // Transmision
    if (Tx_act - Tx_ant >= Tx_fin){
      Tx_ant = Tx_act;
      // Se envia el dato a UbiDots
      WifiCon = 1;
      if (WiFi.status() != WL_CONNECTED){
        WifiCon = 0;
      }
      snprintf(msg1, MSG_BUFFER_SIZE, "%.2f", Humedad);
      snprintf(msg2, MSG_BUFFER_SIZE, "%.2f", Temperatura);
      client.publish("esp32/Humedad", msg1);
      client.publish("esp32/Temperatura", msg2);
      client.loop();
    }

    // Recepcion
    if (Rx_act - Rx_ant >= Rx_fin){
      Rx_ant = Rx_act;
      // Se verifica datos de Ubidots
      if (!client.connected()){
        reconnect();
        client.subscribe("esp32/Led1");
        client.subscribe("esp32/Led2");
      }
      client.loop();
    }
  } else {
    // rutina para AP + WebServer
    server.handleClient();
  }  
  delay(10);
  detect_long_press();
}

// funciones para responder al cliente desde el webserver:
// load404(), loadIndex(), loadFunctionsJS(), restartESP(), saveSettings()

void load404(){
  server.send(200, "text/html", data_get404());
}

void loadIndex(){
  server.send(200, "text/html", data_getIndexHTML());
}

void loadFunctionsJS(){
  server.send(200, "text/javascript", data_getFunctionsJS());
}

void restartESP(){
  server.send(200, "text/json", "true");
  ESP.restart();
}

void saveSettings(){
  if (server.hasArg("ssid"))
    settings.ssid = server.arg("ssid");
  if (server.hasArg("password"))
    settings.password = server.arg("password");

  settings.save();
  server.send(200, "text/json", "true");
  STA_mode_onRst();
}

// Rutina para verificar si ya se guardó SSID y PWD del cliente
// is_STA_mode retorna true si ya se guardaron
bool is_STA_mode(){
  if (EEPROM.read(flagAdr))
    return true;
  else
    return false;
}

void AP_mode_onRst(){
  EEPROM.write(flagAdr, 0);
  EEPROM.commit();
  delay(100);
  ESP.restart();
}

void STA_mode_onRst(){
  EEPROM.write(flagAdr, 1);
  EEPROM.commit();
  delay(100);
  ESP.restart();
}

void detect_long_press(){
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_LEFT);

  if (lastState == HIGH && currentState == LOW) // button is pressed
    pressedTime = millis();
  else if (lastState == LOW && currentState == HIGH){ 
    // button is released
    releasedTime = millis();

    Serial.println("releasedtime" + (String)releasedTime);
    Serial.println("pressedtime" + (String)pressedTime);

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration > LONG_PRESS_TIME){
      Serial.println("(Hard reset) returning to AP mode");
      delay(500);
      AP_mode_onRst();
    }
  }

  // save the last state
  lastState = currentState;
}