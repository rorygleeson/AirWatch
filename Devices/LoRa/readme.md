AirWatch LoRa
=============

This project aims to build a cheap affordable air quality monitoring device. 
2 hardware version are provided, one for powering via 5v usb cable, the other for a solar powered version. 

Version 1 
--------------------

It uses the Adafruit Feather M0 RFM9X dev board. (ATSAMD21G18 ARM Cortex M0 processor and SX127x LoRa® based module)

It uses the following sensors:


BME280 - Temperature, humidity and altitude (pressure). Humidty effects the particle matter count of the PMS5003, hence why included.

PMS 5003 - Particle Matter. PM1.0, PM2.5 and PM10.0 concentration in both standard & enviromental units

MICS-6814 - C0, N02 and NH3 (Carbon Monoxide, Nitrogen Dioxide and Ammonia)


Particular attention is getting this device to work well in sleep mode, and last for as long as possible on batteries/solar. 
However, bear in mind the particle matter sensor (PMS5003) consists of a small fan, that must be run to suck in the air. This must be run for about 30 seconds, before taking a reading. 

The PMS5003 does have an enable pin (SET) which can be pulled low to turn the sensor (and fan) off. If we do not use this, apart fom using a lot of power, the PMS5003 will only last for 8000 hours of operation. So it is very important that our Feather M0, is turning the PMS5003 on and off. 


To do: Yet to focus on sleep mode for bme280, the lora radio chip itself, the mics-6814 ..

The MICS-6814 has a warm up period of 20 mins before readings are accurate. 

To do: Yet to focus on sleep mode for bme280, the lora radio chip itself, the mics-6814 ..



Non Solar Powered (power via mains)
-------------------------------------

![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/LoRa/LoRa-NonSolar.jpg)



 Solar Powered
-------------------------------------


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/LoRa/LoRa-Solar.jpg)




AirWatch LoRa Hardware Bill of Materials
----------------------------------------

Adafruit Feather M0 RFMx 
https://www.adafruit.com/product/3178 average price: $34.95 USD


BME280P
https://www.aliexpress.com/af/bme280-sensor.html?SearchText=bme280+sensor
average price: US $2.17  USD


PMS 5003
https://www.aliexpress.com/af/PMS5003.html?SearchText=PMS5003 average price: US $13.30 USD


Mics-6814 Chip 
average price: $30 USD on ebay

 

5V DC 1A POWER ADAPTER WITH MICRO USB PLUG

https://www.auselectronicsdirect.com.au/5v-dc-1a-power-adapter-with-micro-usb-plug?gclid=EAIaIQobChMI6qyM-dmQ4gIVQo-PCh2wrABwEAQYBiABEgLqBvD_BwE average price: $7.95 AUD /  5.56 USD


Enclosure

https://www.jaycar.com.au/jiffy-box-grey-130-x-68-x-44mm/p/HB6023 average price: $3.95 AUD  / 2.76 USD




1 X 3.3K Resistor
Connector wires
PCB solder board

Total price Non Solar ~  $88  USD



Extra components required for solar:

Pololu Step-up Voltage Ref
https://www.pololu.com/product/2115    average price $5.70 AUD / $3.95 USD 


Solar Lipo Charger (3.7V) 
https://www.dfrobot.com/product-1139.html  average price $7 AUD / $4.90 USD 


2 Solar Panel (9v 220mA)
https://www.dfrobot.com/product-1005.html   average price $11.25 AUD / $7.90 USD 


Total price Non Solar ~  $105  USD


Libraries:
==================


Adafruit BME280 Library
https://github.com/adafruit/Adafruit_BME280_Library

Adafruit Sleepy dog
https://github.com/adafruit/Adafruit_SleepyDog

MCCI LoRaWAN LMIC Library
https://github.com/mcci-catena/arduino-lmic


Adafruit unified sensor
https://github.com/adafruit/Adafruit_Sensor


12C-Senor-lib
https://github.com/orgua/iLib































