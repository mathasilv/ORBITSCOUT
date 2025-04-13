#include "SatelliteTracker.h"
#include <TimeLib.h>
#include <math.h>
#include "Config.h"
#include "gps.h"
#include "OrientationManager.h"
#include "NotificationManager.h"
#include "MenuManager.h"
#include "DisplayConstants.h"

// Constantes para conversão de tempo
static constexpr double JD_UNIX_EPOCH   = 2440587.5;
static constexpr double SECONDS_PER_DAY = 86400.0;

// Note que as constantes referentes à barra de progresso foram removidas pois não são utilizadas neste arquivo.

extern OrientationManager orientationManager;  // Certifique-se de declarar essa instância global
extern NotificationManager notificationManager;
extern MenuManager menuManager;         // Gerenciador de menus

//
// Construtor da classe SatelliteTracker
//
SatelliteTracker::SatelliteTracker()
    : currentSatelliteIndex(-1),
      currentUnixTime(0)
{
    // Inicializa o pino do buzzer e garante que esteja desligado
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Outras inicializações podem ser adicionadas aqui
}

//=============================================================================
// Função para desenhar a área exclusiva de notificações
//=============================================================================
static void drawNotificationArea() {
    Area notificationArea = { NOTIFICATION_X, NOTIFICATION_Y,
                              NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT,
                              NOTIFICATION_HEADER_HEIGHT };
    // Utiliza o método estático de MenuManager para desenhar a área
    MenuManager::drawArea(notificationArea, "NOTIFICATION", TFT_BLACK, TFT_WHITE, TFT_WHITE);
}

//
// Desenha a passagem em um gráfico polar
//
void SatelliteTracker::drawPassPolar(int centerX, int centerY, int radius, const PassData &pass) {
    // Define a fonte para textos (ajuste conforme necessário)
    tft.setTextFont(1);

    // 1) Desenha o círculo principal que representa o horizonte
    tft.drawCircle(centerX, centerY, radius, TFT_WHITE);

    // 2) Desenha círculos de referência para 30° e 60° de elevação
    int r30 = static_cast<int>(radius * (1 - 30.0 / 90.0));
    int r60 = static_cast<int>(radius * (1 - 60.0 / 90.0));
    tft.drawCircle(centerX, centerY, r30, TFT_WHITE);
    tft.drawCircle(centerX, centerY, r60, TFT_WHITE);

    // 3) Desenha as linhas N-S e E-W e os rótulos
    tft.drawLine(centerX, centerY - radius, centerX, centerY + radius, TFT_WHITE); // N-S
    tft.drawLine(centerX - radius, centerY, centerX + radius, centerY, TFT_WHITE); // E-W

    tft.drawString("N", centerX - 2, centerY - radius - 10);
    tft.drawString("S", centerX - 2, centerY + radius + 5);
    tft.drawString("E", centerX + radius + 5, centerY - 3);
    tft.drawString("W", centerX - radius - 10, centerY - 3);

    // 4) Desenha a trajetória do satélite
    int prevX = -1, prevY = -1;
    for (size_t j = 0; j < pass.path.size(); j++) {
        double az = pass.path[j].azimuth;
        double el = pass.path[j].elevation;
        // Mapeia a elevação para um raio proporcional
        double r = (1.0 - (el / 90.0)) * radius;
        // Converte a direção (azimute) em ângulo em radianos, ajustando para coordenadas polares
        double thetaRad = (90.0 - az) * PI / 180.0;

        int xPos = centerX + static_cast<int>(r * cos(thetaRad));
        int yPos = centerY - static_cast<int>(r * sin(thetaRad));

        // Desenha um ponto na posição calculada
        tft.drawPixel(xPos, yPos, TFT_WHITE);

        // Se houver um ponto anterior, liga-os com uma linha
        if (j > 0) {
            tft.drawLine(prevX, prevY, xPos, yPos, TFT_WHITE);
        }
        prevX = xPos;
        prevY = yPos;
    }
    orientationManager.drawOrientationPointer(centerX, centerY, radius);
    // orientationManager.printOrientation();
}

//
// Desenha o marcador atual do satélite em tempo real
//
void SatelliteTracker::drawCurrentSatMarker(int centerX, int centerY, int radius,
                                              double az, double el, SatelliteMarkerState &markerState) 
{
    // Converte (az, el) para coordenadas polares
    double r = (1.0 - (el / 90.0)) * radius;  
    double thetaDeg = 90.0 - az;              
    double thetaRad = thetaDeg * PI / 180.0;  

    int xPos = centerX + static_cast<int>(r * cos(thetaRad));
    int yPos = centerY - static_cast<int>(r * sin(thetaRad));

    // Limpa o marcador anterior, se existir
    if (markerState.lastX != -1 && markerState.lastY != -1) {
        tft.fillCircle(markerState.lastX, markerState.lastY, 3, TFT_BLACK);
    }

    // Desenha o marcador atual (um círculo pequeno)
    tft.fillCircle(xPos, yPos, 3, TFT_WHITE);

    // Atualiza o estado do marcador
    markerState.lastX = xPos;
    markerState.lastY = yPos;
}

//
// Retorna os dados de um satélite dado o índice
//
const SatelliteData& SatelliteTracker::getSatellite(int index) const {
    static SatelliteData defaultSatellite = {"Unknown", "", ""};
    if (index < 0 || index >= static_cast<int>(satellites.size())) {
        Serial.printf("Erro: Índice inválido (%d)\n", index);
        return defaultSatellite;
    }
    return satellites[index];
}

//
// Inicializa o satélite selecionado utilizando seus TLEs
//
void SatelliteTracker::initSatellite(int index) {
    if (index < 0 || index >= static_cast<int>(satellites.size())) {
        Serial.println("Índice de satélite inválido!");
        return;
    }
    currentSatelliteIndex = index;
    sat.init(satellites[index].name,
             satellites[index].tle_line1,
             satellites[index].tle_line2);
}

//
// Funções estáticas auxiliares para conversão entre Unix Time e Julian Date
//
static double unixToJulian(unsigned long unixTime) {
    return (static_cast<double>(unixTime) / SECONDS_PER_DAY) + JD_UNIX_EPOCH;
}

static unsigned long julianToUnix(double jd) {
    return static_cast<unsigned long>((jd - JD_UNIX_EPOCH) * SECONDS_PER_DAY);
}

//
// Converte o tempo atual do sistema (usando dados do GPS) para Unix Time
//
unsigned long SatelliteTracker::calculateUnixTime() {
    struct tm timeinfo;
    timeinfo.tm_year  = getCurrentYear() - 1900;
    timeinfo.tm_mon   = getCurrentMonth() - 1;
    timeinfo.tm_mday  = getCurrentDay();
    timeinfo.tm_hour  = getCurrentHour();
    timeinfo.tm_min   = getCurrentMinute();
    timeinfo.tm_sec   = getCurrentSecond();
    timeinfo.tm_isdst = 0;
    return static_cast<unsigned long>(mktime(&timeinfo));
}

//
// Atualiza a posição do satélite em tempo real, utilizando os dados do GPS e SGP4
//
void SatelliteTracker::updateAzElRealTime() {
    if (currentSatelliteIndex < 0 || currentSatelliteIndex >= static_cast<int>(satellites.size()))
        return;

    updateGPS(); // Atualiza os dados do GPS

    // Atualiza os cálculos SGP4 com o tempo atual
    sat.findsat(calculateUnixTime());

    // Define a posição do observador com base nos dados atuais do GPS
    sat.site(getCurrentLatitude(), getCurrentLongitude(), getCurrentAltitude());
}

//
// Gera passagens do satélite para uma duração (em segundos) a partir do tempo atual
//
void SatelliteTracker::updateAndGeneratePasses(double lat, double lon, double alt, unsigned long duracao) {
    // Valida as coordenadas
    if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0 || alt < 0.0) {
        Serial.println("Coordenadas inválidas.");
        return;
    }
    updateGPS();
    sat.site(lat, lon, alt);

    passes.clear();

    unsigned long startUnixTime = calculateUnixTime();
    unsigned long endUnixTime   = startUnixTime + duracao;
    unsigned long localUnixTime = startUnixTime;

    // Inicializa o ponto de predição
    if (!sat.initpredpoint(localUnixTime, 0.0)) {
        Serial.println("Erro initpredpoint.");
        return;
    }

    passinfo overpass;

    while (localUnixTime <= endUnixTime) {
        // Procura a próxima passagem
        if (sat.nextpass(&overpass, 500, false, 10.0)) {
            PassData passData;
            unsigned long passStartUnix = julianToUnix(overpass.jdstart);
            unsigned long passEndUnix   = julianToUnix(overpass.jdstop);

            if (passEndUnix < passStartUnix) {
                Serial.println("Dados de passagem inválidos.");
                continue;
            }

            passData.startPassUnix = passStartUnix;
            passData.endPassUnix   = passEndUnix;

            // Coleta pontos a cada 10 segundos durante a passagem
            for (unsigned long t = passStartUnix; t <= passEndUnix; t += 10) {
                sat.findsat(t);
                SatPosition pos;
                pos.timestamp = t;
                pos.azimuth   = sat.satAz;
                pos.elevation = sat.satEl;
                passData.path.push_back(pos);
            }
            passes.push_back(passData);

            // Avança 5 minutos após o fim da passagem
            localUnixTime = passEndUnix + 300;
        } else {
            // Se não encontrar passagem, incrementa 5 minutos
            localUnixTime += 300;
        }
    }
    Serial.printf("Total de passagens geradas: %d\n", (int)passes.size());
}

//
// Carrega os TLEs a partir de um arquivo no SPIFFS
//
bool SatelliteTracker::loadTLEFile(const char* filePath) {
    File file = SPIFFS.open(filePath, FILE_READ);
    if (!file) {
        Serial.printf("Arquivo %s não encontrado.\n", filePath);
        satellites.clear();
        return false;
    }

    satellites.clear();
    while (file.available()) {
        // Lê o nome do satélite
        String name = file.readStringUntil('\n');
        if (name.length() == 0) break;
        name.trim();

        // Lê as duas linhas do TLE
        String line1 = file.readStringUntil('\n');
        line1.trim();
        String line2 = file.readStringUntil('\n');
        line2.trim();

        // Validação básica dos TLEs (as linhas devem começar com "1 " e "2 ")
        if (!line1.startsWith("1 ") || !line2.startsWith("2 ")) {
            Serial.printf("TLE inválido para satélite '%s'. Pulando.\n", name.c_str());
            continue;
        }

        SatelliteData satData;
        strncpy(satData.name, name.c_str(), sizeof(satData.name) - 1);
        satData.name[sizeof(satData.name) - 1] = '\0';
        strncpy(satData.tle_line1, line1.c_str(), sizeof(satData.tle_line1) - 1);
        satData.tle_line1[sizeof(satData.tle_line1) - 1] = '\0';
        strncpy(satData.tle_line2, line2.c_str(), sizeof(satData.tle_line2) - 1);
        satData.tle_line2[sizeof(satData.tle_line2) - 1] = '\0';

        satellites.push_back(satData);
    }
    file.close();

    Serial.printf("Carregado: %d satélites de %s\n", (int)satellites.size(), filePath);
    return !satellites.empty();
}

//
// Salva um payload de TLEs em um arquivo no SPIFFS
//
bool SatelliteTracker::saveTLEToSPIFFS(const String& payload, const char* filePath) {
    // Verifica se o payload contém múltiplos de 3 linhas
    int lines = 0;
    for (char c : payload) {
        if (c == '\n') lines++;
    }
    if (lines % 3 != 0) {
        Serial.println("TLE malformado: não contém múltiplos de 3 linhas.");
        return false;
    }

    File file = SPIFFS.open(filePath, FILE_WRITE);
    if (file) {
        file.print(payload);
        file.close();
        Serial.printf("TLE salvo com sucesso em %s.\n", filePath);
        return true;
    } else {
        Serial.printf("Erro ao salvar TLE em %s.\n", filePath);
        return false;
    }
}

//
// Imprime os TLEs carregados no Serial
//
void SatelliteTracker::printTLEs() const {
    for (size_t i = 0; i < satellites.size(); i++) {
        Serial.printf("Satélite %zu:\n", i + 1);
        Serial.printf("Nome: %s\n", satellites[i].name);
        Serial.printf("Linha1: %s\n", satellites[i].tle_line1);
        Serial.printf("Linha2: %s\n\n", satellites[i].tle_line2);
    }
}

//
// Formata um Unix Time para uma string, em formato curto ou longo
//
void SatelliteTracker::formatUnixTime(time_t unixTime, char* buffer, size_t bufferSize, bool shortFormat) {
    tmElements_t tm;
    breakTime(unixTime, tm);  // Converte 'unixTime' em dia, mês, ano, etc.

    if (shortFormat) {
        // Formato curto: "DD/MM HH:MM"
        snprintf(buffer, bufferSize, "%02d/%02d %02d:%02d",
                 tm.Day, tm.Month, tm.Hour, tm.Minute);
    } else {
        // Formato longo: "DD/MM/AAAA HH:MM:SS"
        snprintf(buffer, bufferSize, "%02d/%02d/%04d %02d:%02d:%02d",
                 tm.Day, tm.Month, tm.Year + 1970, tm.Hour, tm.Minute, tm.Second);
    }
}

//
// Atualiza a posição do satélite utilizando o SGP4 e os dados do GPS
//
void SatelliteTracker::updateSatellitePosition(unsigned long currentTime) {
    // Calcula a posição do satélite para o tempo atual
    sat.findsat(currentTime);
    // Atualiza a posição do observador utilizando os dados do GPS
    sat.site(getCurrentLatitude(), getCurrentLongitude(), getCurrentAltitude());
}

//
// Retorna a elevação atual do satélite, conforme calculado pelo SGP4
//
double SatelliteTracker::getCurrentElevation() const {
    return sat.satEl;
}

//=============================================================================
// Exibe e permite a visualização de cada passagem
//=============================================================================
void SatelliteTracker::showEachPass() {
    const auto& passes = getPasses();

    if (passes.empty()) {
        Serial.println("[showEachPass] Nenhuma passagem encontrada.");
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextFont(2);
        tft.drawString("No passes found", 10, 10);
        delay(2000);
        return;
    }

    Serial.println("[showEachPass] Exibindo passagens.");
    SatelliteMarkerState markerState;
    int currentPass = 0;

    while (true) {
        updateAzElRealTime();

        // Desenha a passagem e a posição atual
        drawPassPolar(120, 120, 100, passes[currentPass]);
        drawCurrentSatMarker(120, 120, 100,
                             getAzimuth(),
                             getElevation(),
                             markerState);

        tft.drawRect(5, 5, 230, 310, TFT_WHITE);

        // Formata os horários de início e fim
        char buffer_start[25];
        char buffer_end[25];
        time_t startTime = passes[currentPass].startPassUnix;
        time_t endTime   = passes[currentPass].endPassUnix;
        time_t localStartTime = startTime + getTimezone() * SECS_PER_HOUR;
        time_t localEndTime   = endTime + getTimezone() * SECS_PER_HOUR;
        formatUnixTime(localStartTime, buffer_start, sizeof(buffer_start));
        formatUnixTime(localEndTime, buffer_end, sizeof(buffer_end));
        unsigned long dur = endTime - startTime;

        // Exibe informações na tela
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 250);
        tft.print("Current Az: ");
        tft.print(getAzimuth(), 2);
        tft.drawChar(248, tft.getCursorX(), 247);
        tft.setCursor(10, 260);
        tft.print("Current El: ");
        tft.print(getElevation(), 2);
        tft.drawChar(248, tft.getCursorX(), 257);
        tft.setCursor(10, 270);
        tft.print("Pass: ");
        tft.print(currentPass + 1);
        tft.print("/");
        tft.print(passes.size());
        tft.setCursor(10, 280);
        tft.print("AOS: ");
        tft.print(buffer_start);
        tft.setCursor(10, 290);
        tft.print("LOS: ");
        tft.print(buffer_end);
        tft.setCursor(10, 300);
        tft.print("Duration: ");
        tft.print(dur);
        tft.print(" s");

        // Tratamento dos botões
        if (digitalRead(BTN_NEXT) == LOW) {
            Serial.printf("[showEachPass] Próxima passagem (%d/%d).\n", currentPass + 1, (int)passes.size());
            tft.fillScreen(TFT_BLACK);
            currentPass = (currentPass + 1) % passes.size();
            delay(200);
        }
        else if (digitalRead(BTN_PREV) == LOW) {
            Serial.printf("[showEachPass] Passagem anterior (%d/%d).\n", currentPass + 1, (int)passes.size());
            tft.fillScreen(TFT_BLACK);
            currentPass = (currentPass - 1 + passes.size()) % passes.size();
            delay(200);
        }
        else if (digitalRead(BTN_SELECT) == LOW) {
            Serial.printf("[showEachPass] Adicionando notificação para a passagem %d.\n", currentPass + 1);
            delay(200);
            // Adiciona a notificação (funcionalidade já existente)
            notificationManager.addNotification(
                getCurrentSatelliteIndex(), 
                passes[currentPass].startPassUnix, 
                passes[currentPass].endPassUnix
            );
            
            // Obtém o nome do satélite atual
            String satName = String(getSatellite(getCurrentSatelliteIndex()).name);
            
            // Formata o horário de início da passagem (horário do alerta)
            char startTimeStr[25];
            formatUnixTime(passes[currentPass].startPassUnix, startTimeStr, sizeof(startTimeStr));
            
            // Cria a mensagem a ser exibida (usando '\n' para indicar quebras de linha)
            String message = "You will receive an alert\n";
            message += "from satellite " + satName + "\nat " + startTimeStr;
                
            tft.fillRect(
                TLE_UPDATE_X - 2,
                TLE_UPDATE_Y - 2,
                TLE_UPDATE_WIDTH + 4,
                TLE_UPDATE_HEIGHT + 4,
                TFT_BLACK
            );
            // Desenha a área de notificação (limpa a área e desenha o cabeçalho)
            drawNotificationArea();
            
            // Configura o estilo do texto para a mensagem
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextFont(1);

            // Define as coordenadas iniciais dentro da área (com pequena margem)
            int notifX = NOTIFICATION_X + 5;
            int notifY = NOTIFICATION_Y + NOTIFICATION_HEADER_HEIGHT + 5;
            int lineSpacing = 15;  // Espaçamento entre linhas
            
            // Divide a mensagem em linhas (usando '\n') e exibe cada linha
            int lineIndex = 0;
            int startPos = 0;
            int msgLength = message.length();
            for (int i = 0; i <= msgLength; i++) {
                if (i == msgLength || message[i] == '\n') {
                    String line = message.substring(startPos, i);
                    tft.drawString(line, notifX, notifY + lineIndex * lineSpacing);
                    lineIndex++;
                    startPos = i + 1;
                }
            }
            
            delay(2000);  // Tempo para o usuário ler a mensagem
            tft.fillScreen(TFT_BLACK);  // Limpa a tela após exibir a mensagem
        }
        else if (digitalRead(BTN_BACK) == LOW) {
            Serial.println("[showEachPass] Saindo da visualização de passagens.");
            tft.fillScreen(TFT_BLACK);
            delay(200);
            break;
        }
        delay(100);
    }
}

void SatelliteTracker::trackSatellite() {
    Serial.println("[trackSatellite] Entrando no menu de seleção de satélite.");
    tft.fillScreen(TFT_BLACK);

    static int selectedSatellite = 0;
    static int topMenuIndex = 0;
    const int maxVisibleItems = 19;
    int previousSatellite = -1;

    delay(200);

    while (true) {
        if (previousSatellite != selectedSatellite) {
            // Desenha a área do menu com cabeçalho
            Area menuArea = { MENU_X, MENU_Y,
                              MENU_WIDTH, MENU_HEIGHT,
                              MENU_HEADER_HEIGHT };
            MenuManager::drawArea(menuArea, "SELECT SATELLITE", TFT_BLACK, TFT_WHITE, TFT_WHITE);

            // Limpa a área interna para evitar sobreposição de nomes
            tft.fillRect(MENU_X + 1, 
                         MENU_Y + MENU_HEADER_HEIGHT + 1,
                         MENU_WIDTH - 2,
                         MENU_HEIGHT - MENU_HEADER_HEIGHT - 2, 
                         TFT_BLACK);

            // Desenha a lista de satélites
            tft.setTextFont(2);
            int numSatellites = getNumSatellites();
            int lastIndex = topMenuIndex + maxVisibleItems - 1;
            if (lastIndex >= numSatellites) {
                lastIndex = numSatellites - 1;
            }
            int posY = MENU_Y + MENU_HEADER_HEIGHT + 5;
            for (int i = topMenuIndex; i <= lastIndex; i++) {
                if (i >= numSatellites) break;
                if (i == selectedSatellite) {
                    tft.setTextColor(TFT_BLACK, TFT_WHITE);
                } else {
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                }
                String satName = String(getSatellite(i).name);
                tft.drawString(satName, MENU_X + 5, posY);
                posY += MENU_ITEM_SPACING;
            }
            previousSatellite = selectedSatellite;
            Serial.printf("[trackSatellite] Satélite selecionado: %d\n", selectedSatellite);
        }
        // Navegação pelos botões
        if (digitalRead(BTN_NEXT) == LOW) {
            selectedSatellite = (selectedSatellite + 1) % getNumSatellites();
            if (selectedSatellite == 0) { // Se voltou para o início
                topMenuIndex = 0;
            } else if (selectedSatellite > (topMenuIndex + maxVisibleItems - 1)) {
                topMenuIndex = selectedSatellite - (maxVisibleItems - 1);
            }
            delay(200);
        }
        else if (digitalRead(BTN_PREV) == LOW) {
            selectedSatellite = (selectedSatellite - 1 + getNumSatellites()) % getNumSatellites();
            if (selectedSatellite == getNumSatellites() - 1) { // Se voltou para o fim
                if(getNumSatellites() > maxVisibleItems) {
                    topMenuIndex = getNumSatellites() - maxVisibleItems;
                } else {
                    topMenuIndex = 0;
                }
            } else if (selectedSatellite < topMenuIndex) {
                topMenuIndex = selectedSatellite;
            }
            delay(200);
        }
        else if (digitalRead(BTN_SELECT) == LOW) {
            Serial.printf("[trackSatellite] Satélite %d selecionado.\n", selectedSatellite);
            tft.fillScreen(TFT_BLACK);
            delay(500);
            initSatellite(selectedSatellite);
            updateAndGeneratePasses(
                getCurrentLatitude(),
                getCurrentLongitude(),
                getCurrentAltitude(),
                86400 // 24 horas
            );
            showEachPass();
            previousSatellite = -1;
        }
        else if (digitalRead(BTN_BACK) == LOW) {
            Serial.println("[trackSatellite] Saindo do menu de seleção de satélite.");
            tft.fillScreen(TFT_BLACK);
            delay(200);
            break;
        }
        delay(50);
    }

    menuManager.back();
}

void SatelliteTracker::manualTrack() {
    // Define as coordenadas do centro e o raio do gráfico (ajuste conforme necessário)
    const int centerX = 120;
    const int centerY = 120;
    const int radius  = 100;
    
    // Limpa a tela e desenha o gráfico de fundo
    tft.fillScreen(TFT_BLACK);

    // Loop principal da visualização manual
    while (true) {

        tft.setTextFont(1);

        // --- Desenhar o gráfico de fundo ---
        // (1) Círculo principal (horizonte)
        tft.drawCircle(centerX, centerY, radius, TFT_WHITE);

        // (2) Círculos de referência para 30° e 60° de "elevação"
        int r30 = static_cast<int>(radius * (1 - 30.0 / 90.0));
        int r60 = static_cast<int>(radius * (1 - 60.0 / 90.0));
        tft.drawCircle(centerX, centerY, r30, TFT_WHITE);
        tft.drawCircle(centerX, centerY, r60, TFT_WHITE);

        // (3) Linhas cardeais e seus rótulos
        tft.drawLine(centerX, centerY - radius, centerX, centerY + radius, TFT_WHITE); // N-S
        tft.drawLine(centerX - radius, centerY, centerX + radius, centerY, TFT_WHITE); // E-W
        tft.drawString("N", centerX - 2, centerY - radius - 10);
        tft.drawString("S", centerX - 2, centerY + radius + 5);
        tft.drawString("E", centerX + radius + 5, centerY - 3);
        tft.drawString("W", centerX - radius - 10, centerY - 3);

        // --- Desenhar o ponteiro de orientação ---
        orientationManager.drawOrientationPointer(centerX, centerY, radius);

        // --- Exibir os valores de azimute e "elevação" do dispositivo ---
        sensors_event_t event;
        orientationManager.getSensorEvent(&event);
        float heading = event.orientation.x;
        float pitch   = - event.orientation.z;  // Ajuste se necessário

        // Limpa a área onde os textos serão exibidos (para evitar sobreposição)
        tft.fillRect(5, 240, 230, 20, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(5, 240);
        tft.printf("Az: %.1f  El: %.1f", heading, pitch);

        // --- Verificar se o botão "voltar" foi pressionado para sair ---
        if (digitalRead(BTN_BACK) == LOW) {
            // Sai do modo manual e limpa a tela antes de retornar ao menu
            tft.fillScreen(TFT_BLACK);
            delay(200);
            break;
        }

        delay(100); // Pequeno delay para suavizar a atualização
    }
    menuManager.drawMenu();
}
