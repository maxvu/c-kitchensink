#ifndef PARSE_H
#define PARSE_H

// Parse values from a C-string.
// Value is stored in `val`, 0 returned on success.
// "Flag" here is "boolean".
// (Surely there is a better name for this?)

int parse_i64 ( const char * str, long * val, int * inv );
int parse_u64 ( const char * str, unsigned long * val, int * inv );
int parse_flag ( const char * str, int * val, int * inv );

#endif

