\#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "FastLED.h"

#ifndef STASSID
#define STASSID "Username"
#define STAPSK  "Password"
#endif

#define VERSION "1.0.1"
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

#define NUM_LEDS 22

#define DATA_PIN D4 //GPIO 04
//#define CLOCK_PIN 13
#define TWO_HUNDRED_PI 628
 
CRGB leds[NUM_LEDS];
int waitingLED = 0; 
void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      leds[waitingLED] = CHSV(160, 255, 128);
      FastLED.show();
      delay(500);
      Serial.print(".");
      waitingLED++;
  }
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

    if(WL_CONNECTED){
    
    for(int i = 0; i < NUM_LEDS; i++){
    leds[i].g = 255; 
    }
    FastLED.show();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("LightBox_1");

  // No authentication by default
   ArduinoOTA.setPassword("LightBox");

  // Password can be set with it's md5 value as well
   //MD5(LightBox) = 21232f297a57a5a743894a0e4a801fc3
   //ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


   server.on("/", handleRoot);

  server.on("/Test", [](){
    
    Serial.println("Test Connection Sent");
    server.send(200, "text/plain", "Test Message");
  });

  
  server.on("/Light", HTTP_GET, [](){
     Serial.println("Color Sent");
  if(server.hasArg("color")){
    int color = server.arg("color").toInt();
    Serial.println(color);
    changeColors(color);
  }else{
    Serial.println("No Color");
  }
    
    server.send(200, "text/plain", "Color Changed");
  });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}

void handleRoot() {
  //digitalWrite(led, 1);
  //server.send(200, "text/plain", "hello from esp8266!");
  //digitalWrite(led, 0);

  String cmd;     
      cmd += "<!DOCTYPE HTML>\r\n";
      cmd += "<html>\r\n";
      cmd += "<header><title>ESP8266 Webserver</title><h1>\"ESP8266 Web Server Control\"</h1></header>";
      cmd += "<head>";
      cmd += "<meta http-equiv='refresh' content='5'/>";
      cmd += "</head>";
      cmd += "<p>Hello World</p>";
      /*if(device1){
        cmd +=("<br/>Device1  : ON");
      }
      else{
        cmd +=("<br/>Device1  : OFF");
      }
      
      if(device2){
        cmd +=("<br/>Device2  : ON");
      }
      else{
        cmd +=("<br/>Device2  : OFF");
      }
           
       if(device3){
        cmd +=("<br/>Device3  : ON");
      }
      else{
        cmd +=("<br/>Device3  : OFF");
      }
      
      if(device4){
        cmd +=("<br/>Device4  : ON");
      }
      else{
        cmd +=("<br/>Device4  : OFF");
      }*/
           
      cmd += "<html>\r\n";
      server.send(200, "text/html", cmd);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  
}

void changeColors(int color){
  for(int i = 0; i < NUM_LEDS; i++){
    //Serial.println(color);
    leds[i] = color;
    //FastLED.show();
  }
  FastLED.show();
}
