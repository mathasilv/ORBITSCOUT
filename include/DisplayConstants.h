#ifndef DISPLAYCONSTANTS_H
#define DISPLAYCONSTANTS_H

// Área exclusiva para atualização do TLE
static constexpr int TLE_UPDATE_X = 20;
static constexpr int TLE_UPDATE_Y = 120;
static constexpr int TLE_UPDATE_WIDTH = 200;
static constexpr int TLE_UPDATE_HEIGHT = 80;
static constexpr int TLE_UPDATE_HEADER_HEIGHT = 20;

// Área exclusiva para notificações
static constexpr int NOTIFICATION_X = 20;
static constexpr int NOTIFICATION_Y = 120;
static constexpr int NOTIFICATION_WIDTH = 200;
static constexpr int NOTIFICATION_HEIGHT = 80;
static constexpr int NOTIFICATION_HEADER_HEIGHT = 20;

// Área padrão para os menus (seleção de fonte TLE, seleção de satélite, etc.)
static constexpr int MENU_X = 5;
static constexpr int MENU_Y = 5;
static constexpr int MENU_WIDTH = 230;
static constexpr int MENU_HEIGHT = 315;
static constexpr int MENU_HEADER_HEIGHT = 20;

// Outras constantes para espaçamentos e posições
static constexpr int MENU_ITEM_SPACING = 15;
static constexpr int PROGRESS_BAR_X = 30;
static constexpr int PROGRESS_BAR_Y = 170;
static constexpr int PROGRESS_BAR_WIDTH = 180;
static constexpr int PROGRESS_BAR_HEIGHT = 20;

// ----------------------------------------------------------
// Constantes para a barra de RSSI e limites do sinal
// ----------------------------------------------------------
constexpr int RSSI_BAR_X      = 207;
constexpr int RSSI_BAR_Y      = 173;
constexpr int RSSI_BAR_WIDTH  = 20;
constexpr int RSSI_BAR_HEIGHT = 132;

constexpr int RSSI_MIN = -90;  // Sinal muito fraco
constexpr int RSSI_MAX = -30;  // Sinal muito forte


// ----------------------------------------------------------
// Novas constantes para a área de instruções de conexão
// ----------------------------------------------------------
constexpr int INSTRUCTIONS_AREA_X             = 20;
constexpr int INSTRUCTIONS_AREA_Y             = 120;
constexpr int INSTRUCTIONS_AREA_WIDTH         = 200;
constexpr int INSTRUCTIONS_AREA_HEIGHT        = 80;
constexpr int INSTRUCTIONS_AREA_HEADER_HEIGHT = 20;


//==============================================================================
// Espaço de nomes com constantes para o layout do menu
//==============================================================================

  // Cabeçalho
  constexpr int HEADER_X = 5;
  constexpr int HEADER_Y = 5;
  constexpr int HEADER_WIDTH = 230;
  constexpr int HEADER_HEIGHT = 20;
  
  // Área do Menu
static constexpr int MAIN_MENU_X = 5;
static constexpr int MAIN_MENU_Y = 30;
static constexpr int MAIN_MENU_WIDTH = 110;
static constexpr int MAIN_MENU_HEIGHT = 110;
static constexpr int MAIN_MENU_HEADER_HEIGHT = 20;

  
  // Área GNSS
  constexpr int GNSS_X = 125;
  constexpr int GNSS_Y = 30;
  constexpr int GNSS_WIDTH = 110;
  constexpr int GNSS_HEIGHT = 110;
  constexpr int GNSS_HEADER_HEIGHT = 20;
  
  // Área de Sistema (SYS INFO)
  constexpr int SYS_X = 125;
  constexpr int SYS_Y = 146;
  constexpr int SYS_WIDTH = 110;
  constexpr int SYS_HEIGHT = 174;
  constexpr int SYS_HEADER_HEIGHT = 20;
  
  // Área de TLE (TLE INFO)
  constexpr int TLE_X = 5;
  constexpr int TLE_Y = 146;
  constexpr int TLE_WIDTH = 110;
  constexpr int TLE_HEIGHT = 58;
  constexpr int TLE_HEADER_HEIGHT = 20;
  
  // Área de Alertas (ALERTS)
  constexpr int ALERTS_X = 5;
  constexpr int ALERTS_Y = 210;
  constexpr int ALERTS_WIDTH = 110;
  constexpr int ALERTS_HEIGHT = 110;
  constexpr int ALERTS_HEADER_HEIGHT = 20;
  
#endif // DISPLAYCONSTANTS_H
