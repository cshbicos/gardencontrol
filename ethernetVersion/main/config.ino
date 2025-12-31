#include <EEPROM.h>
#include "headers.h"

#define MAGIC_KEY_0 13
#define MAGIC_KEY_1 38

void readPersistedSettings(Runtime *runtime) {
  //setup some sensible defaults
  defaultMQTTSettings(runtime);
  defaultMoistureSettings(runtime);

  //check the magic key so we know we've writte this particular EEPROM
  if (EEPROM.read(0) != MAGIC_KEY_0 || EEPROM.read(1) != MAGIC_KEY_1
      || EEPROM.read(2) != runtime->boardConfig->boardVersion) {
    writePersistedSettings(runtime);
    return;
  }

  int cur = 3;
  int maxLength = EEPROM.length();

  int end = cur + config_read_unit8(cur++);
  runtime->mqttRuntime->ip = "";
  for (; cur < maxLength && cur < end; cur++)
    runtime->mqttRuntime->ip += config_read_char(cur);
  DEBUG_LINE("Loaded IP " + runtime->mqttRuntime->ip);
  if (cur == maxLength) return;

  end = cur + config_read_unit8(cur++);
  runtime->mqttRuntime->port = "";
  for (; cur < maxLength && cur < end; cur++)
    runtime->mqttRuntime->port += config_read_char(cur);
  DEBUG_LINE("Loaded port " + runtime->mqttRuntime->port);
  if (cur == maxLength) return;

  end = cur + config_read_unit8(cur++);
  runtime->mqttRuntime->topic = "";
  for (; cur < maxLength && cur < end; cur++)
    runtime->mqttRuntime->topic += config_read_char(cur);
  DEBUG_LINE("Loaded topic " + runtime->mqttRuntime->topic);
  if (cur == maxLength) return;

  int intSize = sizeof(unsigned int);
  for (int i = 0; i < ANALOG_PIN_COUNT; i++) {
    if (runtime->boardConfig->analogPin[i] == MOISTURE) {
      EEPROM.get(cur, runtime->moistureRuntime->pollTime[i]);
      cur += intSize;
      if (cur >= maxLength)
        break;
    }
  }
}

void writePersistedSettings(Runtime *runtime) {
  //write the magic key so we know we've writte this particular EEPROM
  EEPROM.write(0, MAGIC_KEY_0);
  EEPROM.write(1, MAGIC_KEY_1);
  EEPROM.write(2, runtime->boardConfig->boardVersion);

  int cur = 3;
  int maxLength = EEPROM.length();

  config_write_unit8(cur++, runtime->mqttRuntime->ip.length());
  for (int i = 0; i < runtime->mqttRuntime->ip.length() && cur < maxLength; i++)
    config_write_char(cur++, runtime->mqttRuntime->ip.charAt(i));
  if (cur == maxLength) return;

  config_write_unit8(cur, runtime->mqttRuntime->port.length());
  cur += 1;
  for (int i = 0; i < runtime->mqttRuntime->port.length() && cur < maxLength; i++)
    config_write_char(cur++, runtime->mqttRuntime->port.charAt(i));
  if (cur == maxLength) return;

  config_write_unit8(cur, runtime->mqttRuntime->topic.length());
  cur += 1;
  for (int i = 0; i < runtime->mqttRuntime->topic.length() && cur < maxLength; i++)
    config_write_char(cur++, runtime->mqttRuntime->topic.charAt(i));
  if (cur == maxLength) return;

  int intSize = sizeof(unsigned int);
  for (int i = 0; i < ANALOG_PIN_COUNT; i++) {
    if (runtime->boardConfig->analogPin[i] == MOISTURE) {
      EEPROM.put(cur, runtime->moistureRuntime->pollTime[i]);
      cur += intSize;
      if (cur >= maxLength)
        break;
    }
  }
}

inline char config_read_char(int address) {
  return (char)EEPROM.read(address);
}

inline uint8_t config_read_unit8(int address) {
  return EEPROM.read(address);
}

inline void config_write_char(int address, char c) {
  EEPROM.write(address, (uint8_t)c);
}

inline void config_write_unit8(int address, uint8_t i) {
  EEPROM.write(address, i);
}
