#ifndef ORIENTATION_MANAGER_H
#define ORIENTATION_MANAGER_H

#include <Arduino.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <utility/imumaths.h> // Para imu::Vector<3>
#include <TFT_eSPI.h>

/**
 * @brief Classe para gerenciar o sensor inercial BNO055 e fornecer dados de orientação.
 *
 * A classe inicializa o sensor, obtém os ângulos de Euler (onde, convencionalmente,
 * x = heading, y = roll e z = pitch) e calcula a posição do ponteiro que indica a direção
 * atual. Agora, além do heading, o pitch (elevação) é usado para modular a distância do
 * ponteiro ao centro, representando graficamente a inclinação.
 */
class OrientationManager {
public:
    /**
     * @brief Construtor da classe OrientationManager.
     *
     * @param sensorID Identificador do sensor (padrão: 55).
     * @param address Endereço I2C do sensor (padrão: BNO055_ADDRESS_A, geralmente 0x28).
     */
    OrientationManager(int sensorID = 55, uint8_t address = BNO055_ADDRESS_A);

    /**
     * @brief Inicializa o sensor BNO055.
     *
     * Tenta iniciar o sensor e, se bem-sucedido, configura o uso do cristal externo.
     *
     * @return true se a inicialização for bem-sucedida, false caso contrário.
     */
    bool begin();


    /**
     * @brief Obtém os ângulos de Euler do sensor.
     *
     * @return Um vetor com os ângulos de Euler (yaw, pitch, roll) em graus.
     */
    imu::Vector<3> getEulerAngles();

    /**
     * @brief Calcula as coordenadas do ponteiro que indica a orientação.
     *
     * Utiliza o heading e o pitch para determinar a posição do ponteiro. O heading é
     * ajustado subtraindo 90° para que 0° (norte) fique no topo. O pitch modula o raio
     * efetivo:
     *  - Se pitch = 0°, factor = 1 (ponteiro na borda do círculo);
     *  - Se pitch = +90°, factor = 0 (ponteiro no centro);
     *  - Se pitch = -90°, factor = 2 (ponteiro mais afastado).
     *
     * @param centerX Coordenada X do centro.
     * @param centerY Coordenada Y do centro.
     * @param radius Raio máximo (em pixels).
     * @param pointerX (Saída) Coordenada X calculada do ponteiro.
     * @param pointerY (Saída) Coordenada Y calculada do ponteiro.
     */
    void computePointer(int centerX, int centerY, int radius, int &pointerX, int &pointerY);

    /**
     * @brief Desenha o ponteiro de orientação no display.
     *
     * Desenha uma linha do centro até o ponteiro e um pequeno círculo na extremidade,
     * representando a direção atual. O ponteiro incorpora o heading e o pitch.
     *
     * @param centerX Coordenada X do centro do círculo.
     * @param centerY Coordenada Y do centro do círculo.
     * @param radius Raio máximo (em pixels) para o ponteiro.
     */
    void drawOrientationPointer(int centerX, int centerY, int radius);

    /**
     * @brief Imprime os valores de orientação no Serial.
     */
    void printOrientation();

    bool getSensorEvent(sensors_event_t *event) {
        return bno.getEvent(event);
    }

private:
    Adafruit_BNO055 bno;  ///< Objeto do sensor BNO055.
    int lastPointerX;     ///< Última posição X desenhada do ponteiro (para limpeza).
    int lastPointerY;     ///< Última posição Y desenhada do ponteiro (para limpeza).
};

#endif // ORIENTATION_MANAGER_H
