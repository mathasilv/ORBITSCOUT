#include "NotificationManager.h"
#include "SatelliteTracker.h"  // Para acessar funções do rastreador, como calculateUnixTime(), formatUnixTime() e getSatellite()
#include "Config.h"            // Para funções auxiliares como getTimezone() e SECS_PER_HOUR
#include <algorithm>

// Objeto externo para o rastreador de satélites (declarado na main ou em outro módulo central)
extern SatelliteTracker tracker;

//
// Construtor da classe NotificationManager
//
NotificationManager::NotificationManager() {
    // Inicializa o pino do buzzer e garante que ele esteja desligado inicialmente
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

//
// Adiciona uma nova notificação para um determinado satélite e intervalo de passagem
//
void NotificationManager::addNotification(int satelliteIndex, unsigned long passStartUnix, unsigned long passEndUnix) {
    notifications.push_back(Notification(satelliteIndex, passStartUnix, passEndUnix));
    Serial.println("Notificação adicionada com sucesso.");
    Serial.printf("Satélite: %d, Início: %lu, Fim: %lu\n", satelliteIndex, passStartUnix, passEndUnix);
}

//
// Verifica e aciona notificações conforme o tempo atual e a elevação do satélite
//
void NotificationManager::checkNotifications() {
    // Obtém o tempo atual a partir do rastreador (usando dados do GPS)
    unsigned long currentTime = tracker.calculateUnixTime();

    // Itera pelas notificações
    for (auto &notification : notifications) {
        // Se a notificação já foi acionada ou a passagem já expirou, pula-a
        if (notification.notified || currentTime > notification.passEndUnix) {
            continue;
        }
        
        // Se o tempo atual está dentro do intervalo da passagem...
        if (currentTime >= notification.passStartUnix && currentTime <= notification.passEndUnix) {
            // Atualiza a posição do satélite usando os métodos públicos do rastreador
            tracker.updateSatellitePosition(currentTime);

            // Se a elevação atual do satélite for maior que zero, aciona o alerta
            if (tracker.getCurrentElevation() > 0.0) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(1000);  // Duração do alerta (pode ser substituído por uma lógica não bloqueante)
                digitalWrite(BUZZER_PIN, LOW);
                notification.notified = true;
                Serial.println("Notificação acionada!");
            }
        }
    }

    // Remove notificações que já foram acionadas ou cujo período expirou
    notifications.erase(
        std::remove_if(notifications.begin(), notifications.end(),
                       [&](const Notification &n) { return n.notified || currentTime > n.passEndUnix; }),
        notifications.end()
    );
}

//
// Retorna as notificações atualmente armazenadas (constante)
//
const std::vector<Notification>& NotificationManager::getNotifications() const {
    return notifications;
}

//
// Exibe informações das notificações na interface gráfica
//
void NotificationManager::showNotificationInfo() {
    // Define a área onde serão exibidas as notificações
    const int xArea = 10;
    const int yArea = 235;
    const int wArea = 100;
    const int hArea = 80;

    // Limpa a área no display
    tft.fillRect(xArea, yArea, wArea, hArea, TFT_BLACK);

    // Se não houver notificações, não há nada a exibir
    if (notifications.empty()) {
        return;
    }

    // Configura a fonte e a cor do texto
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    char buffer[20];     // Buffer para formatar a data/hora
    const int lineHeight = 10; // Altura de cada linha
    const int maxToShow = 3;   // Número máximo de notificações a exibir

    // Exibe no máximo 'maxToShow' notificações
    int count = min(static_cast<int>(notifications.size()), maxToShow);
    for (int i = 0; i < count; i++) {
        const Notification &notif = notifications[i];

        // Obtém o nome do satélite (usando o rastreador; getSatellite retorna uma estrutura SatelliteData)
        const SatelliteData &satData = tracker.getSatellite(notif.satelliteIndex);
        String satName = satData.name;

        // Converte o horário de início da passagem para o fuso horário local
        unsigned long localPassStart = notif.passStartUnix + getTimezone() * SECS_PER_HOUR;
        tracker.formatUnixTime(localPassStart, buffer, sizeof(buffer), true);

        // Calcula a posição vertical deste bloco de notificação
        int blockY = yArea + i * (lineHeight * 3);

        // Desenha o nome do satélite e o horário formatado na área definida
        tft.drawString(satName, xArea, blockY);
        tft.drawString(buffer, xArea, blockY + lineHeight);
    }
}
