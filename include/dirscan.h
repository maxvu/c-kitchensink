#ifndef DIRSCAN_H
#define DIRSCAN_H

#ifndef DIRSCAN_BUFLEN
#define DIRSCAN_BUFLEN 128
#endif

/*
    Wrapper for `scandir()`.
    Avoids some string manipulation.
    Currently, uses only UNIX path separator.

    All return values 0 on success.
*/

typedef struct dirscan {
    const char * rut; // initial dir path passed in
    dirent **    ent;
    int          max; // number of entries, total
    int          cur; // current index
    char         buf[ DIRSCAN_BUFLEN ]; // buffer
} dirscan;

// Initialize the struct. (No scan, no allocation.)
int dirscan_init ( dirscan * scn, const char * path );

// Perform scan on first step.
int dirscan_step ( dirscan * scn );

// Whether something went wrong on the first step.
// (In order to tell the difference between an
// empty directory and a failed scan.)
int dirscan_err ( dirscan * scn );

// The name of the file, without its path.
const char * dirscan_basename ( dirscan * scn );

// The path of the file, relative to the root.
const char * dirscan_fullpath ( dirscan * scn );

// The extension of the file. (The last dot and everything after.)
// Empty string on a valid name, NULL on an invalid one.
const char * dirscan_extension ( dirscan * scn );

// Free dirents.
void dirscan_destroy ( dirscan * scn );

#endif

