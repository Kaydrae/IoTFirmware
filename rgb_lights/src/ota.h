#include <ArduinoOTA.h>

//simple ota settup
class OTA
{
public:
  //constructor
  OTA() {

  }

  //call to settup OTA
  void settup(const char * device_name) {
    /*
     * Start of ArduinoOTA settup
    */
    //set the name of this device
    ArduinoOTA.setHostname(device_name);
    
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
  }

  //calls the OTA handle 
  void handle() {
    ArduinoOTA.handle();
  }
};
