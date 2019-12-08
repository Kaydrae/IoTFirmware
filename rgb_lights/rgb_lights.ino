#include <ESP8266WiFi.h>
#include "FastLED.h"
#include "config.h"
#include "ota.h"

#ifndef DEFAULTS
//default device information
#define DEVICE_ROOM "1"
#define DEVICE_NAME "Light Box #1"
#define DEVICE_TYPE "rgb_light"
#define DEVICE_VERSION "1.0.0"
#define DEVICE_SPESIFIC "leds|15"

//dilimier used to seperate information
#define DILIM ":"
#endif

//pin for FastLED
#define DATA_PIN D4

//variable instances of the constants
String device_room = DEVICE_ROOM;
String device_name = DEVICE_NAME;
String device_type = DEVICE_TYPE;
String device_version = DEVICE_VERSION;
String device_spesific = DEVICE_SPESIFIC;

String dilim = DILIM;

//types of indicator light outputs
enum indicator {
  wifi_connected,
  server_disconnected,
  off,
  revert
};

//device config
Configuration conf(true);

//OTA handler
OTA ota;

//array for containing led information
CRGB leds[NUM_LEDS];

/*------------------------
  Function Definitions
------------------------*/

//change the color of all the lights
void set_all(int color);

//change the color of a single light
void set_single(int light_index, int color);

//set the colors of the all leds based on the backup in memory
void load_all();

//set the color of a single led based on the backup in memory
void load_single(int index);

//takes a command from the server and responds accordingly
void process_command(String command, WiFiClient & client);

//changes the color of the indicator light based on the status of the program
void set_indicator(indicator choice);

/*----------------------------------------
  Arduino Settup and Loop Deffinitions
----------------------------------------*/

void setup()
{
  //open a serial port and wait a second
  Serial.begin(115200); delay(1000);

  //set the config to default manually by uncommenting this line and then removing it later
  //conf.save();

  //start the device boot
  Serial.println("Device Boot Initialized...");

  //load the device config from flash
  conf.load();

  //add leds to the led array
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  //connect to wifi
  Serial.println("Starting WiFi connection attempt...");
  WiFi.begin(conf.ssid, conf.password);

  Serial.println("Waiting for connection to be established...");

  //current led which is blue
  int waiting_led = 0;

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

  //display the light config
  load_all();

  //start the OTA service
  ota.settup(device_name.c_str());

  //set indicator light for wifi connection
  set_indicator(indicator::wifi_connected);

  Serial.print("WiFi connected with IP: "); Serial.println(WiFi.localIP());
  Serial.println("Boot finished. Starting device loop...\n\n\n");
}

//the main loop will try to keep this device connected to the main server, if ever the client gets disconnected the
void loop()
{
  //client object
  WiFiClient client;

  //try to connect the client to the server
  if (!client.connect(conf.host, conf.port)) {
      Serial.println("Connection to server failed! Trying again in 10 seconds to restablish a connection...");

      // error lights + reconnect delay
      for(int i = 0; i < 10; i++) {
        set_indicator(indicator::server_disconnected);
        delay(500);
        set_indicator(indicator::off);
        delay(500);
      }

      //go back to the start of the main loop
      return;
  }

  Serial.println("Connected to server successful!");

  //revert the indicator LED
  set_indicator(indicator::revert);

  Serial.println("Waiting for server commands...");

  //loop here while the client is connected
  while (client.connected()) {
    //check for a OTA update
    ota.handle();

    //read the cleint buffer until there is a newline
    String line = client.readStringUntil('\n');

    //only execute the 'command' if it is not an empty line
    if(line != "") {
      Serial.println("Server Command Recieved: '" + line + "'");

      //process the command
      process_command(line, client);
    }
    delay(100);
  }

  Serial.println("Connection to the server has been lost. Trying again in 10 seconds to restablish connection...");

  //indicator light error loop
  for(int i = 0; i < 10; i++) {
    set_indicator(indicator::server_disconnected);
    delay(500);
    set_indicator(indicator::off);
    delay(500);
  }
}

/* ------------------------
  Function Declarations
-------------------------*/

//takes a command from the server and responds accordingly
void process_command(String command, WiFiClient & client) {
  Serial.println("Processing Command: '" + command + "'");

  int a_index = 0, s_index = 0;
  String cmd_split[10], temp;

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
      break;
    }
  }

  if(cmd_split[0] == "getinfo") {
    Serial.println("Sending device info to server...");

    //send the device info to the server (seperated by special dilimiter "#") IMPORTANT: order must be | device_room -> device name -> device type -> device version -> device_spesific
    client.print(device_room + dilim + device_name + dilim + device_type + dilim + device_version + dilim + device_spesific);
  }

  if(cmd_split[0] == "heart") {
    Serial.println("Responding to heartbeat command...");

    client.print("beat");
  }

  //save the current config of the device to flash
  if(cmd_split[0] == "save") {
    Serial.println("Saving current device config...");
    conf.save();
  }

  //end the client's connection with the server
  if(cmd_split[0] == "end") {
    Serial.println("Ending connection with the server...");
    client.stop();
  }

  //set the color of the leds
  if(cmd_split[0] == "setcolor") {
    if(cmd_split[1] == "all") {
      Serial.println("Changing the color of the lights to the following: " + cmd_split[2]);

      set_all(strtol(cmd_split[2].c_str(), NULL, 16));
    } else if(cmd_split[1] == "single") {
      Serial.println("Changing the color of light #" + cmd_split[2] + " to " + cmd_split[3]);

      set_single(strtol(cmd_split[2].c_str(), NULL, 10) - 1, strtol(cmd_split[3].c_str(), NULL, 16));
    }
  }
}

//change the color of all the lights
void set_all(int color) {
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = color;
    conf.led_colors[i] = color;
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
  conf.led_colors[light_index] = color;
  FastLED.show();
}

//set the colors of the all leds based on the backup in memory
void load_all() {
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = conf.led_colors[i];
  }
  FastLED.show();
}

//set the color of a single led based on the backup in memory
void load_single(int index) {
  //block attempts to access invalid light indexes
  if (index > NUM_LEDS - 1) {
    return;
  }

  leds[index] = conf.led_colors[index];
  FastLED.show();
}

//sets the indicator LED to a given color
void set_indicator(indicator choice) {
  if (choice == indicator::wifi_connected) {
    leds[0] = 65280;
  }
  else if (choice == indicator::server_disconnected) {
    leds[0] = 16711680;
  }
  else if (choice == indicator::off) {
    leds[0] = 0;
  }
  else if (choice == indicator::revert) {
    leds[0] = conf.led_colors[0];
  }

  FastLED.show();
}
