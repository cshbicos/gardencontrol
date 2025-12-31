#include "headers.h"

#define MOISTURE_OFFSET A0
#define MIN_POLL_TIME 100

void defaultMoistureSettings(Runtime *runtime) {
  for (int i = 0; i < ANALOG_PIN_COUNT; i++) {
    runtime->moistureRuntime->pollTime[i] = 0;
    runtime->moistureRuntime->lastCheck[i] = 0;
  }
}

void setupMoisture(Runtime *runtime) {
  for (int i = 0; i < ANALOG_PIN_COUNT; i++) {
    if (runtime->boardConfig->analogPin[i] == MOISTURE) {
      pinMode(MOISTURE_OFFSET + i, INPUT);

      DEBUG_STRING(F("set moisture sensore on analog pin "))
      DEBUG_LINE(i)
    }
  }
}

/*
 * Reset the last checks so we start sending again
 */
void connectMoisture(Runtime *runtime) {
  int sensor = -1;
  for (int i = 0; i < ANALOG_PIN_COUNT; i++) {
    if (runtime->boardConfig->analogPin[i] != MOISTURE)
      continue;
    sensor++;

    runtime->moistureRuntime->lastCheck[i] = 0;

    String topic = runtime->mqttRuntime->topic + "/moisture/" + sensor + "/pollTime";
    String value = String(runtime->moistureRuntime->pollTime[i]);
    runtime->mqttClient->publish(topic.c_str(), value.c_str(), true);


    runtime->mqttClient->subscribe(topic.c_str());
  }
}

void runMoistureSensor(Runtime *runtime) {
  unsigned long curTime = millis();
  int sensor = -1;

  for (int i = 0; i < ANALOG_PIN_COUNT; i++) {
    if (runtime->boardConfig->analogPin[i] != MOISTURE)
      continue;
    sensor++;

    if (runtime->moistureRuntime->pollTime[i] < MIN_POLL_TIME)
      //anything < MIN_POLL_TIME will spam too hard
      continue;

    if (curTime > runtime->moistureRuntime->lastCheck[i] && (runtime->moistureRuntime->lastCheck[i] + runtime->moistureRuntime->pollTime[i]) > curTime) {
      //we only need to check the sensor if the time resets OR we waited for the POLL time to elaps
      continue;
    }

    runtime->moistureRuntime->lastCheck[i] = curTime;
    int sensorValue = analogRead(MOISTURE_OFFSET + i);

    String topic = runtime->mqttRuntime->topic + "/moisture/" + sensor + "/value";
    String value = String(sensorValue);
    runtime->mqttClient->publish(topic.c_str(), value.c_str(), true);
  }
}

void handleMoistureMQTTMessage(Runtime *runtime, String *subtopic, String *value) {

  int sensor = -1;

  for (int i = 0; i < ANALOG_PIN_COUNT; i++) {
    if (runtime->boardConfig->analogPin[i] != MOISTURE)
      continue;
    sensor++;

    String sensorStr = subtopic->substring(0, subtopic->indexOf("/"));
    if (sensorStr.toInt() != sensor)
      continue;


    if (subtopic->endsWith("/pollTime")) {
      int newPollTime = value->toInt();
      if (newPollTime < MIN_POLL_TIME && newPollTime > 0) {
        newPollTime = MIN_POLL_TIME;
        String topic = runtime->mqttRuntime->topic + "/moisture/" + sensor + "/pollTime";
        runtime->mqttClient->publish(topic.c_str(), String(newPollTime).c_str(), true);
      } else {
        runtime->moistureRuntime->pollTime[i] = newPollTime;
        runtime->moistureRuntime->lastCheck[i] = 0;
        writePersistedSettings(runtime);
      }
    }
  }
}