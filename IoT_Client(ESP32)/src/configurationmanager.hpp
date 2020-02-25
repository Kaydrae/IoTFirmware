#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER

#include <Arduino.h>
#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
    #include "html.h"
#elif defined(ESP32)
    #include <WiFi.h>
    #include <BLEDevice.h>
    #include <BLEUtils.h>
    #include <BLEServer.h>
#else
    #error "ERROR: Not using the ESP32 or ESP8266"
#endif

#include "memorymanager.hpp"


namespace configmanager {
    namespace {
#ifdef ESP8266
        const char* AP_SSID = "IOT_DEVICE";
        const char* AP_PASS = "iotdevice";

        IPAddress ap_local(192,168,1,1);
        IPAddress ap_gateway(192,168,1,254);
        IPAddress ap_subnet(255,255,255,0);

        ESP8266WebServer server;
#elif defined(ESP32)
        #define BLE_NAME "IOT_DEVICE_"
        #define BLE_SERVICE_ID  "4be6bc6d-315a-473b-a73a-c0b5c694beed"
        #define BLE_CHAR_ID     "34bd2e30-84f9-4fd9-82b7-e2b3dccf5fa3"

        BLEServer* pServer;
        BLEService* pService;
        BLECharacteristic* pCharacteristic;
#endif
    }

    void manage_connection(memorymanager& memory, void (*success_func)(void), void (*fail_func)(void));
    bool wifi_connect(const char *ssid, const char *pass);
    void setup_server();
    void run_server(memorymanager& memory);
#ifdef ESP8266
    void handleConnect();
    void handleNotFound();
#elif defined(ESP32)
    
#endif
}

//function to manage wifi connection
void configmanager::manage_connection(memorymanager& memory, void (*success_func)(void), void (*fail_func)(void)) {
    if (!wifi_connect(memory.config.SSID, memory.config.PASSWD)) {
        fail_func();
        setup_server();
        run_server(memory);
    }
    success_func();
}

//function to check wifi conenction
bool configmanager::wifi_connect(const char *ssid, const char *pass) {
    Serial.println("<> Attempting WiFi Connection <>");
    WiFi.mode(WIFI_STA);    // Set the ESP mode to station

    //connect to wifi
    WiFi.begin(ssid, pass);

    int reconnects = 0;

    // Wait to see if connection fails or connects
    while (WiFi.status() != WL_CONNECTED && reconnects < 4) {
        if (WiFi.waitForConnectResult() != WL_CONNECTED) {
            reconnects++;
        }
    }
    if (reconnects >= 10)
        return false;   // return false if fail
    Serial.println("<> Connected to WiFi <>");
    return true;        // return true if succeed
}

// function to setup the server
void configmanager::setup_server()
{
    Serial.println();
#ifdef ESP8266
    Serial.println("<> Setting up AP Mode <>");

    // disconnect wifi and setup ap mode
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);

    WiFi.softAPConfig(ap_local, ap_gateway, ap_subnet);
    WiFi.softAP(AP_SSID, AP_PASS);

    server.on("/", handleConnect);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.print("SSID: "); Serial.println(AP_SSID);
    Serial.print("PASSWD: "); Serial.println(AP_PASS);
    Serial.print("SERVER: "); Serial.println(ap_local);
#else
    Serial.println("<> Starting BLE Server <>");
    char chip_name[64];
    uint64_t chip_id = ESP.getEfuseMac();
    itoa(chip_id, chip_name, 16);
    char ble_full_name[11+64];
    sprintf(ble_full_name, "%s%s", BLE_NAME, chip_name);

    BLEDevice::init(ble_full_name);
    
    pServer = BLEDevice::createServer();
    pService = pServer->createService(BLE_SERVICE_ID);
    pCharacteristic = pService->createCharacteristic(
        BLE_CHAR_ID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic->setValue("Hello, there!");
    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_ID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("<> BLE Server Started <>");
    Serial.print("BLE Name: "); Serial.println(ble_full_name); 
#endif


}

 // function to run the server
void configmanager::run_server(memorymanager& memory) {
#ifdef ESP8266
    while (!(server.hasArg("ssid") && server.hasArg("passwd") && server.hasArg("host") && server.hasArg("port") && server.hasArg("uuid")
    && server.hasArg("data"))) {
        server.handleClient();
    }

    server.arg("ssid").toCharArray(memory.config.SSID, server.arg("ssid").length()+1);
    server.arg("passwd").toCharArray(memory.config.PASSWD, server.arg("passwd").length()+1);

    Serial.println(memory.config.SSID);

    memory.write();

    delay(1000);

    ESP.reset();
#elif defined(ESP32)
    while (true);

    ESP.restart();
#else
    #error "ERROR: Not using ESP32 or ESP8266."
#endif
}

#ifdef ESP8266
 //function to handle connection
void configmanager::handleConnect() {
    if (server.hasArg("ssid") && server.hasArg("passwd") && server.hasArg("host") && server.hasArg("port") && server.hasArg("uuid")
    && server.hasArg("data")) {
        server.send(200, "text/html", "<h1>Resetting...</h1>");
    }
    else
        server.send(200, "text/html", html_data);
}

//function to handle 404 errors
void configmanager::handleNotFound() {
    server.send(404, "text/plain", "<h1>404: File Not Found</h1><p>Please try a different file</p>");
}
#endif

#endif // CONFIG_MANAGER