#ifndef BACKLIGHTCONTROL_H
#define BACKLIGHTCONTROL_H

#include <Arduino.h>
#include <TFT_eSPI.h>

/**
 * @brief Inicializa o backlight utilizando PWM.
 *
 * Configura o canal PWM para o pino informado e define o brilho inicial.
 *
 * @param pin Pino de controle do backlight.
 */
void initializeBacklight(uint8_t pin);

/**
 * @brief Atualiza o brilho do backlight com base nos botões NEXT e PREV.
 *
 * Esta função deve ser chamada periodicamente para ajustar o brilho.
 *
 * @param btnNextState Estado atual do botão NEXT.
 * @param btnPrevState Estado atual do botão PREV.
 */
void updateBacklight(int btnNextState, int btnPrevState);

/**
 * @brief Define manualmente o brilho do backlight.
 *
 * @param value Valor de brilho (0–255).
 */
void setBacklightBrightness(uint8_t value);

/**
 * @brief Retorna o valor atual do brilho do backlight.
 *
 * @return Valor de brilho (0–255).
 */
uint8_t getBacklightBrightness();

/**
 * @brief Desenha ou atualiza a barra de brilho na tela TFT.
 *
 * Converte o valor de brilho para percentual (0–100) e desenha a barra
 * apenas se o valor tiver mudado ou se for solicitada uma atualização forçada.
 *
 * @param forceUpdate Se verdadeiro, força a atualização da barra.
 */
void drawBrightnessBar(bool forceUpdate);

/**
 * @brief Modo de controle do backlight.
 *
 * Permite ao usuário ajustar o brilho utilizando os botões NEXT e PREV.
 * O loop é encerrado quando o botão BACK é pressionado, momento em que o menu
 * principal é redesenhado.
 */
void controlBacklight();

#endif // BACKLIGHTCONTROL_H
