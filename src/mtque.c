#include <stdlib.h>
#include "mtque.h"

const char * errmsgs[] = {
    "ok",
    "null arg passed",
    "failed initializing mutex",
    "failed initializing conditional variable",
    "failed locking mutex",
    "couldn't allocate job",
    "queue capacity cannot be zero",
    "queue is stopping",
    "maximum number of readers reached",
    0
};

struct mtque_job {
    void      * data;
    mtque_job * prev;
    mtque_job * next;
};

const char * mtque_errmsg ( int rc ) {
    if ( rc >= sizeof( errmsgs ) - 1 )
        return "unknown error";
    return errmsgs[ rc ];
}

int mtque_init ( mtque * que ) {
    if ( !que )
        return MTQUE_ERR_NULLARG;
    que->head = NULL;
    if ( 0 != pthread_mutex_init( &que->mutx, NULL ) )
        return MTQUE_ERR_MTXINIT;
    if ( 0 != pthread_cond_init( &que->work, NULL ) )
        return MTQUE_ERR_CNDINIT;
    if ( 0 != pthread_cond_init( &que->done, NULL ) )
        return MTQUE_ERR_CNDINIT;
    que->stop = 0;
    que->nwtg = 0;
    que->njob = 0;
    que->limt = UINT16_MAX;
    return MTQUE_ERR_OK;
}

int mtque_setcapacity ( mtque * que, uint16_t cap ) {
    if ( !que )
        return MTQUE_ERR_NULLARG;
    if ( !cap )
        return MTQUE_ERR_ZEROCAP;
    if ( 0 != pthread_mutex_lock( &que->mutx ) )
        return MTQUE_ERR_MTXLOCK;
    que->limt = cap;
    pthread_mutex_unlock( &que->mutx );
    return MTQUE_ERR_OK;
}

int mtque_push ( mtque * que, void * data ) {
    mtque_job * job;

    if ( !que || !data )
        return MTQUE_ERR_NULLARG;
    if ( !( job = malloc( sizeof( mtque_job ) ) ) )
        return MTQUE_ERR_MALLOC;
    job->data = data;
    job->prev = NULL;
    job->next = NULL;
    if ( 0 != pthread_mutex_lock( &que->mutx ) )
        return MTQUE_ERR_MTXLOCK;
    while ( !que->stop && que->njob >= que->limt )
        pthread_cond_wait( &que->done, &que->mutx );
    if ( que->stop ) {
        pthread_mutex_unlock( &que->mutx );
        return MTQUE_ERR_STOP;
    }
    if ( !que->head ) {
        que->head = que->tail = job;
    } else {
        job->next = que->head;
        que->head->prev = job;
        que->head = job;
    }
    que->njob++;
    pthread_cond_signal( &que->work );
    pthread_mutex_unlock( &que->mutx );
    return MTQUE_ERR_OK;
}

int mtque_pop ( mtque * que, void ** data ) {
    mtque_job * job;

    if ( !que || !data )
        return MTQUE_ERR_NULLARG;
    if ( 0 != pthread_mutex_lock( &que->mutx ) )
        return MTQUE_ERR_MTXLOCK;
    if ( que->nwtg == UINT16_MAX ) {
        pthread_mutex_unlock( &que->mutx );
        return MTQUE_ERR_MAXREAD;
    }
    que->nwtg++;
    while ( !que->stop && que->njob == 0 )
        pthread_cond_wait( &que->work, &que->mutx );
    if ( que->stop && !que->njob ) {
        pthread_mutex_unlock( &que->mutx );
        return MTQUE_ERR_STOP;
    }
    job = que->tail;
    if ( que->head == que->tail ) {
        que->head = que->tail = NULL;
    } else {
        job->prev->next = NULL;
        que->tail = job->prev;
    }
    que->njob--;
    que->nwtg--;
    pthread_cond_signal( &que->done );
    pthread_mutex_unlock( &que->mutx );
    *data = job->data;
    free( job );
    return MTQUE_ERR_OK;
}

int mtque_stop ( mtque * que ) {
    if ( !que )
        return MTQUE_ERR_NULLARG;
    if ( 0 != pthread_mutex_lock( &que->mutx ) )
        return MTQUE_ERR_MTXLOCK;
    que->stop = 1;
    pthread_cond_broadcast( &que->work );
    pthread_mutex_unlock( &que->mutx );
    return MTQUE_ERR_OK;
}

int mtque_destroy ( mtque * que ) {
    mtque_job * cur;
    mtque_job * nxt;

    if ( !que )
        return MTQUE_ERR_NULLARG;
    cur = que->head;
    while ( cur ) {
        nxt = cur->next;
        free( cur );
        cur = nxt;
    }

    pthread_mutex_destroy( &que->mutx );
    pthread_cond_destroy( &que->work );
    pthread_cond_destroy( &que->done );
    return MTQUE_ERR_OK;
}


