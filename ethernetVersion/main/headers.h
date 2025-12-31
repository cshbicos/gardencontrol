
#ifndef GARDENCONTROL_H
#define GARDENCONTROL_H

//for debugging use the upper two, for production the lower two
#define DEBUG_STRING(...) Serial.print(__VA_ARGS__);
#define DEBUG_LINE(...) \
  Serial.print(__VA_ARGS__); \
  Serial.print("\n");

//#define DEBUG_STRING(...)
//#define DEBUG_LINE(...)

#include <PubSubClient.h>
#include <Ethernet.h>

byte MAC_ADDRESS[] = {
  0xDE, 0x12, 0x34, 0x56, 0x78, 0xE9
};

enum MQTTState {
  NEW, CONNECTED, DISCONNECTED, INVALID
};


#define ANALOG_PIN_COUNT 6
#define DIGITAL_PIN_COUNT 14
enum PinType {
  UNUSED, RELAY, MOISTURE
};

struct BoardConfig{
  uint8_t boardVersion;
  PinType digitalPin[DIGITAL_PIN_COUNT];
  PinType analogPin[ANALOG_PIN_COUNT];
};

struct MQTTRuntime {
  String ip;
  String port;
  String topic;
  MQTTState state;
};

struct MoistureRuntime {
  unsigned long lastCheck[ANALOG_PIN_COUNT];
  unsigned int pollTime[ANALOG_PIN_COUNT];
};

struct Runtime{
  BoardConfig *boardConfig;
  EthernetClient *ethClient;
  EthernetServer *httpServer;
  PubSubClient *mqttClient;
  MQTTRuntime *mqttRuntime;
  MoistureRuntime *moistureRuntime;
};


#endif