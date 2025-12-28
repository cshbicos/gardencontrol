#include <EEPROM.h>
#include "headers.h"

void writeMQTTSettings(MQTTSettings *settings);

const char configHTML[] PROGMEM = {
  "<!DOCTYPE html><html><body><h2>MQTT Server Details</h2><form action=\"/\" method=\"post\"><label for=\"i\">MQTT Server IP</label><br><input type=\"text\" id=\"i\" name=\"i\" value=\"%0\"><br><label for=\"p\">MQTT Server Port:</label><br><input type=\"number\" id=\"p\" name=\"p\" value=\"%1\"><br><label for=\"t\">MQTT Topic:</label><br><input type=\"text\" id=\"t\" name=\"t\" value=\"%2\"><br><label for=\"s\">Connection Successful</label><input type=\"checkbox\" id=\"s\" name=\"s\" %3 disabled><br><input type=\"submit\" value=\"Save\"></form></body></html>"
};

const char errorHTML[] PROGMEM = { "<!DOCTYPE html><html><body>Error</body></html>" };

void writeMQTTSettings(MQTTSettings *settings) {
  //write the magic key so we know we've writte this particular EEPROM
  EEPROM.write(0, 13);
  EEPROM.write(1, 37);

  int cur = 2;
  EEPROM.write(cur, (uint8_t)  settings->ip.length());
  cur += 1;

  for (int i = 0; i < settings->ip.length(); i++) {
    EEPROM.write(cur, (uint8_t) settings->ip.charAt(i));
    cur += 1;
  }

  EEPROM.write(cur, (uint8_t)  settings->port.length());
  cur += 1;

  for (int i = 0; i < settings->port.length(); i++) {
    EEPROM.write(cur, (uint8_t)  settings->port.charAt(i));
    cur += 1;
  }

  EEPROM.write(cur, (uint8_t)  settings->topic.length());
  cur += 1;

  for (int i = 0; i < settings->topic.length(); i++) {
    EEPROM.write(cur, (uint8_t)  settings->topic.charAt(i));
    cur += 1;
  }
}

MQTTSettings readMQTTSettings() {
  MQTTSettings ret = { "192.168.0.1", "1883", "gardencontroller", NEW };


  //check the magic key so we know we've writte this particular EEPROM
  if (EEPROM.read(0) != 13 || EEPROM.read(1) != 37) {
    writeMQTTSettings(&ret);
    return ret;
  }

  int cur = 2;
  uint8_t len = EEPROM.read(cur);
  cur += 1;
  ret.ip = "";
  for (int i = 0; i < len; i++) {
    ret.ip += (char) EEPROM.read(cur);
    cur += 1;
  }
  DEBUG_LINE("Loaded IP " + ret.ip);

  len = EEPROM.read(cur);
  cur += 1;
  ret.port = "";
  for (int i = 0; i < len; i++) {
    ret.port += (char) EEPROM.read(cur);
    cur += 1;
  }
  DEBUG_LINE("Loaded port " + ret.port);

  len = EEPROM.read(cur);
  cur += 1;
  ret.topic = "";
  for (int i = 0; i < len; i++) {
    ret.topic += (char) EEPROM.read(cur);
    cur += 1;
  }
  DEBUG_LINE("Loaded topic " + ret.topic);
}