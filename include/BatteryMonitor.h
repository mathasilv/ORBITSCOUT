#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <Arduino.h>
#include "Config.h"

/**
 * @brief Classe para monitorar a bateria.
 *
 * Essa classe realiza a leitura do pino de bateria, converte a leitura do ADC em tensão real e calcula a porcentagem de carga da bateria.
 */
class BatteryMonitor {
public:
    /// Construtor padrão.
    BatteryMonitor();

    /**
     * @brief Inicializa o pino da bateria.
     *
     * Configura o pino definido em Config.h como entrada.
     */
    void begin();

    /**
     * @brief Atualiza as medições da bateria.
     *
     * Realiza a leitura analógica, converte a tensão medida e calcula a porcentagem de carga.
     */
    void update();

    /**
     * @brief Retorna a tensão atual da bateria.
     * @return Tensão medida.
     */
    float getVoltage() const;

    /**
     * @brief Retorna a porcentagem de carga da bateria.
     * @return Porcentagem de carga (entre 0 e 100).
     */
    float getPercentage() const;

private:
    float _actualVoltage;      ///< Tensão real da bateria
    float _batteryPercentage;  ///< Porcentagem de carga da bateria

    // Constantes para conversão (valores específicos para o hardware)
    static constexpr float MAX_BATTERY_VOLTAGE = 4.0;    
    static constexpr float MIN_BATTERY_VOLTAGE = 3.7;   
    static constexpr int ADC_RESOLUTION = 4096;         
    static constexpr float DIVISOR_FACTOR = 2.0;         

    /**
     * @brief Limita um valor entre dois limites.
     *
     * @param x Valor a ser limitado.
     * @param a Limite inferior.
     * @param b Limite superior.
     * @return Valor limitado entre a e b.
     */
    float constrainFloat(float x, float a, float b) const;
};

#endif // BATTERYMONITOR_H
