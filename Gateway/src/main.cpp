#include "dataconnection.h"
#include "header.h"
#include "notify.h"
#include "private.h"

// Sensors
//  #include "motionsensor.h"
//  #include "co2sensor.h"
//  #include "distancesensor.h"
//  #include "sixDOFsensor.h"
// #include "envsensor.h"
// #include "pressAltsensor.h"
// #include "soundlevelsensor.h"
// #include "bloodoxygensensor.h"    //unreliable HR measurement

// LED
#define LED 2
int ledState = LOW;                     // ledState used to set the LED
unsigned long previousMillis = 0;       // will store last time LED was updated
unsigned long setupPreviousMillis = 0;  // will store last time setup BLE was updated

int timecounter = 0;

// RGB LED
#define PIN 5
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(10, PIN, NEO_GRB + NEO_KHZ800);

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(PUSHBUTTON, INPUT_PULLUP);

  flickerLed();
  initvector();  

  setupwifi();

  while (!deviceConnected) {
    unsigned long setupMillis = millis();
    if (setupMillis - setupPreviousMillis >= 1000) {
      setupPreviousMillis = setupMillis;
      BLEConnectionNotify();
    }
  }

// Sensors
#ifdef MOTION_PIN
  pinMode(MOTION_PIN, INPUT);
#endif
#ifdef CO2SENSOR_H
  pinMode(BOOL_PIN, INPUT);      // set pin to input
  digitalWrite(BOOL_PIN, HIGH);  // turn on pullup resistors
#endif
#ifdef DISTANCESENSOR_H
  Wire.begin();
  sensor.begin(0x50);
  sensor.setMode(sensor.eContinuous, sensor.eHigh);
  sensor.start();
#endif
#ifdef SIXDOFSENSOR_H
  Wire.begin();
  accelgyro.initialize();
#endif
#ifdef ENVSENSOR_H
  while (environment.begin() != 0) {
    Serial.println(".");
    delay(1000);
  }
#endif
#ifdef PRESSALTSENSOR_H
  Wire.begin();
  bmp280.begin();
#endif
#ifdef SOUNDLEVELSENSOR_H
  // pinMode(SoundSensorPin, INPUT);
#endif
#ifdef BLODOXYGENSENSOR_H
  while (false == MAX30102.begin()) {
    Serial.println(".");
    delay(1000);
  }
  MAX30102.sensorStartCollect();
#endif
}

void loop() {
  Notify();

  if (digitalRead(PUSHBUTTON) == LOW) {
    timecounter = 0;
  }

  if (WiFi.status() == WL_CONNECTED && !tb.connected()) {
    pixel.setPixelColor(0, pixel.Color(0, 0, 255));  // blue
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    pixel.setBrightness(50);
    pixel.show();
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
    Serial.println("Connected to ThingsBoard");
  }
  if (WiFi.status() == WL_CONNECTED && tb.connected() && sensorData.order.size() > 0) {
    pixel.setPixelColor(0, pixel.Color(0, 255, 0));  // green
    // Serial << "Order:" << sensorData.order << endl;

    switch (sensorData.order.front()) {
      case TIME:
        tb.sendTelemetryString("time", sensorData.time.front().c_str());
        sensorData.time.remove(0);
        break;
      case HR:
        tb.sendTelemetryInt("HR", sensorData.heartrate.front());
        sensorData.heartrate.remove(0);
        break;
      case LAT:
        tb.sendTelemetryString("latitude", sensorData.latitude.front().c_str());
        sensorData.latitude.remove(0);
        break;
      case LON:
        tb.sendTelemetryString("longitude", sensorData.longitude.front().c_str());
        sensorData.longitude.remove(0);
        break;
      case POWER:
        tb.sendTelemetryInt("power", sensorData.power.front());
        sensorData.power.remove(0);
        break;
      case SPEED:
        tb.sendTelemetryData("speed", sensorData.speed.front());
        sensorData.speed.remove(0);
        break;
      case DISTANCE:
        tb.sendTelemetryData("distance", sensorData.distance.front());
        sensorData.distance.remove(0);
        break;
      case ALTITUDE:
        tb.sendTelemetryInt("altitude", sensorData.altitude.front());
        sensorData.altitude.remove(0);
        break;
      case BATTERY:
        tb.sendTelemetryInt("battery", sensorData.battery.front());
        sensorData.battery.remove(0);
        break;
      default:
        Serial.println("Error: unknown sensor data" + String(sensorData.order.front()));
        break;
    }
    tb.sendTelemetryInt("test", random(0, 100));

    // cleaning duties
    sensorData.order.remove(0);
    delay(100);  // to keep the connection high, below 100ms will cause connection to drop
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi reconnecting");
    pixel.setPixelColor(0, pixel.Color(255, 0, 0));  // red
    pixel.show();
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  pixel.setBrightness(100);
  pixel.show();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 2000) {
    previousMillis = currentMillis;
    ledState == LOW ? ledState = HIGH : ledState = LOW;
    digitalWrite(LED, ledState);
  }

// Sensors
#ifdef MOTION_PIN
  if (digitalRead(MOTION_PIN))
    Serial.println("Motion Detected");
  else
    Serial.println("No Motion Detected");
#endif
#ifdef CO2SENSOR_H
  loopco2();
#endif
#ifdef DISTANCESENSOR_H
  Serial.print("Distance: ");
  Serial.println(sensor.getDistance());
#endif
#ifdef SIXDOFSENSOR_H
  sixDOFsensor();
#endif
#ifdef ENVSENSOR_H
  envsensor();  // remove delay later

#endif
#ifdef PRESSALTSENSOR_H
  pressAltsensor();
#endif
#ifdef SOUNDLEVELSENSOR_H
  soundlevel();
#endif
#ifdef BLODOXYGENSENSOR_H
  bloodoxygen();
#endif

  // tb.loop();
}

void pixelOn() {
  pixel.setBrightness(50);
  pixel.setPixelColor(0, pixel.Color(255, 192, 203));  // pink
  pixel.show();
}

void pixelOff() {
  pixel.clear();
  pixel.setBrightness(0);
  pixel.show();
}

void flickerLed() {
  pixelOff();
  delay(50);
  pixelOn();
  delay(50);
  pixelOff();
  delay(50);
  pixelOn();
}