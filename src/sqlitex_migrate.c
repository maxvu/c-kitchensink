#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "sqlitex.h"

static int sqlitex_open ( const char * file, sqlite3 ** dbh ) {
    if ( SQLITE_OK != sqlite3_initialize() )
        return 1;
    if ( SQLITE_OK != sqlite3_open( file, dbh ) )
        return 2;
    if ( sqlite_exec( *dbh, "pragma journal_mode = WAL;" ) ) {
        sqlite3_close( *dbh );
        return 3;
    }
    if ( sqlite_exec( *dbh, "pragma synchronous = normal;" ) ) {
        sqlite3_close( *dbh );
        return 4;
    }
    return 0;
}

static int sqlitex_exec ( sqlite3 * dbh, const char * sql ) {
    if ( !dbh || !sql )
        return 1;
    sqlite3_stmt * stm;
    const char * tal;
    int drc;
    while ( *sql ) {
        while ( isspace( *sql ) )
            sql++;
        if ( !*sql )
            break;
        if ( SQLITE_OK != sqlite3_prepare( dbh, sql, -1, &stm, &tal ) )
            return 2;
        drc = sqlite3_step( stm );
        if ( drc != SQLITE_ROW && drc != SQLITE_DONE ) {
            sqlite3_finalize( stm );
            return 3;
        }
        sqlite3_finalize( stm );
        sql = tal;
    }
    return 0;
}

static int sqlitex_uv_get ( sqlite3 * dbh, size_t * uv ) {
    if ( !dbh || !uv )
        return 1;
    sqlite3_stmt * stm;
    const char * sql = "pragma userversion;";
    if ( SQLITE_OK != sqlite3_prepare( dbh, sql, -1, &stm, NULL ) )
        return 2;
    if ( SQLITE_DONE != sqlite3_step( stm ) ) {
        sqlite3_finalize( stm );
        return 3;
    }
    *uv = sqlite3_column_int( stm, 0 );
    sqlite3_finalize( stm );
    return 0;
}

static int sqlitex_uv_set ( sqlite3 * dbh, size_t uv ) {
    if ( !dbh )
        return 1;
    sqlite3_stmt * stm;
    const char * sql = "pragma userversion(?);";
    if ( SQLITE_OK != sqlite3_prepare( dbh, sql, -1, &stm, NULL ) )
        return 2;
    if ( SQLITE_OK != sqlite3_bind_int( stm, 0, uv ) ) {
        sqlite3_finalize( stm );
        return 3;
    }
    if ( SQLITE_OK != sqlite3_step( stm ) ) {
        sqlite3_finalize( stm );
        return 4;
    }
    sqlite3_finalize( stm );
    return 0;
}

int sqlitex_migrate ( sqlite3 * dbh, const char * dir ) {
    int    rc  = 0;
    size_t uvp = 0;
    size_t uvc = 0;
    size_t uvn = 0;
    dirwalk wlk;
    char * sql = NULL;

    if ( !dbh || !dir ) {
        log_err( "missing migration parameters" );
        return 1;
    }
    if ( dirwalk_init( &wlk, dir ) ) {
        log_err( "couldn't open dir '%s'", dir );
        return 2;
    }
    log_dbg( "starting migration" );
    if ( sqlite_exec( dbh, "begin transaction;" ) ) {
        log_err( "couldn't start transaction", dir );
        return 3;
    }
    if ( sqlite_uv_get( dbh, &uvp ) ) {
        log_err( "failed retrieving userversion", dir );
        return 3;
    }
    log_dbg( "read userversion %d", uvp );
    while ( dirwalk_step( &wlk ) ) {
        if ( 0 != strcmp( dirwalk_extension( &wlk ), ".sql" ) ) {
            log_dbg( "skipping %s -- not sql", dirwalk_basename( &wlk ) );
            continue;
        }
        if ( 1 != sscanf( dirwalk_basename( &wlk ), "%ld", &uvn ) ) {
            log_dbg( "skipping %s -- not properly id'ed", dirwalk_basename( &wlk ) );
            continue;
        }
        if ( uvn <= uvc ) {
            log_dbg( "skipping %s -- already applied", dirwalk_basename( &wlk ) );
            continue;
        }
        if ( uvn != uvc + 1 ) {
            log_err( "%s out-of-sequence", dirwalk_basename( &wlk ) );
            rc = 4;
            goto end;
        }
        if ( dirwalk_open( &wlk, &sql ) ) {
            log_err( "couldn't read '%s'", dirwalk_basename( &wlk ) );
            rc = 6;
            goto end;
        }
        if ( sqlite_exec( dbh, sql ) ) {
            log_err( "failed running '%s': %s",
                dirwalk_basename( &wlk ), sqlite3_errmsg( dbh ) );
            rc = 8;
            goto end;
        }
        uvc = uvn;
        free( sql );
        sql = NULL;
    }

    if ( uvc == uvp ) {
        log_dbg( "db schema already up-to-date" );
        goto end;
    }
    if ( sqlite_uv_set( dbh, uvc ) ) {
        log_err( "failed setting userversion %d", uvc );
        rc = 10;
        goto end;
    }
    if ( sqlite_exec( dbh, "commit;" ) ) {
        log_err( "failed commiting txn" );
        rc = 12;
        goto end;
    }
    log_dbg( "successfully migrated from v%d to v%d", uvp, uvc );

    end:
    if ( sql )
        free( sql );
    dirwalk_destroy( &wlk );
    return rc;
}

