#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Sgp4.h>
#include <Ticker.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Definições do display OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Definições dos botões
#define BUTTON_PIN_NEXT 25
#define BUTTON_PIN_PREV 26
#define BUTTON_PIN_SELECT 27  // Botão adicional para selecionar uma opção

// Constantes e objetos globais
Ticker tkSecond;
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
Sgp4 sat;
int timezone = -4;
int currentSatelliteIndex = 0;
int currentMenuIndex = 0;
int topMenuIndex = 0;  // Index do item superior visível no menu

// Variáveis de tempo
int year, mon, day, hr, minute;
double sec;

// Estrutura para armazenar dados TLE de vários satélites
struct SatelliteData {
    char name[20];
    char tle_line1[70];
    char tle_line2[70];
};

// Lista de satélites
SatelliteData satellites[] = {
    {"NOAA 18", "1 28654U 05018A   24243.75797502  .00000533  00000-0  30732-3 0  9995", "2 28654  98.8706 320.0940 0013856 208.0464 151.9963 14.13303826993792"},
    {"NOAA 19", "1 33591U 09005A   24243.86168433  .00000539  00000-0  31282-3 0  9995", "2 33591  99.0410 300.4304 0014805  83.1049 277.1806 14.13096718802172"},
    {"NOAA 15", "1 25338U 98030A   24243.78988673  .00000627  00000-0  27672-3 0  9994", "2 25338  98.5660 269.3509 0011493 105.6073 254.6376 14.26684098368076"},
    {"METEOR M2", "1 40069U 14037A   24244.58341547  .00000495  00000-0  24728-3 0  9991", "2 40069  98.4425 235.4727 0006641 131.1049 229.0703 14.21039520526338"},
    {"METEOR M2-3", "1 57166U 23091A   24243.81137974  .00000136  00000-0  78355-4 0  9997", "2 57166  98.7067 297.7936 0002842 274.1616  85.9238 14.23918791 61243"},
    {"METEOR M2-4", "1 59051U 24039A   24243.84785053  .00000201  00000-0  11004-3 0  9995", "2 59051  98.6047 203.9276 0005860 269.1480  90.9026 14.22257640 26102"},
    {"METEOR M2-2", "1 44387U 19038A   24244.54875932  .00000608  00000-0  28462-3 0  9995", "2 44387  98.8346 214.0964 0002430  96.5923 263.5532 14.23949318268113"},
    {"ISS (ZARYA)", "1 25544U 98067A   24244.61296303  .00007110  00000-0  13764-3 0  9990", "2 25544  51.6414  91.2147 0005455 149.8476 323.9777 15.50045178394477"},
};
const int numSatellites = sizeof(satellites) / sizeof(SatelliteData);
const int maxVisibleItems = SCREEN_HEIGHT / 12;

// Função para exibir o menu com rolagem
void displayMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    if (currentMenuIndex == 0) {
        display.setCursor(0, 0);
        display.print("Selecione Satelite:");

        for (int i = 0; i < maxVisibleItems; i++) {
            int satelliteIndex = topMenuIndex + i;
            if (satelliteIndex >= numSatellites) break;

            display.setCursor(0, 10 + i * 10);
            if (satelliteIndex == currentSatelliteIndex) {
                display.print("> ");
            } else {
                display.print("  ");
            }
            display.print(satellites[satelliteIndex].name);
        }
    } else if (currentMenuIndex == 1) {
        display.setCursor(0, 0);
        display.print("Monitorando:");
        display.setCursor(0, 10);
        display.print(satellites[currentSatelliteIndex].name);
    }

    display.display();
}

// Função para atualizar os dados do satélite a cada segundo
void Second_Tick() {
    if (currentMenuIndex == 1) {  // Only update if on the monitoring screen
        invjday(sat.satJd, timezone, true, year, mon, day, hr, minute, sec);
        
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        // Display date and time
        display.setCursor(0, 0);
        display.print(String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));

        // Display satellite name
        display.setCursor(0, 10);
        display.print("Sat: " + String(satellites[currentSatelliteIndex].name));

        // Display satellite azimuth and elevation
        display.setCursor(0, 20);
        display.print("Az: " + String(sat.satAz) + " El: " + String(sat.satEl));

        // Display satellite distance
        display.setCursor(0, 30);
        display.print("Dist: " + String(sat.satDist) + " km");

        // Display satellite visibility status
        display.setCursor(0, 40);
        switch (sat.satVis) {
            case -2:
                display.print("Vis: Under horizon");
                break;
            case -1:
                display.print("Vis: Daylight");
                break;
            default:
                display.print("Vis: " + String(sat.satVis)); // 0: eclipsed - 1000: visible
                break;
        }

        // Update the display with all the new data
        display.display();
    }
}


// Função para inicializar os dados do satélite
void initSatellite(int index) {
    sat.init(satellites[index].name, satellites[index].tle_line1, satellites[index].tle_line2);

    double jdC = sat.satrec.jdsatepoch;
    invjday(jdC, timezone, true, year, mon, day, hr, minute, sec);
}

// Função para processar dados do GPS e atualizar a localização do satélite
void processGPSData() {
    if (gps.date.isUpdated() && gps.time.isUpdated()) {
        struct tm timeinfo;
        timeinfo.tm_year = gps.date.year() - 1900;
        timeinfo.tm_mon = gps.date.month() - 1;
        timeinfo.tm_mday = gps.date.day();
        timeinfo.tm_hour = gps.time.hour();
        timeinfo.tm_min = gps.time.minute();
        timeinfo.tm_sec = gps.time.second();
        timeinfo.tm_isdst = 0;

        time_t unixtime = mktime(&timeinfo);

        if (gps.location.isValid() && gps.altitude.isValid()) {
            double lat = gps.location.lat();
            double lon = gps.location.lng();
            double alt = gps.altitude.meters();
            sat.site(lat, lon, alt);
        }

        sat.findsat(static_cast<unsigned long>(unixtime));
    }
}

// Função para gerenciar os botões e o menu
void handleButtonPresses() {
    if (digitalRead(BUTTON_PIN_NEXT) == LOW) {
        if (currentMenuIndex == 0) {  // Navega entre os satélites
            currentSatelliteIndex = (currentSatelliteIndex + 1) % numSatellites;

            if (currentSatelliteIndex >= topMenuIndex + maxVisibleItems) {
                topMenuIndex++;
            }
            displayMenu();
        }
        delay(200); // Debounce
    }

    if (digitalRead(BUTTON_PIN_PREV) == LOW) {
        if (currentMenuIndex == 0) {  // Navega entre os satélites
            currentSatelliteIndex = (currentSatelliteIndex - 1 + numSatellites) % numSatellites;

            if (currentSatelliteIndex < topMenuIndex) {
                topMenuIndex--;
            }
            displayMenu();
        }
        delay(200); // Debounce
    }

    if (digitalRead(BUTTON_PIN_SELECT) == LOW) {
        if (currentMenuIndex == 0) {  // Seleciona o satélite e inicia o monitoramento
            initSatellite(currentSatelliteIndex);
            currentMenuIndex = 1;  // Vai para a tela de monitoramento
        } else if (currentMenuIndex == 1) {  // Volta para o menu de seleção
            currentMenuIndex = 0;
        }
        displayMenu();
        delay(200); // Debounce
    }
}

void setup() {
    Serial.begin(115200);
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

    // Inicializa o display OLED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Falha na inicialização do display SSD1306"));
        for(;;);
    }
    display.clearDisplay();
    display.display();
    
    // Configura os pinos dos botões
    pinMode(BUTTON_PIN_NEXT, INPUT_PULLUP);
    pinMode(BUTTON_PIN_PREV, INPUT_PULLUP);
    pinMode(BUTTON_PIN_SELECT, INPUT_PULLUP);

    // Exibe o menu inicial
    displayMenu();
    
    // Configura o ticker para atualizar os dados do satélite a cada segundo
    tkSecond.attach(1, Second_Tick);
}

void loop() {
    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
        processGPSData();
    }
    handleButtonPresses();
}
