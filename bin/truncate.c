#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;
    char *erMsg = 0;
    int rc;

    // Open database
    rc = sqlite3_open("librarydb.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    // Truncate table
    char *sql_truncate = "DELETE FROM students";

    rc = sqlite3_exec(db, sql_truncate, 0, 0, &erMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", erMsg);
        sqlite3_free(erMsg);
    } else {
        fprintf(stdout, "All rows truncated successfully\n");
    }

    // Close database
    sqlite3_close(db);

    return 0;
}
