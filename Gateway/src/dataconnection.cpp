#include "dataconnection.h"
#include "header.h" 

//Hotspot 
// const char* ssid = "JarneHotspot";
// const char* password = "PasswordJarne";

//WiFi
const char* ssid = "ASK4 WiFi";
const char* password = "";

void setupwifi() {
  WiFi.mode(WIFI_STA); //init wifi mode
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());
}