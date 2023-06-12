#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <ThingsBoard.h>    
#include <Adafruit_NeoPixel.h>
#include <Vector.h>
#include <Streaming.h>

#define SERVICE_UUID        "5fafc201-2510-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "5eb5483e-2510-4688-b7f5-ea07361b26a8"

#define PUSHBUTTON 27
extern int timecounter;

// LED
void pixelOn();
void pixelOff();
void flickerLed();

enum dataOrder { TIME, HR, LAT, LON, POWER, SPEED, DISTANCE, ALTITUDE, BATTERY };

//Data struct
struct SensorData {
  Vector<String> time;
  Vector<int> heartrate;
  Vector<String> latitude;
  Vector<String> longitude;
  Vector<int> power;
  Vector<double> speed;
  Vector<double> distance;
  Vector<int> altitude;
  Vector<int> battery;
  Vector<int> order;
};

extern SensorData sensorData;