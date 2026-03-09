/* WiFi + MQTT on pico 2W
03/05/2026
*/

/* To do:
  Get watchdog working
  Add pico analog and digital publish
  Add pico digital subscribe
*/

#include "define.h"
#include "fileoperations.h"

void callback(char* topic, byte* payload, unsigned int length) {//No subscriptions. MQTT Callback not used for temp and RH
/*  String msg;
  for (unsigned int i = 0; i < length; i++)
    msg += (char)payload[i];

  // Expected topic: host/gpio/N
  String t = String(topic);

  if (t.startsWith("pico/gpio/")) {
    int index = t.substring(10).toInt();  // Extract N

    if (index >= 0 && index < NUM_OUT) {
      if (msg == "ON") {
        digitalWrite(outputs[index], HIGH);
        client.publish("pico/status", ("OUT " + String(index) + " ON").c_str());
      }
      else if (msg == "OFF") {
        digitalWrite(outputs[index], LOW);
        client.publish("pico/status", ("OUT " + String(index) + " OFF").c_str());
      }
    }
  }*/
}

void ensureWiFi() {// Non-blocking WiFi reconnect

  if (WiFi.status() == WL_CONNECTED) {
//    Serial.println("WiFi connected");
    return;
  }

  unsigned long now = millis();
  if (now - lastWiFiReconnect < 3000) {
    Serial.println("Connecting to WiFi");
    return;
  }
  lastWiFiReconnect = now;
  WiFi.begin(cfg.ssid.c_str(),cfg.wifiPass.c_str());
}

void ensureMQTT() {// Non-blocking MQTT reconnect
  if (mqtt.connected()) {
//    Serial.println("MQTT connected");
    return;
  }
  unsigned long now = millis();
  if (now - lastMQTTReconnect < 2000) {
    Serial.println("Connecting to MQTT");
    return;
  }
  lastMQTTReconnect = now;

  if (mqtt.connect(cfg.host.c_str(),cfg.mqttUser.c_str(),cfg.mqttPass.c_str())) {//No subscriptions for this version
    String s=cfg.host+"/gpio/#";
    mqtt.subscribe(s.c_str());
  }
}

void pollInputs() {// Input polling (non-blocking)
unsigned long now = millis();
double ftmp,ftmp1;
String topic,msg;

  if (now - lastInputCheck < 1000) return; //Polling every 1 sec
  lastInputCheck = now;

Serial.println("Send data");
Serial.flush(); //Wait for all output characters to finish

    ledToggle ^= 1;
    digitalWrite(LED_BUILTIN,ledToggle);

    if (bme280) {
      ftmp = bme.readTemperature(); //Temp in C
      ftmp = 1.8 * ftmp + 32; //Convert to F
      if (ftmp>200) while (1); //I2C not working so reset
      topic = cfg.host+"/t";
      msg=String(ftmp,1);
      mqtt.publish(topic.c_str(), msg.c_str());

      ftmp = bme.readHumidity();
      topic = cfg.host+"/rh";
      msg=String(ftmp,0); ;
      mqtt.publish(topic.c_str(), msg.c_str());

      ftmp = bme.readPressure(); //Pressure in hPa
      ftmp = ftmp * 0.0002953;  //Pressure in inches Hg
      topic = cfg.host+"/bp";
      msg=String(ftmp,2); ;
      mqtt.publish(topic.c_str(), msg.c_str());
    }

    else if (hdc302x) {
      hdc.readTemperatureHumidityOnDemand(ftmp,ftmp1, TRIGGERMODE_LP0);
      ftmp = 1.8 * ftmp + 32; //Convert to F
      if (ftmp>200) while (1); //I2C not working so reset
      topic = cfg.host+"/t";
      msg=String(ftmp,1); ;
      mqtt.publish(topic.c_str(), msg.c_str());

      topic = cfg.host+"/rh";
      msg=String(ftmp1,0); ;
      mqtt.publish(topic.c_str(), msg.c_str());
    }

      else if (ad1015) {
        for (int i=0;i<=3;++i) {
          ftmp=ads.computeVolts(i);
          topic = cfg.host+"/ad"+String(i + 4);
          msg=String(ftmp1,3); ;
          mqtt.publish(topic.c_str(), msg.c_str());
        }
      }
  }

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  ledToggle=0;

  Serial.println("Version 0.003");
  delay(2000);
  
  fscfg.setAutoFormat(false); //Configure the little file system
  LittleFS.setConfig(fscfg);
  LittleFS.begin();
  Serial.println("");

  if (!loadConfig()) {
    Serial.println("No config found. Starting setup wizard...");
    runSetupWizard();
  }

loop:

  Serial.println(cfg.ssid);
  Serial.println(cfg.wifiPass);
  Serial.println(cfg.host);
  Serial.println(cfg.mqttHost);
  Serial.println(cfg.mqttPort);
  Serial.println(cfg.mqttUser);
  Serial.println(cfg.mqttPass);

  Serial.flush(); //Wait for all output characters to finish
  Serial.println("Press any key to run setup wizard...");
  delay(3000);
  if (Serial.available()){
    runSetupWizard();
    goto loop;
  }

  watchdogActivity=0;
   
  ad1015=ads.begin(); //Setup A-D converter
  if (ad1015) Serial.println("AD1015 Found");
  else Serial.println("AD1015 Absent");
  bme280=bme.begin(); //Setup the BME280 temp, humidity, baramoter
  if (bme280) Serial.println("BME280 Found");
  else Serial.println("BME280 Absent");
  hdc302x=hdc.begin(0x44, &Wire);
  if (hdc302x) Serial.println("HDC302x Found");
  else Serial.println("HDC302x Absent");
  Serial.flush(); //Wait for all output characters to finish

  WiFi.setHostname(cfg.host.c_str());
  WiFi.begin(cfg.ssid.c_str(),cfg.wifiPass.c_str());
  mqtt.setServer(cfg.mqttHost.c_str(),cfg.mqttPort);
  mqtt.setCallback(callback);
}

void loop() {// Main Loop (fully non-blocking)
  ensureWiFi();
  ensureMQTT();

  if (mqtt.connected())
    mqtt.loop();  // Non-blocking

  pollInputs();
}
