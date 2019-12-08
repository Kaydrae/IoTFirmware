#include <ESP8266WiFi.h>
#include "config.h"
#include "ota.h"

#ifndef CONSTS
//device information
#define DEVICE_ROOM "robotics_lab"    //the id of room the device is located in
#define DEVICE_NAME "Template Device" //the name of the device
#define DEVICE_TYPE "template"        //the type of the device
#define DEVICE_VERSION "1.0.0"        //version of the device
#define DEVICE_SPESIFIC "template_data|Hello World"  //device spesific data: converted to json

/* Device Spesific Data Formatting Example:
 *  Example data: "lights|20;default_color|FF00FF"
 *  would be converted to the following JSON data
 *  JSON = {
 *    lights: "20",
 *    default_color: "FF00FF"
 *  }
*/

//dilimier used to seperate information (depends on server, dont change unless needed)
#define DILIM ":"
#endif

//variable instances of the constants so they have the correct type
String device_room = DEVICE_ROOM;
String device_name = DEVICE_NAME;
String device_type = DEVICE_TYPE;
String device_version = DEVICE_VERSION;
String device_spesific = DEVICE_SPESIFIC;
String dilim = DILIM;

//device config
Configuration conf(true);

//OTA handler
OTA ota;

/*------------------------
  Function Definitions
------------------------*/

//takes a command from the server and responds accordingly
void process_command(String command, WiFiClient & client);

/*----------------------------------------
  Arduino Settup and Loop Deffinitions
----------------------------------------*/

void setup()
{
  //open a serial port
  Serial.begin(115200); delay(1000);

  //start the device boot
  Serial.println("Device Boot Initialized...");

  //begin connection to WiFi
  Serial.println("Starting WiFi connection attempt...");
  WiFi.begin(conf.ssid, conf.password);
  Serial.println("Waiting for connection to be established...");

  // waiting for the esp to connect to wifi loop
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...");
    delay(100);
  }

  //start the OTA service
  ota.settup(device_name.c_str());

  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Boot finished. Starting device loop...\n\n\n");
}

//the main loop will try to keep this device connected to the main server, if ever the client gets disconnected the
void loop()
{
    //client connection object
    WiFiClient client;

    //try to connect the client to the server
    if (!client.connect(conf.host, conf.port)) {
        Serial.println("Connection to server failed! Trying again in 10 seconds to restablish a connection...");
        return;
    }

    Serial.println("Connected to server successful!");
    Serial.println("Waiting for server commands...");

    //loop here while the client is connected
    while (client.connected()) {
      //check for a OTA update
      ArduinoOTA.handle();

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

    //fallback to main loop if the client loses connection to the server
    Serial.println("Connection to the server has been lost. Trying again in 10 seconds to restablish connection...");
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
}
