/*
If host=hpghmqtt1

Publish:
hpghmqtt1/do0 Pico
hpghmqtt1/do1 Pico
hpghmqtt1/do2 Pico
hpghmqtt1/do3 Pico
hpghmqtt1/ad1 Pico
hpghmqtt1/ad2 Pico
hpghmqtt1/ad3 Pico
hpghmqtt1/t bme280 or hdc302x
hpghmqtt1/rh bme280 or hdc302x
hpghmqtt1/bp bme280
hpghmqtt1/ad4 ad1015
hpghmqtt1/ad5 ad1015
hpghmqtt1/ad6 ad1015
hpghmqtt1/ad7 ad1015

and subscribe to:
hpghmqtt1/di1 Pico
hpghmqtt1/di2 Pico
hpghmqtt1/di3 Pico
hpghmqtt1/di4 Pico

Pico pin assignment:
1 GP0 UART0 TX
2 GP1 UART0 RX
3 GND
4 GP2 RS485 enable
5 GP3 Spare
6 GP4 I2C0 SDA (For 12 bit A-D and BME280)
7 GP5 I2C0 SCL (For 12 bit A-D and BME280)
8 GND
9 GP6 D1
10 GP7 D2
11 GP8 D3
12 GP9 D4
13 GND
14 GP10 D5
15 GP11 D6
16 GP12 D7
17 GP13 D8
18 GND
19 GP14 D9
20 GP15 D10
21 GP16 D11
22 GP17 D12
23 GND
24 GP18 D13
25 GP19 D14
26 GP20 D15
27 GP21 D16
28 GND
29 GP22 D17
30 RUN
31 GP26 ADC0 AI1 D18
32 GP27 ADC1 AI2 D19
33 ANALOG GND
34 GP28 ADC2 AI3 D20
35 ADC_VREF
36 3.3V out
37 3.3V enable
38 GND
39 VSYS
40 VBUS
*/

#include <Adafruit_ADS1X15.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HDC302x.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "stdint.h"
#include "hardware/watchdog.h"
#include "LittleFS.h"
#include "RP2040_PWM.h"

#define UARTTX 0
#define UARTRX 1
#define RTS 2
#define unused 3
#define SDA 4
#define SCL 5
#define D1 6
#define D2 7
#define D3 8
#define D4 9
#define D5 10
#define D6 11
#define D7 12
#define D8 13
#define D9 14
#define D10 15
#define D11 16
#define D12 17
#define D13 18
#define D14 19
#define D15 20
#define D16 21
#define D17 22
#define D18 26
#define D19 27
#define D20 28
#define PIN_LEN 4 // Number of GPIO to control 4 input, 4 output
#define DEBOUNCE 5 //Number of 10 mS debounce counts

Adafruit_ADS1015 ads; //Setup A-D converter on I2C
Adafruit_BME280 bme; //Setup temp, humidity, bararometer on I2C
Adafruit_HDC302x hdc=Adafruit_HDC302x();

RP2040_PWM* PWM_Instance[PIN_LEN];

LittleFSConfig fscfg;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastMQTTReconnect = 0;// Timing
unsigned long lastWiFiReconnect = 0;
unsigned long lastInputCheck    = 0;
unsigned long watchdogActivity;
bool ledToggle,ad1015,bme280,hdc302x;
//String ssid,wifiPass,host,mqttServer,mqttPort,mqttUser,mqttPass; //Remember String is a special arduino class

/*
uint8_t  event_counter_control[PIN_LEN*2]; //Event counter current state and # in new state
uint32_t
  freqC[PIN_LEN], //Frequency measurement counter
  freqT[PIN_LEN], //Timer for freq measure
  freqL[PIN_LEN]; //Last freq reading
*/

