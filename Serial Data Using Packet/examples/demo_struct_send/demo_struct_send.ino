/*

Author:Dharmendra Kumar Yadav

*/
#include <simpleSerial.h>

#define LED 13

simple_serial_msg send_msg;

typedef struct _demo_struct{
  uint8_t id;
  uint8_t x;
  uint8_t y;
}demo_struct;

/* This is the structure to send */
demo_struct s1;

void setup() {

  pinMode( LED, OUTPUT );
  
  Serial.begin( 115200 );

  /* Initializing the structure */
  s1.id = 1;
  s1.x = 10;
  s1.y = 20;

  /* Preparing the simple serial message to send */
  send_msg.ver = 1;
  send_msg.title = 0;
  send_msg.len = sizeof( s1 );
  send_msg.message = (uint8_t *)( &s1 );
  
}

void loop() {
  
  simpleSerial :: send_simple_serial_message( send_msg );
  digitalWrite( LED, HIGH ); 
  delay(1000);
  digitalWrite( LED, LOW );
  delay(1000);
  
}
