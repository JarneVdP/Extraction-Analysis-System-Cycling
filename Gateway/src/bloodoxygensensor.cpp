#include "header.h"
#include "bloodoxygensensor.h"

DFRobot_BloodOxygen_S_I2C MAX30102(&Wire ,I2C_ADDRESS);

void bloodoxygen(){
	MAX30102.getHeartbeatSPO2();
  Serial.print("SPO2 is : ");
  Serial.print(MAX30102._sHeartbeatSPO2.SPO2);
  Serial.println("%");
  Serial.print("heart rate is : ");
  Serial.print(MAX30102._sHeartbeatSPO2.Heartbeat);
  Serial.println("Times/min");
  Serial.print("Temperature value of the board is : ");
  Serial.print(MAX30102.getTemperature_C());
  Serial.println(" ℃");
  //The sensor updates the data every 4 seconds. Has to be changed to millis in case it is used 
  delay(4000);
}