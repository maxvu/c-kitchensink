#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "log.h"

#define LOG_LEVEL_OFF      0
#define LOG_LEVEL_CRITICAL 1
#define LOG_LEVEL_ERROR    2
#define LOG_LEVEL_WARN     3
#define LOG_LEVEL_NOTICE   4
#define LOG_LEVEL_INFO     5
#define LOG_LEVEL_DEBUG    6
#define LOG_LEVEL_TRACE    7

// We need to list the names in order to read them from environment.
const char * lt_names[] = {
	"off", "critical", "error", "warn", "notice", "info",
	"debug", "trace", 0 };

// Symbols used to indicate severity.
char lt_abbrs[] = { ' ', '#', '&', '%', '!', ':', '.', ' ', 0 };

const char * lt_ansi_colors[] = {
    "",         // off
    "\e[1;91m", // critical, bright red
    "\e[0;31m", // error, red
    "\e[1;33m", // warn, yellow
    "\e[1;m",   // notice, bright
    "",         // info
    "\e[33;2m", // debug, dim
    "\e[33;2m", // trace, dim
    0
};

 const char ansi_reset[] = "\e[0m";

struct {
	int lvl;          // severity to log
	int clr;          // whether to use color
	const char * tft; // "time format"
} cfg;

// Lookup int level from string name.
int log_levelfromname ( const char * name ) {
	if ( !name )
		return -1;
	for ( int i = 0; lt_names[ i ]; i++ )
		if ( 0 == strcasecmp( lt_names[ i ], name ) )
			return i;
	return -1;
}

void log_init () {
	cfg.lvl = LOG_LEVEL_INFO;
	cfg.clr = 0;

	const char * opt_lvl = getenv( LOG_ENVVAR_LEVEL );
	const char * opt_clr = getenv( LOG_ENVVAR_COLOR );

	if ( opt_lvl ) {
		int sev = log_levelfromname( opt_lvl );
		if ( sev >= 0 )
			cfg.lvl = sev;
	}

	if ( opt_clr ) {
		if ( 0 == strcasecmp( "y", opt_clr ) )
			cfg.clr = 1;
		else if ( 0 == strcasecmp( "yes", opt_clr ) )
			cfg.clr = 1;
		else if ( 0 == strcasecmp( "on", opt_clr ) )
			cfg.clr = 1;
		else if ( 0 == strcasecmp( "1", opt_clr ) )
			cfg.clr = 1;
	}
}

void log_destroy () {
	// ...
}

size_t timestamp ( char * buf, size_t n, const char * fmt ) {
	time_t    tr = time( NULL );
	struct tm ti;

	localtime_r( &tr, &ti );
	return strftime( buf, n, LOG_TIMEFORMAT, &ti );
}

void log_write ( int lvl, const char * fmt, va_list args ) {
	if ( lvl > cfg.lvl )
		return;
	char   buf[ LOG_BUFFER_SIZE ];
	size_t pos;
	int    res;

	pos = 0;

	// Write the color escape string.
	pos += snprintf( buf + pos, LOG_BUFFER_SIZE - pos - 1, "%s",
		cfg.clr ? lt_ansi_colors[ lvl ] : "" );

	// Add timestamp.
	pos += timestamp( buf + pos, LOG_BUFFER_SIZE - pos - 1, cfg.tft );

	// Write the severity symbol.
	pos += snprintf( buf + pos, LOG_BUFFER_SIZE - pos - 1, "%c ", lt_abbrs[ lvl ] );

	// Write user message.
	pos += vsnprintf( buf + pos, LOG_BUFFER_SIZE - pos - 1, fmt, args );

	// Reserve space for ANSI escape and newline.
	res = cfg.clr ? sizeof( ansi_reset ) + 2 : 2;
	if ( pos >= LOG_BUFFER_SIZE - res )
		pos = LOG_BUFFER_SIZE - res;

	pos += snprintf( buf + pos, res, "%s\n", cfg.clr ? ansi_reset : "" );
	fwrite( buf, 1, pos, stderr );
}

void log_crt ( const char * fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	log_write( LOG_LEVEL_CRITICAL, fmt, args );
	va_end( args );
}

void log_err ( const char * fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	log_write( LOG_LEVEL_ERROR, fmt, args );
	va_end( args );
}

void log_wrn ( const char * fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	log_write( LOG_LEVEL_WARN, fmt, args );
	va_end( args );
}

void log_ntc ( const char * fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	log_write( LOG_LEVEL_NOTICE, fmt, args );
	va_end( args );
}

void log_nfo ( const char * fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	log_write( LOG_LEVEL_INFO, fmt, args );
	va_end( args );
}

void log_dbg ( const char * fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	log_write( LOG_LEVEL_DEBUG, fmt, args );
	va_end( args );
}

void log_trc ( const char * fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	log_write( LOG_LEVEL_TRACE, fmt, args );
	va_end( args );
}
