#include <WiFi.h>
#include <TFT_eSPI.h>
#include <OrbitScoutWiFi.h>  // Biblioteca tzapu/WiFiManager
#include "ProgressBar.h"
#include "Config.h"          // Certifique-se de que BTN_BACK está definido aqui
#include "MenuManager.h"
#include "DisplayConstants.h"

// O objeto TFT é declarado externamente (por exemplo, na main)
extern TFT_eSPI tft;


// Variável para armazenar o estado anterior da barra de RSSI
static int prevRSSIPercent = -1;

// ----------------------------------------------------------
// Instância global do WiFiManager para uso não bloqueante
// ----------------------------------------------------------
WiFiManager wifiManager;

// ----------------------------------------------------------
// Função para desenhar a barra de RSSI na tela TFT (já existente)
// ----------------------------------------------------------
void drawRSSIBar(bool forceUpdate = false) {
    if (WiFi.status() != WL_CONNECTED) {
        if (forceUpdate || (prevRSSIPercent != 0)) {
            prevRSSIPercent = 0;
            clearProgressBar(RSSI_BAR_X, RSSI_BAR_Y, RSSI_BAR_WIDTH, RSSI_BAR_HEIGHT);
            drawProgressBar(RSSI_BAR_X, RSSI_BAR_Y, RSSI_BAR_WIDTH, RSSI_BAR_HEIGHT, 0, true);

            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextFont(1);
            tft.drawString("NET", RSSI_BAR_X + 2, RSSI_BAR_Y + RSSI_BAR_HEIGHT + 4);
        }
        return;
    }

    long rssi = WiFi.RSSI();
    if (rssi == 31) {
        Serial.println("RSSI desconhecido.");
        if (prevRSSIPercent != 0) {
            prevRSSIPercent = 0;
            clearProgressBar(RSSI_BAR_X, RSSI_BAR_Y, RSSI_BAR_WIDTH, RSSI_BAR_HEIGHT);
            drawProgressBar(RSSI_BAR_X, RSSI_BAR_Y, RSSI_BAR_WIDTH, RSSI_BAR_HEIGHT, 0, true);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextFont(1);
            tft.drawString("NET", RSSI_BAR_X + 2, RSSI_BAR_Y + RSSI_BAR_HEIGHT + 4);
        }
        return;
    }

    if (rssi < RSSI_MIN) rssi = RSSI_MIN;
    if (rssi > RSSI_MAX) rssi = RSSI_MAX;
    int rssiPercent = map(rssi, RSSI_MIN, RSSI_MAX, 0, 100);

    if (forceUpdate || (rssiPercent != prevRSSIPercent)) {
        prevRSSIPercent = rssiPercent;
        clearProgressBar(RSSI_BAR_X, RSSI_BAR_Y, RSSI_BAR_WIDTH, RSSI_BAR_HEIGHT);
        drawProgressBar(RSSI_BAR_X, RSSI_BAR_Y, RSSI_BAR_WIDTH, RSSI_BAR_HEIGHT, rssiPercent, true);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextFont(1);
        tft.drawString("NET", RSSI_BAR_X + 2, RSSI_BAR_Y + RSSI_BAR_HEIGHT + 4);
    }
}

// ----------------------------------------------------------
// Função para desenhar o cabeçalho do portal de conexão
// ----------------------------------------------------------
void drawPortalHeader() {
    // Define a área para a janela de instruções usando as constantes globais
    Area connectionArea = { 
        INSTRUCTIONS_AREA_X, 
        INSTRUCTIONS_AREA_Y,
        INSTRUCTIONS_AREA_WIDTH, 
        INSTRUCTIONS_AREA_HEIGHT,
        INSTRUCTIONS_AREA_HEADER_HEIGHT 
    };

    // Desenha o cabeçalho utilizando o método estático de MenuManager
    MenuManager::drawArea(connectionArea, "WIFI", TFT_BLACK, TFT_WHITE, TFT_WHITE);
}

// ----------------------------------------------------------
// Função para desenhar as instruções de conexão (texto)
// ----------------------------------------------------------
void drawPortalInstructions() {
    // Configura a fonte e a cor para o texto das instruções
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // Calcula a posição inicial do texto logo abaixo do cabeçalho
    int textY = INSTRUCTIONS_AREA_Y + INSTRUCTIONS_AREA_HEADER_HEIGHT + 5;
    
    tft.drawString("Conecte-se ao", INSTRUCTIONS_AREA_X + 5, textY);
    tft.drawString("AP: OrbitScoutAP", INSTRUCTIONS_AREA_X + 5, textY + 12);
    tft.drawString("Senha: orbitscout", INSTRUCTIONS_AREA_X + 5, textY + 24);

    // Exibe o endereço IP do AP (modo portal cativo) – geralmente 192.168.4.1
    IPAddress apIP = WiFi.softAPIP();
    String ipStr = apIP.toString();
    tft.drawString("IP: " + ipStr, INSTRUCTIONS_AREA_X + 5, textY + 36);
}

// ----------------------------------------------------------
// Função para configurar e conectar ao Wi‑Fi via WiFiManager de forma não bloqueante
// ----------------------------------------------------------
void setupWiFi() {
    Serial.println("\nIniciando WiFiManager...");


    WiFi.mode(WIFI_STA);  // Define o modo de estação

    // Configura o WiFiManager para não bloquear e define um timeout (por exemplo, 60 segundos)
    wifiManager.setConfigPortalBlocking(false);

    // Tenta conectar automaticamente; se falhar, o portal cativo é iniciado
    if (wifiManager.autoConnect("OrbitScoutAP", "orbitscout")) {
        Serial.println("Wi-Fi conectado via WiFiManager!");
        Serial.print("Endereco IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("AutoConnect falhou; o portal cativo esta ativo.");

        tft.fillRect(INSTRUCTIONS_AREA_X - 2, INSTRUCTIONS_AREA_Y - 2,
            INSTRUCTIONS_AREA_WIDTH + 4, INSTRUCTIONS_AREA_HEIGHT + 4, TFT_BLACK);

        drawPortalHeader();
        
    }

    // Se não estiver conectado, entra num loop while que processa o WiFiManager
    // e atualiza as instruções na tela até que a conexão seja estabelecida.
    while (WiFi.status() != WL_CONNECTED) {
        wifiManager.process();             // Processa o portal não bloqueante
        drawPortalInstructions();

    Serial.println("Wi-Fi conectado!");  

    if (digitalRead(BTN_BACK) == LOW) {
        break;
    }
    }
}
