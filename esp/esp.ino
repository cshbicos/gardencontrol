#include <ESP8266WiFi.h>
#include <FS.h>
#include <SPIFFSIniFile.h>
#include <PubSubClient.h>

//#define DS(...) Serial.print(__VA_ARGS__);
//#define DL(...) Serial.println(__VA_ARGS__);
#define DL(...)
#define DS(...)

#define SETTINGS_FILE "/settings.ini"
#define LEN_WIFI_SSID 50
#define LEN_WIFI_PWD 80
#define LEN_MQTT_TOPIC 100
#define LEN_MQTT_SERVER 50
#define LEN_MQTT_PORT 50

#define LEN_SERIAL_BUFFER 200

char SETTING_WIFI_SSID[LEN_WIFI_SSID];
char SETTING_WIFI_PWD[LEN_WIFI_PWD];
char SETTING_MQTT_SERVER[LEN_MQTT_SERVER];
int SETTING_MQTT_PORT = 0;
char SETTING_MQTT_TOPIC[LEN_MQTT_TOPIC];

char SETTING_MQTT_PORT_STR[LEN_MQTT_PORT];

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);



void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  getSettings();

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(SETTING_WIFI_SSID, SETTING_WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DS(".");
  }

  DL("WiFi connected");
  DS("IP address: ");
  DL(WiFi.localIP());

  mqttClient.setServer(SETTING_MQTT_SERVER, SETTING_MQTT_PORT);
  mqttClient.setCallback(callback);

  Serial.println("starting");

}


void loop()
{
  if (!mqttClient.connected()) {
    reconnectMqqt();
  }
  mqttClient.loop();

  handleSerialInput();
}

void handleSerialInput() {
  int len;
  bool subscribe;
  char topic[LEN_SERIAL_BUFFER];
  char value[LEN_SERIAL_BUFFER];

  if (!Serial.available())
    return;

  strncpy(topic, SETTING_MQTT_TOPIC, LEN_MQTT_TOPIC);
  char * subTopicStart = topic + strlen(topic);

  DL("Got something on serial");
  len = Serial.readBytesUntil('\n', subTopicStart, LEN_SERIAL_BUFFER - strlen(topic));
  if (len <= 0) {
    DL("Serial topic could not be read");
    return;
  }

  char command = subTopicStart[0];
  subTopicStart[0] = '/';
  subTopicStart[len] = '\0';

  switch (command) {
    case '>':
      while (!Serial.available()) {
        //DL("Waiting on payload");
        continue;
      }

      len = Serial.readBytesUntil('\n', value, LEN_SERIAL_BUFFER);
      if (len <= 0) {
        DL("Payload could not be read");
        return;
      }
      value[len] = '\0';

      mqttClient.publish(topic, value, true);
      DS("publishing to topic [ ");
      DS(topic);
      DS("] : ");
      DL(value);
      break;
    case '-':
      DS("Unsubscribing topic ");
      DL(topic);
      mqttClient.unsubscribe(topic);
      break;
    case '+':
      DS("Subscribing to topic ");
      DL(topic);
      mqttClient.subscribe(topic);
      break;
    default:
      DL("Unknown topic command");
  }
}

void reconnectMqqt() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    DL("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("gardenControl")) {
      DL("connected - initializing mqtt now");
      Serial.println("/init");
    } else {
      DL("failed, rc=");
      DS(mqttClient.state());
      DL(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char *subtopic = topic + strlen(SETTING_MQTT_TOPIC);
  DS("Message arrived [");
  DS(subtopic);
  DS("]: ");

  payload[length] = '\0';
  DL((char*) payload);

  subtopic[0] = '>';
  Serial.println(subtopic);
  Serial.println((char*) payload);

}

void getSettings()
{
  DL("Reading settings....");

  if (!SPIFFS.begin())
    while (1)
      DL("SPIFFS.begin() failed");

  SPIFFSIniFile ini(SETTINGS_FILE);
  if (!ini.open()) {
    DL("Ini file ");
    DS(SETTINGS_FILE);
    DL(" does not exist");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("wifi", "ssid", SETTING_WIFI_SSID, LEN_WIFI_SSID)) {
    DS("read setting ssid => ");
    DL(SETTING_WIFI_SSID);
  } else {
    DS("Could not read ssid");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("wifi", "pwd", SETTING_WIFI_PWD, LEN_WIFI_PWD)) {
    DS("read setting password => ");
    DL(SETTING_WIFI_PWD);
  } else {
    DS("Could not read password for wifi");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("mqtt", "server", SETTING_MQTT_SERVER, LEN_MQTT_SERVER)) {
    DS("read setting mqtt server => ");
    DL(SETTING_MQTT_SERVER);
  } else {
    DS("Could not read mqtt server");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("mqtt", "port", SETTING_MQTT_PORT_STR, LEN_MQTT_PORT, SETTING_MQTT_PORT)) {
    DS("read setting mqtt port => ");
    DL(SETTING_MQTT_PORT);
  } else {
    DL("Could not read mqtt port");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("mqtt", "topic", SETTING_MQTT_TOPIC, LEN_MQTT_TOPIC)) {
    DS("read setting mqtt topic => ");
    DL(SETTING_MQTT_TOPIC);
  } else {
    DS("Could not read mqtt topic");
    // Cannot do anything else
    while (1) ;
  }

}
