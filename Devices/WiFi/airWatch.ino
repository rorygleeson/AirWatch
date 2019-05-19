#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_system.h>

#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;


#define USE_SERIAL Serial
#define uS_TO_S_FACTOR 1000000 

int attemptsCount = 0;
int wake;
boolean interruptReceived = false;
bool smartconfigDone = false;

int red = 0;
int green = 2;
int blue = 15;
 

// #define PMS_SET_PIN 26U
// #define PMS_RST_PIN 25U

int pmsSetPin = 4;


#define TIME_TO_SLEEP  300 
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module


hw_timer_t * timer = NULL;  // create a hardware timer 

void IRAM_ATTR onTimer()
{
  Serial.println("interrupt recevied......");
  interruptReceived = true;
  Serial.println("interruptRecevied variable set to ......");
  Serial.println(interruptReceived);
}



void setup() 
{
  // put your setup code here, to run once:
  
    Serial.begin(9600);   //use serial0
    Serial.setTimeout(1500); 
    Serial.println("Starting now");
    wake  = wakeup_reason1();
    Serial.println("WAKE IS..\n");                      /* The wake reason allows us to know what woke the esp32 up, ext int or other */
    Serial.println(wake);
 
    // initialize the digital pin as an output.
    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);
    pinMode(pmsSetPin, OUTPUT);

    digitalWrite(red, HIGH);
    digitalWrite(green, HIGH);
    digitalWrite(blue, HIGH);

    
    
    digitalWrite(red, LOW); // Red LED on
    
    Serial.println(F("Start the BME280 ...."));
    bme280.begin();
    
    int counter = 0;
    
    Serial.println("In setup. variable interruptReceoved is..\n"); 
    Serial.println(interruptReceived);


    
    WiFi.mode(WIFI_STA);
    WiFi.begin();

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(2000);
        Serial.print(".");
        counter += 1;
        Serial.println(counter); 
        
              if(counter == 10)
              {
              
                Serial.println("Counter is 10, failed to connect to network..."); 
                Serial.println("exit while loop");
                break;
              }

    }





    if((WiFi.status() == WL_CONNECTED) )
    {
      digitalWrite(red, HIGH); // Red LED off
      digitalWrite(green, LOW); // Green LED on

      
      Serial.println("WiFi Connected.");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());

        

        Serial.println("Read BME280 values in 2 seconds.");
        delay(2000);
        float temperature = bme280.readTemperature();
        float pressure = bme280.readPressure() / 100.0F;
        float humidity = bme280.readHumidity();
        delay(2000);
        Serial.println("Temp is : ");
        Serial.println(temperature);
        Serial.println("Pressure is : ");
        Serial.println(pressure);        
        Serial.println("Humidity is : ");
        Serial.println(humidity);
        
       
                 
        delay(1000);




      Serial.println("Set PMS SET Pin HIGH and wait 20 seconds ......");
      digitalWrite(pmsSetPin, HIGH);
      delay(20000);
          
      if(Serial.find(0x42))
      {    //start to read when detect 0x42
        Serial.readBytes(buf,LENG);

        if(buf[0] == 0x4d){
          if(checkValue(buf,LENG)){
            PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
            PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
            PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
          }           
        } 
      }


    static unsigned long OledTimer=millis();
    
        if (millis() - OledTimer >=1000) 
        {
          OledTimer=millis(); 
      
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
        }




        String clientMac = "";
        unsigned char mac[6];
        WiFi.macAddress(mac);
        clientMac += macToStr(mac);
        Serial.println("My Mac ID is ..");
        Serial.println(clientMac);


        Serial.println("Send event to platform");
        HTTPClient http;
        USE_SERIAL.print("[HTTP] begin...\n");

        
        String url = "http://<your Server>.com/yourscript.php?pms01=";

        url += PM01Value;
        url += "&pms25=";
        url += PM2_5Value;
        url += "&pms10=";
        url += PM10Value;
        url += "&thingName=wemos04";
        url += "&serial=";
        url += clientMac;

        url += "&temperature=";
        url += temperature;
        url += "&pressure=";
        url += pressure;
        url += "&humidity=";
        url += humidity;
        
        USE_SERIAL.print("URL is ...\n");
        USE_SERIAL.print(url);
        
        http.begin(url); //HTTP


        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();


         if(httpCode > 0) 
             {
                  // HTTP header has been send and Server response header has been handled
                  USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

                  // file found at server
                  if(httpCode == HTTP_CODE_OK) 
                  {
                      String payload = http.getString();
                      USE_SERIAL.println("http return code is");
                      USE_SERIAL.println(payload);
                  }
             } 
        else // connected to wifi but http get failed
             {
                USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
                USE_SERIAL.print("WIFI  connected, but update  failed.");
                http.end();
                
             }


      http.end();

      Serial.println("Put the PMS5003 to sleep");


        Serial.println("Set PMS SET Pin LOW and wait 10 seconds ......");
        digitalWrite(pmsSetPin, LOW);
        delay(10000);

      Serial.println("Now go to sleep");
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);esp_deep_sleep_start();



    }
    else
    {
          
              Serial.println("Unable to connect to a WIFI..");
              


              
               /* Set up 5 minute timer. Use 1st timer of 4 */
              /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
              timer = timerBegin(0, 80, true);
              /* Attach onTimer function to our timer */
              timerAttachInterrupt(timer, &onTimer, true);
              /* Set alarm to call onTimer function every second 1 tick is 1us
              => 1 second is 1000000us */
              /* Repeat the alarm (third parameter) */
              timerAlarmWrite(timer, 300000000, true);
              /* Start an alarm */
              timerAlarmEnable(timer);
              Serial.println("Started timer");
              Serial.println("We have setup an interrupt to happen in 5 minutes. In the meantime go into smartconfig mode..");
              Serial.println("Start SmartConfig.");



              digitalWrite(red, HIGH); // Red LED off

              
              digitalWrite(blue, LOW); // Blue LED on



              
              WiFi.beginSmartConfig();
              /* Wait for SmartConfig packet from mobile */
              Serial.println("Waiting for SmartConfig.");
              while (!WiFi.smartConfigDone()) 
              {
                  delay(500);
                  Serial.print(".");
                  if(interruptReceived == true)
                  {
                    Serial.print("Back to while loop after interrupt occured, exit loop");
                    break;
                  }
                  else
                  {
                    Serial.print("*");
                  }
                  
              }

              // we have exited the above while loop by either 1) smartConfig done or 2) 5 minites has passed and an interrupt has happened 
                  
                  if(WiFi.smartConfigDone())
                  {
                    Serial.println("");
                    Serial.println("SmartConfig done.");


                    digitalWrite(blue, HIGH); // Blue LED off

                    flash_led(green,3);
                    delay(2000);
                    /* Wait for WiFi to connect to AP */
                    Serial.println("Waiting for WiFi");
                    while (WiFi.status() != WL_CONNECTED) {
                    delay(500);
                    Serial.print("!");
                    }
                    Serial.println("WiFi Connected.");
                    Serial.print("IP Address: ");
                    Serial.println(WiFi.localIP());
                    flash_led(green,5);
                    Serial.print("RESTART ...................should then find wifi ");
                    esp_restart(); 
                    
                  }
                  else
                  {
                    Serial.println("SmartConfig NOT done, here due to interrupt.");
                    flash_led(red,3);
                  }
              
              
              
    }


  
}





void loop()
{

  if(interruptReceived == true)
  {
    Serial.println("interruptRecevied variable set to ......");
    Serial.println(interruptReceived);
    Serial.println("Disable Timer......");
    timerAlarmDisable(timer);
    Serial.println("Issue a restart in 5 seconds......");
    delay(2000);
    esp_restart(); 
  }
  else
  {
    Serial.println("X=");
    delay(2000);
  }
  
}















void flash_led(int color, int numFlashes)
{
// ensure all are off

digitalWrite(red, HIGH);
digitalWrite(green, HIGH);
digitalWrite(blue, HIGH);

  for (int i = 0; i <= numFlashes; i++) 
  {

    digitalWrite(color, LOW);     // turn on
    delay(1000); 
    digitalWrite(color, HIGH);     // turn off
    delay(1000); 
  }

  
}





char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

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



int wakeup_reason1() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  return wakeup_reason;
}





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
