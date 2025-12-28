#include <PubSubClient.h>
#include "headers.h"

#define RELAY_COUNT 8
#define RELAY_OFFSET 2

void setupRelays() {

  int mode;
  for (int i = 0; i < RELAY_COUNT; i++) {
    pinMode(RELAY_OFFSET + i, LOW);

    DEBUG_STRING("set relay on pin")
    DEBUG_LINE(RELAY_OFFSET + i)
  }
}


/**
 * Send initialization information for each relay and
 * subsequently subscibe to the MQTT channel
 * 
 */
void initAllRelays(PubSubClient* mqttClient, MQTTSettings* mqttSettings) {
  DEBUG_LINE("Initializing Relays to MQTT");

  int mode;
  for (int i = 0; i < RELAY_COUNT; i++) {
    String fullTopic = "/" + mqttSettings->topic + "/relay/" + i;
    mode = digitalRead(RELAY_OFFSET + i);

    //publish the current value
    if (mode == HIGH) {
      mqttClient->publish(fullTopic.c_str(), "ON", true);
    } else {
      mqttClient->publish(fullTopic.c_str(), "OFF", true);
    }

    mqttClient->subscribe(fullTopic.c_str());
  }
}

void setRelay(String subtopic, String value) {

  int relay = atoi(subtopic.c_str());
  if (relay >= 0 && relay < RELAY_COUNT) {
    DEBUG_LINE("Setting relay " + subtopic + " to " + value);

    if (value.equals("ON"))
      pinMode(RELAY_OFFSET + relay, LOW);
    else
      pinMode(RELAY_OFFSET + relay, HIGH);
  }
}
