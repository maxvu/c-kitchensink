#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "dirscan.h"

int dirscan_init ( dirscan * scn, const char * path ) {
    if ( !scn || !path )
        return 1;
    scn->rut = path;
    scn->ent = NULL;
    scn->max = 0;
    scn->cur = 0;
    scn->buf[ 0 ] = 0;
    return 0;
}

int dirscan_step ( dirscan * scn ) {
    if ( !scn )
        return 1;
    if ( !scn->ent ) {
        scn->max = scandir( scn->rut, &scn->ent, NULL, alphasort );
        if ( scn->max < 0 )
            return 2;
        if ( 0 == strcmp( ".", dirscan_basename( scn ) ) )
            scn->cur++;
        if ( 0 == strcmp( "..", dirscan_basename( scn ) ) )
            scn->cur++;
        return scn->cur < scn->max ? 0 : 3;
    }
    if ( scn->cur >= scn->max - 1 )
        return 3;
    scn->cur++;
    return 0;
}

int dirscan_err ( dirscan * scn ) {
    return scn && scn->ent ? 0 : 1;
}

const char * dirscan_basename ( dirscan * scn ) {
    if ( !scn )
        return NULL;
    if ( !scn->ent )
        return NULL;
    if ( scn->cur >= scn->max )
        return NULL;
    return scn->ent[ scn->cur ]->d_name;
}

const char * dirscan_fullpath ( dirscan * scn ) {
    const char * base = dirscan_basename( scn );
    const char * path = scn->rut;
    const char * sep = path[ strlen( path ) - 1 ] == '/' ? "" : "/";
    if ( !base )
        return NULL;
    int n = snprintf( scn->buf, DIRSCAN_BUFLEN,
        "%s%s%s", path, sep, base );
    if ( n > CETUS_DIRSCAN_BUFLEN )
        return NULL;
    return scn->buf;
}

const char * dirscan_extension ( dirscan * scn ) {
    if ( !scn )
        return NULL;
    const char * base;
    if ( !( base = dirscan_basename( scn ) ) )
        return NULL;
    const char * ext = strchr( base, '.' );
    return ext == base ? "" : ext;
}

void dirscan_destroy ( dirscan * scn ) {
    if ( !scn )
        return;
    if ( scn->ent ) {
        for ( int i = 0; i < scn->max; i++ )
            free( scn->ent[ i ] );
        free( scn->ent );
    }
    scn->ent = NULL;
    scn->max = 0;
    scn->cur = 0;
}

