#include <Wire.h>
#include <SPI.h>
#include <printf.h>
#include <string.h>

/////////////////////////////////////////////////////////////
//  RADIO
/////////////////////////////////////////////////////////////

#define RADIO_NODE_ID 1
#define RADIO_CE_PIN 9
#define RADIO_CSP_PIN 10
#define BASE_RECV_ADDR 0x53654e6400
#define BASE_SEND_ADDR 0x5265437600
#define RECV_ADDR '\x01'
#define SEND_ADDR '\x01'
#define RADIO_CHANNEL 0x30
#define RADIO_PAYLOAD_SIZE 32
#define RADIO_PA_LEVEL RF24_PA_HIGH

#define MSG_TYPE_STATUS 0
#define MSG_TYPE_ERROR 1
#define MSG_TYPE_FIELD_REQUEST 2
#define MSG_TYPE_FIELD_SET 3
#define MSG_TYPE_FIELD_RESPONSE 4
#define MSG_TYPE_PROXY 5

#include "RF24.h"

RF24 radio(RADIO_CE_PIN, RADIO_CSP_PIN);

int init_radio(void) {
    radio.begin();
    radio.setPayloadSize(RADIO_PAYLOAD_SIZE);
    radio.setChannel(RADIO_CHANNEL);
    radio.setPALevel(RADIO_PA_LEVEL);
    radio.setDataRate(RF24_250KBPS);

    radio.openWritingPipe(BASE_SEND_ADDR | SEND_ADDR);
    radio.openReadingPipe(1, BASE_RECV_ADDR | RECV_ADDR);

    radio.startListening();
    printf_begin();
    radio.printDetails();
}

int is_message_available(void) {
    return radio.available();
}

void read_msg(unsigned char* buf, int len) {
    return radio.read(buf, len);
}

int send_msg(int node_id, int msg_type, unsigned char* data, char len) {
    //len: -1 for autodetect, 0 for default
    unsigned char buffer[RADIO_PAYLOAD_SIZE] = {0};
    buffer[0] = RADIO_NODE_ID;
    buffer[1] = msg_type;

    if(len == -1) {
       len = strlen((const char*)data) + 1;
    }
    else if(len == 0) {
        len = RADIO_PAYLOAD_SIZE - 2;
    }

    strncpy((char*)&buffer[2], (const char*)data, len);

    radio.stopListening();
    int response = radio.write(buffer, RADIO_PAYLOAD_SIZE);
    radio.startListening();
    return response;
}


/////////////////////////////////////////////////////////////
//  BUZZER
/////////////////////////////////////////////////////////////
#define BUZZER_POWER_PIN 4

#include "pitches.h"

void _play_melody(int* notes, int* durations, int len) {
  for (int i = 0; i < len; i++) {
    int duration = 1000 / durations[i];
    tone(BUZZER_POWER_PIN, notes[i], duration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_POWER_PIN);
  }
}

// Having separate functions for playing melodies is
// better than declaring data globally, it saves memory.
void play_melody__start() {
  int notes[] = {
    NOTE_E3, NOTE_E4, NOTE_E5,
  };
  int durations[] = {
    8, 8, 4,
  };
  _play_melody(notes, durations, 3);
}




/////////////////////////////////////////////////////////////
//  DISPLAY
/////////////////////////////////////////////////////////////

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Do not use reset pin
#define DISPLAY_RESET -1
#define DISPLAY_IIC_ADDRESS 0x3C
#define SSD1306_128_64
Adafruit_SSD1306 display(DISPLAY_RESET);


/////////////////////////////////////////////////////////////
//  SPEED SENSOR
/////////////////////////////////////////////////////////////
#define SPEED_SENSOR_PIN 2
#define SPEED_SENSOR_BUFFER_SIZE 10
// Max number of milliseconds to take into account.
#define SPEED_SENSOR_MAX_TIME_INTERVAL 5000
// The distance (in meters) the bike passes in one wheel rotation 
#define SPEED_SENSOR_WHEEL_PERIMETER 2.333
volatile unsigned int speed_sensor_buffer[SPEED_SENSOR_BUFFER_SIZE] = {0};
volatile unsigned long speed_sensor_last_time = 0;
volatile int speed_sensor_last_buffer_cell = 0;
volatile unsigned long speed_sensor_total_rotates = 0;

void speed_sensor_interrupt() {
  if (millis() - speed_sensor_last_time <  2) {
    // Reed sensor tends to bounce and generate double signals, let's ignore them.
    return;
  }
  speed_sensor_last_buffer_cell = (speed_sensor_last_buffer_cell + 1) % SPEED_SENSOR_BUFFER_SIZE;
  speed_sensor_buffer[speed_sensor_last_buffer_cell] = millis() - speed_sensor_last_time;
  speed_sensor_last_time = millis();
  
  speed_sensor_total_rotates++;
}

unsigned int get_current_speed() {
  // return the current speed in km/h
  unsigned long time_sum = 0;
  unsigned char rotations_count = 0;
  
  unsigned long time_limiter = millis() - speed_sensor_last_time;
    
  for(char i=0; i<SPEED_SENSOR_BUFFER_SIZE; i++) {
    unsigned int val = speed_sensor_buffer[
        (SPEED_SENSOR_BUFFER_SIZE + speed_sensor_last_buffer_cell - i) % SPEED_SENSOR_BUFFER_SIZE
    ];
    
    if (val == 0  || (time_limiter + val) > SPEED_SENSOR_MAX_TIME_INTERVAL) {
      // Limit will be reached after adding this value, stopping.
      break;
    }
      
    rotations_count++;
    time_sum += val;
    time_limiter += val;
  }
                 
  if (time_sum == 0)
    return 0;
  
  unsigned int res = (unsigned int)((SPEED_SENSOR_WHEEL_PERIMETER * rotations_count) / ((float)time_sum/1000.0) * 3.6 * 10);
  Serial.println(res);
  return res;
  //return (unsigned int)((SPEED_SENSOR_WHEEL_PERIMETER * rotations_count) / ((float)time_sum/1000.0) * 3.6 * 10);
}

float get_average_speed() {
  // return a total distance in km.
  return get_total_distance() / (millis() / 1000 / 3600);
}

unsigned int get_total_distance() {
  // return a total distance in km.      
  return (unsigned int) ((SPEED_SENSOR_WHEEL_PERIMETER * speed_sensor_total_rotates) / 1000. * 10);
}

void print_number(unsigned int number) {
  // using `sprintf` with floats requires some additional magic 
  // and will use more memory, it's easier to implement it manually:
  
  display.write('0' + (number / 100));
  display.write('0' + (number / 10 % 10));
  display.write('.');
  display.write('0' + (number % 10));
}

void print_time() {
    unsigned long time = millis() / 1000;
    unsigned char seconds = time % 60;
    time /= 60;
    unsigned char minutes = time % 60;
    time /= 60;
    
    display.write('0' + time / 10);
    display.write('0' + time % 10);
    display.write(':');
    display.write('0' + minutes / 10);
    display.write('0' + minutes % 10);
    display.write(':');
    display.write('0' + seconds / 10);
    display.write('0' + seconds % 10);
}

void refresh_display() {  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.drawRoundRect(0, 0, display.width(), display.height(), 8, WHITE);
  
  display.setTextSize(2);
  
  // Speed
  display.setCursor(8, 32);
  print_number(get_current_speed());
  
  // Distance
  display.setCursor(72, 32);
  print_number(get_total_distance()); 
  
  // Time
  display.setCursor(16, 8);
  print_time();
 
  display.setTextSize(1);
  display.setCursor(20, 50);
  display.print("km/h");
  display.setCursor(72, 50);
  display.print("km total");
  
  display.display();
}

void setup() {
    Serial.begin(115200);
    
    Serial.println("Initializing radio...");
    init_radio();
    
    pinMode(BUZZER_POWER_PIN, OUTPUT);
    
    Serial.println("Initializing display...");
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_IIC_ADDRESS);
    display.clearDisplay();
    display.display();
    
    Serial.println("Setting interrupts...");
    attachInterrupt(digitalPinToInterrupt(SPEED_SENSOR_PIN), speed_sensor_interrupt, FALLING);
    
    // Initialization succeeded, play a cheerful melody
    play_melody__start();
}

void loop() {
  Serial.println(speed_sensor_total_rotates);
  refresh_display();  
  delay(500);
}
