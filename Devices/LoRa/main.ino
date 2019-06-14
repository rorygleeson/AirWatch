/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/



 /****************************************************************************
  * Rory Gleeson
  * AirWatch modifications
  * 
  * Support for BME280 sensor added (Temp, Humidity, Pressure)
  * Support for PMS5003 sensor (PM1.0, PM 2.5, PM 10.0)
  * Added sleep mode for low power consumption. This involves using watchdog and sleep mode
  * Tested on AS923 TTN
  */

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Adafruit_SleepyDog.h>
#include <Arduino.h>

#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;
#define VBATPIN A7

#define NO2 A0
#define NH3 A1
#define CO A2

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]= { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]= { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };

void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}




static uint8_t mydata[] = "20 20 20 20 20 20 20 20 20 20 20 20 20 20 20";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 300;

int sleepcycles = 20;     // for the feather M0 1 sleep cycle is about 15 seconds. So for example for 1 minute sleep, sleep cycles = 4. 5 minute = 20 etc. etc. 
bool sleeping = false;

// PMS Sensor
int pmsSetPin = 12;


const int PM_MESSAGE_LENGTH = 30;
unsigned char pmMessageBuffer[PM_MESSAGE_LENGTH];

//const char PM_MESSAGE_START_TOKEN[2] = {(char) 0x42, (char) 0x4d};      // Rory: commented this out, it was getting corrupted when do_send(&sendjob) is in the code below, Suspect compiler issuer. Moved to function its used in, it is a constant anyway.

const int PM_HEADER_SUM = 0x42+0x4d;
const int PM_1_0_INDEX = 8;  
const int PM_2_5_INDEX = 10;
const int PM_10_INDEX = 12;
int pm1;
int pm25;
int pm10;

float my_no2;
float my_nh3;
float my_co;


int int_no2;
int int_nh3;
int int_co;

byte highbyteno2;
byte lowbyteno2;

byte highbytenh3;
byte lowbytenh3;

byte highbyteco;
byte lowbyteco;


float finalTemp;
int  finalTemp100;
byte highbytetemp;
byte lowbytetemp;


float finalPressure;
int  finalPres100;
byte highbytepres;
byte lowbytepres;

float finalHum;
int  finalHum100;
byte highbytehum;
byte lowbytehum;


byte highbyte1;
byte lowbyte1;


byte highbyte25;
byte lowbyte25;


byte highbyte10;
byte lowbyte10;








// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
};

void setup() {
    
    Serial.begin(9600);delay(5000);
    Serial.println(F("Starting setup routine ..."));
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);


    pinMode(pmsSetPin, OUTPUT);

    delay(1000);
    flash_led(10);
    delay(1000);




// read battery level


     Serial.println(F("Read the battery"));
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: " ); Serial.println(measuredvbat);
 
 
 Serial.println(F("Read the Gases via ADC.. "));
 
  my_no2 = analogRead(NO2);
  Serial.print("NO2: " ); Serial.println(my_no2);


  
  my_nh3 = analogRead(NH3);
  Serial.print("NH#: " ); Serial.println(my_nh3);


  my_co = analogRead(CO);
  Serial.print("CO: " ); Serial.println(my_co);







        

    Serial1.begin(9600);          // for PMS sensor
    Serial1.setTimeout(1500);     // setup PMS sensor
    delay(1000);





    Serial.println(F("Serial 1 started ..now initilise the BME280 start"));
    bme280.begin();

    delay(2000);

    
    Serial.println(F("Serial 1 started ..init and reset LMIC"));
    delay(2000);
    
  

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

 
 
}

void loop() {
 
    Serial.print("in Loop: Must give the sensor time to adjust, wait 20 seconds");
    flash_led(5);
    delay(4000);
    // turn on the PMS5003

    Serial.println("Set PMS SET Pin HIGH and wait 20 seconds ......");
    digitalWrite(pmsSetPin, HIGH);
    delay(20000);


    
    int sleepMS;
 
    Serial.print("Run a do_send ....");
    
    do_send(&sendjob);

    while(sleeping == false)
      {
        os_runloop_once();
      }

   Serial.print("Set Sleeping false");
   sleeping = false;

  Serial.println("Set PMS SET Pin LOW to turn off and wait 10 seconds ......");
  digitalWrite(pmsSetPin, LOW);
  delay(10000);

         
  Serial.print("Enter Watchdog Sleep loop");

 
     for (int i=0;i<sleepcycles;i++)
        {
          sleepMS = Watchdog.sleep();      //sleep ~ 15 seconds * sleepcycles
        }




    
}















void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("artKey: ");
              for (int i=0; i<sizeof(artKey); ++i) {
                Serial.print(artKey[i], HEX);
              }
              Serial.println("");
              Serial.print("nwkKey: ");
              for (int i=0; i<sizeof(nwkKey); ++i) {
                Serial.print(nwkKey[i], HEX);
              }
              Serial.println("");
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
      // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            sleeping = true;
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        delay(1000);
        // read the bme280 data 
        
        float temperature = bme280.readTemperature();
        float pressure = bme280.readPressure() / 100.0F;
        float humidity = bme280.readHumidity();
         
        Serial.println("Temp is : ");
        Serial.println(temperature);
        Serial.println("Pressure is : ");
        Serial.println(pressure);        
        Serial.println("Humidity is : ");
        Serial.println(humidity);
        
       
                 
        delay(1000);
      
        

        // read the PMS sensor to get the data

        Serial.println(F("Read PMS sensor ..."));
        updatePMBuffer();
        
        pm1 = readPM1();    // pm 1.0
        pm25 = readPM25();   // pm 2.5
        pm10 = readPM10();  // pm 10

        Serial.println("PM1.0 is : ");
        Serial.println(pm1);
        
        Serial.println("PM2.5 is : ");
        Serial.println(pm25);

        
        
        
        Serial.println("PM10.0 is : ");
        Serial.println(pm10);

       

         
        
        finalTemp = temperature;
        finalTemp100 =  finalTemp *100;
        highbytetemp = highByte(finalTemp100);
        lowbytetemp = lowByte(finalTemp100);


        finalPressure = pressure;
       // finalPres100 =  finalPressure *100;
        finalPres100 =  finalPressure*1;
        highbytepres = highByte(finalPres100);
        lowbytepres = lowByte(finalPres100);



        finalHum = humidity;
        finalHum100 =  finalHum *100;
        highbytehum = highByte(finalHum100);
        lowbytehum = lowByte(finalHum100);

        highbyte1 = highByte(pm1);
        lowbyte1 = lowByte(pm1);

        highbyte25 = highByte(pm25);
        lowbyte25 = lowByte(pm25);

      
        highbyte10 = highByte(pm10);
        lowbyte10 = lowByte(pm10);
       

        Serial.println("Gases here are  : ");
        Serial.println(my_no2);
        Serial.println(my_nh3);
        Serial.println(my_co);
        
        int_no2 =  my_no2 *1;
        highbyteno2 = highByte(int_no2);
        lowbyteno2 = lowByte(int_no2);

        int_nh3 =  my_nh3 *1;
        highbytenh3 = highByte(int_nh3);
        lowbytenh3 = lowByte(int_nh3);

        int_co =  my_co *1;
        highbyteco = highByte(int_co);
        lowbyteco = lowByte(int_co);

   

        uint8_t messagePacket[] ={highbytetemp,lowbytetemp,highbytepres,lowbytepres,highbytehum,lowbytehum,highbyte1,lowbyte1,highbyte25,lowbyte25,highbyte10,lowbyte10,highbyteno2,lowbyteno2,highbytenh3,lowbytenh3,highbyteco,lowbyteco,};

        Serial.println("ARRAY Is  ");
        int i;
        for (i = 0; i < 18; i = i + 1) {
        Serial.println(messagePacket[i]);
        }
        Serial.println("DONE ARRAY");


        
        Serial.println("Send the message");
        // Prepare upstream data transmission at the next possible time.
       // LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0); working
       
        LMIC_setTxData2(1, messagePacket, sizeof(messagePacket), 0);
       
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}









void updatePMBuffer() 
{
const char PM_MESSAGE_START_TOKEN[2] = {(char) 0x42, (char) 0x4d};

  Serial.println("vars are");
  Serial.println(PM_MESSAGE_START_TOKEN);
  Serial.println(PM_MESSAGE_LENGTH);

  
    char * startToken = (char *) PM_MESSAGE_START_TOKEN;


    
    while ( Serial1.available() > PM_MESSAGE_LENGTH ) 
    {
      
        if ( Serial1.find( startToken ) ) 
        { 
            Serial1.readBytes( pmMessageBuffer, PM_MESSAGE_LENGTH );
        }
        
    }
}


int readPM1() {
    return readPMValueFromBuffer( PM_1_0_INDEX );
}

int readPM25() {
    return readPMValueFromBuffer( PM_2_5_INDEX );
}


int readPM10() {
    return readPMValueFromBuffer( PM_10_INDEX );
}

int readPMValueFromBuffer( int index ) {
    return ((pmMessageBuffer[index]<<8) + pmMessageBuffer[index+1]);
}






void flash_led(int numFlashes)
{
  for (int i = 0; i < numFlashes; i++) 
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    
  }

  
}
