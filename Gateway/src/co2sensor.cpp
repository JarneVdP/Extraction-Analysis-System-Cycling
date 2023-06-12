#include "co2sensor.h"
#include "header.h"

float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE / (2.602 - 3))};

void loopco2() {
  int percentage;
  float volts;

  volts = MGRead(MG_PIN);
  Serial.print("SEN0159:");
  Serial.print(volts);
  Serial.print("V           ");

  percentage = MGGetPercentage(volts, CO2Curve);
  Serial.print("CO2:");
  if (percentage == -1) {
    Serial.print("<400");
  } else {
    Serial.print(percentage);
  }

  Serial.print("ppm");
  Serial.print("\n");

  if (digitalRead(BOOL_PIN)) {
    Serial.print("=====BOOL is HIGH======");
  } else {
    Serial.print("=====BOOL is LOW======");
  }

  Serial.print("\n");

  delay(500);
}

float MGRead(int mg_pin) {
    int i;
    float v=0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += analogRead(mg_pin);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
    return v;
}

int MGGetPercentage(float volts, float *pcurve) {
  if ((volts / DC_GAIN) >= ZERO_POINT_VOLTAGE) {
    return -1;
  } else {
    return pow(10, ((volts / DC_GAIN) - pcurve[1]) / pcurve[2] + pcurve[0]);
  }
}