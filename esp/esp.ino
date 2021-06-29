#include <ESP8266WiFi.h>
#include <FS.h>
#include <SPIFFSIniFile.h>
#include <PubSubClient.h>


#define SETTINGS_FILE "/settings.ini"
#define LEN_WIFI_SSID 50
#define LEN_WIFI_PWD 80
#define LEN_MQTT_TOPIC 100
#define LEN_MQTT_SERVER 50
#define LEN_MQTT_PORT 50

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
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(SETTING_MQTT_SERVER, SETTING_MQTT_PORT);
  mqttClient.setCallback(callback);
}


void loop()
{
  if (!mqttClient.connected()) {
    reconnectMqqt();
  }
  mqttClient.loop();
}

void reconnectMqqt() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("gardenControl")) {
      Serial.println("connected");
      mqttClient.subscribe(SETTING_MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  
}

void getSettings()
{
   Serial.println("Reading settings....");

      
   if (!SPIFFS.begin())
    while (1)
      Serial.println("SPIFFS.begin() failed");
  
  SPIFFSIniFile ini(SETTINGS_FILE);
  if (!ini.open()) {
    Serial.print("Ini file ");
    Serial.print(SETTINGS_FILE);
    Serial.println(" does not exist");
    // Cannot do anything else
    while (1) ;
  }
  
  if (ini.getValue("wifi", "ssid", SETTING_WIFI_SSID, LEN_WIFI_SSID)) {
    Serial.print("read setting ssid => ");
    Serial.println(SETTING_WIFI_SSID);
  }else {
    Serial.print("Could not read ssid");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("wifi", "pwd", SETTING_WIFI_PWD, LEN_WIFI_PWD)) {
    Serial.print("read setting password => ");
    Serial.println(SETTING_WIFI_PWD);
  }else {
    Serial.print("Could not read password for wifi");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("mqtt", "server", SETTING_MQTT_SERVER, LEN_MQTT_SERVER)) {
    Serial.print("read setting mqtt server => ");
    Serial.println(SETTING_MQTT_SERVER);
  }else {
    Serial.print("Could not read mqtt server");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("mqtt", "port", SETTING_MQTT_PORT_STR, LEN_MQTT_PORT, SETTING_MQTT_PORT)) {
    Serial.print("read setting mqtt port => ");
    Serial.println(SETTING_MQTT_PORT);  
  }else {
    Serial.println("Could not read mqtt port");
    // Cannot do anything else
    while (1) ;
  }

  if (ini.getValue("mqtt", "topic", SETTING_MQTT_TOPIC, LEN_MQTT_TOPIC)) {
    Serial.print("read setting mqtt topic => ");
    Serial.println(SETTING_MQTT_TOPIC);
  }else {
    Serial.print("Could not read mqtt topic");
    // Cannot do anything else
    while (1) ;
  }
  
}
