"# FeatherM0-LoRa-TTN-LMIC-Sleep-PMS5003"

This project aims to build a cheap affordable air quality monitoring device. 

Version 1 (complete)

It uses the Adafruit Feather M0 RFM9X dev board. 

It uses the following sensors:

PMS5003 - Particle Matter. PM1.0, PM2.5 and PM10.0 concentration in both standard & enviromental units

BME280 - Temperature, humidity and altitude (pressure). Humidty effects the particle matter count of the PMS5003, hence why included.

Particular attention is getting this device to work well in sleep mode, and last for as long as possible on batteries/solar. 
However, bear in mind the particle matter sensor (PMS5003) consists of a small fan, that must be run to suck in the air. This must be run for about 30 seconds, before taking a reading. 

The PMS5003 does have an enable pin (SET) which can be pulled low to turn the sensor (and fan) off. If we do not use this, apart fom using a lot of power, the PMS5003 will only last for 8000 hours of operation. So it is very important that our Feather M0, is turning the PMS5003 on and off. 

Same with other sensors, and also important to ensure the LoRa chip itself is put to sleep mode when we enter sleep mode with the Feather M0.


Version 2 (coming soon)
MICS-6814 - C0, N02 and NH3 (Carbon Monoxide, Nitrogen Dioxide and Ammonia)
Solar Panel support



![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/LoRa/LORA1-Non-Solar.png)








