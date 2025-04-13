#include "MenuManager.h"
#include "BatteryMonitor.h"
#include "BacklightControl.h"
#include "OrbitScoutWiFi.h"
#include "Config.h"
#include <TimeLib.h>
#include "TleManager.h"
#include "SatelliteTracker.h"
#include "NotificationManager.h"
#include "ProgressBar.h"
#include "DisplayConstants.h"

// Objetos externos definidos em outros módulos
extern BatteryMonitor battery;
extern TleManager tleManager;
extern SatelliteTracker tracker;
extern NotificationManager notificationManager;
extern TFT_eSPI tft; // Objeto do display

//==================================================================
// Implementação do método público estático drawArea
//==================================================================
void MenuManager::drawArea(const Area& area,
                           const String& headerText,
                           int headerTextColor,
                           int headerBgColor,
                           int borderColor) {
    // Desenha a borda da área
    tft.drawRect(area.x, area.y, area.width, area.height, borderColor);
    // Preenche o cabeçalho com a cor de fundo
    tft.fillRect(area.x, area.y, area.width, area.headerHeight, headerBgColor);
    // Configura a fonte usando setFreeFont (note que FreeMonoBold9pt7b já é um ponteiro)
    tft.setFreeFont(&FreeMonoBold9pt7b);
    tft.setTextColor(headerTextColor, headerBgColor);
    // Calcula a largura do texto e o posiciona centralizado no cabeçalho
    int16_t textWidth = tft.textWidth(headerText);
    int16_t textHeight = tft.fontHeight();
    int16_t xText = area.x + (area.width - textWidth) / 2;
    int16_t yText = area.y + (area.headerHeight - textHeight) / 2;
    tft.drawString(headerText, xText, yText);
}

/**
 * @brief Função auxiliar para desenhar o cabeçalho principal.
 */
static void drawHeader() {
    // Define a área para o cabeçalho
    Area headerArea = { HEADER_X, HEADER_Y,
                        HEADER_WIDTH, HEADER_HEIGHT, HEADER_HEIGHT };
    // Como drawArea é estático, pode ser chamado diretamente pela classe:
    MenuManager::drawArea(headerArea, "ORBITSCOUT", TFT_BLACK, TFT_WHITE, TFT_WHITE);
}

/**
 * @brief Função auxiliar para desenhar a área do menu e listar os itens.
 */
static void drawMenuArea(const std::vector<MenuItem>& menu, size_t currentIndex) {
    tft.setFreeFont(&FreeMonoBold9pt7b);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);

    // Contorno e cabeçalho do menu
    tft.drawRect(MAIN_MENU_X, MAIN_MENU_Y,
                 MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, TFT_WHITE);
    tft.fillRect(MAIN_MENU_X, MAIN_MENU_Y,
                 MAIN_MENU_WIDTH, MAIN_MENU_HEADER_HEIGHT, TFT_WHITE);
    String menu_header = "MENU";
    int16_t menuWidth = tft.textWidth(menu_header);
    int16_t menuHeight = tft.fontHeight();
    int16_t xMenu = MAIN_MENU_X + (MAIN_MENU_WIDTH - menuWidth) / 2;
    int16_t yMenu = MAIN_MENU_Y + (MAIN_MENU_HEADER_HEIGHT - menuHeight) / 2;
    tft.drawString(menu_header, xMenu, yMenu);
    
    // Lista de itens do menu
    tft.setTextFont(2);
    int posY = MAIN_MENU_Y + 25; // um pouco abaixo do cabeçalho
    int incY = 20;
    for (size_t i = 0; i < menu.size(); i++) {
        if (i == currentIndex) {
            tft.setTextColor(TFT_BLACK, TFT_WHITE);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        tft.drawString(menu[i].label, MAIN_MENU_X + 5, posY + i * incY);
    }
}

/**
 * @brief Função auxiliar para desenhar a área de dados GNSS.
 */
static void drawGNSSArea() {
    // Define a área de GNSS
    Area gnssArea = { GNSS_X, GNSS_Y,
                      GNSS_WIDTH, GNSS_HEIGHT, GNSS_HEADER_HEIGHT };
    MenuManager::drawArea(gnssArea, "GNSS DATA", TFT_BLACK, TFT_WHITE, TFT_WHITE);

    // Desenha os dados abaixo do cabeçalho
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    int offsetY = GNSS_Y + GNSS_HEADER_HEIGHT + 5;

}

/**
 * @brief Função auxiliar para desenhar a área de informações do sistema (bateria, rede).
 */
static void drawSystemInfoArea() {
    tft.setFreeFont(&FreeMonoBold9pt7b);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    
    tft.drawRect(SYS_X, SYS_Y,
                 SYS_WIDTH, SYS_HEIGHT, TFT_WHITE);
    tft.fillRect(SYS_X, SYS_Y,
                 SYS_WIDTH, SYS_HEADER_HEIGHT, TFT_WHITE);
    String sys_header = "SYS INFO";
    int16_t sysWidth = tft.textWidth(sys_header);
    int16_t sysHeight = tft.fontHeight();
    int16_t xSys = SYS_X + (SYS_WIDTH - sysWidth) / 2;
    int16_t ySys = SYS_Y + (SYS_HEADER_HEIGHT - sysHeight) / 2;
    tft.drawString(sys_header, xSys, ySys);
}

/**
 * @brief Função auxiliar para desenhar a área de informações de TLE.
 */
static void drawTLEArea() {
    // Define a área de TLE
    Area tleArea = { TLE_X, TLE_Y,
                     TLE_WIDTH, TLE_HEIGHT, TLE_HEADER_HEIGHT };
    MenuManager::drawArea(tleArea, "TLE INFO", TFT_BLACK, TFT_WHITE, TFT_WHITE);

    // Exibe as informações da última atualização dos TLEs logo abaixo do cabeçalho
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    int offsetY = TLE_Y + TLE_HEADER_HEIGHT + 5;
    tft.drawString("Last TLE Update:", TLE_X + 5, offsetY);

    UpdateDateTime lastUpdate = {tleManager.loadLastTleUpdateDate().date,
                                 tleManager.loadLastTleUpdateDate().time};
    tft.drawString("Date: " + lastUpdate.date, TLE_X + 5, offsetY + 10);
    tft.drawString("Time: " + lastUpdate.time, TLE_X + 5, offsetY + 20);
}

/**
 * @brief Função auxiliar para desenhar a área de alertas.
 */
static void drawAlertsArea() {
    // Define a área de Alertas
    Area alertsArea = { ALERTS_X, ALERTS_Y,
                        ALERTS_WIDTH, ALERTS_HEIGHT, ALERTS_HEADER_HEIGHT };
    MenuManager::drawArea(alertsArea, "ALERTS", TFT_BLACK, TFT_WHITE, TFT_WHITE);
}

//
// Implementação dos métodos públicos da classe MenuManager
//

MenuManager::MenuManager()
  : _currentIndex(0),
    _lastBatteryUpdate(0),
    _prevBatteryPercentage(-1)
{
    // Limpa o display e desenha a interface estática
    tft.fillScreen(TFT_BLACK);
    refreshDisplay();
}

void MenuManager::addMenuItem(const String& label, std::function<void()> callback) {
    MenuItem item{label, callback};
    _menu.push_back(item);
}

void MenuManager::drawMenu() {
    _currentIndex = 0;
    refreshDisplay();
}

void MenuManager::moveUp() {
    if (_menu.empty()) return;
    _currentIndex = (_currentIndex == 0) ? _menu.size() - 1 : _currentIndex - 1;
    drawMenuArea(_menu, _currentIndex);
}

void MenuManager::moveDown() {
    if (_menu.empty()) return;
    _currentIndex = (_currentIndex + 1) % _menu.size();
    drawMenuArea(_menu, _currentIndex);
}

void MenuManager::selectItem() {
    if (_currentIndex < _menu.size() && _menu[_currentIndex].callback) {
        _menu[_currentIndex].callback();
    }
}

void MenuManager::back() {
    drawMenu();
}

//
// Atualiza os dados estáticos da área GNSS
//
void MenuManager::gnssData(bool forceUpdate) {


    String currentDate = getCurrentDate();
    String currentTime = getCurrentTime();
    float currentLatitude = getCurrentLatitude();
    float currentLongitude = getCurrentLongitude();
    int currentAltitude = getCurrentAltitude();
    int currentSatellites = getCurrentSatellites();
    float currentHDOP = getCurrentHDOP();

    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.drawString(currentDate, 130, 55);
    tft.drawString(currentTime, 130, 65);
    tft.drawString("Lat: " + String(currentLatitude, 6), 130, 75);
    tft.drawString("Lng: " + String(currentLongitude, 6), 130, 85);
    tft.drawString("Alt: " + String(currentAltitude) + " m", 130, 95);
    tft.drawString("Gnss Sats: " + String(currentSatellites), 130, 105);
    tft.drawString("HDOP: " + String(currentHDOP), 130, 115);
}

//
// Atualiza os dados dinâmicos (bateria e rede)
//
void MenuManager::monitorBatteryBar(bool forceUpdate) {
    battery.update();
    float currentBatteryFloat = battery.getPercentage();
    int currentBatteryPercentage = static_cast<int>(round(currentBatteryFloat));

    if (currentBatteryPercentage != _prevBatteryPercentage || forceUpdate) {
        _prevBatteryPercentage = currentBatteryPercentage;
        tft.setTextFont(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("BAT", 134, 309);
        clearProgressBar(133, 173, 20, 132);
        drawProgressBar(133, 173, 20, 132, currentBatteryPercentage, true);
    }
}

//
// Atualiza a interface dinâmica do menu (bateria, rede, GNSS)
//
void MenuManager::update() {
    unsigned long currentMillis = millis();

    if (currentMillis - _lastBatteryUpdate >= _batteryUpdateInterval) {
        _lastBatteryUpdate = currentMillis;
        monitorBatteryBar();  // Atualiza a barra de bateria
        drawRSSIBar(false);   // Atualiza a barra de rede, se implementada
    }
    gnssData(true);
}

//
// Redesenha toda a interface estática do menu
//
void MenuManager::refreshDisplay() {
    tft.fillScreen(TFT_BLACK);
    drawHeader();
    drawMenuArea(_menu, _currentIndex);
    drawGNSSArea();
    drawSystemInfoArea();
    drawTLEArea();
    drawAlertsArea();

    // Atualiza os dados estáticos
    monitorBatteryBar(true);
    drawBrightnessBar(true);
    drawRSSIBar(true);

    notificationManager.showNotificationInfo();
}