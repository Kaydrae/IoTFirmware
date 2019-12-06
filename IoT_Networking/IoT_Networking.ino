#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "FastLED.h"

#ifndef CONSTS
//device information
#define DEVICE_ROOM "1"
#define DEVICE_NAME "Blue Light Box"
#define DEVICE_TYPE "light"
#define DEVICE_VERSION "1.0.0"
#define DEVICE_SPESIFIC "leds|15;" 

//dilimier used to seperate information
#define DILIM ":"

//server information
#define SERVER_IP "192.168.0.119"
#define SERVER_PORT 8000

//network information
#define STASSID "Username"
#define STAPSK  "Password"

//fastled information
#define NUM_LEDS 15
#define TWO_HUNDRED_PI 628
#endif

#define DATA_PIN D4

//variable instances of the constants
String device_room = DEVICE_ROOM;
String device_name = DEVICE_NAME;
String device_type = DEVICE_TYPE;
String device_version = DEVICE_VERSION;
String device_spesific = DEVICE_SPESIFIC;

String dilim = DILIM;

const char* ssid     = STASSID;
const char* password = STAPSK;
 
const char * host = SERVER_IP;
const uint16_t port = SERVER_PORT;

//array for containing led information
CRGB leds[NUM_LEDS];
int waiting_led = 0;

//change the color of all the lights
void set_all(int color) {
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = color;
  }
  FastLED.show();
}

//change the color of a single light
void set_single(int light_index, int color) {
  //block attempts to access invalid light indexes
  if (light_index > NUM_LEDS - 1) {
    return;
  }

  leds[light_index] = color;
  FastLED.show();
}
 
void setup()
{
  //open a serial port
  Serial.begin(115200);
  delay(1000);

  //start the device boot
  Serial.println("Device Boot Initialized...");
  
  //add leds to the led array
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  Serial.println("Starting WiFi connection attempt...");
  WiFi.begin(ssid, password);
  
  Serial.println("Waiting for connection to be established...");

  // waiting for the esp to connect to wifi loop
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...");

    leds[waiting_led] = CHSV(160, 255, 168);
    FastLED.show();
    delay(250);

    //increment the waiting LED,reset the counter if it hits the max # of LEDS
    waiting_led++;
    
    leds[waiting_led - 1] = CHSV(0, 0, 0);
    
    if(waiting_led > NUM_LEDS-1) {
      waiting_led = 0;
    }
  }

  //change lights to green
  set_all(65280);
  
  FastLED.show();
 
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  /*
   * Start of ArduinoOTA settup
  */
  //set the name of this device
  ArduinoOTA.setHostname(device_name.c_str());

  //update start hook for ArduinoOTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("[ArduinoOTA] Starting update " + type);
  });

  //update end hook for ArduinoOTA
  ArduinoOTA.onEnd([]() {
    Serial.println("[ArduinoOTA] Update finished");
  });

  //on progress of update hook for ArduinoOTA
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[ArduinoOTA] Update Progress: %u%%\r", (progress / (total / 100)));
  });

  //on error hook for ArduinoOTA
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ArduinoOTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("[ArduinoOTA] Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("[ArduinoOTA] Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("[ArduinoOTA] Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("[ArduinoOTA] Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("[ArduinoOTA] End Failed");
    }
  });

  //start the over the air service
  ArduinoOTA.begin();
  /*
   * End of ArduinoOTA settup
  */
  
  Serial.println("Boot finished. Starting device loop...\n\n\n");
}

//takes a command from the server and responds accordingly
void processCommand(String command, WiFiClient & client) {
  Serial.println("Processing Command: '" + command + "'");

  int a_index = 0, s_index = 0;
  String cmd_split[10], temp;  

  Serial.println("Splitting string...");
  
  //split the string into an array of strings
  while(true) {
    if (command.charAt(s_index) == ' ') {
      cmd_split[a_index] = temp;
      Serial.println(temp);
      temp = "";
      a_index++;
    }
    else {
      temp += command.charAt(s_index);
    }
    
    //incrememnt the string index
    s_index++;

    //for the last token/if there is no place to cut the string at
    if(command.length() < s_index) {
      cmd_split[a_index] = temp;
      Serial.println(temp);
      break;
    }
  }

  Serial.println("Done splitting string...");
  
  if(cmd_split[0] == "getinfo") {
    Serial.println("Sending device info to server...");

    //send the device info to the server (seperated by special dilimiter "#") IMPORTANT: order must be | device_room -> device name -> device type -> device version -> device_spesific
    client.print(device_room + dilim + device_name + dilim + device_type + dilim + device_version + dilim + device_spesific);
  }

  if(cmd_split[0] == "heart") {
    Serial.println("Responding to heartbeat command...");

    client.print("beat");
  }

  if(cmd_split[0] == "setcolor") {
    if(cmd_split[1] == "all") {
      Serial.println("Changing the color of the lights to the following: " + cmd_split[2]);

      set_all(strtol(cmd_split[2].c_str(), NULL, 16));
    } else if(cmd_split[1] == "single") {
      Serial.println("Changing the color of light #" + cmd_split[2] + " to " + cmd_split[3]);

      set_single(strtol(cmd_split[2].c_str(), NULL, 10) - 1, strtol(cmd_split[3].c_str(), NULL, 16));
    }
  }

  if(cmd_split[0] == "end") {
    client.stop();
  }
}

//the main loop will try to keep this device connected to the main server, if ever the client gets disconnected the 
void loop()
{
    WiFiClient client;

    //try to connect the client to the server
    if (!client.connect(host, port)) {
        Serial.println("Connection to server failed! Trying again in 10 seconds to restablish a connection...");
 
        // error lights + delay
        for(int i = 0; i < 10; i++) {
          delay(500);
          set_all(0);
          delay(500);
          set_all(16711680);
        }
        return;
    }
 
    Serial.println("Connected to server successful!");

    set_all(65535);

    Serial.println("Waiting for server commands...");

    //loop here while the client is connected
    while (client.connected()) {
      ArduinoOTA.handle();
      //read the cleint buffer until there is a newline
      String line = client.readStringUntil('\n');

      //only execute the 'command' if it is not an empty line
      if(line != "") {
        Serial.println("Server Command Recieved: '" + line + "'");
  
        //process the command
        processCommand(line, client);
      }
      delay(100);
    }

    Serial.println("Connection to the server has been lost. Trying again in 10 seconds to restablish connection...");

    // error lights
    for(int i = 0; i < 10; i++) {
      delay(500);
      set_all(0);
      delay(500);
      set_all(16711680);
    }
}
