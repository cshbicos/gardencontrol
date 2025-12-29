#include "headers.h"

#define RELAY_COUNT 8
#define RELAY_OFFSET 2

/**
 * Safety: switch off all relays at startup... Assuming they are solanoids, that will be a good good idea!
 */
void setupRelays() {
  for (int i = 0; i < RELAY_COUNT; i++) {
    pinMode(RELAY_OFFSET + i, LOW);

    DEBUG_STRING("set relay on pin")
    DEBUG_LINE(RELAY_OFFSET + i)
  }
}


/**
 * Send initialization information for each relay and
 * subsequently subscibe to the MQTT channel
 */
void initAllRelays(Runtime *runtime) {
  DEBUG_LINE("Initializing Relays to MQTT");

  int mode;
  for (int i = 0; i < RELAY_COUNT; i++) {
    String fullTopic = "/" + runtime->mqttSettings->topic + "/relay/" + i;
    mode = digitalRead(RELAY_OFFSET + i);

    //publish the current value
    if (mode == HIGH) {
      runtime->mqttClient->publish(fullTopic.c_str(), "ON", true);
    } else {
      runtime->mqttClient->publish(fullTopic.c_str(), "OFF", true);
    }

    runtime->mqttClient->subscribe(fullTopic.c_str());
  }
}

void switchRelay(const char *subtopic, String *value) {

  int relay = atoi(subtopic);
  if (relay >= 0 && relay < RELAY_COUNT) {
    DEBUG_STRING("Setting relay ")
    DEBUG_STRING(subtopic)
    DEBUG_STRING(" to ")
    DEBUG_LINE(value->c_str())

    if (value->equals("ON"))
      pinMode(RELAY_OFFSET + relay, HIGH);
    else
      pinMode(RELAY_OFFSET + relay, LOW);
  }
}
