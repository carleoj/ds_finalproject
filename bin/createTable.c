#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

struct tm getCurrentTime(){
    time_t currentTime;
    struct tm *localTime;
    currentTime = time(NULL);
    localTime = localtime(&currentTime);
    return *localTime;
}

int main() {
    sqlite3 *db;
    char *erMsg = 0;
    int rc;
    struct tm localtime;
    localtime = getCurrentTime();

    // Open database
    rc = sqlite3_open("librarydb.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    // Create table
    char *sql_create = "CREATE TABLE IF NOT EXISTS students (" \
                       "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
                       "Name TEXT NOT NULL, Course TEXT NOT NULL, StudentID TEXT NOT NULL, Book TEXT NOT NULL, Date TEXT NOT NULL);";

    rc = sqlite3_exec(db, sql_create, 0, 0, &erMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", erMsg);
        sqlite3_free(erMsg);
    } else {
        fprintf(stdout, "Table created successfully\n");
    }

    // Convert localtime to string
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &localtime);

    // Insert values into the table
    char sql_insert[200];
    sprintf(sql_insert, "INSERT INTO students (Name, Course, StudentID, Book, Date) VALUES ('Carl', 'BSCS', '22-0252', 'Bible', '%s');", time_str);

    rc = sqlite3_exec(db, sql_insert, 0, 0, &erMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", erMsg);
        sqlite3_free(erMsg);
    } else {
        fprintf(stdout, "Values inserted successfully\n");
    }

    // Close database
    sqlite3_close(db);

    return 0;
}
