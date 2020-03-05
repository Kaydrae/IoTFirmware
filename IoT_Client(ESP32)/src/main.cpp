/*
 * This is the program for running an IoT light client. It manages the connection to the network and server, as well as interprets light
 * commands. Below is the communication structure:
 *                    Byte 0    Byte 1   Byte 2   Byte 3        Byte 4              Byte 5                Byte 6              Byte 7                Byte 8                  Byte 9
 * Byte Structure: [comm type] [action] [speed] [brightness] [primary color red] [primary color green] [primary color blue] [secondary color red] [secondary color green] [secondary color blue]
 * Where:
 * "action" is a number (between 0 and NUM_ACTIONS-1) that indicates the function to call
 * "speed" is a number (0-255) that indicates movement speed of light effects
 * "brightness" is a number (0-255) indicating brightness of the lights
 * "primary color" is the bytes representing the primary color of the LEDs (split into three bytes: r, g, b)
 * "secondary color" is the bytes representing the secondary color of the LEDs (split into three bytes: r, g, b)
 * 
 */ 

//make sure programmer is using ESP32 or ESP8266
#if !defined(ESP32) && !defined(ESP9266)
#error "ERROR: Not using ESP32 or ESP8266"
#endif

#include "configurationmanager.hpp"
#include "iotclient.hpp"
#include "ota.hpp"
#include "memorymanager.hpp"

#include "FastLED.h"

//ignore GCC warning about const char* to String conversion
#pragma GCC diagnostic ignored "-Wwrite-strings"

#define TYPE "rgb_light"
#define NUM_LEDS 15
#define LED_PIN 4
#define INT_PIN 2

/*
  classes
*/
#define NUM_ACTIONS 4     // Change to be the number of actions you want the device to perform

//iotclient class for server communication
class iot_light : public iotclient {
private:
  //enum for specific communication types
  enum light_cmds{
    LIGHT = B01100100
  };

  //array of actions for the device to perform
  void (iot_light::*actions[NUM_ACTIONS])();
  
  //pointer to array of LEDs
  CRGB* leds;

public:
  //constructor
  iot_light(CRGB* _leds): leds(_leds) {
    /* Set the actions array to correspond to functions to perform actions */
    this->actions[0] = &iot_light::static_single_color;     //static single color function
    this->actions[1] = &iot_light::single_color_pulse;      //single color pulse function
  }

  //override of interpreter function from 
  virtual void interpreter(size_t number) override {
    if (this->bytes[0] == light_cmds::LIGHT && number <= 10) {
      Serial.println("<> Running Device Command <>");

      //call the function corresponding to the action the server has called, pass in speed, brightness, primary color, secondary color
      (this->*(actions[this->bytes[1]]))();
    }
  }

  //function for static solid color
  void static_single_color() {
    //set 32-bit primary color (0x00RRGGBB)
    uint32_t primary_color = (uint32_t)(0x00 << 24 | this->bytes[4] << 16 | this->bytes[5] << 8 | this->bytes[6]);
    
    FastLED.setBrightness(this->bytes[3]);  //set the brightness of the LEDs

    for (size_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = primary_color;
    }
    FastLED.show();
  }

  //function for pulsing at a single color
  void single_color_pulse() {
    return;
  }

};

//ota class for simple OTA
OTA updater;

//memory manager class
memorymanager memory;

//leds
CRGB leds[NUM_LEDS];

//led light client
iot_light light_client(leds);

/* Functions */

// ISR for clearing memory
void clear_mem();

//function for successful network connection
void connection_success();

void test();

//function for failed network connection
void connection_fail();
int x = 8;

//setup
void setup()
{
  Serial.begin(9600);
  Serial.println();

  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  // memory.write();
  memory.generate_uuid();

  memory.read();  // Read network configuration

  // Setup data with the client device
  light_client.setdata(memory.config.HOST, &(memory.config.PORT), memory.config.UUID, TYPE, memory.config.DATA);

  Serial.print("SSID: ");
  Serial.println(memory.config.SSID);
  Serial.print("PASSWD: ");
  Serial.println(memory.config.PASSWD);
  Serial.print("HOST: ");
  Serial.println(memory.config.HOST);
  Serial.print("PORT: ");
  Serial.println(memory.config.PORT);
  Serial.print("UUID: ");
  Serial.println(memory.config.UUID);
  Serial.print("DATA: ");
  Serial.println(memory.config.DATA);

  //connect to wifi
  configmanager::manage_connection(memory, connection_success, connection_fail);

  // settup OTA
  updater.settup(memory.config.UUID);

  // Interrupt for button to clear memory
  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(INT_PIN, clear_mem, FALLING);
}

// Loop
void loop()
{
  //ensure connection to the wifi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("<> WiFi Connection Lost Attempting to Reconnect <>");
    configmanager::manage_connection(memory, connection_success, connection_fail);
  }

  //handle the light client
  light_client.handle();

  //handle OTA
  updater.handle();
}

void connection_success() {
  for (size_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
}

void connection_fail() {
  for (size_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
}

void test() {
  Serial.println(x);
}

// ISR to clear memory
#ifdef ESP8266
void ICACHE_RAM_ATTR clear_mem() {
#elif defined(ESP32)
void IRAM_ATTR clear_mem() {
#endif
  memory.clear();   // Call the memory clear function
#ifdef ESP8266
    ESP.reset();      // Reset the ESP
#elif defined(ESP32)
    ESP.restart();
#else 
    #error "ERROR: Not using ESP32 or ESP8266"
#endif
}