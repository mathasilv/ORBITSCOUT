#ifndef CONFIG_H
#define CONFIG_H

// Definições de pinos de botões
#define BTN_NEXT    25
#define BTN_PREV    26
#define BTN_SELECT  27
#define BTN_BACK    14

// Pino do backlight
#define BACKLIGHT_PIN 12

// Pino da bateria
#define BATTERY_PIN 33

// Buzzer pin
#define BUZZER_PIN 13

// Pinos para I2C (SDA e SCL)
#define SDA_PIN 22
#define SCL_PIN 21

// ================================
// Definições para o módulo GPS
// ================================
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD   9600

#endif // CONFIG_H
                                                    