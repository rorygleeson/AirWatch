// AirWatch V3.0
// This version of airwatch for the ESP32 uses WiFiManager 
// WiFiManager works as follows:
// 1  When the ESP starts up, it puts it in Station mode and tries to connect to an access point (to the local WiFi network) previously registered
// 2  If the ESP32 fails to connect or if no network has been registered previously, it places the ESP32 in Access Point mode (AP Mode). 
//    A web server accessible to the IP address 192.168.4.1 is started on the ESP32
// 3  Use any device with a WiFi connection with a browser (computer, smartphone, tablet) to connect to the configuration page




#include <WiFiManager.h>                        // https://github.com/tzapu/WiFiManager
#include <Ticker.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_system.h>
#include <driver/adc.h>
#include <Adafruit_BME280.h>


#define uS_TO_S_FACTOR 1000000 
#define ADC_MAX_VALUE 4095U
#define ADC_MIN_VALUE 0U
#define ADC_SAMPLES 3U
#define ADC_SAMPLE_DELAY 100U
#define TIME_TO_SLEEP  300                      //  sleep for 300 seconds (5 minutes)


int pmsSetPin = 23;                             // PMS enable pin 
int red = 18;                                   // RGB led pins 
int green = 17;
int blue = 16;





// PMS sensor variables
#define LENG 31                                 //  0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];
int PM01Value=0;                                //  define PM1.0 value of the air detector module
int PM2_5Value=0;                               //  define PM2.5 value of the air detector module
int PM10Value=0;                                //  define PM10 value of the air detector module



Ticker setBlue;                                //  declare an instance of ticker for flashing LED blue
Ticker setRed;                                 //  declare an instance of ticker for flashing LED red


int wake;                                       //  we store the wakeup reason, not used at the moment
boolean enableSleep = false;                    //  if you dont want to leave LED indicator on between readings, use sleep mode. set to true.
Adafruit_BME280 bme280;

void setup() {

    Serial.begin(9600);
    Serial.println("Setup: Set PMS SET Pin LOW and Turn RGB off");
    pinMode(pmsSetPin, OUTPUT);
    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);

    // turn off PMS
    digitalWrite(pmsSetPin, LOW);
    // turn off RGB 
    turnRGBOff();
    flash_led(blue,3);  // indicate start of cycle
    delay(2000); // wait 2 seconds
    // explicitly set mode, esp defaults to STA+AP
    WiFi.mode(WIFI_STA); 
    
    wake  = wakeup_reason1();
    Serial.println("Wakeup reason is: ");                      
    Serial.println(wake); 
    

    //WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    wm.setTimeout(300);
    // reset settings - for testing
    // wm.resetSettings();

    // set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wm.setAPCallback(configModeCallback);

    // fetches ssid and pass and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "AirWatch-AccessPoint"
    // and goes into a blocking loop awaiting configuration
    
    if (!wm.autoConnect("AirWatch-AccessPoint")) 
    {
        Serial.println("Failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(1000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("Connected to WIFI network");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
   

    //flash LED green 3 times to indicate successfull wifi connection
    // get wifi signal strenght
    long rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
    flash_led(green,3);
  
}

void loop() {
  // Should only get here after succesfully connecting to WIFI
    
        
        readSensorsAndSendEvent(); 



        if(enableSleep == true)
        {
          Serial.println("Now go to sleep for 5 mins");
          esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
          esp_deep_sleep_start();
        }
        else
        {
          // keep RGB indicator led on, so dont sleep, just wait 5 mins then restart
          Serial.println("Set LED to indicate quality, and wait 5 mins");
          delay(300000); 
          esp_restart();  
        }
    
}



//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) 
{
           Serial.println("Entered WIFI setup mode");
           Serial.println(WiFi.softAPIP());
           //if you used auto generated SSID, print it
           Serial.println(myWiFiManager->getConfigPortalSSID());
           //entered config mode, make led toggle faster. This is blocking anything else from running. Toggle red/blue every second. 
           setBlue.attach(1, setRGBProvMode, 1);
           setRed.attach(2, setRGBProvMode, 2);
}





void setRGBProvMode(int color)  // This function is specific to Ticker, which is used in Wifi setup mode , and therefore must be handled with non blocking code.
{
  
// 1 = blue
// 2 = red
// 3 = green
// 4 = magenta
// 5 = cyan

if(color == 1)
  {
  digitalWrite(red, 255);
  digitalWrite(green, 255);
  digitalWrite(blue, 0);
    
  }
else if(color == 2)
  {
  digitalWrite(red, 0);
  digitalWrite(green, 255);
  digitalWrite(blue, 255);
  }
 
}




void flash_led(int color, int numFlashes)
{
// ensure all are off
  int r = 255;
  int g = 255;
  int b = 255;



 
  if(color == 18)
    r = 0;
  else if (color == 17)
    g = 0;
  else if (color == 16)
    b = 0;


  for (int i = 0; i < numFlashes; i++) 
  {
    RGB_color(r, g, b);     // turn on
    delay(300); 
    RGB_color(255, 255, 255);     // turn ff
    delay(300); 
  }

  delay(2000);  
}






void turnRGBOff()
{
    digitalWrite(red, 255);
    digitalWrite(green, 255);
    digitalWrite(blue, 255);
  
}

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  digitalWrite(red, red_light_value);
  digitalWrite(green, green_light_value);
  digitalWrite(blue, blue_light_value);
}






// PMS5003 functions

char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;
  //Serial.println("  check values .."); 
  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}




int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}






//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
}




//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}




// Returns reason why chip woke up

int wakeup_reason1() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  return wakeup_reason;
}




// Returns Mac Id of chip

String macToStr(const uint8_t* mac)
{
String result;
for (int i = 0; i < 6; ++i) {
result += String(mac[i], 16);
if (i < 5)
result += ':';
}
return result;
}





// MICS 6814 functions




bool mics6814_init(void) {
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
  return true;
}





 
bool mics6814_read(uint16_t* no2, uint16_t* nh3, uint16_t* co) {
  bool result = true;
 
  if ((NULL == no2) || (NULL == nh3) || (NULL == co)) {
    result = false;
  }
 
  uint16_t tempNo2 = 0U;
  uint16_t tempNh3 = 0U;
  uint16_t tempCo = 0U;
  uint8_t count = 0U;
  while ((true == result) && (count < ADC_SAMPLES)) {
    if (count > 0U) {
      delay(ADC_SAMPLE_DELAY);
    }
 
    int temp = adc1_get_raw(ADC1_CHANNEL_4);
    if ((temp >= ADC_MIN_VALUE) && (temp <= ADC_MAX_VALUE)) {
      tempNo2 += (uint16_t)temp;
    } else {
      result = false;
    }
 
    if (true == result) {
      temp = adc1_get_raw(ADC1_CHANNEL_6);
      if ((temp >= ADC_MIN_VALUE) && (temp <= ADC_MAX_VALUE)) {
        tempNh3 += (uint16_t)temp;
      } else {
        result = false;
      }
    }
 
    if (true == result) {
      temp = adc1_get_raw(ADC1_CHANNEL_5);
      if ((temp >= ADC_MIN_VALUE) && (temp <= ADC_MAX_VALUE)) {
        tempCo += (uint16_t)temp;
        count++;
      } else {
        result = false;
      }
    }
  }
 
  if (true == result) {
    *no2 = ADC_MAX_VALUE - (tempNo2 / ADC_SAMPLES);
    *nh3 = ADC_MAX_VALUE - (tempNh3 / ADC_SAMPLES);
    *co = ADC_MAX_VALUE - (tempCo / ADC_SAMPLES);
  }
 
  return result;
}








void readSensorsAndSendEvent()
{
      Serial.println(F("In sensor read function, start reading sensors. Set color to Magenta."));
      RGB_color(0, 255, 0);


      Serial.println(F("Start the BME280 ...."));
      bme280.begin(0x76);
      Serial.println("Read BME280 values...");
      float temperature = bme280.readTemperature();
      float pressure = bme280.readPressure() / 100.0F;
      float humidity = bme280.readHumidity();
      Serial.println("Temp is : ");
      Serial.println(temperature);
      Serial.println("Pressure is : ");
      Serial.println(pressure);        
      Serial.println("Humidity is : ");
      Serial.println(humidity);
    


      Serial.println("Set PMS SET Pin HIGH and wait 20 seconds ......");
      digitalWrite(pmsSetPin, HIGH);
      delay(20000);



      //start to read the PMS5003 when detect 0x42 on serial 
      
      if(Serial.find(0x42))
      {    
        Serial.readBytes(buf,LENG);
        //Serial.println("  detected 42 .."); 
        if(buf[0] == 0x4d){
          if(checkValue(buf,LENG)){
            PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
            PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
            PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
          }           
        } 
      }


      Serial.print("PM1.0: ");  
      Serial.print(PM01Value);
      Serial.println("  ug/m3");            
      Serial.print("PM2.5: ");  
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");     
      Serial.print("PM1 0: ");  
      Serial.print(PM10Value);
      Serial.println("  ug/m3");   
      Serial.println();
        
      
     // now read the gas sensor
     
     Serial.println("Setup gas sensor MICS6814");
     if ( !mics6814_init() ) 
     {
          Serial.println("Waiting to Setup gas sensor MICS6814");
          while(1);
     }

     uint16_t no2 = 0U;
     uint16_t nh3 = 0U;
     uint16_t co = 0U;
     mics6814_read(&no2, &nh3, &co);
     Serial.println("Have read MICS6814 ..");
     Serial.println("Co ..");
     Serial.println(co);
     Serial.println("No2 ..");
     Serial.println(no2);
     Serial.println("Nh3 ..");
     Serial.println(nh3);


    

     String clientMac = "";
     unsigned char mac[6];
     WiFi.macAddress(mac);
     clientMac += macToStr(mac);
     Serial.println("Mac ID is ..");
     Serial.println(clientMac);


     Serial.println("Send event to platform.. ");
     HTTPClient http;
     Serial.print("[HTTP] begin...\n");

     // url format http://aServer.com/aScript.php?pms01=23&pms25=32&pm10=12&thingName=wemos04&serial=a238b327f8f334ed&temeperature=23.4&pressure=1036&humidity=45.2&no2=234&nh3=286&co=23
     
     String url = "http://airwatch.io/update/updateWIFI.php?pms01=";         // point to airwatch. Or update to point to your own server 

     url += PM01Value;
     url += "&pms25=";
     url += PM2_5Value;
     url += "&pms10=";
     url += PM10Value;
     url += "&thingName=wemos0017";
     url += "&serial=";
     url += clientMac;
     url += "&temperature=";
     url += temperature;
     url += "&pressure=";
     url += pressure;
     url += "&humidity=";
     url += humidity;
     url += "&no2=";
     url += no2;
     url += "&nh3=";
     url += nh3;
     url += "&co=";
     url += co;

 
     Serial.print("URL is ...\n");
     Serial.print(url);
     http.begin(url); //HTTP


     Serial.print("[HTTP] GET...\n");
     
     // start connection and send HTTP header
     int httpCode = http.GET();


      if(httpCode > 0) 
      {
                  // HTTP header has been send and Server response header has been handled
                  Serial.printf("[HTTP] GET... return code: %d\n", httpCode);

                  // file found at server
                  if(httpCode == HTTP_CODE_OK) 
                  {
                      String payload = http.getString();
                    
                      http.end();
                      flash_led(green,5);  // indicates the send was successfull
                  }
      } 
      else // connected to wifi but http get failed
      {
                Serial.printf("[HTTP] GET... failed, error code: %s\n", http.errorToString(httpCode).c_str());
                Serial.print("WIFI  connected, but update  failed.");
                http.end();
                flash_led(red,5);  // indicates the send failed
                
       }

  // set the led based on last reading of air quality. 

    if(PM2_5Value >= 0 &&  PM2_5Value <= 26.3)
    {
        // set RGB green, very good or good quality
        RGB_color(255, 0, 255); 
    }
    else if(PM2_5Value >= 26.4 &&  PM2_5Value <= 39.9)
    {
        // set RGB blue , fair quality
        RGB_color(255, 255, 0);  
    }
    else if(PM2_5Value >= 40.0 )
    { 
       
        // set RGB red, poor quality or very poor quality
         RGB_color(0, 255, 255);
    }
    

        Serial.println("Set PMS SET Pin LOW to turn off fan, it only has 8000 hours of operation life span ......");
        digitalWrite(pmsSetPin, LOW);
        delay(2000);


    
  
}
