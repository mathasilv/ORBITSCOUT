#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include "gps.h"
#include "ProgressBar.h"
#include "BatteryMonitor.h"
#include "OrbitScoutWiFi.h"
#include "TleManager.h"
#include "SatelliteTracker.h"

/**
 * @brief Estrutura que representa a data/hora da última atualização do TLE.
 */
struct UpdateDateTime {
    String date;
    String time;
};

/**
 * @brief Estrutura para representar uma área da interface.
 */
struct Area {
    int x;
    int y;
    int width;
    int height;
    int headerHeight;
};

/**
 * @brief Representa um item de menu.
 *
 * Cada item possui um rótulo e uma função de callback que será executada ao ser selecionado.
 */
struct MenuItem {
    String label;                   ///< Título do item
    std::function<void()> callback; ///< Função de callback (opcional)
};

/**
 * @brief Classe MenuManager.
 *
 * Gerencia a interface do menu e atualizações dinâmicas (como GNSS, bateria e rede).
 */
class MenuManager {
public:
    /// Construtor padrão.
    MenuManager();

    /**
     * @brief Adiciona um item ao menu.
     * @param label Rótulo do item.
     * @param callback Função de callback que será executada quando o item for selecionado.
     */
    void addMenuItem(const String& label, std::function<void()> callback = nullptr);

    /// Redesenha o menu e reinicia a seleção.
    void drawMenu();

    /// Move a seleção para cima.
    void moveUp();

    /// Move a seleção para baixo.
    void moveDown();

    /// Executa a ação do item selecionado.
    void selectItem();

    /// Volta (geralmente redesenha o menu principal).
    void back();

    /// Atualiza os dados dinâmicos do menu (GNSS, bateria, rede, etc.).
    void gnssData(bool forceUpdate = false);
    void update();

    /**
     * @brief Desenha uma área com cabeçalho.
     *
     * @param area Estrutura que define a posição, tamanho e altura do cabeçalho.
     * @param headerText Texto a ser exibido no cabeçalho.
     * @param headerTextColor Cor do texto do cabeçalho (padrão: TFT_BLACK).
     * @param headerBgColor Cor de fundo do cabeçalho (padrão: TFT_WHITE).
     * @param borderColor Cor da borda da área (padrão: TFT_WHITE).
     */
    static void drawArea(const Area& area,
                         const String& headerText,
                         int headerTextColor = TFT_BLACK,
                         int headerBgColor = TFT_WHITE,
                         int borderColor = TFT_WHITE);

private:
    std::vector<MenuItem> _menu; ///< Vetor de ite MenuManager::drawAreans do menu.
    size_t _currentIndex;        ///< Índice do item atualmente selecionado.

    // Controle de atualização da bateria
    unsigned long _lastBatteryUpdate = 0;
    const unsigned long _batteryUpdateInterval = 5000; // 5 segundos
    int _prevBatteryPercentage = -1; // Para evitar redesenhos desnecessários

    /**
     * @brief Redesenha toda a interface estática do menu (cabeçalho, áreas, etc.).
     */
    void refreshDisplay();

    /**
     * @brief Atualiza a barra de bateria (com atualização forçada se necessário).
     * @param forceUpdate Se true, força a atualização mesmo que o valor não tenha mudado.
     */
    void monitorBatteryBar(bool forceUpdate = false);
};

#endif // MENU_MANAGER_H
