#include "conshell.h"

conshell_status
cons_init_line( cons_line* nline, Stream* port )
{
    nline->delim = bfromcstr( " :," );
    nline->line = bfromcstralloc ( CONSHELL_CLI_BUFFER_LEN, "" );
    nline->port = port;
}

/** Grab a line from the serial line within a time limit
 *
 * Place new characters as they are found into the cons_line structure passed
 * to the function. If the time limit expires, return the timeout type return
 * code.
 */
conshell_status
cons_poll_line( cons_line* cline, uint32_t timeout )
{
    static char c;
    c = '\0';
    uint32_t time;
    bstring buffer = cline->line;

    //if( buf->len >= ASHCON_CLI_MAX_LENGTH ) return -1;

    if( cline->port->available() == 0 ) {
        return CONSHELL_LINE_TIMEOUT;
    }

    time = millis();
    timeout += time;

    while( time < timeout ) {
        if( cline->port->available() != 0 ) {
            c = cline->port->read();
            bconchar( buffer, c );
        }

        if( c == CONSHELL_CLI_EOL ) {
            return CONSHELL_LINE_END;
        }

        time = millis();
    }
    
    return CONSHELL_LINE_TIMEOUT;
}

int
cons_search_exec( cons_line* cline, cmdlist* cmds )
{
    int returncode = 0;
    conshfunc func;
    blist args;

    args = bsplits( cline->line, cline->delim );

    // Check if its added a newline for some reason
    if( args->qty == 1 ) {
        uint8_t cpos = bstrchr( args->entry[0], '\n' );
        if( cpos != -1 ) bdelete( args->entry[0], cpos, 1 );
    }

    func = (conshfunc) cons_exec_cmd( cmds, args->entry[0] );

    if( func != NULL ) 
        returncode = func( args );
    else 
        returncode = CONSHELL_FUNCTION_NOT_FOUND;

    bstrListDestroy( args );

    return returncode;
}

/** Reset the string to contain no characters
 *
 * Dosn't delete the memory associated with the string in any way
 */
conshell_status
cons_clear_line( cons_line* cline )
{
    bassigncstr( cline->line, "" );
}
