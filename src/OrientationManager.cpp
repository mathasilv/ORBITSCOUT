#include "OrientationManager.h"
#include <Arduino.h>
#include <math.h>

extern TFT_eSPI tft;

OrientationManager::OrientationManager(int sensorID, uint8_t address)
    : bno(sensorID, address),
      lastPointerX(-1),
      lastPointerY(-1)
{
}

bool OrientationManager::begin() {
    if (!bno.begin()) {
        Serial.println("Falha ao inicializar o BNO055!");
        return false;
    }
    return true;
}

imu::Vector<3> OrientationManager::getEulerAngles() {
    sensors_event_t event;
    bno.getEvent(&event);

    return imu::Vector<3>(event.orientation.x, event.orientation.y, event.orientation.z);
}

void OrientationManager::computePointer(int centerX, int centerY, int radius, int &pointerX, int &pointerY) {
    sensors_event_t event;
    bno.getEvent(&event);
    
    float heading = event.orientation.x;
    float pitch = event.orientation.z;
    
    float adjustedHeading = heading + 90.0f;

    float thetaRad = adjustedHeading * PI / 180.0f;

    float factor = (-90.0f - pitch) / 90.0f;

    pointerX = centerX + static_cast<int>(radius * factor * cos(thetaRad));
    pointerY = centerY + static_cast<int>(radius * factor * sin(thetaRad));
}

void OrientationManager::drawOrientationPointer(int centerX, int centerY, int radius) {
    int pointerX, pointerY;
    computePointer(centerX, centerY, radius, pointerX, pointerY);

    if (lastPointerX != -1 && lastPointerY != -1) {
        tft.drawCircle(lastPointerX, lastPointerY, 6, TFT_BLACK);
    }

    tft.drawCircle(pointerX, pointerY, 6, TFT_WHITE);

    lastPointerX = pointerX;
    lastPointerY = pointerY;
}

void OrientationManager::printOrientation() {
    sensors_event_t event;
    bno.getEvent(&event);
    Serial.print("Heading: ");
    Serial.print(event.orientation.x, 2);
    Serial.print(" | Roll: ");
    Serial.print(event.orientation.y, 2);
    Serial.print(" | Pitch: ");
    Serial.println(event.orientation.z, 2);
}