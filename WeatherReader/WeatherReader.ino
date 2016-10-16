#include "suspend.h"
#include "dht.h"
#include "RF24.h"
#include "wireless_device.h"


#define DHT22_PIN PB1
#define DHT22_POWER_PIN PB2
#define DHTTYPE DHT22
#define TEMPERATURE_SHIFT -3


Sleep sleep;
dht DHT;

#ifdef DEBUG
TinyDebugSerial debSerial = TinyDebugSerial();
#endif


#define SEND_BUFFER_LEN 2
unsigned char send_buffer[SEND_BUFFER_LEN] = {0};

#define LOOP_DELAY 600000


void setup() {
  #ifdef DEBUG
    debSerial.begin(9600);
  #endif
 
  init_radio();
  
  #ifdef DEBUG
    printf_begin();
    radio.printDetails();
  #endif
  
  sleep.pwrDownMode();
}


void loop() {
   pinMode(DHT22_POWER_PIN, OUTPUT);
   digitalWrite(DHT22_POWER_PIN, HIGH);
   
   // Some time to wake up DHT22.
   sleep.sleepDelay(1000);
   
   powerUpRadio();
   
   if(DHT.read22(DHT22_PIN) == DHTLIB_OK) {
      #ifdef DEBUG
        debSerial.println("Read successful");
        debSerial.print(DHT.humidity, 1);
        debSerial.print(",\t");
        debSerial.print(DHT.temperature, 1);
        debSerial.println(",\t");
      #endif
      
      send_buffer[0] = (int)DHT.temperature + 100 + TEMPERATURE_SHIFT;
      send_buffer[1] = (int)DHT.humidity;
      send_msg(NODE_ID, MSG_TYPE_STATUS, send_buffer, SEND_BUFFER_LEN);
    }
    else {
      #ifdef DEBUG
        debSerial.println("Read ERROR");
      #endif
      
      send_msg(NODE_ID, MSG_TYPE_ERROR, send_buffer, 0);
    }
    
    powerDownRadio();
    
    pinMode(DHT22_PIN, INPUT);
    pinMode(DHT22_POWER_PIN, INPUT);
    
    sleep.sleepDelay(LOOP_DELAY);
}

