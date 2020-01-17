#include "functions.hpp"
#include "iotclient.hpp"
#include "ota.hpp"

//ignore GCC warning about const char* to String conversion
#pragma GCC diagnostic ignored "-Wwrite-strings"

/*
  device settings
*/
//wifi settings
const char* ssid = "wifi ssid";
const char* pass = "wifi password";

//server settings
const char* host = "server ip";
const int port = 8595; //port

//device info
const char* uuid = "36 Char UUID";
const char* type = "device type ex. rgb_light";
const char* data = "some JSON data ex. { \"name\": \"Test_Light\", \"version\": \"v0.0.1\" }";

/*
  classes
*/
//iotclient class for server communication
iotclient light_client(host, &port, uuid, type, data);

//ota class for simple OTA
OTA updater;

//setup
void setup() {
  Serial.begin(9600);
  Serial.println();

  //connect to wifi
  wifi_connect(ssid, pass);

  //settup OTA
  updater.settup(uuid);
}

void loop() {
  //ensure connection to the wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("<> WiFi Connection Lost Attempting to Reconnect <>");
    wifi_connect(ssid, pass);
  }

  //handle the light client
  light_client.handle();

  //handle OTA
  updater.handle();
}
