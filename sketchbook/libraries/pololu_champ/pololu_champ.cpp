#include "pololu_champ.h"

void
pchamp_write_command(
        pchamp_controller* dev,
        uint8_t* command,
        uint8_t length )
{
    for( uint8_t i = 0; i < length; i++ ) {
        dev->line->write( command[i] );
    }
}

uint16_t
pchamp_request_value( pchamp_controller* dev, uint8_t var )
{
    uint8_t response[2];
    uint16_t value = 0;
    uint32_t time;

    time = millis();

    // Make sure there is nothing incoming on the line
    dev->line->flush();

    // Send the command
    dev->line->write( PCHAMP_DEFAULT );
    dev->line->write( dev->id );
    dev->line->write( PCHAMP_DC_REQUEST_VAR );
    dev->line->write( var );

    // Get the request
    while( dev->line->available() < 2 ) {
        if( ( millis() - time ) > PCHAMP_DEFAULT_TIMEOUT_MS ) {
            return 0xFFFF;
        }
    }
    response[0] = dev->line->read();
    response[1] = dev->line->read();

    value |= response[1] << 8;
    value |= response[0];

    return value;
}

void
pchamp_request_safe_start( pchamp_controller* dev, uint8_t safe )
{
    if( safe == 1 ) {
        safe = PCHAMP_DC_SAFE_START;
    } else {
        safe = PCHAMP_DC_UNSAFE;
    }

    dev->line->write( PCHAMP_DEFAULT );
    dev->line->write( dev->id );
    dev->line->write( safe );
}

void
pchamp_set_target_speed(
        pchamp_controller* dev,
        uint16_t target,
        uint8_t dir )
{
    uint8_t tcommand[2]; // The speed is two bytes as per pololu manual

    // Split speed as per manual (mod 32) and (div 32)
    tcommand[0] = target & 0x1F; // (mod 32)
    tcommand[1] = target >> 5;   // (div 32)

    // Convert dir into corresponding command
    if( dir == PCHAMP_DC_REVERSE ) {
        dir = PCHAMP_DC_MOTOR_REVERSE;
        //Serial.println("REVERSE");
    } else {
        dir = PCHAMP_DC_MOTOR_FORWARD;
        //Serial.println("FORWARD");
    }

    dev->line->write( PCHAMP_DEFAULT );
    dev->line->write( dev->id );
    dev->line->write( dir );
    dev->line->write( tcommand[0] );
    dev->line->write( tcommand[1] );
}

void
pchamp_servo_set_position( pchamp_servo* servo, uint16_t pos )
{
    pos *= 4;

    uint8_t com[] = {
        PCHAMP_DEFAULT,
        servo->controller->id,
        PCHAMP_SERVO_SET_TARGET,
        servo->channel_id,
        pos & 0x7F, // Lower 7 bits of target position
        (pos >> 7) & 0x7F // Bits 7-13 of target position
    };

    pchamp_write_command(
            servo->controller,
            com,
            sizeof(com) );
}

uint16_t
pchamp_servo_request_value( pchamp_servo* servo, uint8_t var, uint8_t channel )
{
    uint8_t response[2];
    uint8_t command_size;
    uint16_t value = 0;
    uint32_t time;

    pchamp_controller* dev = servo->controller;

    uint8_t command[] = {
        PCHAMP_DEFAULT,
        servo->controller->id,
        var,
        servo->channel_id
    };

    command_size = sizeof(command);

    // If the variable doesn't require a channel id, then don't print it
    if( channel == 0 ) {
        command_size -= 1;
    }

    pchamp_write_command(
            servo->controller,
            command,
            command_size );

    time = millis();

    // Make sure there is nothing incoming on the line
    dev->line->flush();

    // Get the request
    while( dev->line->available() < 2 ) {
        if( ( millis() - time ) > PCHAMP_DEFAULT_TIMEOUT_MS ) {
            return 0xFFFF;
        }
    }
    response[0] = dev->line->read();
    response[1] = dev->line->read();

    value |= response[1] << 8;
    value |= response[0];

    return value;
}


uint16_t
pchamp_get_temperature( pchamp_controller* dev)
{
    return pchamp_request_value( dev, PCHAMP_DC_VAR_TEMPERATURE );
}

