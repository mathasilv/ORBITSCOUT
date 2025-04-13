#ifndef ORBITSCOUTWIFI_H
#define ORBITSCOUTWIFI_H

#include <TFT_eSPI.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

extern TFT_eSPI tft;

// Função para inicializar a conexão Wi-Fi
void setupWiFi();
void updateWiFiRSSI();
void drawRSSIBar(bool forceUpdate);

#endif // ORBITSCOUTWIFI_H
