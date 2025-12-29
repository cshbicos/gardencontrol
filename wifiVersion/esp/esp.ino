#include <ESP8266WiFi.h>
#include <FS.h>
#include <SPIFFSIniFile.h>
#include <PubSubClient.h>

//#define DS(...) Serial.print(__VA_ARGS__);
//#define DL(...) Serial.print(__VA_ARGS__); Serial.print("\n");
//#define DL(...) debugToMqtt(__VA_ARGS__, true);
//#define DS(...) debugToMqtt(__VA_ARGS__, false);
#define DL(...) 
#define DS(...) 

#define SETTINGS_FILE "/settings.ini"
#define LEN_WIFI_SSID 50
#define LEN_WIFI_PWD 80
#define LEN_MQTT_TOPIC 100
#define LEN_MQTT_SERVER 50
#define LEN_MQTT_PORT 50

#define LEN_SERIAL_BUFFER 200

#define LEN_DEBUG_BUFFER 200

char SETTING_WIFI_SSID[LEN_WIFI_SSID];
char SETTING_WIFI_PWD[LEN_WIFI_PWD];
char SETTING_MQTT_SERVER[LEN_MQTT_SERVER];
int  SETTING_MQTT_PORT = 0;
char SETTING_MQTT_TOPIC[LEN_MQTT_TOPIC];
char SETTING_MQTT_PORT_STR[LEN_MQTT_PORT];

char DEBUG_BUFFER[LEN_DEBUG_BUFFER];
bool ARDUINO_INIT;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/**
 * Initialize the ESP board
 * 
 * - Read settings for wifi and MQTT
 * - Connect to wifi
 * - Initialize the MQTT connection
 */
void setup() {
  DEBUG_BUFFER[0] = '\0';
  ARDUINO_INIT = false;
  
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

  //this is not something the Arduino will understand, but just to create a newline
  //so the next command is "pure"
  Serial.print("starting\n");

}

/**
 * Do every time
 */
void loop()
{
  if (!mqttClient.connected()) {
    //if we are disconnected, try to reconnect
    reconnectMqqt();
    //because we (were?) disconnected, we are uninitialized
    ARDUINO_INIT = false;
  }
  
  if(ARDUINO_INIT == false){
    //ask the Arduino to initialize itself
    //this will run until the Arduino response with 
    // a "/initStart" command
    Serial.print("/init\n");
  }

  //check if there are any subscriptions to be fetched
  mqttClient.loop();

  //check what the Arduino has been saying to us
  handleSerialInput();
}

/**
 * Retrieve commands from the Arduino
 * 
 */
void handleSerialInput() {
  int len;
  bool subscribe;
  char topic[LEN_SERIAL_BUFFER];
  char value[LEN_SERIAL_BUFFER];

  if (!Serial.available())
    //nothing to be done
    return;

  //preparing the full topic string by setting the "main topic"
  // and then putting the subTopicStart pointer to where the subtopic begins
  strncpy(topic, SETTING_MQTT_TOPIC, LEN_MQTT_TOPIC);
  char * subTopicStart = topic + strlen(topic);

  DL("Got something on serial");
  len = Serial.readBytesUntil('\n', subTopicStart, LEN_SERIAL_BUFFER - strlen(topic));
  if (len <= 0) {
    DL("Serial topic could not be read");
    return;
  }
  DS("Read command line ");
  DL(subTopicStart);

  char command = subTopicStart[0];
  subTopicStart[0] = '/';
  subTopicStart[len] = '\0';

  switch (command) {
    case '>':
      while (!Serial.available()) {
        //active waiting on payload
        continue;
      }

      len = Serial.readBytesUntil('\n', value, LEN_SERIAL_BUFFER);
      if (len <= 0) {
        DL("Payload could not be read");
        return;
      }
      value[len] = '\0';
      DS("Read payload line ");
      DL(value);

      mqttClient.publish(topic, value, true);
      DS("publishing to topic [");
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
    case '/':
      if(strncmp("/initStart", subTopicStart, 10) == 0){
        ARDUINO_INIT = true;
      }
      break;
    default:
      DL("Unknown topic command");
  }
}

/**
 * Reconnect (or initially connect) to Mosquitto server
 * 
 */
void reconnectMqqt() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    DL("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("gardenControl")) {
      DL("connected - initializing mqtt now");
    } else {
      DL("failed, rc=");
      DS(mqttClient.state());
      DL(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/**
 * Debugging output to MQTT instead Serial
 * - to debug live Arduino-ESP communication
 */
void debugToMqtt(const int intVal, bool flushBuffer){
  char buf[12];
  debugToMqtt(itoa(intVal, buf, 10), flushBuffer);
}

/**
 * Debugging output to MQTT instead Serial
 * - to debug live Arduino-ESP communication
 */
void debugToMqtt(const char * txt, bool flushBuffer){
  strcat(DEBUG_BUFFER, txt);
  if(!flushBuffer)
    return;
    
  if (mqttClient.connected()) {
    mqttClient.publish("gardencontrol/debug", DEBUG_BUFFER, false);
  }
  DEBUG_BUFFER[0] = '\0';
}

/**
 * Callback when a subscribed MQTT channel changes values
 */
void callback(char* topic, byte* payload, unsigned int length) {
  char subtopic[LEN_MQTT_TOPIC];
  char serialPayload[LEN_SERIAL_BUFFER];

  //copy the payload and topic because the debugging calls might override those buffers
  //deep within the PublishSubscribe framework!
  strncpy(subtopic, topic + strlen(SETTING_MQTT_TOPIC), LEN_MQTT_TOPIC);
  strncpy(serialPayload, (char*)payload, length);
  serialPayload[length] = '\0';
  
  DS("MQTT Message arrived [");
  DS(subtopic);
  DS("]: ");
  DL((char*) serialPayload);

  subtopic[0] = '>';

  Serial.print(subtopic);
  Serial.print("\n");
  Serial.print((char*) serialPayload);
  Serial.print("\n");

}

/**
 * Load settings from attached flash storage
 * - Loads all settings from an ini file
 * 
 * Hint: Every buffer for the final value must be large enough to contain any given line in the ini file!
 */
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
