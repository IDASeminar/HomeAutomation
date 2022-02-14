#include <Arduino.h>
#include <ArduinoHA.h> //https://github.com/dawidchyrzynski/arduino-home-assistant
#include <ESP8266WiFi.h>
#include <privat.h>

#define LED 2
#define BROKER_ADDR     IPAddress(192,168,1,246)

char UniqID[9];
WiFiClient wificlient;
HADevice device;
HAMqtt mqtt(wificlient, device);
HASwitch led("led", false); // "led" is unique ID of the switch. You should define your own ID.

void onBeforeSwitchStateChanged(bool state, HASwitch* s)
{
    // this callback will be called before publishing new state to HA
    // in some cases there may be delay before onStateChanged is called due to network latency
}

void onSwitchStateChanged(bool state, HASwitch* s)
{
    digitalWrite(LED, (state ? HIGH : LOW));
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.printf("Connecting to %s pass %s \n",WIFI_SSID,WIFI_PASS);
  //Serial.println(WIFI_SSID);
  sprintf(&UniqID[0], "%08X", ESP.getChipId());

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }  
  device.setUniqueId((byte *) &UniqID[0],sizeof(UniqID));
  device.setName("Arduino");
  device.setSoftwareVersion("1.0.0");

    // set icon (optional)
    led.setIcon("mdi:lightbulb");
    led.setName("My LED");

    // handle switch state
    led.onBeforeStateChanged(onBeforeSwitchStateChanged); // optional
    led.onStateChanged(onSwitchStateChanged);

    mqtt.begin(mqtt_server,username,password);  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  mqtt.loop();
}