/* WiFi + MQTT on pico 2W
03/05/2026
Supports 0-10 digital inputs
0-3 analog inputs
0-12 digital outputs
BME280,HDC302x,ADS1015
*/

#include "define.h"
#include "fileoperations.h"

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Convert topic to a String for easy parsing
    String t = String(topic);

    // Expect topic like: host/3
    int slash = t.lastIndexOf('/');
    if (slash < 0) return;  // malformed topic

    String indexStr = t.substring(slash + 1);
    int index = indexStr.toInt()-1;

    // Bounds check
    if (index < 0 || index >= (int)sizeof(outputDo)) return;

    // Payload must be at least 1 byte
    if (length == 0) return;

    char c = payload[0];

    if (c == '0') {
        digitalWrite(outputDo[index], LOW);
    } else if (c == '1') {
        digitalWrite(outputDo[index], HIGH);
    }
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
    String s=cfg.host+"/do/#";
    mqtt.subscribe(s.c_str());
  }
}

void pollInputs() {// Input polling (non-blocking)
unsigned long now = millis();
int i;
double ftmp,ftmp1;
String topic,msg;

  if (now - lastInputCheck < 1000) return; //Polling every 1 sec
  lastInputCheck = now;

//Serial.println("Send data");
//Serial.flush(); //Wait for all output characters to finish

    ledToggle ^= 1;
    digitalWrite(LED_BUILTIN,ledToggle);

    for (i=0;i<cfg.mqttDi;++i) {
      topic = cfg.host+"/di/"+String(i+1);
      msg=String(digitalRead(inputDi[i]));
      mqtt.publish(topic.c_str(), msg.c_str());
    }

    for (i=0;i<cfg.mqttAi;++i) {
      topic = cfg.host+"/ad/"+String(i+1);
      msg=String(analogRead(inputAi[i]));
      mqtt.publish(topic.c_str(), msg.c_str());
    }

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

void initPicoIo() {
int i;
  for (i=0;i<sizeof(inputDi);++i) pinMode(inputDi[i], INPUT_PULLUP);
  for (i=0;i<sizeof(outputDo);++i) pinMode(outputDo[i], OUTPUT);
  for (i=0;i<sizeof(inputAi);++i) pinMode(inputAi[i], INPUT);
}

void setup() {
  String s;
  Serial.begin(115200);
  while (!Serial) delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  ledToggle=0;

  Serial.println("");
  Serial.println("Version 0.004");
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

  s="ssid: "+
  Serial.println("ssid: "+cfg.ssid);
  Serial.println("WiFi password: "+cfg.wifiPass);
  Serial.println("Host name: "+cfg.host);
  Serial.println("mqtt host address: "+cfg.mqttHost);
  Serial.println("mqtt port: "+String(cfg.mqttPort));
  Serial.println("mqtt user name: "+cfg.mqttUser);
  Serial.println("mqtt password: "+cfg.mqttPass);
  Serial.println("Number digital in: "+String(cfg.mqttDi));
  Serial.println("Number analog in: "+String(cfg.mqttAi));
  Serial.println("");

  Serial.flush(); //Wait for all output characters to finish
  Serial.println("Press any key to run setup wizard...");
  delay(3000);
  if (Serial.available()){
    runSetupWizard();
    goto loop;
  }

  initPicoIo();
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
  analogReadResolution(12);

  if (watchdog_caused_reboot()) {
      Serial.println("Rebooted by watchdog!");
  }
  watchdog_enable(8000, 1);

  WiFi.setHostname(cfg.host.c_str());
  WiFi.begin(cfg.ssid.c_str(),cfg.wifiPass.c_str());
  mqtt.setServer(cfg.mqttHost.c_str(),cfg.mqttPort);
  mqtt.setCallback(mqttCallback);
}

void loop() {// Main Loop (fully non-blocking)
  watchdog_update();
  ensureWiFi();
  ensureMQTT();

  if (mqtt.connected())  mqtt.loop();  // Non-blocking

  watchdog_update();
  pollInputs();
}
