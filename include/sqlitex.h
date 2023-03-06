#ifndef SQLITEX_H
#define SQLITEX_H

/*
    Given a path to a directory of SQL files of the form:

        001-users-table-add.sql
        002-settings-table-add.sql

    ...read them sequentially, apply them with exec() in a transaction,
    and use the SQLite "userversion" value to store the most-recently-applied
    one. Ignore files that don't match this pattern.

    Use "log" (also in this collection) to report.
*/
int sqlite_migrate ( sqlite3 * db, const char * dir );

#endif

