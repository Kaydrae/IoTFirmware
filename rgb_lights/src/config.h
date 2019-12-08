#include <EEPROM.h>

#ifndef CONFIG_DEFAULTS
//server information
#define SERVER_HOSTNAME "server.ip"
#define SERVER_PORT 0

//network information
#define STASSID "ssid"
#define STAPSK  "password"

//led information
#define NUM_LEDS 15
#define TWO_HUNDRED_PI 628

//size of esp8226 EEPROM
#define EEPROM_SIZE 4096

//the maximum size of strings that can be stored in EEPROM
#define MAX_STRING_LENGTH 32
#endif

//structure used to store and retrieve device config from flash
typedef struct config_data {
  //keeps track of declared costants so if a change is detected it can overwrite the old data with new defaults
  int max_string_length = MAX_STRING_LENGTH;
  int leds = NUM_LEDS;

  //data to be stored
  char ssid[MAX_STRING_LENGTH];      
  char password[MAX_STRING_LENGTH];
  char host[MAX_STRING_LENGTH];
  uint16_t port;
  int saved_led_colors[NUM_LEDS];
};

//Configuration class deffinition
class Configuration
{
public:
  //config values initialized to defaults
  //wifi information
  char* ssid = STASSID;      
  char* password = STAPSK;

  //server information
  char* host = SERVER_HOSTNAME;
  uint16_t port = SERVER_PORT;

  //led data
  int led_colors[NUM_LEDS];

  //enable/disable serial debug
  bool debug;
  
  //constructor
  Configuration(bool serial_debug = false) {
    //set values
    this->debug = serial_debug;
    
    //init the EEPROM
    EEPROM.begin(EEPROM_SIZE);
  }

  //save current config into non-volitile memory
  void save() {
    //create struct that stores the config data
    config_data data;
    
    //load the current config into a struct that will be written to flash
    put_value(this->ssid, data.ssid);
    put_value(this->password, data.password);
    put_value(this->host, data.host);
    data.port = this->port;
    memcpy(data.saved_led_colors, this->led_colors, sizeof(this->led_colors));

    //serial debug
    if (this->debug) {
      Serial.println(">-----");
      Serial.println("<> Saving the Following Device Config to Flash <>");
      Serial.print("\tSSID: "); Serial.println(data.ssid);
      Serial.print("\tPASSWORD: "); Serial.println(data.password);
      Serial.print("\tHOST: "); Serial.println(data.host);
      Serial.print("\tPORT: "); Serial.println(data.port);
      Serial.print("\tLED COLORS: [ "); for(int i = 0; i < 15; i++) {Serial.print(String(data.saved_led_colors[i]) + ", ");} Serial.println("end ]");
      Serial.println(">-----");
    }
    
    //save the data in the EEPROM at address 0
    EEPROM.put(0, data);

    //commit the EEPROM to flash (ESP8266)
    EEPROM.commit();
  }

  //loads the saved config from flash
  void load() {
    //struct that will read values from memory
    config_data data;

    //read settings from eeprom
    EEPROM.get(0, data);

    //if the max string length doesn't match then overwrite the old save with
    if (data.max_string_length != MAX_STRING_LENGTH) {
      if (this->debug) {
        Serial.println("Config Warning: A change in max length of saved strings has been detected. Overwriting previous .");
      }

      //default the LED color to white
      for (int i = 0; i < NUM_LEDS; i++) {
        this->led_colors[i] = 16777215;
      }
      
      //overwrite invalid save using the default config
      this->save();
      return;
    }

    //set the config values from flash into memory
    get_value(this->ssid, data.ssid);
    get_value(this->password, data.password);
    get_value(this->host, data.host);
    this->port = data.port;

    //check if the number of LEDS of the device has changed, if so reset the LED settings to default save the new config
    if (data.leds != NUM_LEDS) {
      if (this->debug) {
        Serial.println("Config Warning: A change in the number of device LEDS has been detected. Wiping previous saved LED configuration.");
      }

      //save the new default led config
      this->save();
    } else {
      //load the LED colors from from flash into memory
      memcpy(this->led_colors, data.saved_led_colors, sizeof(data.saved_led_colors));
    }
    
    //serial debug
    if (this->debug) {
      Serial.println(">-----");
      Serial.println("<> Loaded the Following Device Config from Flash <>");
      Serial.print("\tSSID: "); Serial.println(this->ssid);
      Serial.print("\tPASSWORD: "); Serial.println(this->password);
      Serial.print("\tHOST: "); Serial.println(this->host);
      Serial.print("\tPORT: "); Serial.println(this->port);
      Serial.print("\tLED COLORS: [ "); for(int i = 0; i < 15; i++) {Serial.print(String(this->led_colors[i]) + ", ");} Serial.println("end ]");
      Serial.println(">-----");
    }
    
    return;
  }

  //move the values from one array to a static array (with x as the size of the static array)
  void put_value(char * str, char static_str[]) {
    int i = 0;
    int x = MAX_STRING_LENGTH;
    
    while(x != 0) {
      static_str[i] = str[i];
      
      //return the sized array once the end of the value is reached
      if (str[i] == '\0') {
        
        //fill the rest of the str array with null chars
        for (int j = 0; j < x; j++) {
          static_str[i] = '\0';
          i++;
        }

        return;
      }
      
      i++;
      x--;
    }
  }

  //move the values from the static array to the dynamic one in memory
  char get_value(char * value, char source[]) {
    value = (char*)malloc(sizeof(source));
    memcpy(value, source, sizeof(source));
  }
};
