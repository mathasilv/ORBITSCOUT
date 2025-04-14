
# OrbitScout: Standalone Satellite Tracker


## Visão Geral

**OrbitScout** é um projeto open-source que transforma um microcontrolador ESP32 em uma estação de acompanhamento de satélites. Com uma interface gráfica em um display TFT, módulos de GPS, sensor de orientação e monitoramento de bateria, o OrbitScout permite prever passagens de satélites, visualizar trajetórias em tempo real e até receber notificações sonoras quando uma passagem se aproxima.

## Funcionalidades

- **Rastreamento de Satélites em Tempo Real:** Utiliza o modelo SGP4 para calcular a posição dos satélites a partir dos dados TLE.
- **Integração com GPS:** Obtém a localização exata do usuário para cálculos precisos das passagens dos satélites.
- **Sensoriamento de Orientação:** Incorpora o sensor BNO055 para fornecer dados de orientação e permitir o rastreamento manual.
- **Interface Gráfica Intuitiva:** Menus interativos com barras de progresso, notificações e visualizações dinâmicas no display TFT.
- **Controle de Backlight:** Ajuste do brilho do display via botões físicos.
- **Monitoramento de Bateria:** Leitura analógica que converte valores do ADC em tensão real e porcentagem de carga.
- **Conectividade WiFi:** Configuração via portal cativo e download automático de dados TLE de várias fontes online.
- **Atualização e Gerenciamento de TLEs:** Permite escolher entre diversas fontes (NOAA, Weather, Engineering, etc.) para manter os dados de órbita atualizados.
- **Sistema de Notificações:** Alertas visuais e sonoros quando um satélite está prestes a passar.

## Requisitos de Hardware

- **Microcontrolador:** ESP32
- **Display:** TFT compatível com a biblioteca
- **GPS:** Módulo GPS (ex.: Neo-6M) para localização
- **Sensor de Orientação:** Adafruit BNO055
- **Circuito de Monitoramento de Bateria:** Divisor de tensão
- **Buzzer:** Para alertas sonoros
- **Botões Físicos:** Para navegação (BTN_NEXT, BTN_PREV, BTN_SELECT, BTN_BACK)
- **Outros:** Fios, protoboard e componentes de suporte para montagem

## Requisitos de Software

- **Plataforma:** Arduino IDE ou PlatformIO
- **Bibliotecas Necessárias:**
  - [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
  - [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus)
  - [Adafruit_BNO055](https://github.com/adafruit/Adafruit_BNO055)
  - [TimeLib](https://github.com/PaulStoffregen/Time)
  - [WiFiManager](https://github.com/tzapu/WiFiManager)
  - [Sgp4](https://github.com/dnwrnr/Arduino-SGP4)

## Estrutura do Projeto

A organização do projeto é modular, com cada componente responsável por uma parte específica do funcionamento do OrbitScout:

```
OrbitScout/
├── src
│   ├── main.cpp                 # Inicialização e loop principal
│   ├── BacklightControl.cpp     # Controle do backlight via PWM
│   ├── BatteryMonitor.cpp       # Leitura e cálculo da bateria
│   ├── gps.cpp                  # Processamento dos dados do GPS
│   ├── MenuManager.cpp          # Sistema de menu e interface de usuário
│   ├── NotificationManager.cpp  # Gerenciamento de notificações e alertas
│   ├── OrbitScoutWiFi.cpp       # Conectividade WiFi e download de TLEs
│   ├── OrientationManager.cpp   # Integração com o sensor BNO055
│   ├── ProgressBar.cpp          # Renderização de barras de progresso
│   ├── SatelliteTracker.cpp     # Rastreamento de satélites com SGP4
│   └── TleManager.cpp           # Atualização e gerenciamento dos dados TLE
└── include
    ├── Config.h                 # Configurações de pinos e constantes
    ├── DisplayConstants.h       # Layout e dimensões do display
    ├── BacklightControl.h       
    ├── BatteryMonitor.h        
    ├── MenuManager.h            
    ├── NotificationManager.h    
    ├── OrbitScoutWiFi.h         
    ├── OrientationManager.h     
    ├── ProgressBar.h            
    ├── SatelliteTracker.h       
    ├── TleManager.h             
    └── TleSources.h             # Fontes de dados TLE
```

## Como Começar

### 1. Clonando o Repositório

```bash
git clone https://github.com/seuusuario/OrbitScout.git
cd OrbitScout
```

### 2. Configuração de Hardware

- Monte o circuito conectando o ESP32 ao TFT, GPS, sensor BNO055, módulo de bateria, buzzer e botões, conforme as definições em `Config.h`.

### 3. Instalação de Dependências

- Instale as bibliotecas necessárias através do Arduino Library Manager ou via PlatformIO conforme listado na seção de Software.

### 4. Upload do Firmware

- Abra o arquivo `main.cpp`.
- Selecione a placa e a porta corretas.
- Compile e faça o upload do firmware para o microcontrolador.

## Uso

- **Navegação:** Utilize os botões físicos para navegar pelos menus e ajustar configurações, como o brilho do display.
- **Configuração WiFi:** Se não estiver conectado a uma rede, o OrbitScout iniciará um portal cativo para que você possa inserir as credenciais WiFi.
- **Rastreamento de Satélites:** No menu principal, acesse as opções de rastreamento para visualizar a posição e trajetória dos satélites. Selecione um satélite e visualize suas passagens.
- **Notificações:** Enquanto visualiza as passagens, pressione o botão SELECT na passagem desejada para configurar um alerta. Você será notificado automaticamente quando o satélite iniciar essa passagem.
- **Monitoramento:** Confira o status da bateria e outros dados dinâmicos na interface do display.


## Contribuição

Contribuições são muito bem-vindas! Sinta-se livre para forkar o repositório, fazer melhorias e submeter pull requests. Se encontrar algum bug ou tiver sugestões, abra uma *issue* para discussão.

## Licença

Este projeto é licenciado sob a [MIT License](LICENSE). Consulte o arquivo de licença para mais detalhes.
