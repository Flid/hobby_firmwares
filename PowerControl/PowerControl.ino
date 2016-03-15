#include <SPI.h>
#include <NewPing.h>
#include "RF24.h"
#include "radio_settings.h"
#include "wireless_device.h"

#define POWER_PIN 4
#define MSG_TYPE_ON 255
#define MSG_TYPE_OFF 254

#define SEND_STATE_EVERY_N_ITERATION 50

unsigned char is_enabled = 0;
unsigned char send_state_in = 1;

#define LOOP_DELAY 100

#define SONAR_TRGGER_PIN  7
#define SONAR_ECHO_PIN   8
#define SONAR_MAX_DISTANCE 200
#define SONAR_HOLD_BUCKETS_COUNT 10
#define SONAR_RELEASE_THRESHOLD 1
#define SONAR_GRAB_THRESHOLD  3


NewPing sonar(SONAR_TRGGER_PIN, SONAR_ECHO_PIN, SONAR_MAX_DISTANCE);

char sonar_state_buckets[SONAR_HOLD_BUCKETS_COUNT] = {0};
char sonar_last_id = 0;
int sonar_state_sum = 0;
char sonar_object_presents = 0;

void set_enabled(char state) {
    is_enabled = state;
    if(is_enabled) {
        digitalWrite(POWER_PIN, HIGH);
    }
    else {
        digitalWrite(POWER_PIN, LOW);
    }
}

void send_state(){
    Serial.print(F("Sending state: "));
    Serial.println(is_enabled);
    unsigned char out_buffer[1];
    out_buffer[0] = is_enabled;
    send_msg(NODE_ID, MSG_TYPE_STATUS, out_buffer, 1);
}


void setup() {
  Serial.begin(115200);
  Serial.println(F("Initializing device..."));

  init_radio();

  set_enabled(0);
  pinMode(POWER_PIN, OUTPUT);
}


void process_message(unsigned char* buffer){
    switch(buffer[0]) {
        case MSG_TYPE_ON:
            Serial.println("Received a message to turn on");
            set_enabled(1);
            send_state();
            break;

        case MSG_TYPE_OFF:
            Serial.println("Received a message to turn off");
            set_enabled(0);
            send_state();
            break;

        default:
            Serial.println(F("Unknown message type\n"));
            send_msg(NODE_ID, MSG_TYPE_ERROR, (unsigned char*)"Unknown msg type", -1);
            return;
    }
    Serial.println(F("Message has been processed\n"));
}


void process_sonar() {
    int uS = sonar.ping();
    char current_state = ((uS / US_ROUNDTRIP_CM) < 100) && uS != 0;

    sonar_last_id = (sonar_last_id + 1) % SONAR_HOLD_BUCKETS_COUNT;
    sonar_state_sum -= sonar_state_buckets[sonar_last_id];
    sonar_state_buckets[sonar_last_id] = current_state;
    sonar_state_sum += sonar_state_buckets[sonar_last_id];

    if (sonar_object_presents && sonar_state_sum <= SONAR_RELEASE_THRESHOLD) {
        // Object disappeared
        sonar_object_presents = 0;
    }

    if (!sonar_object_presents && sonar_state_sum >= SONAR_GRAB_THRESHOLD) {
        // Object disappeared
        sonar_object_presents = 1;

        Serial.println("Manual switch detected.");
        // Switching the power.
        is_enabled = !is_enabled;
        set_enabled(is_enabled);
        send_state();
    }

}


void loop() {
    delay(LOOP_DELAY);

    process_sonar();

    send_state_in--;

    if(send_state_in == 0) {
        send_state_in = SEND_STATE_EVERY_N_ITERATION;
        send_state();
    }

    while(is_message_available()) {
        Serial.print(F("Receiving message...\n"));
        unsigned char msg_buffer[PAYLOAD_SIZE] = {0};
        read_msg(msg_buffer, PAYLOAD_SIZE);
        process_message(msg_buffer);
    }

}

