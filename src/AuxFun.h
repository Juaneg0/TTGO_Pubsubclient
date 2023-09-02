#ifndef AUXFUN_H
#define AUXFUN_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <stdlib.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>


/* ---------------------------- Public Functions ---------------------------- */

void TurnLEDS(char [], char *, byte *, const uint8_t, const uint8_t);

#endif /* AUXFUN_H */