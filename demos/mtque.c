#include <stdlib.h>
#include <unistd.h>
#include "mtque.h"
#include "log.h"

#define NUM_CONSUMERS 4

mtque que;
pthread_t consumers[ NUM_CONSUMERS ];

void * consume ( void * arg ) {
    uintptr_t thread_id = ( uintptr_t ) arg;
    char stop = 0;
    int rc = MTQUE_ERR_OK;
    int * dat;

    log_nfo( "C%d create ", thread_id );

    while ( !stop ) {
        rc = mtque_pop( &que, ( void ** ) &dat );
        switch ( rc ) {
            case MTQUE_ERR_OK:
                log_nfo( "C%d pop %c", thread_id, ( char ) ( 'A' + *dat ) );
                free( dat );
                break;
            case MTQUE_ERR_STOP:
                log_nfo( "C%d stop", thread_id );
                stop = 1;
                break;
            default:
                log_err( "queue pop error: %s", mtque_errmsg( rc ) );
                break;
        }
    }

    return NULL;
}

int main ( int argc, char ** argv ) {
    log_init();

    int rc;

    if ( ( rc = mtque_init( &que ) ) ) {
        log_err( "queue error: %s", mtque_errmsg( rc ) );
        return 1;
    }
    log_nfo( "queue initialized" );

    for ( uintptr_t i = 0; i < NUM_CONSUMERS; i++ ) {
        if ( 0 != pthread_create( &consumers[ i ], NULL, consume, ( void * ) i ) ) {
            log_err( "failed creating consumer thread %lu", i );
            return 2;
        }
    }

    // usleep( 20000 );

    for ( uintptr_t i = 0; i < 5; i++ ) {
        int * job = malloc( sizeof( int ) );
        if ( !job ) {
            log_err( "failed allocating job" );
            return 3;
        }
        *job = i;
        if ( 0 != mtque_push( &que, job ) ) {
            log_err( "failed pushing job" );
            return 5;
        }
        log_nfo( "push %c", ( char ) ( i + 'A' ) );
    }

    if ( ( rc = mtque_stop( &que ) ) ) {
        log_err( "queue error: %s", mtque_errmsg( rc ) );
        return 2;
    }
    log_nfo( "stopped queue" );

    for ( uintptr_t i = 0; i < NUM_CONSUMERS; i++ ) {
        pthread_join( consumers[ i ], NULL );
        log_nfo( "C%d join", i );
    }

    if ( ( rc = mtque_destroy( &que ) ) ) {
        log_err( "queue error: %s", mtque_errmsg( rc ) );
        return 3;
    }

    return 0;
}

