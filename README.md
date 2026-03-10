This program works with the Raspberry pi pico 2W and would probably work on the pico W but has not been tested. It is created using the arduino Earle Philhower system. Select board Raspberry pi pico 2W, Flash file system 4MB and Adafruit tiny USB.

If host=hpghmqtt1

Publish topics:

hpghmqtt1/di/1 (Pico 13)

hpghmqtt1/di/2 (Pico 14)

hpghmqtt1/di/3 (Pico 15)

hpghmqtt1/di/4 (Pico 16)

:

hpghmqtt1/di/10 (Pico 22)


hpghmqtt1/ad/1 (Pico 26)

hpghmqtt1/ad/2 (Pico 27)

hpghmqtt1/ad/3 (Pico 28)

hpghmqtt1/t bme280 or hdc302x temperature degrees F

hpghmqtt1/rh bme280 or hdc302x Relative humidity in %

hpghmqtt1/bp bme280 Barometric reading in inches HG

hpghmqtt1/ad/4 ad1015

hpghmqtt1/ad/5 ad1015

hpghmqtt1/ad/6 ad1015

hpghmqtt1/ad/7 ad1015

and for output pins subscribe topic:

hpghmqtt1/do/1 (Pico 0)

hpghmqtt1/do/2 (Pico 1)

hpghmqtt1/do/3 (Pico 2)

:
hpghmqtt1/do/12 (Pico 12)

(These are on pico IO 0,1,2,3,5,6,7,8,9,10,11,12) 4 and 5 are used for I2C.

Messages (payloads) are 0 for pin low and 1 for pin high.
A-D inputs are (0-4095).
Floating point for degrees F, % RH and inches HG for bme280 or hdc302x. You only use bme280 or hdc302x, never both.
Using the I2C sensors is optional. At startup the system automatically detects which sensor modules are attached if any.

At startup you enter the following on the USB port to configure operation:

WiFi ssid:

WiFi password:

Host name: (Ex hpghmqtt1 you can make this any legal host name and this is the first entry in your topic identifier)

mqtt host: address: (Ex. 192.168.1.164)

mqtt port: (Ususally 1883)

mqtt user name: (Leave blank if none)

mqtt password: (Leave blank if none)

Number digital in: (0-10) Set to 0 if you don't want the values of any digital inputs published.

Number analog in: (0-3 Pico pins 26,27,28) Set to 0 if you don't want any pico analog inputs published.
