#include "header.h"
#include "soundlevelsensor.h"

void soundlevel() {
    float voltageValue, dbValue;
    voltageValue = analogRead(SoundSensorPin) / 4095.0 * VREF;
    Serial.print("voltage: ");
    Serial.println(voltageValue);
    dbValue = voltageValue * 50.0;  //convert voltage to decibel value
    // dbValue = voltageValue * 33.0;  //convert voltage to decibel value
    Serial.print(dbValue,1);
    Serial.println(" dBA");
    delay(125);
}