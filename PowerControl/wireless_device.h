#ifndef _WIRELES_DEVICES
#define _WIRELES_DEVICES

#include "RF24.h"
#include "radio_settings.h"

#define BASE_RECV_ADDR 0x53654e6400
#define BASE_SEND_ADDR 0x5265437600

#define RADIO_CHANNEL 0x30
#define PAYLOAD_SIZE 32

#ifndef RADIO_PA_LEVEL
#define RADIO_PA_LEVEL RF24_PA_HIGH
#endif

#define MSG_TYPE_STATUS 0
#define MSG_TYPE_ERROR 1
#define MSG_TYPE_FIELD_REQUEST 2
#define MSG_TYPE_FIELD_SET 3
#define MSG_TYPE_FIELD_RESPONSE 4
#define MSG_TYPE_PROXY 5

int init_radio(void);

int is_message_available(void);
void read_msg(unsigned char* buf, int len);

int send_msg(int node_id, int msg_type, unsigned char* data, char len);

#endif _WIRELES_DEVICES
