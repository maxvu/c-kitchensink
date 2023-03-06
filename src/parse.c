#include <string.h>
#include <stdlib.h>
#include "parse.h"

int parse_i64 ( const char * str, long * val, int * inv ) {
    if ( !str )
        return 1;
    if ( !strlen( str ) )
        return 2;
    char * end;
    long l = strtol( str, &end, 0 );
    if ( end == str ) {
        if ( inv )
            *inv = 1;
        return 3;
    }
    if ( val )
        *val = l;
    if ( inv )
        *inv = 0;
    return 0;
}

int parse_u64 ( const char * str, unsigned long * val, int * inv ) {
    if ( !str )
        return 1;
    if ( !strlen( str ) )
        return 2;
    char * end;
    long l = strtoul( str, &end, 0 );
    if ( end == str ) {
        if ( inv )
            *inv = 1;
        return 3;
    }
    if ( val )
        *val = l;
    if ( inv )
        *inv = 0;
    return 0;
}

const char * yesses[] = {
    "y", "yes", "t", "true", "on", "enabled", "1", 0
};

const char * noes[] = {
    "n", "no", "f", "false", "off", "disabled", "0", 0
};

int parse_flag ( const char * str, int * val, int * inv ) {
    if ( !str )
        return 1;
    if ( !strlen( str ) )
        return 2;
    for ( int i = 0; yesses[ i ]; i++ ) {
        if ( 0 == strcasecmp( yesses[ i ], str ) ) {
            if ( val )
                *val = 1;
            if ( inv )
                *inv = 0;
            return 0;
        }
    }
    for ( int i = 0; noes[ i ]; i++ ) {
        if ( 0 == strcasecmp( noes[ i ], str ) ) {
            if ( val )
                *val = 0;
            if ( inv )
                *inv = 0;
            return 0;
        }
    }
    return 3;
}

