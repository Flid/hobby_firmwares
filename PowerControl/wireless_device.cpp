#include "wireless_device.h"
#include <printf.h>
#include <string.h>

RF24 radio(RADIO_CE_PIN, RADIO_CSP_PIN);

int init_radio(void) {
    radio.begin();
    radio.setPayloadSize(PAYLOAD_SIZE);
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
    unsigned char buffer[PAYLOAD_SIZE] = {0};
    buffer[0] = NODE_ID;
    buffer[1] = msg_type;

    if(len == -1) {
       len = strlen((const char*)data) + 1;
    }
    else if(len == 0) {
        len = PAYLOAD_SIZE - 2;
    }

    strncpy((char*)&buffer[2], (const char*)data, len);

    radio.stopListening();
    int response = radio.write(buffer, PAYLOAD_SIZE);
    radio.startListening();
    return response;
}
