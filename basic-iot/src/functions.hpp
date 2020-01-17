#ifndef FUNCTIONS
#define FUNCTIONS

//libraries
#include <Arduino.h>
#include <WiFi.h>

//function for ensuring wifi connection
void wifi_connect(const char* ssid, const char* pass) {
  Serial.println("<> Attempting WiFi Connection <>");

  //connect to wifi
  WiFi.begin(ssid, pass);

  //loop until wifi is connected
  while (WiFi.status() != WL_CONNECTED) {
    //if the connection fails try and connect again
    if (WiFi.status() == WL_CONNECT_FAILED) {
      //retry wifi connection
      WiFi.begin(ssid, pass);
    }
  }

  Serial.println("<> Connected to WiFi <>");
}

#endif
