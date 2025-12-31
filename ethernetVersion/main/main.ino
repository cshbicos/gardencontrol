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

//Board Config
struct BoardConfig _boardConfig = {};
//MQTT Runtime
struct MQTTRuntime _mqttRuntime = {};
//Moisture Runtime
struct MoistureRuntime _moistureRuntime = { };


/**
 * Board is hard coded - if you need to change this, you probably had to physically
 * rewire stuff anyway... just bite the bullet and recompile!
 */
void setupBoard(BoardConfig *board){
  board->boardVersion = 2;

  board->digitalPin[0] = UNUSED;
  board->digitalPin[1] = UNUSED;
  board->digitalPin[2] = RELAY;
  board->digitalPin[3] = RELAY;
  board->digitalPin[4] = RELAY;
  board->digitalPin[5] = RELAY;
  board->digitalPin[6] = RELAY;
  board->digitalPin[7] = RELAY;
  board->digitalPin[8] = RELAY;
  board->digitalPin[9] = RELAY;
  board->digitalPin[10] = UNUSED;
  board->digitalPin[11] = UNUSED;
  board->digitalPin[12] = UNUSED;
  board->digitalPin[13] = UNUSED;

  board->analogPin[0] = MOISTURE;
  board->analogPin[1] = UNUSED;
  board->analogPin[2] = UNUSED;
  board->analogPin[3] = UNUSED;
  board->analogPin[4] = UNUSED;
  board->analogPin[5] = UNUSED;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB port only

  while (Ethernet.begin(MAC_ADDRESS) == 0)
    ;  // waiting for a DHCP package... need one to function
  
  DEBUG_STRING(F("My IP address: "));
  DEBUG_LINE(Ethernet.localIP());

  //read MQTT settings from the EEPROM

  //set up the runtime
  globalRuntime.boardConfig = &_boardConfig;
  setupBoard(globalRuntime.boardConfig);

  globalRuntime.ethClient = &_ethClient;
  globalRuntime.mqttClient = &_mqttClient;
  globalRuntime.httpServer = &_httpServer;

  globalRuntime.moistureRuntime = &_moistureRuntime;
  globalRuntime.mqttRuntime = &_mqttRuntime;

  readPersistedSettings(&globalRuntime);

  //setup consumers
  setupMQTT(&globalRuntime);
  setupMoisture(&globalRuntime);
  setupRelays(&globalRuntime);

  // start listening for HTTP clients
  globalRuntime.httpServer->begin();
}

void loop() {

  //always run the webserver
  runHTTPServerLoop(&globalRuntime);

  if(globalRuntime.mqttRuntime->state == INVALID)
    return;

  //either MQTT is DISCONNECTED or NEW - try to connect
  if (globalRuntime.mqttRuntime->state != CONNECTED) {
    //try to do the actual MQTT connection to the server
    connectToMQTT(&globalRuntime);

    if (globalRuntime.mqttClient->connected()) {
      //we connected succesfully for the first time after not being connected

      //modules might need to interact with MQTT
      connectRelays(&globalRuntime);
      connectMoisture(&globalRuntime);
    }
  }

  //are we still connected?
  if (!globalRuntime.mqttClient->connected()) {
    //we're disconnected - no point doing anything else here until we connect
    globalRuntime.mqttRuntime->state = DISCONNECTED;
    
  } else {
    //this probably does nothing, but let's just make sure we mark things as such
    globalRuntime.mqttRuntime->state = CONNECTED;

    //handle incoming MQTT messages
    globalRuntime.mqttClient->loop();

    //write moisture sensor data when its time
    runMoistureSensor(&globalRuntime);
  }
}
