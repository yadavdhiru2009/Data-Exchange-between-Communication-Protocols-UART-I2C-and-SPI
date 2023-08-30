/*

Author:Dharmendra Kumar Yadav

*/
#include "simpleSerial.h"

#define LED 13

simple_serial_msg recv_msg;

typedef struct _demo_struct{
  uint8_t id;
  uint8_t x;
  uint8_t y;
}demo_struct;

demo_struct s1;

void setup() {

  pinMode( LED, OUTPUT );

  Serial.begin( 115200 );

}

void loop() {

  simpleSerial :: screen_bytes();

  if( simpleSerial :: simple_serial_available() )
  {
    simpleSerial :: get_simple_serial_message( &recv_msg );

    memcpy( &s1, recv_msg.message, recv_msg.len );

    if( s1.id == 1 & s1.x == 10 && s1.y == 20 )
    {
      digitalWrite( LED, HIGH );
      delay( 1000 );
      digitalWrite( LED, LOW );
    }

    /* clear the message */
    memset( &s1, 0, sizeof(s1) );
  
    simpleSerial :: simple_serial_delete( recv_msg );
  }

}
