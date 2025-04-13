#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "Config.h"  // Inclui as definições dos pinos e outras configurações

// ======================
// Funções de Inicialização e Atualização do GPS
// ======================
/**
 * @brief Inicializa a comunicação com o módulo GPS e o SPIFFS.
 */
void setupGPS();

/**
 * @brief Processa os dados recebidos do GPS via Serial.
 *
 * Lê todos os caracteres disponíveis e os envia para o objeto TinyGPSPlus.
 */
void updateGPS();

/**
 * @brief Atualiza os dados do GPS e a última posição conhecida.
 *
 * Esta função centraliza a atualização dos dados do GPS, processando as informações
 * recebidas e atualizando a posição (com fallback via SPIFFS, se necessário).
 * Deve ser chamada uma vez por ciclo de loop.
 */
void updateGPSData();

// ======================
// Funções de manipulação de fuso horário
// ======================
/**
 * @brief Retorna o fuso horário atual.
 * @return Fuso horário atual.
 */
int getTimezone();

/**
 * @brief Define o fuso horário.
 * @param tz Valor do fuso (entre -12 e 14).
 */
void setTimezone(int tz);

// ======================
// Funções Getter COM fallback via SPIFFS para Latitude, Longitude e Altitude
// ======================
/**
 * @brief Retorna a latitude atual.
 *
 * Se o dado do GPS for inválido, utiliza o último valor salvo no SPIFFS.
 * @return Latitude.
 */
double getCurrentLatitude();

/**
 * @brief Retorna a longitude atual.
 *
 * Se o dado do GPS for inválido, utiliza o último valor salvo no SPIFFS.
 * @return Longitude.
 */
double getCurrentLongitude();

/**
 * @brief Retorna a altitude atual em metros.
 *
 * Se o dado do GPS for inválido, utiliza o último valor salvo no SPIFFS.
 * @return Altitude.
 */
double getCurrentAltitude();

// ======================
// Funções Getter SEM fallback (dados atuais, sem SPIFFS) para data/hora
// ======================
/**
 * @brief Retorna a data atual no formato "DD/MM/AAAA".
 * @return Data atual.
 */
String getCurrentDate();

/**
 * @brief Retorna o horário atual (considerando o fuso) no formato "HH:MM:SS".
 * @return Horário atual.
 */
String getCurrentTime();

// ======================
// Outras funções GET para dados do GPS (velocidade, curso, satélites, hdop)
// ======================
/**
 * @brief Retorna a velocidade atual em km/h.
 * @return Velocidade atual.
 */
double getCurrentSpeed();

/**
 * @brief Retorna o curso (direção) atual em graus.
 * @return Curso atual.
 */
double getCurrentCourse();

/**
 * @brief Retorna o número de satélites atualmente utilizados.
 * @return Número de satélites.
 */
int getCurrentSatellites();

/**
 * @brief Retorna o valor do HDOP (Horizontal Dilution of Precision).
 * @return HDOP.
 */
double getCurrentHDOP();

// ======================
// Funções GET adicionais para data/hora
// ======================
/**
 * @brief Retorna o ano atual.
 * @return Ano atual.
 */
int getCurrentYear();

/**
 * @brief Retorna o mês atual.
 * @return Mês atual.
 */
int getCurrentMonth();

/**
 * @brief Retorna o dia atual.
 * @return Dia atual.
 */
int getCurrentDay();

/**
 * @brief Retorna a hora atual.
 * @return Hora atual.
 */
int getCurrentHour();

/**
 * @brief Retorna o minuto atual.
 * @return Minuto atual.
 */
int getCurrentMinute();

/**
 * @brief Retorna o segundo atual.
 * @return Segundo atual.
 */
int getCurrentSecond();

#endif // GPS_H
