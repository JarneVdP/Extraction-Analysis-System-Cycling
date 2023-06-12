#include "header.h"
#include "envsensor.h"

DFRobot_EnvironmentalSensor environment(/*addr = */ SEN050X_DEFAULT_DEVICE_ADDRESS, /*pWire = */ &Wire);

void envsensor() {
  //Print the data obtained from sensor
  Serial.println("-------------------------------");
  Serial.print("Temp: ");
  Serial.print(environment.getTemperature(TEMP_C));
  Serial.println("C");
  Serial.print("Temp: ");
  Serial.print(environment.getTemperature(TEMP_F));
  Serial.println("F");
  Serial.print("Humidity: ");
  Serial.print(environment.getHumidity());
  Serial.println(" %");
  Serial.print("Ultraviolet intensity: ");
  Serial.print(environment.getUltravioletIntensity());
  Serial.println(" mw/cm2");
  Serial.print("LuminousIntensity: ");
  Serial.print(environment.getLuminousIntensity());
  Serial.println(" lx");
  Serial.print("Atmospheric pressure: ");
  Serial.print(environment.getAtmospherePressure(HPA));
  Serial.println(" hpa");
  Serial.print("Altitude: ");
  Serial.print(environment.getElevation());
  Serial.println(" m");
  Serial.println("-------------------------------");
  delay(2000);
}