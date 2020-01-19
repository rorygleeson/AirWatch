AirWatch WiFi
==============


Hardware Used:

ESP32 (Wemos Lolin32 dev board)

BME280P sensor (Temp, Pressure, Humidity)

PMS5003 Particle matter sensor, PM 1.0, 2.5 and 10.0

MICS 6814 gas sensor. NO2, NH3, CO

Power via 5v micro usb 

The AirWatch device needs to be supplied the credentials for the WIFI network it will use. 
This code uses the ESP32 "SmartConfig" feature. (more details of smartconfig here https://www.switchdoc.com/2018/06/tutorial-esp32-bc24-provisioning-for-wifi/ )



Hardware
=================

![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/WIFI.png)




AirWatch WiFi Hardware Bill of Materials
----------------------------------------

WeMos D1 LOLIN32 ESP32 Wifi Bluetooth development board ~ price: $5 USD

BME280P ~ price: $2 USD


PMS 5003 ~ price: $13 USD

MICS 6814 Gas Sensor Measures NO2, NH3, CO ~ price: $23 USD
 

5V DC 1A POWER ADAPTER WITH MICRO USB PLUG ~ price: $5 USD


Enclosure ~ price: $3 USD

RGB LED ~ price: $3 USD



1 X 3.3K Resistor
3 X  10K Resistor

Connector wires
PCB solder board

Total cost of hardware (approx)  =  ~ $54  USD




WiFi Setup
==========

1 Ensure your mobile phone is connected to the same WiFi network that the AirWatch device will use.

2 Download the app from Android play store. 	

  App name: EspTouch: SmartConfig for ESP8622, ESP32
  


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/esptouchapp.PNG)




3 Plug in the AirWatch device. The blue LED will flash 3 times. 

4 Wait 20 seconds, the Red LED will flash 3 times (indicating it cannot connect to any WiFi networks). This is normal for first time operation on a new WiFi network. 


5 Wait until LED starts to toggle between blue and red. The AirWatch device is now in “WiFi Setup Mode”.

6 Open the ESPTouch SmartConfig app on your mobile device, enter the WiFi SSID and Password. 

7 Hit “Confirm”to send the details to the device. 

8 The EspTouch SmartConfig app will show a success message if completed successfully. Device will restart. 

*9 The AirWatch device will flash the green LED 10 times to indicate the WiFi setup was successful. It will then restart. 

OR

9 The AirWatch device will flash red 10 times if after providing the WiFi details, AirWatch device was still unable to connect to the network. Device will restart.

10 Login to airwatch app to view your data. 




AirWatch Operation Cycle
========================


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/AirWatchOperation.png)








LED status 
==========


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/AirWatchLED.png)





