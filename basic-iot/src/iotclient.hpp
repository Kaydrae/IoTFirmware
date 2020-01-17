#ifndef IOT_CLIENT
#define IOT_CLIENT

//libraries
#include <WiFiClient.h>

//commands from the server
enum commands {
  //reserved byte deffinitions
  getinfo = B00000001,
  heartbeat = B00000010,
  end_connection = B00000011,
  end_message = B11111111,

  //device spesific byte deffinitions
};

//iot client class
class iotclient {
private:
  //iot_client's client connection
  WiFiClient client;

  //host info
  const char* host;

  //port info
  const int* port;

  //ready bytes array for the client
  uint8_t bytes[1024];

  //uuid of the iotclient
  String uuid;

  //type of device the iotclient is
  String type;

  //json string data of the  device
  String data;

  //dilimiter
  String dilim = "##";

public:
  //constructor
  iotclient(const char* host, const int* port, const char* uuid, const char* type, const char* data) {
    //server information
    this->host = host;
    this->port = port;

    //client information
    this->uuid = uuid;
    this->type = type;
    this->data = data;
  }

  //connect the client to the server
  void connect() {
    Serial.println("<> Attempting to Connect to Server <>");

    //keep trying to connect until it is sucessful
    while(!client.connect(this->host, *this->port)) {
      Serial.print("Connecting...");
      delay(1000);
    }

    Serial.println("<> Connected to Server <>");
  }

  //main client loop, must be called for the client to work
  void handle() {
    //if the client is not already connected then connect them
    if (!this->client.connected()) {
      this->connect();
    } else {
      //once the client is avaliable start processing socket data
      if (this->client.available()) {
        //read the buffer into the byte array
        size_t total = client.readBytesUntil(commands::end_message, this->bytes, 1024);

        //if a message was recived then interpret the message
        if (total > 0) {
          Serial.println("<> Interpreting " + String(total) + " Bytes <>");
          this->interpret(total);
        }
      }
    }

    delay(10);
  }

  //interprets the message sent from the server
  void interpret(size_t number) {
    Serial.println("<> Interpreting the following array of bytes <>");

    for (int i = 0; i < number; i++) {
      Serial.println(this->bytes[i]);
    }

    Serial.println("<> END <>");

    int i = 0;

    //if the client has recived the getinfo command
    if (this->bytes[i] == commands::getinfo) {
      Serial.println("<> Sending Device Data to Server: ");
      Serial.println("UUID:  "+ this->uuid);
      Serial.println("type: " + this->type);
      Serial.println("data: " + this->data);

      //send the device info to the server
      this->client.print(this->uuid + this->dilim + this->type + this->dilim + this->data);
    }

    //if the client has recived the getinfo command
    else if (this->bytes[i] == commands::heartbeat) {
      Serial.print("<> Responding to Heartbeat Request <>");

      //respond so the server keeps the connection alive
      this->client.print("beat");
    }

    //if the client has recived the end_connection command
    else if (this->bytes[i] == commands::end_connection) {
      Serial.print("<> Responding to Heartbeat Request <>");
      //do something on disconnect if you want
    }
  }

  //return a bool signifying if the client is connected or not
  bool connected() {
    return this->client.connected();
  }
};

#endif
