#include "AuxFun.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <stdlib.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>

void TurnLEDS(char LED[], char *topic, byte *payload, const uint8_t LED1, const uint8_t LED2){
  
  int tam = (strlen(topic)) - 4;
  char pos = (char)(topic[tam]);
  char indx = (int)(pos - '0') - 1;
  int val = (char)payload[0];

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