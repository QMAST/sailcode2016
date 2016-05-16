/** This program runs on a secondary arduino.  It monitors the voltages, currents
* 	of the system, as well as monitors the winch encoders.  These values are sent
*	to the primary arduino via serial communication.
*
*	Note: Serial.write() is used rather than Serial.print(), because Serial.print()
*	transforms the value into ASCII characters (so a value of 1 would be sent as
*	a value of 49).  To avoid extra unnecessary transformations and possible loss
*	of data, each number is sent over serial as a single byte (8 bits) using
*	Serial.write().
*
*	Note: Serial.write() can only transfer 1 byte at a time, so don't try to send it
*	a full integer (32 bits).
*/

#define CONSOLE_BAUD 19200

#define ATTO_0_VOLT_PIN A2
#define ATTO_0_CURR_PIN A3

#define ATTO_1_VOLT_PIN A0
#define ATTO_1_CURR_PIN A1

#define ADC_CONSTANT 4.89 // mV per tick

#define WIRE_CMD_BATT_VOLT 0x01
#define WIRE_CMD_BATT_CURR 0x02

#define WIRE_CMD_CHRG_VOLT 0x03
#define WIRE_CMD_CHRG_CURR 0x04

#define WIRE_CMD_GET_W1_TICKS 0x05
#define WIRE_CMD_CLR_W1_TICKS 0x06

#define WIRE_CMD_GET_W2_TICKS 0x07
#define WIRE_CMD_CLR_W2_TICKS 0x08

#define WIRE_CMD_GETANDCLR_W1_TICKS 0x09
#define WIRE_CMD_GETANDCLR_W2_TICKS 0x0A

// These interrupt pins refer to Arduino UNO
#define ENCODER_INTERRUPT_W1    0 // Pin 2
#define ENCODER_INTERRUPT_W2    1 // Pin 3

// Variables to track optical ticks
volatile uint16_t enc_w1_ticks = 0;
volatile uint16_t enc_w2_ticks = 0;

uint8_t incoming_cmd_buf[8] = { 0 };
uint8_t incoming_cmd_buf_size = 0; // How many bytes were written

//Set up prototypes
void count_w1_tick();
void count_w2_tick();
void incoming_handler();
void request_handler();

void setup()
{
    Serial.begin( CONSOLE_BAUD );

    attachInterrupt( ENCODER_INTERRUPT_W1, count_w1_tick, FALLING );
    attachInterrupt( ENCODER_INTERRUPT_W2, count_w2_tick, FALLING );
}

void loop()
{
    /*delay(1000);*/
    if( Serial.available() > 0 ) {
        incoming_handler();
        request_handler();
    }
}

void count_w1_tick()
{
    cli();
    enc_w1_ticks++;
    sei();
}

void count_w2_tick()
{
    cli();
    enc_w2_ticks++;
    sei();
}

/** Returns the voltage in mV for the given pin number
 */
uint16_t
get_voltage( uint8_t pin )
{
    // ADC between 0 and 1023
    uint16_t val;
    val = (uint16_t) analogRead( pin );
    val *= ADC_CONSTANT;

    return val;
}

/** Uses voltage reading from attopilot to get voltage in mV
 */
uint16_t
get_atto_volt( uint8_t pin )
{
    return ( get_voltage(pin) / 242.3F ) * 1000L;
}

/** Uses voltage reading from attopilot to get current in mA
 */
uint16_t
get_atto_curr( uint8_t pin )
{
    return ( get_voltage(pin) / 73.2F ) * 1000L;
}

void incoming_handler()
{
    incoming_cmd_buf_size = 0;

    for( uint8_t i = 0;
            i < sizeof(incoming_cmd_buf)
        &&  Serial.available() > 0;
        i++ )
    {
        incoming_cmd_buf[i] = Serial.read();
        incoming_cmd_buf_size++;
    }
}

/** Use the first byte in the incoming command buffer to return the right
 * result
 *
 * Returns two bytes of whatever type of data is requested. If the sensai board
 * requests more than that, then grasshopper provides 0xFF for the extra bytes
 */
void request_handler()
{
  uint16_t val = 0;         // Holds sensor data

  if( incoming_cmd_buf[0] == WIRE_CMD_BATT_VOLT ) {
        val = get_atto_volt( ATTO_0_VOLT_PIN );
		writeValue(val);
  } 
  else if(  incoming_cmd_buf[0] == WIRE_CMD_BATT_CURR ) {
        val = get_atto_curr( ATTO_0_CURR_PIN );
		writeValue(val);
  } 
  else if(  incoming_cmd_buf[0] == WIRE_CMD_CHRG_VOLT ) {
        val = get_atto_volt( ATTO_1_VOLT_PIN );
		writeValue(val);
  } 
  else if(  incoming_cmd_buf[0] == WIRE_CMD_CHRG_CURR ) {
        val = get_atto_curr( ATTO_1_CURR_PIN );
		writeValue(val);
  } 
  else if(  incoming_cmd_buf[0] == WIRE_CMD_GET_W1_TICKS ) {
        val = enc_w1_ticks;
		writeValue(val);
  } 
  else if(  incoming_cmd_buf[0] == WIRE_CMD_CLR_W1_TICKS ) {
        cli();
        enc_w1_ticks = 0;
        sei();
		writeValue(0x0000);
  }
  else if(  incoming_cmd_buf[0] == WIRE_CMD_GET_W2_TICKS ) {
        val = enc_w2_ticks;
		writeValue(val);
  } 
  else if(  incoming_cmd_buf[0] == WIRE_CMD_CLR_W2_TICKS ) {
        cli();
        enc_w2_ticks = 0;
        sei();
        writeValue(0x0000);
  } 
  else if(  incoming_cmd_buf[0] == WIRE_CMD_GETANDCLR_W1_TICKS ) {
		cli();
		val = enc_w1_ticks;
		enc_w1_ticks = 0;
		sei();
		writeValue(val);	
	} 
	else if(  incoming_cmd_buf[0] == WIRE_CMD_GETANDCLR_W2_TICKS ) {
		cli();
		val = enc_w2_ticks;
		enc_w2_ticks = 0;
		sei();
		writeValue(val);		
	} 
	else if(  incoming_cmd_buf[0] == 'z' ) {
        static char buf[40];
        snprintf( buf, sizeof(buf),
                ("EW1:%u EW2:%u BV:%u BC:%u\n"),
                enc_w1_ticks,
                enc_w2_ticks,
                get_atto_volt( ATTO_0_VOLT_PIN ),
                get_atto_volt( ATTO_0_CURR_PIN )
            );
        Serial.print( buf );
    } 
    else {
        Serial.write( incoming_cmd_buf, sizeof(incoming_cmd_buf) );
    }
}

void writeValue(uint16_t val){
	uint8_t val_high = 0;		// Holds high bit
	uint8_t val_low = 0;		// Holds Low bit
	//Mask high and low bits to send
	val_high = (val >> 8) & 0xFF;
	val_low = val & 0xFF;
	Serial.write(val_high);
	Serial.write(val_low);
}

