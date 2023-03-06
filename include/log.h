#ifndef LOG_H
#define LOG_H

/*
    A simple, optionally-colored console logger.
    Variadic calls are passed through standard stdio.h functions.
    (So be careful with those arguments!)
    Configuration is stored statically, with log messages on the stack.

    Configured through environment.
    When not colored, symbols (e.g. '#')  are used to indicate level.
    This is to give a visual indication, without taking up too much space.
*/

// The size of the stack-allocated log message buffer.
// Longer messages will be truncated.
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 256
#endif

#if LOG_BUFFER_SIZE < 64
#error LOG_BUFFER_SIZE must be >= 64
#endif

// For production, elide calls to and remove strings interned for levels
// debug and trace.
#ifndef LOG_SQUELCH_DEBUG
#define LOG_SQUELCH_DEBUG 0
#endif

// The name of the envvar that controls log level.
// Default is "info".
#ifndef LOG_ENVVAR_LEVEL
#define LOG_ENVVAR_LEVEL "LOGLEVEL"
#endif

// The name of the envvar that controls whether to use ANSI terminal colors.
// Default is "0".
#ifndef LOG_ENVVAR_COLOR
#define LOG_ENVVAR_COLOR "LOGCOLOR"
#endif

// The format of the timestamp at the beginning of each log line.
// Corresponds to a strftime() call.
#ifndef LOG_TIMEFORMAT
#define LOG_TIMEFORMAT "%F %T "
#endif

// Must be called before logging.
void log_init ();

// Destruction not currently needed.
void log_destroy ();

void log_crt ( const char * fmt, ... ); // "critical"
void log_err ( const char * fmt, ... ); // "error"
void log_wrn ( const char * fmt, ... ); // "warn"
void log_ntc ( const char * fmt, ... ); // "notice"
void log_nfo ( const char * fmt, ... ); // "info"
void log_dbg ( const char * fmt, ... ); // "debug"
void log_trc ( const char * fmt, ... ); // "trace"

#endif
