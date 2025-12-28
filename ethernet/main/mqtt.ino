#include <PubSubClient.h>
#include "headers.h"


void connectToMqtt(PubSubClient* mqttClient, MQTTSettings* mqttSettings) {
  if (mqttSettings->state == NEW) {
    int port = atoi(mqttSettings->port.c_str());
    DEBUG_LINE("Setting MQTT server to " + mqttSettings->ip + " / " + port)
    mqttClient->setServer(mqttSettings->ip.c_str(), port);
    mqttClient->setCallback(callback);
  }

  bool connected = mqttClient->connect("gardenBox");
  if (connected) {
    mqttSettings->state = CONNECTED;
    DEBUG_LINE("MQTT client connected");
  } else {
    mqttSettings->state = DISCONNECTED;
    DEBUG_STRING("MQTT client not connected - state ");
    DEBUG_LINE(mqttClient->state());
  }
}


/**
 * Callback when a subscribed MQTT channel changes values
 */
void callback(char* topic, byte* payloadRaw, unsigned int length) {
  String subtopic = String(topic).substring(mqttSettings.topic.length() + 2);
  String payload = "";
  for (int i = 0; i < length; i++)
    if (payloadRaw[i] != '\n')
      payload += (char)payloadRaw[i];

  DEBUG_LINE("MQTT Message arrived [" + subtopic + "]: " + payload);

  String mainSubtopic = subtopic.substring(0, subtopic.indexOf("/"));
  if(mainSubtopic.equals("relay")){
    setRelay(subtopic.substring(mainSubtopic.length() + 1), payload);
  }
}