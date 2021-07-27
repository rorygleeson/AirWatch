AirWatch WiFi
==============


Hardware Used:

ESP32 (Wemos Lolin32 dev board)

BME280P sensor (Temp, Pressure, Humidity)

PMS5003 Particle matter sensor, PM 1.0, 2.5 and 10.0

MICS 6814 gas sensor. NO2, NH3, CO

Power via 5v micro usb 

The AirWatch device needs to be supplied the credentials for the WIFI network it will use. 
This code uses https://github.com/tzapu/WiFiManager to manage this process. 


Hardware
=================

![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/WIFI.png)


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/1.jpg)

![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/4.jpg)


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/7.jpg)

![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/8.jpg)

WiFi Setup
==========

1 Plug in the AirWatch device. The blue LED will flash 3 times to indicate start of cycle.  

2 If you have already provided the WiFi details to the device, and it can connect to the WiFi, the green led will flash 3 times. Device willl try for 20 seconds to connect to WiFi. 

3 If the device fails to connect to your WiFi, for example If this is the first time setting the device up on the WiFi network, the device will go into “WiFi Setup Mode”. The LED toggles between blue and red. 

4 On your mobile phone, scan available WiFi networks. You will see a network called "AirWatch-AccessPoint". Connect to this network. Your phone will tell you that the network cannot connect to the internet, this is fine, proceed and connect phone to WiFi network called "AirWatch-AccessPoint". 

5 Open a browser on your phone, and enter the following: 192.168.4.1

7 The WiFi manager page should open. Click on the "Configure Wifi". 

8 Select your WiFi network, and enter the WiFi network password. Device will restart. 

9 The AirWatch device will flash the green LED 3 times to indicate the WiFi setup was successful.

OR

9 The AirWatch device will fail to connect to WiFi and repeat the above steps. 




AirWatch Operation Cycle
========================


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/AirWatchOperation.png)








LED status 
==========


![alt text](https://github.com/rorygleeson/AirWatch/blob/master/Devices/WiFi/AirWatchLED.png)





