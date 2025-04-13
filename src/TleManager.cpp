#include "TleManager.h"
#include "MenuManager.h"
#include "ProgressBar.h"
#include "Config.h"
#include "OrbitScoutWiFi.h"
#include "BacklightControl.h"
#include "gps.h"
#include <TimeLib.h>
#include "NotificationManager.h"
#include <HTTPClient.h>
#include "SPIFFS.h"  // Para uso do SPIFFS
#include "DisplayConstants.h"

// Declaração dos objetos globais utilizados
extern NotificationManager notificationManager;
extern MenuManager menuManager;         // Gerenciador de menus
extern SatelliteTracker tracker;        // Rastreador de satélites

//=============================================================================
// Função para desenhar a área exclusiva de atualização do TLE
//=============================================================================
static void drawTleUpdateArea() {
    Area tleUpdateArea = { TLE_UPDATE_X, TLE_UPDATE_Y,
                           TLE_UPDATE_WIDTH, TLE_UPDATE_HEIGHT,
                           TLE_UPDATE_HEADER_HEIGHT };
    // Utiliza o método estático de MenuManager para desenhar a área
    MenuManager::drawArea(tleUpdateArea, "TLE UPDATE", TFT_BLACK, TFT_WHITE, TFT_WHITE);
}

//=============================================================================
// Implementação da classe TleManager
//=============================================================================
TleManager::TleManager() {
    // Inicializações adicionais, se necessário
}

/**
 * Salva a data/hora da última atualização do TLE no SPIFFS.
 */
void TleManager::saveLastTleUpdateDate() {
    String currentDate = getCurrentDate();
    String currentTime = getCurrentTime();
    
    // Define o caminho do arquivo para salvar a data e hora
    const char* dateFilePath = "/last_update_date.txt";

    // Abre o arquivo para escrita (sobrescreve se existir)
    fs::File dateFile = SPIFFS.open(dateFilePath, FILE_WRITE);
    if (!dateFile) {
        Serial.printf("[saveLastTleUpdateDate] Erro ao abrir %s para escrita.\n", dateFilePath);
        return;
    }

    // Escreve a data e hora em linhas separadas
    dateFile.println(currentDate);
    dateFile.println(currentTime);
    dateFile.close();

    // Imprime no Serial Monitor
    Serial.printf("[saveLastTleUpdateDate] Data/hora da última atualização salva em %s:\nData: %s\nHora: %s\n", 
                  dateFilePath, currentDate.c_str(), currentTime.c_str());
}

/**
 * Carrega a data/hora da última atualização do TLE a partir do SPIFFS.
 */
DateTime TleManager::loadLastTleUpdateDate() {
    const char* dateFilePath = "/last_update_date.txt";
    DateTime updateDateTime = {"Date not available", "Time not available"};

    if (!SPIFFS.exists(dateFilePath)) {
        Serial.printf("[loadLastTleUpdateDate] Arquivo %s não encontrado.\n", dateFilePath);
        return updateDateTime;
    }

    File dateFile = SPIFFS.open(dateFilePath, FILE_READ);
    if (!dateFile) {
        Serial.printf("[loadLastTleUpdateDate] Erro ao abrir %s para leitura.\n", dateFilePath);
        return updateDateTime;
    }

    // Lê a data e hora em linhas separadas
    if (dateFile.available()) {
        updateDateTime.date = dateFile.readStringUntil('\n');
        updateDateTime.date.trim();
    }

    if (dateFile.available()) {
        updateDateTime.time = dateFile.readStringUntil('\n');
        updateDateTime.time.trim();
    }

    dateFile.close();
    Serial.printf("[loadLastTleUpdateDate] Data carregada: %s, hora: %s\n", 
                  updateDateTime.date.c_str(), updateDateTime.time.c_str());
    return updateDateTime;
}

/**
 * Faz o download de um arquivo exibindo uma barra de progresso e percentual,
 * utilizando a área exclusiva de atualização do TLE.
 */
bool TleManager::downloadFileWithProgress(const char* url, const char* filePath) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        int totalSize = http.getSize();
        Serial.printf("[downloadFileWithProgress] Tamanho do arquivo: %d bytes\n", totalSize);

        fs::File file = SPIFFS.open(filePath, FILE_WRITE);
        if (!file) {
            Serial.println("[downloadFileWithProgress] Erro ao abrir o arquivo para escrita.");
            http.end();
            return false;
        }

        WiFiClient *stream = http.getStreamPtr();
        int downloaded = 0;
        uint8_t buff[128];

        // Limpa a área da barra de progresso dentro da área de atualização do TLE
        clearProgressBar(PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT);
        tft.fillRect(PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, TFT_BLACK);

        while (http.connected() && (downloaded < totalSize || totalSize == -1)) {
            size_t sizeAvailable = stream->available();
            if (sizeAvailable) {
                int chunkSize = stream->readBytes(buff, (sizeAvailable > sizeof(buff)) ? sizeof(buff) : sizeAvailable);
                file.write(buff, chunkSize);
                downloaded += chunkSize;
                if (totalSize > 0) {
                    int progress = (downloaded * 100) / totalSize;
                    // Atualiza a barra de progresso na área de atualização do TLE
                    drawProgressBar(PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, progress, false);
                }
            }
            delay(10); // Pequeno delay para suavizar a atualização
        }

        file.close();
        http.end();
        Serial.println("[downloadFileWithProgress] Download concluído.");
        return true;
    } else {
        Serial.printf("[downloadFileWithProgress] Erro no download. Código HTTP: %d\n", httpCode);
        http.end();
        return false;
    }
}

//=============================================================================
// Carrega os dados TLE a partir de uma URL específica
//=============================================================================
void TleManager::loadTleFromUrl(const char* tleUrl, const char* filePath) {
    // Verifica se o arquivo já existe no SPIFFS
    if (SPIFFS.exists(filePath)) {
        Serial.printf("[loadTleFromUrl] Arquivo %s já existe. Pulando download...\n", filePath);
        if (tracker.loadTLEFile(filePath)) {
            tracker.trackSatellite();
        }
        return;
    }

    // Se o Wi-Fi estiver conectado, tenta fazer o download do arquivo
    if (WiFi.status() == WL_CONNECTED) {
        bool ok = downloadFileWithProgress(tleUrl, filePath);
        if (ok) {
            if (tracker.loadTLEFile(filePath)) {
                Serial.printf("[loadTleFromUrl] TLE baixado e carregado a partir de %s\n", filePath);
                tracker.trackSatellite();
            }
        } else {
            Serial.println("[loadTleFromUrl] Download com barra de progresso falhou.");
        }
    } else {
        Serial.println("[loadTleFromUrl] Wi-Fi não está conectado.");
    }
}

void TleManager::selectTleSource() {
    Serial.println("[selectTleSource] Entrando no menu de seleção de fonte TLE.");
    tft.fillScreen(TFT_BLACK);
    delay(200);
    
    static int selectedUrlIndex = 0; // Índice atualmente selecionado
    static int topMenuIndex = 0;     // Índice do topo da lista visível
    const int maxVisibleItems = 19;  // Número máximo de itens visíveis
    int previousUrlIndex = -1;       // Para detectar mudanças na seleção

    while (true) {
        // Atualiza a tela somente se a seleção tiver mudado
        if (previousUrlIndex != selectedUrlIndex) {
            // Desenha a área do menu com cabeçalho
            Area menuArea = { MENU_X, MENU_Y,
                              MENU_WIDTH, MENU_HEIGHT,
                              MENU_HEADER_HEIGHT };
            MenuManager::drawArea(menuArea, "SELECT TLE SOURCE", TFT_BLACK, TFT_WHITE, TFT_WHITE);
            
            // Limpa a área interna para evitar sobreposição de nomes
            tft.fillRect(MENU_X + 1, 
                         MENU_Y + MENU_HEADER_HEIGHT + 1,
                         MENU_WIDTH - 2,
                         MENU_HEIGHT - MENU_HEADER_HEIGHT - 2, 
                         TFT_BLACK);

            // Desenha a lista de fontes TLE
            tft.setTextFont(2);
            int lastIndex = topMenuIndex + maxVisibleItems - 1;
            if (lastIndex >= TLE_SOURCE_COUNT) {
                lastIndex = TLE_SOURCE_COUNT - 1;
            }
            int posY = MENU_Y + MENU_HEADER_HEIGHT + 5;
            for (int i = topMenuIndex; i <= lastIndex; i++) {
                if (i >= TLE_SOURCE_COUNT) break;
                if (i == selectedUrlIndex) {
                    tft.setTextColor(TFT_BLACK, TFT_WHITE);
                } else {
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                }
                String sourceName = String(tleSources[i].group);
                tft.drawString(sourceName, MENU_X + 5, posY);
                posY += MENU_ITEM_SPACING;
            }
            previousUrlIndex = selectedUrlIndex;
            Serial.printf("[selectTleSource] Índice selecionado: %d\n", selectedUrlIndex);
        }

        // Navegação pelos botões
        if (digitalRead(BTN_NEXT) == LOW) {
            selectedUrlIndex = (selectedUrlIndex + 1) % TLE_SOURCE_COUNT;
            // Se voltar para o início, mostra o topo da lista
            if (selectedUrlIndex == 0) {
                topMenuIndex = 0;
            } else if (selectedUrlIndex > (topMenuIndex + maxVisibleItems - 1)) {
                topMenuIndex = selectedUrlIndex - (maxVisibleItems - 1);
            }
            delay(200);
        }
        else if (digitalRead(BTN_PREV) == LOW) {
            selectedUrlIndex = (selectedUrlIndex - 1 + TLE_SOURCE_COUNT) % TLE_SOURCE_COUNT;
            // Se voltar para o final, ajusta o topo para mostrar os últimos itens
            if (selectedUrlIndex == TLE_SOURCE_COUNT - 1) {
                if(TLE_SOURCE_COUNT > maxVisibleItems) {
                    topMenuIndex = TLE_SOURCE_COUNT - maxVisibleItems;
                } else {
                    topMenuIndex = 0;
                }
            } else if (selectedUrlIndex < topMenuIndex) {
                topMenuIndex = selectedUrlIndex;
            }
            delay(200);
        }
        else if (digitalRead(BTN_SELECT) == LOW) {
            Serial.printf("[selectTleSource] Fonte selecionada no índice: %d\n", selectedUrlIndex);
            tft.fillScreen(TFT_BLACK);
            loadTleFromUrl(tleSources[selectedUrlIndex].url, tleSources[selectedUrlIndex].fileName);
            break;
        }
        else if (digitalRead(BTN_BACK) == LOW) {
            Serial.println("[selectTleSource] Saindo do menu de seleção de fonte TLE.");
            tft.fillScreen(TFT_BLACK);
            delay(200);
            menuManager.back();
            break;
        }
        delay(50);
    }
}

//=============================================================================
// Força a atualização de todos os TLEs utilizando a área exclusiva de atualização
//=============================================================================
void TleManager::forceUpdateAllTle() {
    Serial.println("[forceUpdateAllTle] Iniciando a atualização de TODOS os TLEs...");

    setupWiFi();

    // Se, após setupWiFi, o Wi-Fi não estiver conectado, cancela a operação
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[forceUpdateAllTle] Conexão Wi-Fi não estabelecida. Cancelando atualização do TLE.");
        menuManager.drawMenu();
        return;
    }

    tft.fillRect(TLE_UPDATE_X - 2, TLE_UPDATE_Y - 2,
                 TLE_UPDATE_WIDTH + 4, TLE_UPDATE_HEIGHT + 4, TFT_BLACK);

    drawTleUpdateArea();
    
    for (int i = 0; i < TLE_SOURCE_COUNT; i++) {
        tft.setTextFont(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        
        // Obtém a string do grupo
        String groupStr = tleSources[i].group;
        // Calcula a posição centralizada do texto dentro da área definida
        int16_t textWidth = tft.textWidth(groupStr);
        int16_t xCentered = TLE_UPDATE_X + ((TLE_UPDATE_WIDTH - textWidth) / 2);
        
        // Limpa a área onde o texto será desenhado para evitar sobreposição
        tft.fillRect(TLE_UPDATE_X + 5, 
                     TLE_UPDATE_Y + TLE_UPDATE_HEADER_HEIGHT, 
                     TLE_UPDATE_WIDTH - 10, 25, TFT_BLACK);
        
        // Desenha a string centralizada
        tft.drawString(groupStr, xCentered, TLE_UPDATE_Y + TLE_UPDATE_HEADER_HEIGHT + 5);

        const char* url = tleSources[i].url;
        const char* filePath = tleSources[i].fileName;

        // Remove o arquivo existente para forçar o novo download
        if (SPIFFS.exists(filePath)) {
            SPIFFS.remove(filePath);
            Serial.printf("[forceUpdateAllTle] Arquivo %s removido\n", filePath);
        }

        bool ok = downloadFileWithProgress(url, filePath);

        if (ok) {
            if (tracker.loadTLEFile(filePath)) {
                Serial.printf("[forceUpdateAllTle] TLE baixado e carregado a partir de %s\n", filePath);
            } else {
                Serial.printf("[forceUpdateAllTle] Falha ao carregar TLE de %s\n", filePath);
                String errorMsg = "Failed to load";
                int16_t errWidth = tft.textWidth(errorMsg);
                int16_t xCenteredErr = TLE_UPDATE_X + ((TLE_UPDATE_WIDTH - errWidth) / 2);
                // Limpa a área para evitar sobreposição
                tft.fillRect(TLE_UPDATE_X + 5,
                             TLE_UPDATE_Y + TLE_UPDATE_HEADER_HEIGHT + 50,
                             TLE_UPDATE_WIDTH - 10, 20, TFT_BLACK);
                tft.drawString(errorMsg, xCenteredErr, TLE_UPDATE_Y + TLE_UPDATE_HEADER_HEIGHT + 50);
            }
        } else {
            Serial.printf("[forceUpdateAllTle] Falha no download do TLE de %s\n", url);
            String errorMsg = "Download failed";
            int16_t errWidth = tft.textWidth(errorMsg);
            int16_t xCenteredErr = TLE_UPDATE_X + ((TLE_UPDATE_WIDTH - errWidth) / 2);
            // Limpa a área para evitar sobreposição
            tft.fillRect(TLE_UPDATE_X + 5,
                         TLE_UPDATE_Y + TLE_UPDATE_HEADER_HEIGHT + 30,
                         TLE_UPDATE_WIDTH - 10, 20, TFT_BLACK);
            tft.drawString(errorMsg, xCenteredErr, TLE_UPDATE_Y + TLE_UPDATE_HEADER_HEIGHT + 30);
        }

        delay(1000);
    }

    // Após atualizar todos os TLEs, salva a data da atualização
    saveLastTleUpdateDate();

    menuManager.back();
    Serial.println("[forceUpdateAllTle] Atualização de TODOS os TLEs com barra de progresso concluída.");
}
