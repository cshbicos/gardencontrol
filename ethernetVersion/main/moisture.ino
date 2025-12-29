#include "headers.h"
#define MOISTURE_PIN A0
#define MOISTURE_WAIT_PERIOD 5000


void runMoistureSensor(Runtime *runtime) {
  unsigned long curTime = millis();

  if (curTime > runtime->moisture->lastCheck && (runtime->moisture->lastCheck + MOISTURE_WAIT_PERIOD) > curTime) {
    //we only need to check the sensor if the time resets OR we waited for MOISTURE_WAIT_PERIOD
    return;
  }
  runtime->moisture->lastCheck = curTime;
  int sensorValue = analogRead(MOISTURE_PIN);

  String topic = "/" + runtime->mqttSettings->topic + "/moisture/0";
  String value = String(sensorValue);
  runtime->mqttClient->publish( topic.c_str(), value.c_str(), true );
}