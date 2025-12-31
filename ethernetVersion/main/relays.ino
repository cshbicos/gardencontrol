#include "headers.h"


#define RELAY_OFFSET 0

/**
 * Safety: switch off all relays at startup... Assuming they are solanoids, that will be a good good idea!
 */
void setupRelays(Runtime *runtime) {

  for (int i = 0; i < DIGITAL_PIN_COUNT; i++) {
    if (runtime->boardConfig->digitalPin[i] != RELAY)
      continue;

    pinMode(RELAY_OFFSET + i, OUTPUT);
    digitalWrite(RELAY_OFFSET + i, LOW);

    DEBUG_STRING(F("set relay on pin"))
    DEBUG_LINE(i)
  }
}


/**
 * Send initialization information for each relay and
 * subsequently subscibe to the MQTT channel
 */
void connectRelays(Runtime *runtime) {
  DEBUG_LINE(F("Initializing Relays to MQTT"));

  int relay = 0;
  for (int i = 0; i < DIGITAL_PIN_COUNT; i++) {
    if (runtime->boardConfig->digitalPin[i] != RELAY)
      continue;

    int pinVal = digitalRead(RELAY_OFFSET + i);


    String topic = runtime->mqttRuntime->topic + "/relay/" + relay;
    //publish the current value
    if (pinVal == HIGH) {
      runtime->mqttClient->publish(topic.c_str(), "ON", true);
    } else {
      runtime->mqttClient->publish(topic.c_str(), "OFF", true);
    }

    runtime->mqttClient->subscribe(topic.c_str());
    relay++;
  }
}

void handleRelayMQTTMessage(Runtime *runtime, String *subtopic, String *value) {
  int relay = -1;

  for (int i = 0; i < DIGITAL_PIN_COUNT; i++) {
    if (runtime->boardConfig->digitalPin[i] != RELAY)
      continue;

    relay++;
    
    if(relay != atoi(subtopic->c_str()))
      continue;

    if (value->equals("ON")) {
      DEBUG_LINE(F("Setting relay ON"))
      digitalWrite(RELAY_OFFSET + i, HIGH);
    } else {
      DEBUG_LINE(F("Setting relay OFF"))
      digitalWrite(RELAY_OFFSET + i, LOW);
    }
  }
}
