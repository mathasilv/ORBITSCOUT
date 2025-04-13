#include "gps.h"
#include <SPIFFS.h>
#include <Arduino.h>
#include <TinyGPSPlus.h>

// Instância do objeto TinyGPSPlus e do HardwareSerial para o GPS
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// Variável para armazenar o fuso horário (padrão = 0)
static int timezone = 0;

// ---------------------------------------------------------------------
// Estrutura para armazenar a última posição conhecida
// ---------------------------------------------------------------------
struct LatLonAlt {
  double latitude;
  double longitude;
  double altitude;
};

// Valor padrão para a última posição conhecida
static LatLonAlt lastPosition = {0.0, 0.0, 0.0};

// Thresholds para detectar variação significativa e evitar escrita frequente no SPIFFS
const double LAT_THRESHOLD = 0.0001;
const double LON_THRESHOLD = 0.0001;
const double ALT_THRESHOLD = 1.0; // em metros

// Flag para modo debug
static const bool DEBUG = false;

//
// Função para armazenar a posição em SPIFFS.
// Grava somente se a posição tiver mudado significativamente em relação à última salva.
//
static void storeLatLonAltInSPIFFS(const LatLonAlt &pos) {
  // Abre o arquivo para escrita (sobrescreve os dados anteriores)
  File file = SPIFFS.open("/gps_data.bin", FILE_WRITE);
  if (!file) {
    if (DEBUG) Serial.println("Erro ao abrir arquivo para escrita no SPIFFS.");
    return;
  }
  double values[3] = { pos.latitude, pos.longitude, pos.altitude };
  size_t written = file.write(reinterpret_cast<const uint8_t*>(values), sizeof(values));
  if (written != sizeof(values)) {
    if (DEBUG) Serial.println("Erro ao escrever dados completos no SPIFFS.");
  } else {
    if (DEBUG) Serial.println("Dados do GPS salvos no SPIFFS.");
  }
  file.close();
}

//
// Função para ler a posição armazenada no SPIFFS.
// Se ocorrer erro, inicializa com valores padrão e regrava.
//
static void readLatLonAltFromSPIFFS(LatLonAlt &pos) {
  File file = SPIFFS.open("/gps_data.bin", FILE_READ);
  if (!file) {
    if (DEBUG) Serial.println("Erro ao abrir arquivo para leitura no SPIFFS.");
    pos = {0.0, 0.0, 0.0};
    storeLatLonAltInSPIFFS(pos);
    return;
  }
  double values[3] = {0.0, 0.0, 0.0};
  const size_t expectedSize = sizeof(values);
  if (file.size() < expectedSize) {
    if (DEBUG) Serial.println("Tamanho do arquivo no SPIFFS inválido. Inicializando com valores padrão.");
    pos = {0.0, 0.0, 0.0};
    file.close();
    storeLatLonAltInSPIFFS(pos);
    return;
  }
  size_t readCount = file.read(reinterpret_cast<uint8_t*>(values), expectedSize);
  if (readCount != expectedSize) {
    if (DEBUG) Serial.println("Erro ao ler dados completos do SPIFFS. Inicializando com valores padrão.");
    pos = {0.0, 0.0, 0.0};
    file.close();
    storeLatLonAltInSPIFFS(pos);
    return;
  }
  pos.latitude  = values[0];
  pos.longitude = values[1];
  pos.altitude  = values[2];
  file.close();
}

void sendUBXCommand(const uint8_t *msg, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
      gpsSerial.write(msg[i]);
  }
  Serial.println("Comando enviado.");
}

uint8_t enableAllGNSS[] = { 
  0xB5, 0x62, 0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00, 0x20, 0x06, 0x00, 0x01, 0x00, 0x01, 0x01, 
  0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x00, 0xD1, 0x23 
};


//
// Inicialização do GPS e do SPIFFS.
// Caso o SPIFFS não seja iniciado corretamente, exibe mensagem de erro.
//
void setupGPS() {
  // Inicializa o SPIFFS (com formatação caso necessário)
  if (!SPIFFS.begin(true)) {
    Serial.println("Falha ao montar SPIFFS.");
    return;
  }

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS inicializado.");
  sendUBXCommand(enableAllGNSS, sizeof(enableAllGNSS));

}

//
// Processa os dados recebidos do GPS.
// Deve ser chamado com frequência (por exemplo, no loop principal).
//
void updateGPS() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
    Serial.print(c);
  }
}

void updateLastPosition() {
  if (gps.location.isValid() && gps.altitude.isValid()) {
    double currentLat = gps.location.lat();
    double currentLng = gps.location.lng();
    double currentAlt = gps.altitude.meters();

    // Verifica se lastPosition ainda está com os valores padrão
    bool isDefault = (lastPosition.latitude == 0.0 &&
                      lastPosition.longitude == 0.0 &&
                      lastPosition.altitude == 0.0);

    // Se for o primeiro fix (valores padrão) ou se a mudança for maior que os thresholds,
    // atualiza a posição e armazena no SPIFFS
    if (isDefault ||
        fabs(currentLat - lastPosition.latitude) > LAT_THRESHOLD ||
        fabs(currentLng - lastPosition.longitude) > LON_THRESHOLD ||
        fabs(currentAlt - lastPosition.altitude) > ALT_THRESHOLD) {
      lastPosition.latitude  = currentLat;
      lastPosition.longitude = currentLng;
      lastPosition.altitude  = currentAlt;
      storeLatLonAltInSPIFFS(lastPosition);
    }
  } else {
    // Se não houver fix válido, tenta ler a última posição salva no SPIFFS
    readLatLonAltFromSPIFFS(lastPosition);
  }
}



//
// Função central para atualizar os dados do GPS e a última posição.
// Chame esta função no início de cada ciclo do loop principal.
//
void updateGPSData() {
  updateGPS();
  updateLastPosition();
}

// ------------------------
// Funções para acessar os dados
// ------------------------

int getTimezone() {
  return timezone;
}

void setTimezone(int tz) {
  if (tz < -12 || tz > 14) {
    Serial.println("Fuso horário inválido. Use valores entre -12 e 14.");
    return;
  }
  timezone = tz;
}

double getCurrentLatitude() {
  return lastPosition.latitude;
}

double getCurrentLongitude() {
  return lastPosition.longitude;
}

double getCurrentAltitude() {
  return lastPosition.altitude;
}

//
// Retorna a data atual formatada.
// Caso não haja fix válido, retorna "00/00/0000".
//
String getCurrentDate() {
  if (!gps.date.isValid()) {
    return "00/00/0000";
  }
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d",
           gps.date.day(), gps.date.month(), gps.date.year());
  return String(buffer);
}

//
// Retorna a hora atual formatada considerando o fuso horário.
// Observe que o ajuste de fuso horário afeta somente a hora,
// ou seja, se o ajuste resultar em mudança de data, este não é alterado.
//
String getCurrentTime() {
  if (!gps.time.isValid()) {
    return "00:00:00";
  }
  // Ajuste do fuso horário com módulo para manter a hora entre 0 e 23
  int hora = (gps.time.hour() + timezone + 24) % 24;
  
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
           hora, gps.time.minute(), gps.time.second());
  return String(buffer);
}

double getCurrentSpeed() {
  if (gps.speed.isValid()) {
    return gps.speed.kmph();
  } else {
    if (DEBUG) Serial.println("Velocidade inválida.");
    return 0.0;
  }
}

double getCurrentCourse() {
  if (gps.course.isValid()) {
    return gps.course.deg();
  } else {
    if (DEBUG) Serial.println("Curso inválido.");
    return 0.0;
  }
}

int getCurrentSatellites() {
  if (gps.satellites.isValid()) {
    return gps.satellites.value();
  } else {
    if (DEBUG) Serial.println("Satélites inválidos.");
    return 0;
  }
}

double getCurrentHDOP() {
  if (gps.hdop.isValid()) {
    return gps.hdop.hdop();
  } else {
    if (DEBUG) Serial.println("HDOP inválido.");
    return 0.0;
  }
}

int getCurrentYear() {
  if (gps.date.isValid()) {
    return gps.date.year();
  } else {
    if (DEBUG) Serial.println("Ano inválido.");
    return 0;
  }
}

int getCurrentMonth() {
  if (gps.date.isValid()) {
    return gps.date.month();
  } else {
    if (DEBUG) Serial.println("Mês inválido.");
    return 0;
  }
}

int getCurrentDay() {
  if (gps.date.isValid()) {
    return gps.date.day();
  } else {
    if (DEBUG) Serial.println("Dia inválido.");
    return 0;
  }
}

int getCurrentHour() {
  if (gps.time.isValid()) {
    return gps.time.hour();
  } else {
    if (DEBUG) Serial.println("Hora inválida.");
    return 0;
  }
}

int getCurrentMinute() {
  if (gps.time.isValid()) {
    return gps.time.minute();
  } else {
    if (DEBUG) Serial.println("Minuto inválido.");
    return 0;
  }
}

int getCurrentSecond() {
  if (gps.time.isValid()) {
    return gps.time.second();
  } else {
    if (DEBUG) Serial.println("Segundo inválido.");
    return 0;
  }
}
