#include "headers.h"

//hard coded
#define MQTT_CLIENT_NAME "gardenBox"


void defaultMQTTSettings(Runtime* runtime) {
  runtime->mqttRuntime->ip = "192.168.0.1";
  runtime->mqttRuntime->port = "1883";
  runtime->mqttRuntime->topic = "gardencontrol";
}

void setupMQTT(Runtime* runtime) {
  runtime->mqttRuntime->state = NEW;
}

/* 
 * Do the initial connection every time the server/port is changed (or set up the first time)
 */
void connectToMQTT(Runtime* runtime) {
  if (runtime->mqttRuntime->state == NEW) {
    DEBUG_LINE(F("trying to connect to MQTT"));
    //first disconnect in case we are already connected
    runtime->mqttClient->disconnect();

    //if the state is NEW (set by HTTP setting change or first load - set the server and callback)
    int port = atoi(runtime->mqttRuntime->port.c_str());

    DEBUG_LINE("Setting MQTT server to " + runtime->mqttRuntime->ip + " / " + port)
    runtime->mqttClient->setServer(runtime->mqttRuntime->ip.c_str(), port);
    runtime->mqttClient->setCallback(mqttCallback);
  }

  bool connected = runtime->mqttClient->connect(MQTT_CLIENT_NAME);
  if (connected) {
    DEBUG_LINE(F("MQTT client connected"));

    runtime->mqttRuntime->state = CONNECTED;
  } else {
    // DEBUG_STRING(F("MQTT client not connected - state "));
    //DEBUG_LINE(runtime->mqttClient->state());

    runtime->mqttRuntime->state = DISCONNECTED;
  }
}

bool isMQTTSettingsValid(Runtime* runtime) {
  if (runtime->mqttRuntime->ip.length() <= 0)
    return false;

  if (runtime->mqttRuntime->port.length() <= 0)
    return false;

  if (runtime->mqttRuntime->topic.length() <= 0)
    return false;

  int port = atoi(runtime->mqttRuntime->port.c_str());
  if (port <= 0 || port > 65535)
    return false;

  return true;
}

/**
 * Callback when a subscribed MQTT channel changes values
 */
void mqttCallback(char* topic, byte* payloadRaw, unsigned int length) {
  //not nice but has to be done here :(
  Runtime* runtime = &globalRuntime;


  //parse something like "gardencontroller/relay/0" and get "relay/0"
  String subtopic = String(topic).substring(runtime->mqttRuntime->topic.length() + 1);
  //from "relay/0", get the "relay" to execute something specific
  String mainSubtopic = subtopic.substring(0, subtopic.indexOf("/"));
  //Specific topic data (e.g. "0" in relay/0)
  String topicSpecific = subtopic.substring(mainSubtopic.length() + 1);

  //parse payload
  String payload = "";
  for (int i = 0; i < length; i++)
    if (payloadRaw[i] != '\n')
      payload += (char)payloadRaw[i];

  DEBUG_LINE("MQTT Message arrived [" + subtopic + "]: " + payload);

  if (mainSubtopic.equals("relay")) {
    handleRelayMQTTMessage(runtime, &topicSpecific, &payload);
  } else if (mainSubtopic.equals("moisture")) {
    handleMoistureMQTTMessage(runtime, &topicSpecific, &payload);
  }
}