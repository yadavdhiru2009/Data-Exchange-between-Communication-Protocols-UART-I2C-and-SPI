/*
 * @author : Ashutosh Singh Parmar
 * @brief : This file contains function declarations of simple serial library
 */
#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>
#include <SimplyAtomic.h>

//#define SIMPLE_SERIAL_BYTES_TO_BUFFER

#define SIMPLE_SERIAL_PREAMBLE 0X0D
#define SIMPLE_SERIAL_FRAME_END 0X1E

#define SIMPLE_SERIAL_VERSION1 0X01

#define SIMPLE_SERIAL_MAX_LENGTH 50

#define MESSAGE_BUFFER_SIZE 10

#define BUFFER0_SIZE 200

#define BUFFER1_SIZE 200

typedef enum SIMPLE_SERIAL_PACKET_SEGMENT{ PREAMBLE_WAIT = 1, VERSION_WAIT = 2, TITLE_WAIT_1 = 3, LENGTH_WAIT_1 = 4, MESSAGE_WAIT_1 = 5, END_WAIT_1 = 6 }SIMPLE_SERIAL_PACKET_SEGMENT;

typedef struct simple_serial_msg {
  uint8_t ver;
  uint8_t title;
  uint8_t len;
  uint8_t * message;
}simple_serial_msg;

namespace simpleSerial
{

    uint8_t byte_available();

    uint8_t byte_read();
    
    uint8_t simple_serial_available();
    
    char get_simple_serial_message( simple_serial_msg * );

    void screen_bytes();

    char send_simple_serial_message( simple_serial_msg );

    void simple_serial_delete( simple_serial_msg );
}

#endif
