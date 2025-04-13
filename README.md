

# OrbitScout

**OrbitScout** é um sistema embarcado de rastreamento de satélites que dispensa a necessidade do usuário inserir manualmente sua localização ou data/hora. O sistema utiliza um módulo GPS para obter, em tempo real, a posição e o tempo atual, e um sensor inercial para auxiliar no tracking, permitindo que o usuário visualize a trajetória real do satélite conforme a orientação do dispositivo.

---

## Sumário

- [Visão Geral](#visão-geral)
- [Principais Funcionalidades](#principais-funcionalidades)
- [Arquitetura e Módulos](#arquitetura-e-módulos)
  - [Configuração (Config.h)](#configuração-configh)
  - [Monitoramento de Bateria (BatteryMonitor)](#monitoramento-de-bateria-batterymonitor)
  - [GPS para Localização e Tempo Real (GPS.h)](#gps-para-localização-e-tempo-real-gpsh)
  - [Interface e Menu (MenuManager)](#interface-e-menu-menumanager)
  - [Barras de Progresso (ProgressBar)](#barras-de-progresso-progressbar)
  - [Conexão Wi‑Fi (WiFiManager)](#conexão-wifi-wifimanager)
  - [Rastreamento de Satélites (SatelliteTracker)](#rastreamento-de-satélites-satellitetracker)
  - [Notificações (NotificationManager)](#notificações-notificationmanager)
  - [Orientação e Tracking (OrientationManager)](#orientação-e-tracking-orientationmanager)
  - [Gerenciamento de TLE (TleManager)](#gerenciamento-de-tle-tlemanager)
- [Pinagens](#pinagens)
- [Requisitos de Hardware e Dependências](#requisitos-de-hardware-e-dependências)
- [Como Compilar e Carregar](#como-compilar-e-carregar)
- [Licença](#licença)

---

## Visão Geral

O OrbitScout foi desenvolvido para rastrear satélites de forma autônoma. Graças à integração com um módulo GPS, o sistema coleta automaticamente a localização e a hora atuais, dispensando a entrada manual. Além disso, o sensor inercial auxilia na exibição da trajetória real do satélite, ajustando o gráfico de rastreamento de acordo com a orientação do dispositivo.

---

## Principais Funcionalidades

- **Localização e Hora Automáticas:**  
  O GPS fornece, em tempo real, dados de latitude, longitude, altitude e hora, eliminando a necessidade de configuração manual.

- **Rastreamento de Satélites:**  
  A partir dos TLEs (Two-Line Elements) obtidos via Wi‑Fi, o sistema utiliza cálculos orbitais (SGP4) para determinar a posição dos satélites e gerar previsões de passagens.

- **Visualização da Trajetória Real:**  
  Os dados do sensor inercial permitem que o sistema exiba a trajetória real do satélite, ajustando o gráfico de tracking conforme a orientação do dispositivo.

- **Interface Gráfica Completa:**  
  Exibe menus interativos, gráficos (barras de progresso para brilho, bateria e sinal Wi‑Fi), informações GNSS e dados dos TLEs.

- **Notificações e Alertas:**  
  Permite configurar alertas sonoros para notificar quando um satélite estiver visível (elevação > 0).

- **Atualização Automática de TLEs via Celestrak:**  
  O sistema utiliza a API da Celestrak para obter os TLEs mais atualizados, armazenando-os no SPIFFS.

- **Conexão Wi‑Fi:**  
  Conecta-se a uma rede Wi‑Fi para baixar os TLEs e atualizar os dados dos satélites, exibindo a qualidade do sinal (RSSI) na interface.

---

## Arquitetura e Módulos

### Configuração (Config.h)

Contém todas as definições de pinos e parâmetros essenciais para o funcionamento do sistema:
- **Botões:** `BTN_NEXT`, `BTN_PREV`, `BTN_SELECT`, `BTN_BACK`
- **Saídas:** Pino para backlight e para o buzzer.
- **Entradas:** Pino de leitura da bateria.
- **Comunicação:** Pinos I2C (SDA e SCL) e pinos para o módulo GPS (RX e TX).

---

### Monitoramento de Bateria (BatteryMonitor)

- **Função:**  
  Realiza a leitura da tensão da bateria através do ADC, converte para tensão real utilizando um divisor e calcula a porcentagem de carga.

---

### GPS para Localização e Tempo Real (GPS.h)

- **Função:**  
  Obtém automaticamente a localização (latitude, longitude, altitude) e a hora atual utilizando o módulo GPS. Em caso de dados inválidos, utiliza os valores salvos via SPIFFS como fallback.

---

### Interface e Menu (MenuManager)

- **Função:**  
  Gerencia a interface gráfica no display TFT, permitindo a navegação por meio de botões e a execução de funções associadas aos itens do menu.

---

### Barras de Progresso (ProgressBar)

- **Função:**  
  Desenha barras de progresso no display para indicar indicadores visuais, como brilho, carga da bateria e qualidade do sinal Wi‑Fi.

---

### Conexão Wi‑Fi (WiFiManager)

- **Função:**  
  Gerencia a conexão do dispositivo à rede Wi‑Fi, essencial para o download dos TLEs. Também exibe graficamente a qualidade do sinal (RSSI) no display.

---

### Rastreamento de Satélites (SatelliteTracker)

- **Função:**  
  Utiliza os TLEs para calcular a posição dos satélites via a biblioteca SGP4, gerar previsões de passagens (AOS/LOS) e exibir trajetórias num gráfico polar.

---

### Notificações (NotificationManager)

- **Função:**  
  Permite ao usuário configurar notificações (alertas sonoros) para passagens de satélites. O sistema verifica periodicamente se o satélite está visível (elevação > 0) e aciona o alerta, se necessário.

---

### Orientação e Tracking (OrientationManager)

- **Função:**  
  Utiliza o sensor inercial BNO55 para obter os ângulos de Euler e auxiliar na visualização da trajetória real do satélite. Com base nos dados de heading e pitch, calcula a posição de um ponteiro que indica a direção e o deslocamento relativo à posição central do gráfico.

---

### Gerenciamento de TLE (TleManager)

- **Função:**  
  Baixa e atualiza os TLEs dos satélites a partir de fontes online, utilizando a API da Celestrak para garantir que os dados estejam sempre atualizados. Os TLEs são armazenados no SPIFFS para uso no rastreamento.

---

## Pinagens

Esta seção reúne os principais pinos utilizados no projeto (baseado em uma configuração para ESP32):

- **Display TFT (ST7789):**
  - **MOSI:** GPIO 23
  - **SCLK:** GPIO 18
  - **CS (Chip Select):** GPIO 5
  - **DC (Data/Command):** GPIO 2
  - **RST (Reset):** GPIO 4
  - **BL (Backlight):** GPIO 15

- **Módulo GPS:**
  - **RX:** (definido em Config.h, conforme seu esquema – normalmente um pino disponível)
  - **TX:** (definido em Config.h)

- **Sensor Inercial:**
  - Conectado via I2C:
    - **SDA:** GPIO definido (consulte a seção de Configuração – por exemplo, GPIO 22)
    - **SCL:** GPIO definido (por exemplo, GPIO 21)

- **Bateria:**
  - **Pino de leitura:** Definido em Config.h (ex.: GPIO 33)

- **Botões de Navegação:**
  - **BTN_NEXT, BTN_PREV, BTN_SELECT, BTN_BACK:** Definidos em Config.h

- **Buzzer:**
  - **Pino:** Definido em Config.h (ex.: GPIO 13)

> **Nota:**  
> As pinagens exatas podem ser ajustadas no arquivo `Config.h` conforme o hardware disponível. Verifique e adapte os números dos pinos de acordo com o seu projeto.

---

## Requisitos de Hardware e Dependências

### Hardware

- **Microcontrolador:** Ex. ESP32 (o código utiliza bibliotecas como WiFi, SPIFFS e funções PWM).  
- **Display TFT:** Compatível com a biblioteca TFT_eSPI e configurado para o driver ST7789.  
- **Módulo GPS:** BN280 Responsável por fornecer localização e tempo em tempo real.  
- **Sensor Inercial:** BNO055 para orientação e auxílio no tracking.  
- **Bateria:** Monitorada via pino ADC (com divisor de tensão adequado).  
- **Botões Físicos:** Quatro botões para navegação (NEXT, PREV, SELECT, BACK).  
- **Buzzer:** Para alertas sonoros.

### Dependências (Bibliotecas Arduino)

- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- [WiFi](https://www.arduino.cc/reference/en/libraries/wifi/)
- [SPIFFS](https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS)
- [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus)
- [BNO055](https://github.com/adafruit/Adafruit_BNO055)
- [Sgp4](https://github.com/ianmcgregor/Sgp4) (ou versão compatível)
- [TimeLib](https://github.com/PaulStoffregen/Time)

---

## Como Compilar e Carregar

1. **Configuração do Ambiente:**  
   - Utilize o Arduino IDE ou PlatformIO.  
   - Se estiver usando um ESP32, selecione a placa correta e a porta serial apropriada.

2. **Instale as Dependências:**  
   - Instale todas as bibliotecas listadas na seção de dependências.

3. **Carregamento do Firmware:**  
   - Compile o projeto.  
   - Carregue o firmware no dispositivo.


---

## Licença

Este projeto é disponibilizado sob [especifique sua licença – ex.: MIT License].  
Consulte o arquivo `LICENSE` para maiores detalhes.

---
