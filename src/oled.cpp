#include "oled.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Arduino.h>

void Oled_Ini(TFT_eSPI tft){
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

void Show_Data(TFT_eSPI tft, char WifiCon, float Humedad, float Temperatura){
    tft.setRotation(3);
    if (WifiCon = 1){
        tft.drawString("WiFi", 200, 5, 2);
    }
    tft.drawString((String(Humedad, 2)) + '%', 10, 100, 4);
    tft.drawString((String(Temperatura, 2)) + 'C', 140, 100, 4);
    Serial.print(Humedad);
    Serial.print("% \t");
    Serial.print(Temperatura);
    Serial.println("C");
}