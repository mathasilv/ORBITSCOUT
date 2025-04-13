#include <Arduino.h>
#include <TFT_eSPI.h>
#include "gps.h" 
#include "SatelliteTracker.h"
#include <Wire.h>
#include <math.h>
#include "MenuManager.h"
#include <TimeLib.h> 
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <FS.h>
#include <BacklightControl.h>
#include "OrbitScoutWiFi.h"
#include "ProgressBar.h"
#include "BatteryMonitor.h"
#include "Config.h" 
#include "TleManager.h"
#include "NotificationManager.h"
#include "OrientationManager.h"
#include "boot.h"

// ---------------------------------------------------------------------
// Objetos globais do projeto
// ---------------------------------------------------------------------
TFT_eSPI tft = TFT_eSPI();                       // Objeto do display TFT
NotificationManager notificationManager;         // Gerenciador de notificações
MenuManager menuManager;                         // Gerenciador do menu
SatelliteTracker tracker;                        // Rastreador de satélites
BatteryMonitor battery;                          // Monitoramento de bateria
TleManager tleManager;                           // Gerenciador de TLEs
OrientationManager orientationManager(55, BNO055_ADDRESS_B);


// ---------------------------------------------------------------------
// Função auxiliar para exibir uma mensagem na tela centralizada
// ---------------------------------------------------------------------
void showSetupMessage(const String &msg, int posY, int duration = 200) {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  // Centraliza o texto horizontalmente
  int16_t textWidth = tft.textWidth(msg);
  int16_t posX = (tft.width() - textWidth) / 2;
  tft.fillRect(0, posY, 240, 20, TFT_BLACK);
  tft.drawString(msg, posX, posY);
  delay(duration);
}

// ---------------------------------------------------------------------
// Setup: Inicialização de todos os módulos e da interface
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Define the total number of steps (including the menu) and the increment for each step
  const int totalSteps = 9;  // Steps: Display, Backlight, Buttons, Battery, GPS, TLE Manager, Menu, Timezone, Final Menu
  int stepIncrement = 100 / totalSteps;
  int progress = 0;
  
  // Parameters for the progress bar (adjust as needed)
  const int progressBarX = 20;
  const int progressBarY = 210;
  const int progressBarWidth = 200;
  const int progressBarHeight = 20;

  // Step 1: Initialize the TFT display
  // showSetupMessage("Initializing TFT display...", 265);
  tft.init();
  tft.invertDisplay(false);
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  tft.pushImage(
    0,       // posX
    10,       // posY
    240,     // Image width1
    240,     // Image height
    boot     // Name of the array in boot.h
  );

  // Step 2: Initialize the backlight
  // showSetupMessage("Initializing backlight...", 265);
  initializeBacklight(BACKLIGHT_PIN);
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  // Step 5: Configure the GPS
  // showSetupMessage("Configuring GPS...", 265);
  setupGPS();
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  // Step 3: Configure the buttons
  // showSetupMessage("Configuring buttons...", 265);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  // Step 4: Initialize battery monitoring
  // showSetupMessage("Initializing battery...", 265);
  // battery.begin();
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  // Step 6: Initialize the TLE Manager
  // showSetupMessage("Initializing TLE Manager...", 265);
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  // Step 7: Configure the menu
  // showSetupMessage("Configuring menu...", 265);
  menuManager.addMenuItem("TRACK SAT", []() { tleManager.selectTleSource(); });
  menuManager.addMenuItem("TLE UPDATE", []() { tleManager.forceUpdateAllTle(); });
  menuManager.addMenuItem("BRIGHTNESS", controlBacklight);
  menuManager.addMenuItem("MANUAL TRACK", []() { tracker.manualTrack(); });
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  // Step 8: Set the timezone
  // showSetupMessage("Setting timezone...", 265);
  setTimezone(-3);
  progress += stepIncrement;
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);

  // Initialize the orientation sensor (Compass/BNO055)
  // showSetupMessage("Initializing Orientation...", 1000);
  if (!orientationManager.begin()) {
    Serial.println("Erro ao iniciar o OrientationManager!");
    // showSetupMessage("Sensor Error", 2400);
  }

  // Step 9: Display the main menu
  // showSetupMessage("Starting Menu...", 265);
  progress = 100; // Complete the progress bar
  drawProgressBar(progressBarX, progressBarY, progressBarWidth, progressBarHeight, progress, false);


  // Clear the screen to start the main interface
  tft.fillScreen(TFT_BLACK);
  
  // Initialize the first satellite (default: index 0)
  tracker.initSatellite(0);

  // Draw the main menu
  menuManager.drawMenu();
}

// ---------------------------------------------------------------------
// Loop: Atualiza dados e trata a navegação do menu
// ---------------------------------------------------------------------
void loop() {
  updateGPSData();
  menuManager.update();
  notificationManager.checkNotifications();

  if (digitalRead(BTN_NEXT) == LOW) {
    menuManager.moveDown();
    delay(200);
  } 
  else if (digitalRead(BTN_PREV) == LOW) {
    menuManager.moveUp();
    delay(200);
  } 
  else if (digitalRead(BTN_SELECT) == LOW) {
    menuManager.selectItem();
    delay(200);
  } 
  else if (digitalRead(BTN_BACK) == LOW) {
    menuManager.back();
    delay(200);
  }
}