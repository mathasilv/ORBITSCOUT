#pragma once
#ifndef TLE_MANAGER_H
#define TLE_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include "SatelliteTracker.h"
#include "gps.h"
#include "TleSources.h"

/**
 * @brief Estrutura para armazenar data e hora.
 */
struct DateTime {
    String date; ///< Data no formato "DD/MM/AAAA"
    String time; ///< Hora no formato "HH:MM:SS"
};

/**
 * @brief Classe para gerenciar a atualização dos TLEs.
 *
 * Essa classe é responsável por:
 * - Inicializar o sistema de arquivos SPIFFS;
 * - Fazer download dos TLEs a partir de diversas fontes;
 * - Permitir a seleção de fontes TLE e forçar a atualização completa.
 */
class TleManager {
public:
    /// Construtor padrão
    TleManager();

    /**
     * @brief Salva a data e hora da última atualização dos TLEs no SPIFFS.
     */
    void saveLastTleUpdateDate();

    /**
     * @brief Carrega a data e hora da última atualização dos TLEs do SPIFFS.
     * @return DateTime com os valores carregados ou valores padrão caso não existam dados.
     */
    DateTime loadLastTleUpdateDate();

    /**
     * @brief Exibe um menu para o usuário selecionar a fonte dos TLEs.
     */
    void selectTleSource();

    /**
     * @brief Força a atualização de todos os TLEs.
     */
    void forceUpdateAllTle();

    /**
     * @brief Carrega os TLEs a partir de uma URL específica.
     *
     * Se o arquivo já existir no SPIFFS, tenta carregá-lo; caso contrário, efetua o download.
     *
     * @param tleUrl URL para o download dos TLEs.
     * @param filePath Caminho do arquivo onde os TLEs serão salvos.
     */
    void loadTleFromUrl(const char* tleUrl, const char* filePath);


private:

    /**
     * @brief Efetua o download de um arquivo com exibição de barra de progresso.
     *
     * @param url URL para download.
     * @param filePath Caminho no SPIFFS para salvar o arquivo.
     * @return true se o download ocorrer com sucesso; false caso contrário.
     */
    bool downloadFileWithProgress(const char* url, const char* filePath);
};

#endif // TLE_MANAGER_H
