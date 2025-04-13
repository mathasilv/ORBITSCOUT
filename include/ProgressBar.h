#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <TFT_eSPI.h>

/**
 * @brief Desenha uma barra de progresso na tela TFT.
 * 
 * @param x Posição X do canto superior esquerdo da barra.
 * @param y Posição Y do canto superior esquerdo da barra.
 * @param w Largura total da barra.
 * @param h Altura total da barra.
 * @param progress Percentual de progresso (0 a 100).
 * @param vertical Se true, desenha a barra na vertical (de baixo para cima); se false, na horizontal (da esquerda para a direita).
 */
void drawProgressBar(int x, int y, int w, int h, int progress, bool vertical);

/**
 * @brief Limpa a área onde a barra de progresso foi desenhada.
 * 
 * @param x Posição X do canto superior esquerdo da área.
 * @param y Posição Y do canto superior esquerdo da área.
 * @param w Largura da área.
 * @param h Altura da área.
 */
void clearProgressBar(int x, int y, int w, int h);

#endif // PROGRESSBAR_H
