#include <EEPROM.h>
#include <avr/pgmspace.h>
#include "headers.h"

void writeMQTTSettings(MQTTSettings *settings);

const char configHTML[] PROGMEM = {
  "<!DOCTYPE html><html><head><title>Garden Controller</title></head>"
  "<body><h2>Garden Controller Setup</h2><form action=\"/\" method=\"post\">%3"
  "<label for=\"i\">MQTT Server:</label><br><input type=\"text\" id=\"i\" name=\"i\" value=\"%0\"><br>"
  "<label for=\"p\">MQTT Port:</label><br><input type=\"number\" id=\"p\" name=\"p\" value=\"%1\"><br>"
  "<label for=\"t\">MQTT Topic:</label><br><input type=\"text\" id=\"t\" name=\"t\" value=\"%2\"><br><br>"
  "<input type=\"submit\" value=\"Save\">&nbsp;<input type=\"button\" value=\"Refresh\" onclick=\"window.location.href='/'\"></form>"
  "<p>Details <a href=\"https://github.com/cshbicos/gardencontrol\">here</a></p></body></html>"
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

void readMQTTSettings(MQTTSettings *settings) {
  settings->ip = "192.168.0.1";
  settings->port = "1883";
  settings->topic = "gardencontroller";
  settings->state = NEW;

  //check the magic key so we know we've writte this particular EEPROM
  if (EEPROM.read(0) != 13 || EEPROM.read(1) != 37) {
    writeMQTTSettings(settings);
    return;
  }

  int cur = 2;
  uint8_t len = EEPROM.read(cur);
  cur += 1;
  settings->ip = "";
  for (int i = 0; i < len; i++) {
    settings->ip += (char) EEPROM.read(cur);
    cur += 1;
  }
  DEBUG_LINE("Loaded IP " + settings->ip);

  len = EEPROM.read(cur);
  cur += 1;
  settings->port = "";
  for (int i = 0; i < len; i++) {
    settings->port += (char) EEPROM.read(cur);
    cur += 1;
  }
  DEBUG_LINE("Loaded port " + settings->port);

  len = EEPROM.read(cur);
  cur += 1;
  settings->topic = "";
  for (int i = 0; i < len; i++) {
    settings->topic += (char) EEPROM.read(cur);
    cur += 1;
  }
  DEBUG_LINE("Loaded topic " + settings->topic);

}