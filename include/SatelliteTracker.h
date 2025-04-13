#pragma once
#ifndef SATELLITE_TRACKER_H
#define SATELLITE_TRACKER_H

#include <Sgp4.h>
#include <gps.h>
#include <vector>      // Para std::vector
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include "Config.h"    // Para definições de pinos, incluindo BUZZER_PIN
#include "gps.h"

// Objeto TFT é declarado externamente (por exemplo, na main)
extern TFT_eSPI tft;

/**
 * @brief Estrutura para armazenar os dados básicos de um satélite (TLE).
 */
struct SatelliteData {
    char name[50];       ///< Nome do satélite (ajuste o tamanho conforme necessário)
    char tle_line1[100]; ///< Linha 1 do TLE (tamanho usual ~69 caracteres)
    char tle_line2[100]; ///< Linha 2 do TLE (tamanho usual ~69 caracteres)
};

/**
 * @brief Estrutura para representar um ponto na trajetória do satélite.
 */
struct SatPosition {
    unsigned long timestamp; ///< Timestamp do ponto
    double azimuth;          ///< Azimute em graus
    double elevation;        ///< Elevação em graus
};

/**
 * @brief Estrutura que representa uma passagem completa (com início, fim e trajetória).
 */
struct PassData {
    unsigned long startPassUnix;       ///< Início da passagem (Unix Time)
    unsigned long endPassUnix;         ///< Fim da passagem (Unix Time)
    std::vector<SatPosition> path;     ///< Vetor com pontos (az, el) durante a passagem
};

/**
 * @brief Estrutura para manter o estado do marcador do satélite na interface.
 */
struct SatelliteMarkerState {
    int lastX; ///< Última posição X desenhada
    int lastY; ///< Última posição Y desenhada

    SatelliteMarkerState() : lastX(-1), lastY(-1) {}
};

/**
 * @brief Classe para rastreamento de satélites.
 *
 * Essa classe gerencia:
 * - O carregamento e armazenamento dos TLEs;
 * - Cálculos de posição utilizando a biblioteca SGP4;
 * - Geração de passagens (AOS, LOS e trajetória);
 * - Desenho de informações e trajetórias no display TFT.
 */
class SatelliteTracker {
private:
    std::vector<SatelliteData> satellites;   ///< Lista dinâmica de satélites
    Sgp4 sat;                                ///< Objeto SGP4 para cálculos orbitais

    unsigned long currentUnixTime;

    int currentSatelliteIndex;               ///< Índice do satélite selecionado
    std::vector<PassData> passes;            ///< Lista de passagens geradas

public:
    /// Construtor da classe.
    SatelliteTracker();

    ////////// Métodos de Desenho //////////

    /**
     * @brief Desenha uma passagem no formato polar.
     *
     * @param centerX Coordenada X do centro.
     * @param centerY Coordenada Y do centro.
     * @param radius Raio do círculo principal.
     * @param pass Estrutura com os dados da passagem.
     */
    void drawPassPolar(int centerX, int centerY, int radius, const PassData &pass);

    /**
     * @brief Desenha o marcador do satélite na interface.
     *
     * @param centerX Coordenada X do centro.
     * @param centerY Coordenada Y do centro.
     * @param radius Raio do gráfico.
     * @param az Azimute atual.
     * @param el Elevação atual.
     * @param markerState Estado atual do marcador (para limpeza e atualização).
     */
    void drawCurrentSatMarker(int centerX, int centerY, int radius,
                              double az, double el, SatelliteMarkerState &markerState);

    ////////// Métodos de Gerenciamento e Formatação de Tempo //////////

    /**
     * @brief Formata um Unix Time para uma string.
     *
     * @param unixTime Tempo em Unix.
     * @param buffer Buffer para armazenar a string formatada.
     * @param bufferSize Tamanho do buffer.
     * @param shortFormat Se verdadeiro, usa formato curto ("DD/MM HH:MM"); caso contrário, formato longo ("DD/MM/AAAA HH:MM:SS").
     */
    void formatUnixTime(time_t unixTime, char* buffer, size_t bufferSize, bool shortFormat = false);

    /**
     * @brief Calcula o Unix Time atual utilizando os dados do GPS.
     *
     * @return Tempo atual em Unix.
     */
    unsigned long calculateUnixTime();

    ////////// Métodos de Inicialização e Atualização dos Satélites //////////

    /**
     * @brief Inicializa um satélite selecionado com base nos TLEs.
     *
     * @param index Índice do satélite na lista.
     */
    void initSatellite(int index);

    /**
     * @brief Atualiza continuamente a posição do satélite em tempo real.
     *
     * Atualiza os cálculos SGP4 utilizando os dados atuais do GPS.
     */
    void updateAzElRealTime();

    /**
     * @brief Atualiza a posição do satélite e gera passagens para um período especificado.
     *
     * @param lat Latitude do observador.
     * @param lon Longitude do observador.
     * @param alt Altitude do observador.
     * @param duracao Duração (em segundos) para gerar passagens.
     */
    void updateAndGeneratePasses(double lat, double lon, double alt, unsigned long duracao);

    /**
     * @brief Atualiza a posição do satélite utilizando os dados do GPS e SGP4.
     *
     * @param currentTime Tempo atual em Unix.
     */
    void updateSatellitePosition(unsigned long currentTime);

    ////////// Métodos para Carregamento/Armazenamento dos TLEs //////////

    /**
     * @brief Carrega os TLEs a partir de um arquivo específico do SPIFFS.
     *
     * @param filePath Caminho do arquivo.
     * @return true se os TLEs foram carregados com sucesso; false caso contrário.
     */
    bool loadTLEFile(const char* filePath);

    /**
     * @brief Salva os TLEs fornecidos em um arquivo específico do SPIFFS.
     *
     * @param payload Conteúdo dos TLEs.
     * @param filePath Caminho do arquivo.
     * @return true se os TLEs foram salvos com sucesso; false caso contrário.
     */
    bool saveTLEToSPIFFS(const String& payload, const char* filePath);

    /**
     * @brief Imprime os TLEs carregados no Serial (para depuração).
     */
    void printTLEs() const;

    ////////// Getters e Métodos Auxiliares //////////

    /**
     * @brief Retorna o tempo Unix atual calculado.
     *
     * @return currentUnixTime.
     */
    unsigned long getCurrentUnix() const { return currentUnixTime; }

    /**
     * @brief Retorna o azimute do satélite calculado pelo SGP4.
     *
     * @return Azimute.
     */
    double getAzimuth() const { return sat.satAz; }

    /**
     * @brief Retorna a elevação do satélite calculada pelo SGP4.
     *
     * @return Elevação.
     */
    double getElevation() const { return sat.satEl; }

    /**
     * @brief Retorna o número de satélites carregados.
     *
     * @return Tamanho do vetor de satélites.
     */
    size_t getNumSatellites() const { return satellites.size(); }

    /**
     * @brief Retorna o índice do satélite atualmente selecionado.
     *
     * @return Índice do satélite.
     */
    int getCurrentSatelliteIndex() const { return currentSatelliteIndex; }

    /**
     * @brief Retorna os dados de um satélite específico.
     *
     * @param index Índice do satélite.
     * @return Referência aos dados do satélite.
     */
    const SatelliteData& getSatellite(int index) const;

    /**
     * @brief Retorna as passagens geradas.
     *
     * @return Constante referência ao vetor de passagens.
     */
    const std::vector<PassData>& getPasses() const { return passes; }

    /// Remove todas as passagens armazenadas.
    void clearPasses() { passes.clear(); }
    
    /**
     * @brief Retorna a elevação atual do satélite (calculada pelo SGP4).
     *
     * @return Elevação.
     */
    double getCurrentElevation() const;

    void manualTrack();  // Função para rastreamento manual com base na orientação do dispositivo

        /**
     * @brief Efetua o download de um arquivo com exibição de barra de progresso.
     *
     * @param url URL para download.
     * @param filePath Caminho no SPIFFS para salvar o arquivo.
     * @return true se o download ocorrer com sucesso; false caso contrário.
     */
    bool downloadFileWithProgress(const char* url, const char* filePath);

    /**
     * @brief Exibe e permite selecionar uma passagem de satélite para notificação.
     */
    void showEachPass();

    /**
     * @brief Exibe e permite a seleção de um satélite.
     *
     * Inicializa o satélite e gera a previsão de passagens.
     */
    void trackSatellite();
};

#endif // SATELLITE_TRACKER_H
