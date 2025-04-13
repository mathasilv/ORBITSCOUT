#include "BacklightControl.h"
#include <Arduino.h>
#include <math.h>          // Para a função round()
#include "ProgressBar.h"   // Funções: drawProgressBar(), clearProgressBar()
#include "MenuManager.h"   // Para acesso à função de redesenho do menu
#include "Config.h"        // Contém defines: BTN_NEXT, BTN_PREV, BTN_BACK, BACKLIGHT_PIN, etc.

// ----------------------------------------------------------
// Definições de PWM para o backlight
// ----------------------------------------------------------
#define BACKLIGHT_PWM_CHANNEL 0
#define BACKLIGHT_PWM_FREQ    5000
#define BACKLIGHT_PWM_RES     8

// ----------------------------------------------------------
// Variáveis estáticas para manter o estado do brilho
// ----------------------------------------------------------
static uint8_t brightness = 128;            // Brilho inicial (50%)
static const uint8_t BRIGHTNESS_STEP = 10;    // Passo para aumentar/diminuir o brilho

// Pino do backlight (definido durante a inicialização)
static uint8_t backlightPin;

// Declaração externa do MenuManager (definido em outro módulo)
extern MenuManager menuManager;

// Objeto global do display (definido, por exemplo, na main)
extern TFT_eSPI tft;

/**
 * @brief Inicializa o backlight.
 *
 * Configura o canal PWM e define o valor inicial de brilho.
 *
 * @param pin Pino onde o backlight está conectado.
 */
void initializeBacklight(uint8_t pin) {
    backlightPin = pin;
    ledcSetup(BACKLIGHT_PWM_CHANNEL, BACKLIGHT_PWM_FREQ, BACKLIGHT_PWM_RES);
    ledcAttachPin(backlightPin, BACKLIGHT_PWM_CHANNEL);
    ledcWrite(BACKLIGHT_PWM_CHANNEL, brightness);
}

/**
 * @brief Atualiza o brilho com base nos botões NEXT e PREV.
 *
 * Utiliza um debounce para evitar múltiplos ajustes em curto intervalo.
 *
 * @param btnNextState Estado do botão NEXT.
 * @param btnPrevState Estado do botão PREV.
 */
void updateBacklight(int btnNextState, int btnPrevState) {
    static unsigned long lastPressTime = 0;
    const unsigned long debounceDelay = 200; // Intervalo de debounce (ms)
    unsigned long currentTime = millis();

    // Se o botão NEXT for pressionado, aumenta o brilho
    if (btnNextState == LOW && (currentTime - lastPressTime) > debounceDelay) {
        brightness = constrain(brightness + BRIGHTNESS_STEP, 0, 255);
        ledcWrite(BACKLIGHT_PWM_CHANNEL, brightness);
        Serial.printf("Brilho aumentado para %d\n", brightness);
        lastPressTime = currentTime;
    }

    // Se o botão PREV for pressionado, diminui o brilho
    if (btnPrevState == LOW && (currentTime - lastPressTime) > debounceDelay) {
        brightness = constrain(brightness - BRIGHTNESS_STEP, 0, 255);
        ledcWrite(BACKLIGHT_PWM_CHANNEL, brightness);
        Serial.printf("Brilho diminuído para %d\n", brightness);
        lastPressTime = currentTime;
    }
}

/**
 * @brief Define manualmente o brilho do backlight.
 *
 * Ajusta o brilho para o valor especificado e atualiza o PWM.
 *
 * @param value Valor de brilho desejado (0 a 255).
 */
void setBacklightBrightness(uint8_t value) {
    brightness = constrain(value, 0, 255);
    ledcWrite(BACKLIGHT_PWM_CHANNEL, brightness);
    Serial.printf("Brilho definido para %d\n", brightness);
}

/**
 * @brief Retorna o brilho atual.
 *
 * @return Valor atual de brilho (0 a 255).
 */
uint8_t getBacklightBrightness() {
    return brightness;
}

/**
 * @brief Desenha a barra de brilho na tela TFT.
 *
 * Converte o valor de brilho para percentual e desenha a barra apenas se
 * houver alteração ou se for forçada a atualização.
 *
 * @param forceUpdate Se verdadeiro, força o redesenho da barra.
 */
void drawBrightnessBar(bool forceUpdate = false) {
    static int prevBrightnessPercentage = -1;
    int brightnessPercentage = round((brightness * 100.0) / 255.0);

    if (brightnessPercentage != prevBrightnessPercentage || forceUpdate) {
        prevBrightnessPercentage = brightnessPercentage;
        clearProgressBar(170, 173, 20, 132);
        drawProgressBar(170, 173, 20, 132, brightnessPercentage, true);
        tft.drawString("BRT", 172, 309);
    }
}

/**
 * @brief Loop de controle do backlight.
 *
 * Permite ao usuário ajustar o brilho utilizando os botões NEXT e PREV.
 * O loop é finalizado quando o botão BACK é pressionado, momento em que o
 * menu principal é redesenhado.
 */
void controlBacklight() {
    while (true) {
        int btnNextState = digitalRead(BTN_NEXT);
        int btnPrevState = digitalRead(BTN_PREV);
        int btnBackState = digitalRead(BTN_BACK);

        updateBacklight(btnNextState, btnPrevState);
        drawBrightnessBar();

        if (btnBackState == LOW) {
            break;
        }
        delay(50);
    }
    menuManager.drawMenu();
}
