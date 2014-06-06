#include <Wire.h>

#define CONSOLE_BAUD 19200

#define THIS_I2C_ADDR 0x2C

#define ATTO_0_VOLT_PIN A0
#define ATTO_0_CURR_PIN A1

#define ATTO_1_VOLT_PIN A2
#define ATTO_1_CURR_PIN A3

#define I2C_OFFSET 2800

#define ADC_CONSTANT 4.89 // mV per tick

#define WIRE_CMD_BATT_VOLT 0x01
#define WIRE_CMD_BATT_CURR 0x02

#define WIRE_CMD_CHRG_VOLT 0x03
#define WIRE_CMD_CHRG_CURR 0x04

#define WIRE_CMD_GET_W1_TICKS 0x05
#define WIRE_CMD_CLR_W1_TICKS 0x06

#define WIRE_CMD_GET_W2_TICKS 0x07
#define WIRE_CMD_CLR_W2_TICKS 0x08

// These interrupt pins refer to Arduino UNO
#define ENCODER_INTERRUPT_W1    0 // Pin 2
#define ENCODER_INTERRUPT_W2    1 // Pin 3

// Variables to track optical ticks
volatile uint16_t enc_w1_ticks = 0;
volatile uint16_t enc_w2_ticks = 0;

uint8_t incoming_cmd_buf[8] = { 0 };
uint8_t incoming_cmd_buf_size = 0; // How many bytes were written

void setup()
{
    Serial.begin(CONSOLE_BAUD);

    Wire.begin( THIS_I2C_ADDR ); // Join i2c network as slave
    Wire.onReceive( incoming_handler );
    Wire.onRequest( request_handler );

    attachInterrupt( ENCODER_INTERRUPT_W1, count_w1_tick, FALLING );
    attachInterrupt( ENCODER_INTERRUPT_W2, count_w2_tick, FALLING );
}

void loop()
{
    static char buf[40];

    snprintf( buf, sizeof(buf),
            ("EW1:%u EW2:%u BV:%u BC:%u\n"),
            enc_w1_ticks,
            enc_w2_ticks,
            get_atto_volt( ATTO_0_VOLT_PIN ),
            get_atto_volt( ATTO_0_CURR_PIN )
        );
    Serial.print( buf );

    /*delay(1000);*/
}

void
count_w1_tick()
{
    cli();
    enc_w1_ticks++;
    sei();
}

void
count_w2_tick()
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

void
incoming_handler( int nbytes )
{
    incoming_cmd_buf_size = 0;

    for( uint8_t i = 0;
            i < sizeof(incoming_cmd_buf)
        &&  Wire.available() > 0;
        i++ )
    {
        incoming_cmd_buf[i] = Wire.read();
        incoming_cmd_buf_size++;
    }
}

/** Use the first byte in the incoming command buffer to return the right
 * result
 *
 * Returns two bytes of whatever type of data is requested. If the sensai board
 * requests more than that, then grasshopper provides 0xFF for the extra bytes
 */
void
request_handler()
{
    uint16_t val = 0;        // Holds sensor data
    uint8_t sbuf[2] = { 0 }; // Holds data split into bytes

    if(         incoming_cmd_buf[0] == WIRE_CMD_BATT_VOLT ) {
        val = get_atto_volt( ATTO_0_VOLT_PIN ) - I2C_OFFSET;
        sbuf[0] = val;
        sbuf[1] = val >> 8;
        Wire.write( sbuf, sizeof(sbuf) );

    } else if(  incoming_cmd_buf[0] == WIRE_CMD_BATT_CURR ) {
        val = get_atto_curr( ATTO_0_CURR_PIN );
        sbuf[0] = val;
        sbuf[1] = val >> 8;
        Wire.write( sbuf, sizeof(sbuf) );

    } else if(  incoming_cmd_buf[0] == WIRE_CMD_CHRG_VOLT ) {
        val = get_atto_volt( ATTO_1_VOLT_PIN );
        sbuf[0] = val;
        sbuf[1] = val >> 8;
        Wire.write( sbuf, sizeof(sbuf) );

    } else if(  incoming_cmd_buf[0] == WIRE_CMD_CHRG_CURR ) {
        val = get_atto_curr( ATTO_1_CURR_PIN );
        sbuf[0] = val;
        sbuf[1] = val >> 8;
        Wire.write( sbuf, sizeof(sbuf) );

    } else if(  incoming_cmd_buf[0] == WIRE_CMD_GET_W1_TICKS ) {
        val = enc_w1_ticks;
        sbuf[0] = val;
        sbuf[1] = val >> 8;
        Wire.write( sbuf, sizeof(sbuf) );

    } else if(  incoming_cmd_buf[0] == WIRE_CMD_CLR_W1_TICKS ) {
        cli();
        enc_w1_ticks = 0;
        sei();
        Wire.write( 0x0 );
        Wire.write( 0x0 );

    } else if(  incoming_cmd_buf[0] == WIRE_CMD_GET_W2_TICKS ) {
        val = enc_w2_ticks;
        sbuf[0] = val;
        sbuf[1] = val >> 8;
        Wire.write( sbuf, sizeof(sbuf) );

    } else if(  incoming_cmd_buf[0] == WIRE_CMD_CLR_W2_TICKS ) {
        cli();
        enc_w2_ticks = 0;
        sei();
        Wire.write( 0x0 );
        Wire.write( 0x0 );

    } else {
        Wire.write( incoming_cmd_buf, sizeof(incoming_cmd_buf) );
    }
}
// vim:ft=c:

