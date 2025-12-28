#include <PubSubClient.h>
#include <Ethernet.h>
#include "headers.h"

byte mac[] = {
  0xDE, 0x12, 0x34, 0x56, 0x78, 0xE9
};

MQTTSettings mqttSettings;

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
EthernetServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  while (Ethernet.begin(mac) == 0)
    ;  // waiting for a DHCP package... need one to function


  DEBUG_STRING("My IP address: ");
  DEBUG_LINE(Ethernet.localIP());

  mqttSettings = readMQTTSettings();

  // start listening for HTTP clients
  server.begin();
}

void loop() {

  runWebServerLoop(&server, &mqttSettings);

  if (mqttSettings.state != CONNECTED) {
    DEBUG_LINE("trying to connect to MQTT");
    //if we are disconnected, try to reconnect
    connectToMqtt(&mqttClient, &mqttSettings);
    DEBUG_LINE("end trying to connect to MQTT");

    if (mqttClient.connected())
      initAllRelays(&mqttClient, &mqttSettings);
  }


  if (mqttClient.connected()) {
    mqttSettings.state = CONNECTED;
    mqttClient.loop();
  } else {
    mqttSettings.state = DISCONNECTED;
  }

}
