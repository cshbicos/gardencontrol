#include "headers.h"

//hard coded
#define MQTT_CLIENT_NAME "gardenBox"

/* 
 * Do the initial connection every time the server/port is changed (or set up the first time)
 */
void connectToMQTT(Runtime* runtime) {
  if (runtime->mqttSettings->state == NEW) {
    DEBUG_LINE(F("trying to connect to MQTT"));
    //first disconnect in case we are already connected
    runtime->mqttClient->disconnect();

    //if the state is NEW (set by HTTP setting change or first load - set the server and callback)
    int port = atoi(runtime->mqttSettings->port.c_str());

    DEBUG_LINE("Setting MQTT server to " + runtime->mqttSettings->ip + " / " + port)
    runtime->mqttClient->setServer(runtime->mqttSettings->ip.c_str(), port);
    runtime->mqttClient->setCallback(callback);
  }

  bool connected = runtime->mqttClient->connect(MQTT_CLIENT_NAME);
  if (connected) {
    DEBUG_LINE(F("MQTT client connected"));

    runtime->mqttSettings->state = CONNECTED;
  } else {
    DEBUG_STRING(F("MQTT client not connected - state "));
    DEBUG_LINE(runtime->mqttClient->state());

    runtime->mqttSettings->state = DISCONNECTED;
  }
}

bool isMQTTSettingsValid(MQTTSettings* settings) {
  if (settings->ip.length() <= 0)
    return false;

  if (settings->port.length() <= 0)
    return false;

  if (settings->topic.length() <= 0)
    return false;

  int port = atoi(settings->port.c_str());
  if (port <= 0 || port > 65535)
    return false;

  return true;
}

/**
 * Callback when a subscribed MQTT channel changes values
 */
void callback(char* topic, byte* payloadRaw, unsigned int length) {

  //parse something like "/gardencontroller/relay/0" and get "relay/0"
  String subtopic = String(topic).substring(globalRuntime.mqttSettings->topic.length() + 2);

  //parse payload
  String payload = "";
  for (int i = 0; i < length; i++)
    if (payloadRaw[i] != '\n')
      payload += (char)payloadRaw[i];

  DEBUG_LINE("MQTT Message arrived [" + subtopic + "]: " + payload);

  //from "relay/0", get the "relay" to execute something specific
  String mainSubtopic = subtopic.substring(0, subtopic.indexOf("/"));
  if (mainSubtopic.equals("relay")) {
    //parse the "0" from "relay/0" so we know what relay pin to switch
    switchRelay(subtopic.substring(mainSubtopic.length() + 1).c_str(), &payload);
  }
}