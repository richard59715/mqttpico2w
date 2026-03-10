/*
If host=hpghmqtt1

Publish topic:
hpghmqtt1/di/1 Pico
hpghmqtt1/di/2 Pico
hpghmqtt1/di/3 Pico
hpghmqtt1/di/4 Pico
:
hpghmqtt1/di/10 Pico

hpghmqtt1/ad/1 Pico
hpghmqtt1/ad/2 Pico
hpghmqtt1/ad/3 Pico

hpghmqtt1/t bme280 or hdc302x
hpghmqtt1/rh bme280 or hdc302x
hpghmqtt1/bp bme280

hpghmqtt1/ad/4 ad1015
hpghmqtt1/ad/5 ad1015
hpghmqtt1/ad/6 ad1015
hpghmqtt1/ad/7 ad1015

and for output pins subscribe topic:
hpghmqtt1/do/1 Pico
hpghmqtt1/do/2 Pico
hpghmqtt1/do/3 Pico
:
hpghmqtt1/do/12 Pico

Pico pin assignment:
1 GP0
2 GP1
3 GND
4 GP2
5 GP3
6 GP4 I2C0 SDA (For 12 bit A-D and BME280)
7 GP5 I2C0 SCL (For 12 bit A-D and BME280)
8 GND
9 GP6
10 GP7
11 GP8
12 GP9
13 GND
14 GP10
15 GP11
16 GP12
17 GP13
18 GND
19 GP14
20 GP15
21 GP16
22 GP17
23 GND
24 GP18
25 GP19
26 GP20
27 GP21
28 GND
29 GP22
30 RUN
31 GP26 ADC0
32 GP27 ADC1
33 ANALOG GND
34 GP28 ADC2
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

uint8_t inputDi[]={13,14,15,16,17,18,19,20,21,22};
uint8_t inputAi[]={26,27,28};
uint8_t outputDo[]={0,1,2,3,5,6,7,8,9,10,11,12};

Adafruit_ADS1015 ads; //Setup A-D converter on I2C
Adafruit_BME280 bme; //Setup temp, humidity, bararometer on I2C
Adafruit_HDC302x hdc=Adafruit_HDC302x();


LittleFSConfig fscfg;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastMQTTReconnect = 0;
unsigned long lastWiFiReconnect = 0;
unsigned long lastInputCheck    = 0;
bool ledToggle,ad1015,bme280,hdc302x;
