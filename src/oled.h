#ifndef OLED_H
#define OLED_H

#include <TFT_eSPI.h>
#include <SPI.h>
#include <Arduino.h>

/* ---------------------------- Public Functions ---------------------------- */

void Oled_Ini(TFT_eSPI);
void Show_Data(TFT_eSPI, char, float, float);

#endif /* OLED_H */