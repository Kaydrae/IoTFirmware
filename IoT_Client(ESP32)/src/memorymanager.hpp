#ifndef MEM_MANAGE
#define MEM_MANAGE

#include <EEPROM.h>
#include <ESPRandom.h>

/* Defines */
#define EEPROM_MEM_SIZE 4096        // Size of EEPROM memory
#define DEFAULT_DATA    "default"

#define MAX_SSID_LEN 32             // Size of SSID phrase
#define MAX_HOST_LEN 64             // Size of server host phrase
#define MAX_UUID_LEN 36             // Size of UUID phrase
#define MAX_JSON_LEN 256            // Size of JSON data
#define MAX_PASSWD_LEN 32           // Size of password phrase

/* Type definitions */

// Struct type for storing network configuration
typedef struct
{   
    char valid_write = 'w';                                             // Character to check if data has been written
    char SSID[MAX_SSID_LEN] = "RobotHouse";                             // SSID of WiFi
    char PASSWD[MAX_PASSWD_LEN] = "R0botHous3Rule$";                    // Password of WiFi
    char HOST[MAX_HOST_LEN] = "10.1.1.182";                             // Hostname of server
    int PORT = 8595;                                                    // Port for server
    char UUID[MAX_UUID_LEN] = "default";                                // UUID of device
    char TEMP[32] = "";
    char DATA[MAX_JSON_LEN] = "{\"name\": \"Test Light\", \"room\": \"engineering_lab\"}";  // JSON data for device
} WIFI_CONFIG;

/*
    Class for managing memory
*/
class memorymanager
{
public:
    WIFI_CONFIG config;                     // Struct implementation for storing configuration data

    // Constructor
    memorymanager() {}

    // Function to begin the memoy manager
    void begin() 
    {
        EEPROM.begin(EEPROM_MEM_SIZE);      // Begin the EEPROM memory address
    }

    // Function to check if data is properly stored in memory
    bool isDataStored()
    {
        return (EEPROM.read(0) == 'w');
    }

    // Function to write to memory
    void write()
    {
        EEPROM.put(0, this->config);        // Write the current network configuration to memory
        EEPROM.commit();                    // Return if the data was committed to memory
    }

    void generate_uuid() {
        uint8_t uuid_arr[16];
        ESPRandom::uuid(uuid_arr);
        String uuid_str = ESPRandom::uuidToString(uuid_arr);
        for (size_t i = 0; i < MAX_UUID_LEN; i++) {
            this->config.UUID[i] = uuid_str[i];
        }
    }

    // Function to read from memory
    void read()
    {
        EEPROM.get(0, this->config);        // Get network configuration from EEPROM
    }

    // Function to clear memory
    void clear()
    {
        Serial.println("<> Clearing memory... <>");
        
        // Clear memory
        for (size_t i = 0; i < EEPROM_MEM_SIZE; i++) {
            EEPROM.write(i, 255);
        }

        EEPROM.commit();                    // Commit it to memory

        Serial.println("<> Done. <>");
    }

private:
};

#endif // MEM_MANNAGE