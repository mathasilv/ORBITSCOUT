#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <vector>
#include <Arduino.h>
#include <TFT_eSPI.h>    // Biblioteca para o display
#include <TimeLib.h>     // Para manipulação de tempo

/**
 * @brief Estrutura para armazenar informações de uma notificação.
 *
 * Cada notificação associa um índice de satélite a um intervalo de tempo
 * (passStartUnix e passEndUnix) e possui uma flag que indica se a notificação
 * já foi acionada.
 */
struct Notification {
    int satelliteIndex;           ///< Índice do satélite relacionado à notificação
    unsigned long passStartUnix;  ///< Início da passagem (Unix Time)
    unsigned long passEndUnix;    ///< Fim da passagem (Unix Time)
    bool notified;                ///< Flag para indicar se a notificação já foi acionada

    /**
     * @brief Construtor da notificação.
     * 
     * @param satIndex Índice do satélite.
     * @param start Início da passagem (Unix Time).
     * @param end Fim da passagem (Unix Time).
     */
    Notification(int satIndex, unsigned long start, unsigned long end)
        : satelliteIndex(satIndex), passStartUnix(start), passEndUnix(end), notified(false) {}
};

/**
 * @brief Classe para gerenciar notificações de passagens de satélites.
 *
 * Essa classe permite adicionar notificações, verificar se alguma notificação
 * deve ser acionada (por exemplo, acionando um buzzer) e exibir informações
 * das notificações na interface do usuário (display TFT).
 */
class NotificationManager {
public:
    /// Construtor padrão.
    NotificationManager();

    /**
     * @brief Adiciona uma nova notificação.
     *
     * @param satelliteIndex Índice do satélite.
     * @param passStartUnix Início da passagem (Unix Time).
     * @param passEndUnix Fim da passagem (Unix Time).
     */
    void addNotification(int satelliteIndex, unsigned long passStartUnix, unsigned long passEndUnix);

    /**
     * @brief Verifica e aciona as notificações.
     *
     * Essa função deve ser chamada periodicamente para avaliar se o tempo
     * atual está dentro do intervalo de alguma notificação. Se sim, o alerta
     * é acionado.
     */
    void checkNotifications();

    /**
     * @brief Retorna a lista de notificações.
     *
     * @return Constante referência ao vetor de notificações.
     */
    const std::vector<Notification>& getNotifications() const;

    /**
     * @brief Exibe as notificações na interface do usuário (ex.: no display TFT).
     */
    void showNotificationInfo();

private:
    std::vector<Notification> notifications;  ///< Vetor que armazena as notificações
};

#endif // NOTIFICATION_MANAGER_H
