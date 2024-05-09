/*
    SUBJECT: CS028 - Data Structures
    PROGRAMMER: Carl Jimroe M. Paño
    TEACHER: Neila Paglinawan-Muñez

    DATA STRUCTURES FINAL PROJECT:
    DIGITAL LIBRARY BOOK LENDING SYSTEM

    PROGRAM DESCRIPTION: This project serves as the final requirement in completion of the CS-028 Data Structures subject.
                         The DIGITAL LIBRARY BOOK LENDING SYSTEM is a console-based application program that can be used to
                         simplify the process of traditional book borrowing in the library. It integrates a file-based database
                         using SQLite for faster and more secure data storage. It implements the basic Queue data strcuture
                         to make the inserting and removing of elements or records in order just like on a line.
*/
//HEADERS USED:
#include <stdio.h>      //standard library header for C
#include <stdlib.h>     //standard library header for dynamic memory allocations
#include <stdbool.h>    //standard library for boolean
#include <string.h>     //string header for string manipulation functions
#include <ctype.h>      //for isspace function
#include <time.h>       //time header to get the current time from the system
#include <unistd.h>     //for sleep function
#include <sqlite3.h>    //SQLite database header imports necessary functions to communicate with the database

const int SECRET_KEY = 1234;    //modifiable password for truncating table, must only be known by the admin / librarian

 sqlite3 *connectToDatabase(); //function to open a connection to the SQLite database

typedef struct {               //strcuture to be used as the type inside the Queue
    char id[10];
    char name[30];
    char course[30];
    char book[30];
    struct tm datetime;       //tm structure from time header
} DETAILS;

typedef struct {
    DETAILS *data;            //using the DETAILS structure and is stored as an array
    int front, rear, size;    //Queue data structure's field
    unsigned capacity;
} Queue;

 //FUNCTIONS USED:

 int getUserChoice();                       //prompts the user for the selection of operation

 struct tm getCurrentTime();                //function to get the current time from the system

 Queue *createQueue(unsigned capacity);     //function that creates the Queue data structure

 int isFull(Queue *q);                      //checks to see if the Queue is on overflow

 int isEmpty(Queue *q);                     //checks to see if the Queue is underflow

 void enqueue(Queue *queue, sqlite3 *database);     //Queue operation to insert a record at the tail

 void dequeue(Queue *queue, sqlite3 *db);   //Queue operation to remove the record ats the front

 void printQueue(Queue *queue);             //function to print the current inserted records in the Queue

 void searchRecord(Queue *queue);           //added function to search a record in the queue with Student's ID Number

 void printDatabase();                      //uses SQL and SQLite functions to print the database on the console

 void truncateTable(const char *database);   //function to delete all tables from the database

 int enterSecretKey();                     //function to validate the ownership of the database

 void delay(int seconds);                   //delay function for printing period (".") for each 0.3 seconds

 void processDatabase(sqlite3 *database); //function to get the database operation

 void printDatabaseByStudentID(sqlite3 *database, const char *studentID);   //print the database with the student id as the argument

 void deleteRecord(sqlite3 *database);       //function to remove the record given the record number

 void printHelp();                          //function to show the manual of the program

 void printAtExit();                        //added function to just print something when user ends the program

int main() {

    sqlite3 *db = connectToDatabase();                      //establishes a database connection
    Queue *queue = createQueue(100);                        //uses the createQueue function create the Queue data structure

    int choice = 0;
    printf("  \t\t--[DIGITAL LIBRARY BOOK LENDING SYSTEM]--\n");
    while (choice != 8) {
        choice = getUserChoice();           //gets the user input
        switch (choice) {
            case 1:
                enqueue(queue, db);             //lets user input record fields
                break;
            case 2:
                dequeue(queue, db);         //removes the record from the Queue
                break;
            case 3:
                printQueue(queue);          //prints all the record from the Queue
                break;
            case 4:
                searchRecord(queue);
                break;
            case 5:
                printDatabase(db);          //shows all the saved record from the database
                break;
            case 6:
                processDatabase(db);
                break;
            case 7:
                printHelp();                //prints the user manual
                break;
            case 8:
                printAtExit();
                sqlite3_close(db);          //closes the database
                exit(0);                    //program terminates
                break;
            default:
                break;
        }
    }
    return 0;
}

int getUserChoice() {
    int choice;
    char input[256];
    printf("\n+-----------------------------MAIN MENU------------------------------+\n"
           "|1. ADD A RECORD                   5. PRINT DATABASE                 |\n|2. REMOVE A RECORD"
           "                6. PROCESS DATABASE               |\n|3. PRINT QUEUE                    7. HELP                           |"
           "\n|4. SEARCH RECORD ON QUEUE         8. EXIT                           |"
           "\n+--------------------------------------------------------------------+"
           "\n= ");

    fgets(input, sizeof(input), stdin);

    if (sscanf(input, "%d", &choice) != 1) {                //Check if the input is a number, sscanf parses the string
        printf("Please enter a number between 1 and 8.\n"); // If sscanf couldn't read a number, handle the error
        return -1;
    }

    if (choice < 1 || choice > 8) {                         // Check if the entered choice is between 1 and 6
        printf("Invalid choice. Please enter a number between 1 and 8.\n");
        return 0;
    }

    return choice;
}

struct tm getCurrentTime(){
    time_t currentTime;         //gets the current time from the system
    struct tm *localTime;
    currentTime = time(NULL);
    localTime = localtime(&currentTime);
    return *localTime;
};

Queue *createQueue(unsigned capacity) {
    Queue *queue = (Queue *)malloc(sizeof(Queue)); //dynamically allocates Queue
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = -1;                              //basic queue initializes rear to zero
    queue->data = (DETAILS *)malloc(queue->capacity * sizeof(DETAILS)); //dynamically allocates the array of DETAILS structure
    return queue;
}
int isFull(Queue *q) {
    return (q->size == q->capacity);       //checks for Queue overflow
}
int isEmpty(Queue *q) {
    return (q->size == 0);                 //checks for Queue underflow
}

void enqueue(Queue *queue, sqlite3 *database) {
    if (isFull(queue)) {
        printf("Queue is full, cannot enqueue.\n");
        return;
    }

    char newID[10];
    printf("\nEnter your student ID: ");
    fgets(newID, sizeof(newID), stdin);

    newID[strcspn(newID, "\n")] = '\0'; // Remove the newline character from the input

    // Check if the student ID already exists in the database
    char *sql_select = "SELECT Book FROM students WHERE StudentID = ?;";
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(database, sql_select, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
        return;
    }

    // Bind parameter to the prepared statement
    sqlite3_bind_text(stmt, 1, newID, -1, SQLITE_STATIC);

    // Execute the prepared statement
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // If the student ID exists in the database, prompt for the new book
        printf("Student ID %s is already in the database.\n", newID);
        printf("Enter the new book (or type 1 to cancel): ");
        fflush(stdout); // Flush the output buffer to ensure the message appears immediately without buffering
        fgets(queue->data[queue->size].book, 30, stdin);
        if (strcmp(queue->data[queue->size].book, "1\n") == 0) {
            printf("\nOperation cancelled.\n");
            sqlite3_finalize(stmt);
            return;
        }

        // Trim newline character from the book input
        queue->data[queue->size].book[strcspn(queue->data[queue->size].book, "\n")] = '\0';

        // Copy course from the existing record
        strncpy(queue->data[queue->size].course, (const char *)sqlite3_column_text(stmt, 0), sizeof(queue->data[queue->size].course) - 1);

        struct tm currentTime = getCurrentTime();
        memcpy(&(queue->data[queue->size].datetime), &currentTime, sizeof(struct tm));

        // Copy name and ID
        strncpy(queue->data[queue->size].name, "Name", sizeof(queue->data[queue->size].name) - 1); // Replace "Name" with the appropriate value
        strncpy(queue->data[queue->size].id, newID, sizeof(queue->data[queue->size].id) - 1);

        printf("\nNew book added for Student ID %s.\n", newID);
        queue->rear++;
        queue->size++;
    } else if (rc == SQLITE_DONE) {
        // If the student ID does not exist in the database, proceed with adding the record
        printf("Enter your name: ");
        fgets(queue->data[queue->size].name, 30, stdin);
        printf("Enter your course: ");
        fgets(queue->data[queue->size].course, 30, stdin);
        printf("Enter the book: ");
        fgets(queue->data[queue->size].book, 30, stdin);

        // Trim newline character from all inputs
        queue->data[queue->size].name[strcspn(queue->data[queue->size].name, "\n")] = '\0';
        queue->data[queue->size].course[strcspn(queue->data[queue->size].course, "\n")] = '\0';
        queue->data[queue->size].book[strcspn(queue->data[queue->size].book, "\n")] = '\0';

        struct tm currentTime = getCurrentTime();
        memcpy(&(queue->data[queue->size].datetime), &currentTime, sizeof(struct tm));

        strncpy(queue->data[queue->size].id, newID, sizeof(queue->data[queue->size].id) - 1);

        int cancel = 0;
        int validInput;

        do {
            printf("\n-------------------+\nNew Record Added   |\n-1. CANCEL RECORD  |");
            printf("\n 1. CONFIRM RECORD |\n-------------------+\n= ");

            validInput = scanf("%d", &cancel);

            if (validInput != 1) {
                printf("\nInvalid input. Please enter a number.\n");
                scanf("%*[^\n]"); // Clear the input buffer
                continue;
            }

            switch(cancel){
                case -1:
                    printf("\nUser cancelled operation..\n");
                    break;
                case 1:
                    printf("\nNew Record Enqueued!\n");
                    queue->rear++;
                    queue->size++;
                    break;
                default:
                    printf("\nPlease select from the two operations...\n");
            }
        } while (cancel != -1 && cancel != 1);

        int c;
        while ((c = getchar()) != '\n' && c != EOF); // clear input buffer
    } else {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
    }

    sqlite3_finalize(stmt);
}

void dequeue(Queue *queue, sqlite3 *db) {
    if (isEmpty(queue)) {
        printf("\nQueue is empty, cannot dequeue.\n");
        return;
    }

    printf("\nREMOVED RECORD:\n");
    printf("{\n    ID:      %s", queue->data[queue->front].id);
    printf("\n    Name:    %s", queue->data[queue->front].name);
    printf("    Course:  %s", queue->data[queue->front].course);
    printf("    Book:    %s", queue->data[queue->front].book);
    printf("    Date:    %d-%02d-%02d\n    Time:    %2d:%02d:%02d\n}", queue->data->datetime.tm_year + 1900,
           queue->data->datetime.tm_mon + 1, queue->data->datetime.tm_mday, queue->data->datetime.tm_hour,
           queue->data->datetime.tm_min, queue->data->datetime.tm_sec);

    int choose = 0;
    bool validInput = false;

    while (!validInput) {
        printf("\n1. Confirm Reservation\n2. Delete Reservation\n3. Cancel\n= ");
        char input[10];
        fgets(input, sizeof(input), stdin);
        choose = atoi(input);

        if (choose == 1 || choose == 2 || choose == 3) {
            validInput = true;
        } else {
            printf("Invalid input. Please enter 1, 2, or 3.\n");
        }
    }

    if (choose == 1) {
        // Insert the dequeued record into the database
        char *sql_insert = "INSERT INTO students (Name, Course, StudentID, Book, Date) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt *stmt;
        int rc;

        rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            return;
        }

        // Bind parameters to the prepared statement
        sqlite3_bind_text(stmt, 1, queue->data[queue->front].name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, queue->data[queue->front].course, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, queue->data[queue->front].id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, queue->data[queue->front].book, -1, SQLITE_STATIC);
        char date_str[20];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", &queue->data[queue->front].datetime);
        sqlite3_bind_text(stmt, 5, date_str, -1, SQLITE_STATIC);

        // Execute the prepared statement
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        } else {
            printf("Record confirmed and inserted into the database.\n");
        }

        // Finalize the statement
        sqlite3_finalize(stmt);
    } else if (choose == 3) {
        printf("\nOperation cancelled\n");
        fflush(stdin);
        return;
    }

    for (int i = queue->front; i < queue->rear; ++i) {
        memcpy(&(queue->data[i]), &(queue->data[i + 1]), sizeof(DETAILS));
    }

    queue->rear--;
    queue->size--;
    printf("Record successfully dequeued!\n");
}

void printQueue(Queue *queue) {
    if (isEmpty(queue)) {
        printf("\nQueue is empty.\n");
        return;
    }

    printf("\nPrinting Queue");
    delay(1);
    printf("\n\n----- Start of Queue -----\n");

    for (int i = queue->front; i <= queue->rear; i++) {
        printf("\n-------------------\n");
        printf("QUEUE NO. %d:\n", i - queue->front + 1);
        printf("-------------------\n");
        printf("ID:      %s\n", queue->data[i].id);
        printf("Name:    %s\n", queue->data[i].name);
        printf("Course:  %s\n", queue->data[i].course);
        printf("Book:    %s\n", queue->data[i].book);
        printf("Date:    %d-%02d-%02d\nTime:    %02d:%02d:%02d\n",
               queue->data[i].datetime.tm_year + 1900,
               queue->data[i].datetime.tm_mon + 1,
               queue->data[i].datetime.tm_mday,
               queue->data[i].datetime.tm_hour,
               queue->data[i].datetime.tm_min,
               queue->data[i].datetime.tm_sec);
    }

    printf("\n----- End of Queue -----\n");
}


sqlite3 *connectToDatabase() {
    sqlite3 *db;
    char *erMsg = 0;
    int rc;
    rc = sqlite3_open("librarydb.db", &db);     //passing the name of the database file
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));   //prints the error message in opening the database
        exit(1);
    }

    char *sql_create = "CREATE TABLE IF NOT EXISTS students ("     //SQL query to create the table if it does not exist
                       "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "Name TEXT NOT NULL, Course TEXT NOT NULL, StudentID TEXT NOT NULL, Book TEXT NOT NULL, Date TEXT NOT NULL);";

    rc = sqlite3_exec(db, sql_create, 0, 0, &erMsg);    //executes the query
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", erMsg);      //prints error message if there is any
        sqlite3_free(erMsg);
        exit(1);
    }

    return db;
}

void searchRecord(Queue *queue) {
    int searchRec;
    char input[256];

    printf("\n1. Search by Student ID\n2. Search by Book Name\n");
    printf("= ");

    // Input validation for integer input
    while (fgets(input, sizeof(input), stdin)) {
        if (sscanf(input, "%d", &searchRec) == 1) {
            if (searchRec == 1 || searchRec == 2)
                break; // Valid input, break the loop
            else
                printf("Invalid choice. Please enter 1 or 2: ");
        } else {
            printf("Invalid input. Please enter an integer: ");
        }
    }

    if (searchRec == 1) {
        char searchId[10];
        printf("\nEnter the Student ID: ");
        fgets(searchId, sizeof(searchId), stdin);

        searchId[strcspn(searchId, "\n")] = '\0';   // Remove the newline character if present
        printf("\n--Printing Records for Student ID: %s--\n", searchId);

        int found = 0;
        for (int i = queue->front; i <= queue->rear; i++) {
            if (strcmp(queue->data[i].id, searchId) == 0) { // Compare strings using strcmp
                found = 1;
                printf("\n-------------------\nISSUE NO. %d:      |\n-------------------\n", i - queue->front + 1);
                printf("ID:      %s", queue->data[i].id);
                printf("\nName:    %s", queue->data[i].name);
                printf("Course:  %s", queue->data[i].course);
                printf("Book:    %s", queue->data[i].book);

                printf("Date:    %d-%02d-%02d\nTime:    %02d:%02d:%02d\n",          // Print date and time
                       queue->data[i].datetime.tm_year + 1900,
                       queue->data[i].datetime.tm_mon + 1,
                       queue->data[i].datetime.tm_mday,
                       queue->data[i].datetime.tm_hour,
                       queue->data[i].datetime.tm_min,
                       queue->data[i].datetime.tm_sec);
            }
        }
        if (!found) {
            printf("No records found for Student ID '%s'\n", searchId);
        }
    }else if (searchRec == 2) {
        // Search by Book Name
        char searchBook[30];
        printf("\nEnter the Book Name: ");
        fgets(searchBook, sizeof(searchBook), stdin);

        // Remove newline character from searchBook
        searchBook[strcspn(searchBook, "\n")] = '\0';

        // Trim leading and trailing spaces from searchBook
        char trimmedSearchBook[30];
        int len = strlen(searchBook);
        int start = 0, end = len - 1;
        while (isspace(searchBook[start])) {
            start++;
        }
        while (end >= 0 && isspace(searchBook[end])) {
            end--;
        }
        strncpy(trimmedSearchBook, searchBook + start, end - start + 1);
        trimmedSearchBook[end - start + 1] = '\0';

        printf("\n--Printing Records for Book: %s--\n", trimmedSearchBook);

        int found = 0;
        for (int i = queue->front; i <= queue->rear; i++) {
            char tempBook[30];
            strncpy(tempBook, queue->data[i].book, sizeof(tempBook)); // Make a temporary copy to trim
            tempBook[sizeof(tempBook) - 1] = '\0'; // Ensure null-termination

            // Trim leading and trailing spaces from tempBook
            char trimmedTempBook[30];
            int tempLen = strlen(tempBook);
            int tempStart = 0, tempEnd = tempLen - 1;
            while (isspace(tempBook[tempStart])) {
                tempStart++;
            }
            while (tempEnd >= 0 && isspace(tempBook[tempEnd])) {
                tempEnd--;
            }
            strncpy(trimmedTempBook, tempBook + tempStart, tempEnd - tempStart + 1);
            trimmedTempBook[tempEnd - tempStart + 1] = '\0';

            if (strcmp(trimmedTempBook, trimmedSearchBook) == 0) { // Compare trimmed book names using strcmp
                found = 1;
                // Print the record details
                printf("\n-------------------\nISSUE NO. %d:      |\n-------------------\n", i - queue->front + 1);
                printf("ID:      %s", queue->data[i].id);
                printf("\nName:    %s", queue->data[i].name);
                printf("Course:  %s", queue->data[i].course);
                printf("Book:    %s", queue->data[i].book);

                printf("Date:    %d-%02d-%02d\nTime:    %02d:%02d:%02d\n",          // Print date and time
                       queue->data[i].datetime.tm_year + 1900,
                       queue->data[i].datetime.tm_mon + 1,
                       queue->data[i].datetime.tm_mday,
                       queue->data[i].datetime.tm_hour,
                       queue->data[i].datetime.tm_min,
                       queue->data[i].datetime.tm_sec);
            }
        }
        if (!found) {
            printf("No records found for the book '%s'\n", trimmedSearchBook);
        }
    } else {
        printf("Invalid choice.\n");
    }
}

void printDatabase(sqlite3 *db) {
    void trimNewline(char *str) {
        char *newline = strchr(str, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }
    }
    char *sql_select = "SELECT id, StudentID, Name, Course, Book, Date FROM students;";
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return;
    }
    printf("\n+---------------------------------+");
    printf("\n|       RESERVATION DATABASE      |");
    printf("\n+---------------------------------+\n\n");
    printf("%-12s %-20s %-20s %-10s %-30s %-20s\n", "Record No.", "StudentID", "Name", "Course", "Book", "Date & Time");
    printf("%-12s %-20s %-20s %-10s %-30s %-20s\n", "----------", "--------------------", "--------------------", "----------", "------------------------------", "--------------------");

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *studentID = sqlite3_column_text(stmt, 1);
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *course = sqlite3_column_text(stmt, 3);
        const unsigned char *book = sqlite3_column_text(stmt, 4);
        const unsigned char *date = sqlite3_column_text(stmt, 5);

        // Trim newline characters from each string
        char name_trimmed[30], course_trimmed[30], studentID_trimmed[10], book_trimmed[30], date_trimmed[20];
        snprintf(studentID_trimmed, sizeof(studentID_trimmed), "%s", studentID);
        snprintf(name_trimmed, sizeof(name_trimmed), "%s", name);
        snprintf(course_trimmed, sizeof(course_trimmed), "%s", course);
        snprintf(book_trimmed, sizeof(book_trimmed), "%s", book);
        snprintf(date_trimmed, sizeof(date_trimmed), "%s", date);

        trimNewline(studentID_trimmed);
        trimNewline(name_trimmed);
        trimNewline(course_trimmed);
        trimNewline(book_trimmed);
        trimNewline(date_trimmed);

        printf("%-12d %-20s %-20s %-10s %-30s %-20s\n",
               id,
               studentID_trimmed,
               name_trimmed,
               course_trimmed,
               book_trimmed,
               date_trimmed);
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    }
}


void truncateTable(const char *database) {
    sqlite3 *db;
    char *erMsg = 0;
    int rc;

    // Open database
    rc = sqlite3_open(database, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    } else {
        fprintf(stderr, "Opening database");
        delay(1);
    }

    // Truncate table
    char *sql_truncate = "DELETE FROM students; DELETE FROM sqlite_sequence WHERE name = 'students';";

    rc = sqlite3_exec(db, sql_truncate, 0, 0, &erMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", erMsg);
        sqlite3_free(erMsg);
    } else {
        fprintf(stdout, "\n\nTABLE TRUNCATED SUCCESSFULLY!\n");
    }

    // Close database
    sqlite3_close(db);
}

int enterSecretKey(){
    int key;
    printf("\nEnter the secret key: ");
    scanf("%d",&key);
    if(key == SECRET_KEY){
        fflush(stdin);
        return 1;
    }else{
        printf("\nEntered key is incorrect\n");
        fflush(stdin);
        return -1;
    }
}

void trimNewline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void processDatabase(sqlite3 *database) {
    int dbOp;
    printf("\nPLEASE SELECT OPERATION\n1. Search a record\n2. Delete a record\n3. Delete all records\n= ");

    char input[256];
    fgets(input, sizeof(input), stdin);

    if (sscanf(input, "%d", &dbOp) != 1) {
        printf("Invalid input. Please enter a number.\n");
        return;
    }

    switch (dbOp) {
        case 1:
            // Implement search by Student ID
            break;
        case 2:
            deleteRecord(database);
            break;
        case 3:
            if (enterSecretKey() > 0) {
                truncateTable("librarydb.db");
            }
            break;
        default:
            printf("Invalid option. Please enter 1, 2, or 3.\n");
            break;
    }
}


void printDatabaseByStudentID(sqlite3 *database, const char *studentID) {
    // Function to trim newline characters from a string
    void trimNewline(char *str) {
        char *newline = strchr(str, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }
    }

    sqlite3_stmt *stmt;
    int rc;

    char *sql_select = "SELECT id, StudentID, Name, Course, Book, Date FROM students WHERE StudentID = ?;";
    rc = sqlite3_prepare_v2(database, sql_select, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
        return;
    }

    // Bind parameter to the prepared statement
    sqlite3_bind_text(stmt, 1, studentID, -1, SQLITE_STATIC);

    printf("\n+---------------------------------+");
    printf("\n|       RESERVATION DATABASE      |");
    printf("\n+---------------------------------+\n\n");
    printf("%-12s %-20s %-20s %-10s %-30s %-20s\n", "Record No.", "StudentID", "Name", "Course", "Book", "Date & Time");
    printf("%-12s %-20s %-20s %-10s %-30s %-20s\n", "----------", "--------------------", "--------------------", "----------", "------------------------------", "--------------------");

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *studentID = sqlite3_column_text(stmt, 1);
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *course = sqlite3_column_text(stmt, 3);
        const unsigned char *book = sqlite3_column_text(stmt, 4);
        const unsigned char *date = sqlite3_column_text(stmt, 5);

        // Trim newline characters from each string
        char name_trimmed[30], course_trimmed[30], studentID_trimmed[10], book_trimmed[30], date_trimmed[20];
        snprintf(studentID_trimmed, sizeof(studentID_trimmed), "%s", studentID);
        snprintf(name_trimmed, sizeof(name_trimmed), "%s", name);
        snprintf(course_trimmed, sizeof(course_trimmed), "%s", course);
        snprintf(book_trimmed, sizeof(book_trimmed), "%s", book);
        snprintf(date_trimmed, sizeof(date_trimmed), "%s", date);

        trimNewline(studentID_trimmed);
        trimNewline(name_trimmed);
        trimNewline(course_trimmed);
        trimNewline(book_trimmed);
        trimNewline(date_trimmed);

        printf("%-12d %-20s %-20s %-10s %-30s %-20s\n",
               id,
               studentID_trimmed,
               name_trimmed,
               course_trimmed,
               book_trimmed,
               date_trimmed);
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
    }
}

void deleteRecord(sqlite3 *database) {
    int recordNo;

    printf("\nEnter the Record Number to delete: ");
    scanf("%d", &recordNo);
    fflush(stdin);

    char *sql_delete = "DELETE FROM students WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(database, sql_delete, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
        return;
    }

    // Bind parameter to the prepared statement
    sqlite3_bind_int(stmt, 1, recordNo);

    // Execute the prepared statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
        return;
    }

    // Check if any rows were affected
    if (sqlite3_changes(database) > 0) {
        printf("Record deleted successfully.\n");
    } else {
        printf("Record not found or deletion unsuccessful.\n");
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    // Check if the table is empty
    int rowCount;
    sqlite3_stmt *countStmt;
    char *sql_count = "SELECT COUNT(*) FROM students;";
    rc = sqlite3_prepare_v2(database, sql_count, -1, &countStmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
        return;
    }
    rc = sqlite3_step(countStmt);
    if (rc == SQLITE_ROW) {
        rowCount = sqlite3_column_int(countStmt, 0);
    }
    sqlite3_finalize(countStmt);

    // If the table is empty, reset the record number to zero
    if (rowCount == 0) {
        char *sql_reset = "DELETE FROM sqlite_sequence WHERE name = 'students';";
        rc = sqlite3_exec(database, sql_reset, 0, 0, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(database));
            return;
        }
    }
}

void delay(int seconds) { //function for visual aids enhancements in the program
    int i;
    for (i = 0; i < seconds * 5; i++) { // Multiply seconds by 10 for 0.3 second intervals
        printf(".");
        fflush(stdout); // Flush the output buffer to ensure the period is printed immediately
        usleep(300000); // Sleep for 0.3 seconds (300 milliseconds)
    }
}

void printHelp(){ //prints the user manual to the console
    printf("\nThis program serves as a tool to make book borrowing faster\n\nType 1 to enter the borrower's information\n"\
           "Type 2 to process confirmation of the book to be borrowed\nType 3 to print the list of unprocessed book requests\n"\
           "Type 4 to print the saved record from the database\nType 5 to show the database\nType 6 to delete all the rows on the database table\n\n");
}

void printAtExit(){
    printf("\n****************************************************\n*              Program Terminates...               *\n*"
           "                                                  *"
           "\n* THANK YOU FOR USING MY CONSOLE APP! :D - Carl P. *\n*************"
           "***************************************\n");
}
