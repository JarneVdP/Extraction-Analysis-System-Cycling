#include "pressAltsensor.h"

#include "header.h"

BMP280 bmp280;

void pressAltsensor() {
  uint32_t pressure = bmp280.getPressure();
  float temperature = bmp280.getTemperature();
  float altitude = bmp280.calAltitude(pressure);

  // Print the results
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C \t");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println("Pa");
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println("m");
    
}