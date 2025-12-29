#include <PubSubClient.h>
#include <Ethernet.h>
#include "headers.h"

//main structure for all consumers to find their "stuff" on
struct Runtime globalRuntime;

//ethernet client (for MQTT)
EthernetClient _ethClient = EthernetClient();
//MQTT client
PubSubClient _mqttClient = PubSubClient(_ethClient);
//Web Server, always on port 80
EthernetServer _httpServer = EthernetServer(80);
//MQTT Settings
struct MQTTSettings _mqttSettings = {};
//Moisture Settings
struct Moisture _moisture = { 0 };


void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB port only

  while (Ethernet.begin(MAC_ADDRESS) == 0)
    ;  // waiting for a DHCP package... need one to function

  //setup relays to switch them all off!
  setupRelays();

  DEBUG_STRING(F("My IP address: "));
  DEBUG_LINE(Ethernet.localIP());

  //read MQTT settings from the EEPROM

  //set up the runtime
  globalRuntime.ethClient = &_ethClient;
  globalRuntime.mqttClient = &_mqttClient;
  globalRuntime.httpServer = &_httpServer;
  globalRuntime.moisture = &_moisture;
  globalRuntime.mqttSettings = &_mqttSettings;


  readMQTTSettings(globalRuntime.mqttSettings);

  // start listening for HTTP clients
  globalRuntime.httpServer->begin();
}

void loop() {

  //always run the webserver
  runWebServerLoop(&globalRuntime);

  if(globalRuntime.mqttSettings->state == INVALID)
    return;

  //either MQTT is DISCONNECTED or NEW - try to connect
  if (globalRuntime.mqttSettings->state != CONNECTED) {
    //try to do the actual MQTT connection to the server
    connectToMQTT(&globalRuntime);

    if (globalRuntime.mqttClient->connected()) {
      //we connected succesfully for the first time after not being connected

      //sending the current relay states to MQTT for sync purposes
      initAllRelays(&globalRuntime);
    }
  }

  //are we still connected?
  if (!globalRuntime.mqttClient->connected()) {
    //we're disconnected - no point doing anything else here until we connect
    globalRuntime.mqttSettings->state = DISCONNECTED;
    
  } else {
    //this probably does nothing, but let's just make sure we mark things as such
    globalRuntime.mqttSettings->state = CONNECTED;

    //handle incoming MQTT messages
    globalRuntime.mqttClient->loop();

    //write moisture sensor data when its time
    runMoistureSensor(&globalRuntime);
  }
}
