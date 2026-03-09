void flush() { //Read extraneous characters
  while (Serial.available()) Serial.read();
}

struct Config {
  String ssid="TP-Link_F7C4";
  String wifiPass="98026449";
  String host="hpghmqtt1";
  String mqttHost="192.168.1.164";
  int    mqttPort=1883;
  String mqttUser="";
  String mqttPass="";
};

Config cfg;

bool loadConfig() {
//  if (!LittleFS.begin()) return false;

  if (!LittleFS.exists("/config.txt")) return false;

  File f = LittleFS.open("/config.txt", "r");
  if (!f) return false;

  cfg.ssid     = f.readStringUntil('\n'); cfg.ssid.trim();
  cfg.wifiPass = f.readStringUntil('\n'); cfg.wifiPass.trim();
  cfg.host = f.readStringUntil('\n'); cfg.host.trim();
  cfg.mqttHost = f.readStringUntil('\n'); cfg.mqttHost.trim();
  cfg.mqttPort = f.readStringUntil('\n').toInt();
  cfg.mqttUser = f.readStringUntil('\n'); cfg.mqttUser.trim();
  cfg.mqttPass = f.readStringUntil('\n'); cfg.mqttPass.trim();

  f.close();
  return true;
}

void saveConfig() {
  File f = LittleFS.open("/config.txt", "w");
  f.println(cfg.ssid);
  f.println(cfg.wifiPass);
  f.println(cfg.host);
  f.println(cfg.mqttHost);
  f.println(cfg.mqttPort);
  f.println(cfg.mqttUser);
  f.println(cfg.mqttPass);
  f.close();
}

String readLine() {
  String line = "";
  
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();

      // Newline ends input
      if (c == '\n' || c == '\r') {
        Serial.println();
        return line;
      }

      // Handle backspace (ASCII 8 or 127)
      if (c == 8 || c == 127) {
        if (line.length() > 0) {
          line.remove(line.length() - 1);
          Serial.print("\b \b");   // erase last char on terminal
        }
        continue;
      }

      // Printable characters only
      if (c >= 32 && c <= 126) {
        line += c;
        Serial.print(c);
      }
    }
  }
}

void runSetupWizard() {
  flush();
  Serial.println("\n=== MQTT Pico 2W Setup Wizard ===");

  Serial.print("Enter WiFi SSID: ");
  cfg.ssid = readLine();

  Serial.print("Enter WiFi Password: ");
  cfg.wifiPass = readLine();

  Serial.print("Enter Host name: ");
  cfg.host = readLine();

  Serial.print("Enter MQTT Host (IP or DNS): ");
  cfg.mqttHost = readLine();

  Serial.print("Enter MQTT Port: ");
  cfg.mqttPort = readLine().toInt();

  Serial.print("Enter MQTT Username: ");
  cfg.mqttUser = readLine();

  Serial.print("Enter MQTT Password: ");
  cfg.mqttPass = readLine();

  saveConfig();

  Serial.println("\nConfiguration saved. Rebooting...");
  delay(1000);
  rp2040.reboot();
}
