#include "ProgressBar.h"

// Objeto global do display (deve ser definido na main ou em outro módulo central)
extern TFT_eSPI tft;

/**
 * @brief Desenha uma barra de progresso na tela TFT.
 *
 * Essa função garante que o valor de progresso esteja entre 0 e 100 e,
 * dependendo do parâmetro "vertical", desenha a barra na orientação desejada.
 *
 * @param x Posição X do canto superior esquerdo da barra.
 * @param y Posição Y do canto superior esquerdo da barra.
 * @param w Largura total da barra.
 * @param h Altura total da barra.
 * @param progress Percentual de progresso (0 a 100).
 * @param vertical Se true, desenha a barra verticalmente (de baixo para cima); se false, horizontalmente (da esquerda para a direita).
 */
void drawProgressBar(int x, int y, int w, int h, int progress, bool vertical) {
    // Limita o valor de progress entre 0 e 100
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;

    if (!vertical) {
        // Barra horizontal: calcula a largura preenchida proporcionalmente
        int filledWidth = (w * progress) / 100;
        // Desenha a área preenchida com a cor branca
        tft.fillRect(x, y, filledWidth, h, TFT_WHITE);
    } else {
        // Barra vertical: calcula a altura preenchida proporcionalmente
        int filledHeight = (h * progress) / 100;
        // Desenha a barra preenchendo de baixo para cima
        int startY = y + (h - filledHeight);
        tft.fillRect(x, startY, w, filledHeight, TFT_WHITE);
    }

    // Desenha a borda da barra para realçar a área de progresso
    tft.drawRect(x - 2, y - 2, w + 4, h + 4, TFT_WHITE);
}

/**
 * @brief Limpa a área onde a barra de progresso foi desenhada.
 *
 * Essa função preenche a área especificada com a cor preta.
 *
 * @param x Posição X do canto superior esquerdo da área.
 * @param y Posição Y do canto superior esquerdo da área.
 * @param w Largura da área.
 * @param h Altura da área.
 */
void clearProgressBar(int x, int y, int w, int h) {
    tft.fillRect(x, y, w, h, TFT_BLACK);
}
