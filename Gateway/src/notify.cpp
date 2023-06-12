#include "notify.h"
#include "header.h"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

unsigned long prevMillis = 0;
unsigned long period = 300;

struct SensorData sensorData;
const int ELEMENT_COUNT_MAX = 50;

String storage_T[ELEMENT_COUNT_MAX];
int storage_HR[ELEMENT_COUNT_MAX];
String storage_Lat[ELEMENT_COUNT_MAX];
String storage_Long[ELEMENT_COUNT_MAX];
int storage_Power[ELEMENT_COUNT_MAX];
double storage_Speed[ELEMENT_COUNT_MAX];
double storage_Distance[ELEMENT_COUNT_MAX];
int storage_Altitude[ELEMENT_COUNT_MAX];
int storage_Battery[ELEMENT_COUNT_MAX];
int storage_Order[4 * ELEMENT_COUNT_MAX];  // make it big enough to store a lot of data. 200 is an estimate

float last_lat, last_lon;

void initvector() {
  sensorData.time.setStorage(storage_T);
  sensorData.heartrate.setStorage(storage_HR);
  sensorData.latitude.setStorage(storage_Lat);
  sensorData.longitude.setStorage(storage_Long);
  sensorData.power.setStorage(storage_Power);
  sensorData.speed.setStorage(storage_Speed);
  sensorData.distance.setStorage(storage_Distance);
  sensorData.altitude.setStorage(storage_Altitude);
  sensorData.battery.setStorage(storage_Battery);
  sensorData.order.setStorage(storage_Order);
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};


class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    byte *pData = pCharacteristic->getData();

    int firstDigit = pData[0] / 10;
    int lastDigit = pData[0] % 10;  // gets used for power and altitude

    if (value.length() > 0x00) {
      // Time
      if (pData[0] == 0) {
        int hour = pData[1];
        int minute = pData[2];
        String formattedTime = "";

        if (hour < 10) formattedTime += "0";
        formattedTime += String(hour) + ":";

        if (minute < 10) formattedTime += "0";
        formattedTime += String(minute);

        timecounter++;
        sensorData.time.push_back(formattedTime);
        sensorData.order.push_back(TIME);
        // Serial << "Time:" << endl;
        // Serial << sensorData.time << endl;
      }

      // Heart rate
      if (pData[0] == 0x01) {
        int heartRate = pData[1];

        sensorData.heartrate.push_back(heartRate);
        sensorData.order.push_back(HR);
        // Serial << "HR:" << endl;
        // Serial << sensorData.heartrate << endl;
      }

      // //Location latitude
      // 0x02 = 2 postitive latitude, 0x20 = 32 negative latitude
      // Location longitude
      // 0x0C = 12 positive longitude, 0x2A = 42 negative longitude
      if (pData[0] == 0x02 || pData[0] == 0x20 || pData[0] == 0x0C || pData[0] == 0x2A) {
        int val1 = pData[1];
        int val2 = pData[2];
        int val3 = pData[3];
        float location = 0.0000;

        if (pData[0] == 0x20 || pData[0] == 0x2A) {  // negative coordinates
          val1 = val1 * -1;
          location = val1 - (val2 * 0.01) - (val3 * 0.0001);
        } else {
          location = val1 + (val2 * 0.01) + (val3 * 0.0001);
        }
        String location_s = String(location, 4);  // convert to string because float kept cutting off the last two digits

        if (pData[0] == 0x02 || pData[0] == 0x20) { // latitude
          if (timecounter > 10 && (abs(location - last_lat) > 0.0050)){Serial.println("Location data filtered"); return;}
          // Serial.printf("Last lat: %f, last lon: %f, current lat: %f, current lon: %f, differences %f ; %f \n", last_lat, last_lon, location, location, abs(location - last_lat), abs(location - last_lon));
          last_lat = location;
          sensorData.latitude.push_back(location_s);
          sensorData.order.push_back(LAT);
        }
        if (pData[0] == 0x0C || pData[0] == 0x2A) { // longitude
          if (timecounter > 10 && (abs(location - last_lon) > 0.0050)){Serial.println("Location data filtered"); return;}
          // Serial.printf("Last lat: %f, last lon: %f, current lat: %f, current lon: %f, differences %f ; %f \n", last_lat, last_lon, location, location, abs(location - last_lat), abs(location - last_lon));
          last_lon = location;
          sensorData.longitude.push_back(location_s);
          sensorData.order.push_back(LON);
        }

        Serial << "Lat:" << endl;
        Serial << sensorData.latitude << endl;
        Serial << "Long:" << endl;
        Serial << sensorData.longitude << endl;
      }

      // //Power
      // //powerhex starts at 0x03 and jumps with increments of 10 (0x0A).
      // //calulation is: power =  receivedpower + firstvalue*255

      // //NOT TESTED YET BECAUSE I DON'T HAVE A POWER METER
      if (lastDigit == 3) {
        int receivedpower = pData[1];
        int power = receivedpower + firstDigit * 255;

        sensorData.power.push_back(power);
        sensorData.order.push_back(POWER);
      }

      // Speed
      if (pData[0] == 0x04) {
        int val1 = pData[1];
        int val2 = pData[2];
        double speed = val1 + (val2 * 0.01);

        sensorData.speed.push_back(speed);
        sensorData.order.push_back(SPEED);
        // Serial << "Speed:" << endl;
        // Serial << sensorData.speed << endl;
      }

      // Distance
      // 0x05 = 5 distance less than 255, 0x0F = 15 distance > 255 => received + 255
      if (pData[0] == 0x05 || pData[0] == 0x0F) {
        int val1 = pData[1];
        int val2 = pData[2];

        if (pData[0] == 0x0F) {
          val1 = val1 + 255;
        }
        double distance = val1 + (val2 * 0.01);

        sensorData.distance.push_back(distance);
        sensorData.order.push_back(DISTANCE);
        // Serial << "Distance:" << endl;
        // Serial << sensorData.distance << endl;
      }

      // Altitude
      // altitudehex starts at 0x06 and jumps with increments of 10 (0x0A).
      // calulation is: altitude =  receivedaltitude + firstvalue*255

      if (lastDigit == 6) {
        int receivedaltitude = pData[1];
        int altitude = receivedaltitude + firstDigit * 255;

        sensorData.altitude.push_back(altitude);
        sensorData.order.push_back(ALTITUDE);

        // Serial << "Altitude:" << endl;
        // Serial << sensorData.altitude << endl;
      }

      // //Battery Garmin edge 530
      if (pData[0] == 0x07) {
        int battery = pData[1];

        // sensorData.battery = battery;
        sensorData.battery.push_back(battery);
        sensorData.order.push_back(BATTERY);
        // Serial << "Battery:" << endl;
        // Serial << sensorData.battery << endl;
      }

      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(pData[i]);
        Serial.print(" ");
      }
      Serial.println();
    }
  }
};

void Notify() {
  // notify changed value
  if (deviceConnected) {                    // Only one client can be connected at a time
    if (millis() - prevMillis >= period) {  // bluetooth stack will go into congestion if too many packets are sent
      prevMillis = millis();
      if (value < 2) {
        Serial.println("Device connected");
      }
      value++;
    }
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected");
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
    value = 0;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("trying to connect");
    oldDeviceConnected = deviceConnected;
  }
}

void BLEConnectionNotify() {
  // Create the BLE Device
  BLEDevice::init("ESP-N");  // It has to be short enough so the uuid is also received (20 bytes total max)
  //Strengthen the BLE power
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); 
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // MyBLECallbacks

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  Serial.println("Characteristic created");

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("Waiting a client connection to notify...");
}