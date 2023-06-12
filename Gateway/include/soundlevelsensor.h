#ifndef SOUNDLEVELSENSOR_H  
#define SOUNDLEVELSENSOR_H

#define SoundSensorPin 34  //this pin read the analog voltage from the sound level meter    
#define VREF  3.3  //voltage on AREF pin,default:operating voltage

void soundlevel();
#endif