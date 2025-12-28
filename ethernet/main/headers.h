
#ifndef GARDENCONTROL_H
#define GARDENCONTROL_H

//for debugging use the upper two, for production the lower two
#define DEBUG_STRING(...) Serial.print(__VA_ARGS__);
#define DEBUG_LINE(...) \
  Serial.print(__VA_ARGS__); \
  Serial.print("\n");

enum MQTTState {
  NEW, CONNECTED, DISCONNECTED
};

struct MQTTSettings {
  String ip;
  String port;
  String topic;
  MQTTState state;
};



#endif