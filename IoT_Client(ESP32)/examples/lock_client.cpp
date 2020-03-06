/*
 * This is the program for running an IoT lock client. It runs the locks (setting and resetting them as well as manages conenctions to
 * to the server). Below is the communication structure:
 *                    Byte 0    Byte 1
 * Byte Structure: [comm type] [action] 
 * Where:
 *  comm type = the type of communication being sent (i.e. lock-specific commmunication)
 *  action = the action to perform with the lock (set/reset)
 * 
 */ 

//make sure programmer is using ESP32 or ESP8266
#if !defined(ESP32) && !defined(ESP9266)
#error "ERROR: Not using ESP32 or ESP8266"
#endif

// #include <EEPROM.h> 

// typedef struct {
//   char temp[32] = "aaaaaa";
// } TEST;

// void setup() {
//   Serial.begin(9600);
//   EEPROM.begin(512);

//   TEST data;
//   for (size_t i = 0; i < 5; i++) {
//     data.temp[i] = 'b';
//   }

//   Serial.println("<> Writing to EEPROM <>");
//   EEPROM.put(0, data);
//   EEPROM.commit();
//   Serial.println("<> Reading from EEPROM <>");
//   TEST read_in;
//   Serial.println(read_in.temp);
//   EEPROM.get(0, read_in);
//   Serial.println(read_in.temp);
// }

// void loop() {

// }

#include "configurationmanager.hpp"
#include "iotclient.hpp"
#include "ota.hpp"
#include "memorymanager.hpp"

//ignore GCC warning about const char* to String conversion
#pragma GCC diagnostic ignored "-Wwrite-strings"

#define TYPE "lock"
#define INT_PIN 2

/*
  classes
*/ 
#define NUM_ACTIONS 2     // Change to be the number of actions you want the device to perform
#define LOCK_PIN 14

//iotclient class for server communication
class iot_lock : public iotclient {
private:
  //enum for specific communication types
  enum lock_cmds{
    LOCK = B01100100
  };

  //array of actions for the device to perform
  void (iot_lock::*actions[NUM_ACTIONS])();

public:
  //constructor
  iot_lock() {
    /* Set the actions array to correspond to functions to perform actions */
    this->actions[0] = &iot_lock::reset_lock;   //function to reset the lock
    this->actions[1] = &iot_lock::set_lock;     //function for setting the lock
    pinMode(LOCK_PIN, OUTPUT);
  }

  //override of interpreter function from 
  virtual void interpreter(size_t number) override {
    if (this->bytes[0] == lock_cmds::LOCK && number <= 2) {
      Serial.println("<> Running Device Command <>");

      //call the function corresponding to the action the server has called
      (this->*(actions[this->bytes[1]]))();
    }
  }

  //function to close the lock
  void set_lock() {
    digitalWrite(LOCK_PIN, LOW);
  }

  //function to open lock
  void reset_lock() {
    digitalWrite(LOCK_PIN, HIGH);
  }

};


//ota class for simple OTA
OTA updater;

//memory manager class
memorymanager memory;

//led light client
iot_lock lock_client;

/* Functions */

// ISR for clearing memory
void clear_mem();

//function for successful network connection
void connection_success();

//function for failed network connection
void connection_fail();

char* default_check = "default";

//setup
void setup()
{
  Serial.begin(9600);   //begin serial monitor
  memory.begin();       //begin memory manager

  Serial.println();

  //check if data is stored in memory
  if (memory.isDataStored()) {
    Serial.println("<> Reading Configuration from EEPROM <>");
    memory.read();  //if it is, read in the memory configuration
  }
  else {
    Serial.println("<> Writing Configuration to EEPROM <>");
    //otherwise...
    memory.generate_uuid();   //generate a new UUID
    memory.write();           //write the configuration to memory
#ifdef ESP8266
    ESP.reset()               //restart the ESP
#elif defined(ESP32)
    ESP.restart();            //restart the ESP
#endif
  }

  // Setup data with the client device
  lock_client.setdata(memory.config.HOST, &(memory.config.PORT), memory.config.UUID, TYPE, memory.config.DATA);

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
  lock_client.handle();

  //handle OTA
  updater.handle();
}

void connection_success() {
  Serial.println("Connection successful!");
}

void connection_fail() {
  Serial.println("Connection failed!");
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