#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <ArduinoJson.h>
#include <privat.h>
#define LED 2


#define PRODUCT "IDA Demo"
char workgroup[32] = "workgroup";
char ha_name[] ="core-mosquitto";

// WiFi credentials.

char mqttTopicStatus[30];

WiFiClient wificlient;
PubSubClient client(wificlient);
char UniqID[9];

void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
}

void toggle() {
static bool ledState;
  //Serial.println("Toggling LED.");
  ledState = !ledState;
  digitalWrite(LED, ledState ? HIGH : LOW);
}

void SendStatus() {
  char Name[30];
  char msg[150];
  DynamicJsonDocument json(1024);
  
  
  Serial.println(mqttTopicStatus);
  sprintf(&Name[0],"IDA-%08X",ESP.getChipId());
  json["unique_id"] = Name;
  json["temperature"]="42.0"; //test data
  json["RSSI"]=WiFi.RSSI();
  json["IPAdr"]=WiFi.localIP();
  serializeJson(json,Serial);
  Serial.println("");
  serializeJson(json,msg);
  client.publish(mqttTopicStatus, msg);
}

void sendMQTTDiscoveryMsg(const char *component,
                          const char *config_key) 
{
  // This is the discovery topic for this specific sensor
  //String discoveryTopic = "homeassistant/sensor/my_sensor_" + UniqID + "/temperature/config";
  char discoveryTopic[100];
  char Name[30];
  sprintf(&discoveryTopic[0],"homeassistant/%s/%08X/%s/config",component,ESP.getChipId(),config_key);

  DynamicJsonDocument doc(1024);
  char buffer[256];
  sprintf(mqttTopicStatus, "/%08X/Status", ESP.getChipId());
  sprintf(&Name[0],"IDA-%08X",ESP.getChipId());
  doc["unique_id"] = Name;
  sprintf(&Name[0],"IDA %08X sensor",ESP.getChipId());
  doc["name"] = Name; // "MySensor " + UniqID + " Temperature";
  doc["stat_t"]   = mqttTopicStatus;
  doc["unit_of_meas"] = "°C";
  doc["dev_cla"] = "temperature";
  doc["frc_upd"] = true;
  // I'm sending a JSON object as the state of this MQTT device
  // so we'll need to unpack this JSON object to get a single value
  // for this specific sensor.
  doc["val_tpl"] = "{{ value_json.temperature|default(0) }}";

  size_t n = serializeJson(doc, buffer);
  serializeJson(doc,Serial);
  client.publish(discoveryTopic, buffer, n);
}

/**
 * MQTT callback to process messages
 * Start, EndPoint : start og slutpunkt
 * Posplus, PosMinus: Manuel frem og tilbage
 */
void callback(char* topic, byte* payload, unsigned int length) {
}

void reconnect() {
 char str[25]; 
  if (client.connect(UniqID,username,password)) {
      Serial.printf("connected %s\n",mqtt_server);
      sprintf(&str[0], "/%08X/cmd", ESP.getChipId());
      client.subscribe(str);
      Serial.printf("Subscribe to %s\n",str);
      //SendStatus();
      sendMQTTDiscoveryMsg("sensor","TestTemp");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  sprintf(&UniqID[0], "%08X", ESP.getChipId());

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  } 
    setClock();
  

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);
  reconnect();
  delay(100);
  SendStatus(); 
}

void loop() {
  toggle();
  delay(100);
  if (WiFi.status() == WL_CONNECTED) {
  //  if (!client.connected()) reconnect(); else 
    client.loop();
  }
}