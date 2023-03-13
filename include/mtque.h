#include <pthread.h>
#include <stdint.h>

#define MTQUE_ERR_OK      0
#define MTQUE_ERR_NULLARG 1
#define MTQUE_ERR_MTXINIT 2
#define MTQUE_ERR_CNDINIT 3
#define MTQUE_ERR_MTXLOCK 4
#define MTQUE_ERR_MALLOC  5
#define MTQUE_ERR_ZEROCAP 6
#define MTQUE_ERR_STOP    7
#define MTQUE_ERR_MAXREAD 8

// #define MTQUE_ERR_ 1
// #define MTQUE_ERR_ 1

typedef struct mtque mtque;
typedef struct mtque_job mtque_job;

typedef struct mtque {
    mtque_job *     head;
    mtque_job *     tail;
    pthread_mutex_t mutx;
    pthread_cond_t  work;     // signal on push()
    pthread_cond_t  done;     // signal on pop()
    unsigned int    stop : 1; // signal on stop()
    uint16_t        nwtg;     // number of readers waiting
    uint16_t        njob;     // # of jobs in queue
    uint16_t        limt;     // queue capacity
} mtque;

int mtque_init ( mtque * que );
int mtque_setcapacity ( mtque * que, uint16_t cap );
int mtque_push ( mtque * que, void * data );
int mtque_pop ( mtque * que, void ** data );
int mtque_stop ( mtque * que );
int mtque_destroy ( mtque * que );
const char * mtque_errmsg ( int err );
