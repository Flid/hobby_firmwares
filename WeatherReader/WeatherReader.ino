//#define DEBUG

#include "suspend.h"


Sleep sleep;

#include "dht.h"

#define DHT22_PIN PB1
#define DHT22_POWER_PIN PB2
#define DHTTYPE DHT22

dht DHT;

TinyDebugSerial debSerial = TinyDebugSerial();

#include "RF24.h"

#ifdef DEBUG
  #include "printf.h"
#endif

#define RADIO_CHANNEL 0x30
#define PAYLOAD_SIZE 32


#define MSG_TYPE_STATUS 0
#define MSG_TYPE_ERROR 1

#define RADIO_CE_PIN 8
#define RADIO_CSP_PIN 7
#define BASE_RECV_ADDR 0x53654e6400
#define BASE_SEND_ADDR 0x5265437600
#define RECV_ADDR '\x01'
#define SEND_ADDR '\x01'
#define NODE_ID 1
#define TEMPERATURE_SHIFT -3


RF24 radio(RADIO_CE_PIN, RADIO_CSP_PIN);
unsigned char buffer[PAYLOAD_SIZE] = {0};

#define LOOP_DELAY 6000000

void setup() {
  #ifdef DEBUG
    debSerial.begin(9600);
  #endif
 
  radio.begin();
  radio.setPayloadSize(PAYLOAD_SIZE);
  radio.setChannel(RADIO_CHANNEL);

  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(BASE_SEND_ADDR | SEND_ADDR);
  radio.openReadingPipe(1, BASE_RECV_ADDR | RECV_ADDR);

  radio.startListening();
  
  #ifdef DEBUG
    printf_begin();
    radio.printDetails();
  #endif
  
  buffer[0] = NODE_ID;
  
  sleep.pwrDownMode();
}




void loop() {
   
   
   pinMode(DHT22_POWER_PIN, OUTPUT);
   digitalWrite(DHT22_POWER_PIN, HIGH);
   
   // Some time to wake up DHT22.
   sleep.sleepDelay(1000);
   
   radio.powerUp();
   
   if(DHT.read22(DHT22_PIN) != DHTLIB_OK) {
      #ifdef DEBUG
        debSerial.println("ERROR");
      #endif
      //return;
    }
    #ifdef DEBUG
      debSerial.print(DHT.humidity, 1);
      debSerial.print(",\t");
      debSerial.print(DHT.temperature, 1);
      debSerial.println(",\t");
    #endif
    
    buffer[1] = MSG_TYPE_STATUS;
    buffer[2] = (int)DHT.temperature + 100 + TEMPERATURE_SHIFT;
    buffer[3] = (int)DHT.humidity;
    
    radio.stopListening();
    int response = radio.write(buffer, PAYLOAD_SIZE);
    radio.startListening();
    radio.powerDown();
    
    pinMode(DHT22_PIN, INPUT);
    pinMode(DHT22_POWER_PIN, INPUT);
    
    sleep.sleepDelay(LOOP_DELAY);
}

