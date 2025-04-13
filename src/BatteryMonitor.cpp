#include "BatteryMonitor.h"

BatteryMonitor::BatteryMonitor()
    : _actualVoltage(0.0), _batteryPercentage(0.0) {}

void BatteryMonitor::begin() {
    // Configura o pino definido como BATTERY_PIN para leitura
    pinMode(BATTERY_PIN, INPUT);
}

void BatteryMonitor::update() {
    // Lê o valor bruto do ADC no pino da bateria
    int rawValue = analogRead(BATTERY_PIN);

    // Converte o valor lido em tensão (considerando a referência de 3.3V e resolução do ADC)
    float measuredVoltage = (rawValue * 3.3) / ADC_RESOLUTION; // Tensão após divisor
    _actualVoltage = measuredVoltage * DIVISOR_FACTOR;         // Tensão real da bateria (considera o divisor)

    // Calcula a porcentagem de carga baseada na tensão real
    _batteryPercentage = ((_actualVoltage - MIN_BATTERY_VOLTAGE) / 
                         (MAX_BATTERY_VOLTAGE - MIN_BATTERY_VOLTAGE)) * 100.0;

    // Limita a porcentagem para o intervalo de 0 a 100%
    _batteryPercentage = constrainFloat(_batteryPercentage, 0.0, 100.0);
}

float BatteryMonitor::getVoltage() const {
    return _actualVoltage;
}

float BatteryMonitor::getPercentage() const {
    return _batteryPercentage;
}

float BatteryMonitor::constrainFloat(float x, float a, float b) const {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}
