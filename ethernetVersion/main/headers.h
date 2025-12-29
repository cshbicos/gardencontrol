
#ifndef GARDENCONTROL_H
#define GARDENCONTROL_H

//for debugging use the upper two, for production the lower two
//#define DEBUG_STRING(...) Serial.print(__VA_ARGS__);
//#define DEBUG_LINE(...) \
  Serial.print(__VA_ARGS__); \
  Serial.print("\n");

#define DEBUG_STRING(...)
#define DEBUG_LINE(...)

#include <PubSubClient.h>
#include <Ethernet.h>

byte MAC_ADDRESS[] = {
  0xDE, 0x12, 0x34, 0x56, 0x78, 0xE9
};

enum MQTTState {
  NEW, CONNECTED, DISCONNECTED, INVALID
};

struct MQTTSettings {
  String ip;
  String port;
  String topic;
  MQTTState state;
};

struct Moisture {
  unsigned long lastCheck;
};

struct Runtime{
  EthernetClient *ethClient;
  EthernetServer *httpServer;
  PubSubClient *mqttClient;
  MQTTSettings *mqttSettings;
  Moisture *moisture;
};


#endif