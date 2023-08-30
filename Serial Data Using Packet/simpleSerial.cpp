/*
 * @author : Ashutosh Singh Parmar
 * @brief : This file contains function definitions of simple serial library
 * @note : The APIs are not thread safe, so do not call them from multiple flows of execution
 */
#include "simpleSerial.h"

/*
 * This is the buffer for holding simple serial messages.
*/
static simple_serial_msg SIMPLE_SERIAL_BUFFER[MESSAGE_BUFFER_SIZE];
static uint8_t msgFilled = 0;
static uint8_t msgInPos = 0;
static uint8_t msgOutPos = 0;

static uint8_t ind0;
static uint8_t buff0[ BUFFER0_SIZE ];

/*
 * This is the buffer for holding characters from the Serial port.
*/
static uint8_t buff1[BUFFER1_SIZE]={0};
static uint8_t filled1 = 0;
static uint8_t inPos1 = 0;
static uint8_t outPos1 = 0;

/*
 * This is used for temporary storage of simple serial message.
*/
static simple_serial_msg MSG;

/*
 * This is the variable used for storing the state.
*/
static SIMPLE_SERIAL_PACKET_SEGMENT simple_serial_state = PREAMBLE_WAIT;

static simple_serial_msg SIMPLE_SERIAL_NULL_MSG = {
  .ver = 0,
  .title = 0,
  .len = 0,
  .message = NULL
};


static uint8_t byt;

static uint8_t rst;

static uint8_t msg_len;
static uint8_t msg_ind;

namespace simpleSerial
{
    /*
     * This function is used to enqueue a character in the character buffer.
     *  
     * UTILITY FUNCTION
     * 
     * PARAMETERS : 
     * ----------
     * 1. uint8_t byt : The byte to enqueue.
     * 
     * RETURNS : char : Error code
     * -------
     * 0 : SUCCESS
     * 1 : FAILED ( queue is full )
    */
    static char byte_enqueue( uint8_t byt )
    {
      if( filled1 < BUFFER1_SIZE )
      {
        buff1[ inPos1++ ] = byt;
        if( inPos1 == BUFFER1_SIZE ) inPos1 = 0;
        ATOMIC(){ filled1++; }
        
        return 0;
      }
      return 1;
    }

    /*
     * This function is used to check the number of bytes available in the character buffer.
     * 
     * PARAMETERS :
     * ----------
     * NONE
     * 
     * RETURNS : uint8_t : The number of bytes in the character buffer.
     * -------
    */
    uint8_t byte_available()
    {
      return filled1;
    }

    /*
     * This function is used to read a byte from the character buffer.
     * 
     * PARAMETERS :
     * ----------
     * NONE
     * 
     * RETURNS : uint8_t : The byte read from the character buffer.
     * -------
     * 
     * NOTE : Use this function only after the byte_available() function has returned a non zero value.
     * ----
    */
    uint8_t byte_read()
    {
      uint8_t byt = buff1[ outPos1++ ];
      if( outPos1 == BUFFER1_SIZE ) outPos1 = 0;
      ATOMIC(){ filled1--; }

      return byt;
    } 

    /*
     * This function returns simple serial messages from the simple serial buffer.
     * 
     * PARAMETERS :
     * ----------
     * 1. simple_serial_message * : Pointer to simple serial message type.
     * 
     * RETURNS : char
     * -------
     * 0 : SUCCESS
     * 1 : FAILED
     * 
     * NOTE :
     * ----
     * The operations inside this function must be thread safe
     */
    char get_simple_serial_message( simple_serial_msg * msg ) {
      if( msgFilled )
      {
        *msg =  SIMPLE_SERIAL_BUFFER[ msgOutPos++ ];
        if ( msgOutPos == MESSAGE_BUFFER_SIZE ) msgOutPos = 0;
        ATOMIC(){ msgFilled --; }
        return 0;
      }
      else
        return 1;
    }

    /*
     * This function is used to enqueue a simple serial message into the buffer.
     * 
     * UTILITY FUNCTION
     * 
     * PARAMTERS :
     * ---------
     * 1. simple_serial_message : A simple serial message object.
     * 
     * RETURNS : char
     * -------
     * 0 : SUCCESS
     * 1 : FAILED
     * 
    */
    static char put_simple_serial_message( simple_serial_msg msg ) {
      if( msgFilled < MESSAGE_BUFFER_SIZE )
      {
        SIMPLE_SERIAL_BUFFER[ msgInPos++ ] = msg;
        if( msgInPos == MESSAGE_BUFFER_SIZE ) msgInPos = 0;
        ATOMIC() { msgFilled ++; }
        return 0;
      }
      else
      {
        return 1;
      }
    }
    
    /*
     * This function returns the number of simple serial messages in the simple serial input buffer
     * 
     * PARAMETERS :
     * ----------
     * NONE
     * 
     * RETURNS : uint8_t
     * -------
     * The number of messages in the simple serial buffer
     * 
     * NOTE :
     * ----
     * ***
     */
    uint8_t simple_serial_available() {
      return msgFilled;
    }

    /*
     * This function is used to check if the user code has requested to reset the simple serial state machine.
     * 
     * UTILITY FUNCTION
     * 
     * PARAMETERS :
     * ----------
     * NONE
     * 
     * RETURNS : NOTHING
     * -------
     * 
     * NOTE : This is just a utility function. 
     * ----
    */
    static void check_reset()
    {
      if( rst )
      {
        // Serial.println( "Transferring to bytes buffer" );
        for( uint8_t i=0; i<ind0; i++ )
        {
          byte_enqueue( buff0[ i ] );
        }
              
        simple_serial_state = PREAMBLE_WAIT;
      }
    }

    /*
     * This function is used to screen bytes from serial port. These characters are screen for simple serial messages.
     * 
     * PARAMETERS :
     * ----------
     * NONE
     * 
     * RETURNS :
     * -------
     * NOTHING
     * 
     * NOTE :
     * ----
     * This function needs to called repeatedly!
     */
    void screen_bytes()
    {
      if ( Serial.available() )
      {
        byt = Serial.read();
        switch( simple_serial_state )
        {
          case PREAMBLE_WAIT:
         
            //Serial.println("Current state : Preamble wait");
            
            
            if( byt == SIMPLE_SERIAL_PREAMBLE )
            {
              ATOMIC() { rst = 0; }

              ind0 = 0;
              buff0[ ind0++ ] = byt;
              
              simple_serial_state = VERSION_WAIT;

              //Serial.println( "Preamble Received" );
            }
            else
            {
              byte_enqueue( byt );
            }
          break;

          case VERSION_WAIT:

            //Serial.println("Current state : Version wait");
            
            buff0[ ind0++ ] = byt;
            
            /* Only version 1 is allowed as of now. */
            if( byt == 1 )
            {
              MSG.ver = byt;
              
              simple_serial_state = TITLE_WAIT_1;

              //Serial.println( "Simple Serial Version 1 detected" );
            }
            else
            {
              ATOMIC() { rst = 1; }
            }

            check_reset();
          break;

          case TITLE_WAIT_1:
          
            buff0[ind0++] = byt;
            
            MSG.title = byt;

            simple_serial_state = LENGTH_WAIT_1;

            //Serial.print( "Title : " );
            //Serial.println( byt );
            
            check_reset();
          break;

          case LENGTH_WAIT_1:

            //Serial.print( "Length : " );
            //Serial.println( byt );
            
            buff0[ind0++] = byt;

            if( byt == 0 )
            {
              //Serial.print( "Empty message : " );
              MSG.len = 0;
              MSG.message = NULL;

              /*if( !put_simple_serial_message( MSG ) )
              {
                // Serial.println( "Message enqueued" );
                // Logic to decide whether to put simple serial packet bytes in the character buffer.
                #ifdef SIMPLE_SERIAL_BYTES_TO_BUFFER
                  transferToCharBuffer();
                #endif
              }
              else
              {
                //Serial.println( "Failed to enqueue message" );
                ATOMIC() { rst = 1; }
              }*/
              simple_serial_state = END_WAIT_1;
              
            }
            else if( byt < SIMPLE_SERIAL_MAX_LENGTH )
            {
               //Serial.print( "Allocating memory : " );
               
               MSG.len = msg_len = byt;
               MSG.message = (uint8_t *)malloc( (byt * sizeof(uint8_t)) );

               if( MSG.message != NULL )
               {
                  //Serial.println( "success" );
                  msg_ind = 0;
                  simple_serial_state = MESSAGE_WAIT_1;
               }
               else
               {
                  //Serial.println( "failed" );
                  ATOMIC() { rst = 1; }
               }
            }
            else
            {
              //Serial.print( "Invalid Length" );
              
              ATOMIC() { rst = 1; }
            }
            
            check_reset();
          break;

          case MESSAGE_WAIT_1:
            //Serial.println( "Message byte" );
            
            buff0[ind0++] = byt;
            *( MSG.message + (msg_ind++) ) = byt;
            msg_len --;

            if( !msg_len )
            {
              simple_serial_state = END_WAIT_1;
            }

            if( rst ) free( MSG.message );
            
            check_reset();
          break;

          case END_WAIT_1:
            //Serial.println( "Current state : End wait" );
            buff0[ind0++] = byt;
            
            if( byt == SIMPLE_SERIAL_FRAME_END )
            {
              //Serial.print( "Frame end received, enqueuing message : ");
              
              if( !put_simple_serial_message( MSG ) )
              {
                //Serial.println( "success" );
                /* Logic to decide whether to put simple serial packet bytes in the character buffer. */
                #ifdef SIMPLE_SERIAL_BYTES_TO_BUFFER
                  transferToCharBuffer();
                #endif

                simple_serial_state = PREAMBLE_WAIT;
              }
              else
              {
                //Serial.println( "failed" );
                ATOMIC() { rst = 1; }
              }
            }
            else
            {
              //Serial.println( "Invalid frame end" );
              ATOMIC() { rst = 1; }
            }

            if( rst ) free( MSG.message );
            
            check_reset();
          break;
        }
      }
    }

    /*
     * This function is used to send simple serial messages over serial uart port.
     * 
     * PARAMETERS :
     * ----------
     * 1. simple_serial_msg msg : Simple serial message structure
     * 
     * RETURNS : char
     * -------
     * 1 : Success
     * 0 : Failed
    */
    char send_simple_serial_message( simple_serial_msg msg )
    {
      Serial.write( SIMPLE_SERIAL_PREAMBLE );
      Serial.write( msg.ver );
      Serial.write( msg.title );
      Serial.write( msg.len );

      if( msg.message != NULL )
      {
        for( uint8_t i = 0; i < msg.len; i++)
        {
          Serial.write( msg.message[i] );
        }
      }

      Serial.write( SIMPLE_SERIAL_FRAME_END );

      return 1;
    }
    
    /*
     * This function is used to delete dynamically allocated memory.
     * 
     * PARAMETERS :
     * ----------
     * 1. simple_serial_msg msg : Simple serial message structure
     * 
     * RETURNS : void
     * -------
     * NOTHING
    */
    void simple_serial_delete( simple_serial_msg msg )
    {
      if( msg.message != NULL ) free( msg.message );
    }
}
