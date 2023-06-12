#ifndef BLODOXYGENSENSOR_H
#define BLODOXYGENSENSOR_H

#include "DFRobot_BloodOxygen_S.h"

#define I2C_ADDRESS    0x57
//DFRobot_BloodOxygen_S_I2C MAX30102(&Wire ,I2C_ADDRESS);
extern DFRobot_BloodOxygen_S_I2C MAX30102;

void bloodoxygen();
#endif