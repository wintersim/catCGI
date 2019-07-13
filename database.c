//
// Created by simon on 5/11/18.
//

#include "database.h"
#include "logger.h"

/*
 * Initializes the database handle and does some error checking
 */

int openDb(sqlite3 **db) {
    int rc;

    //Open database
    rc = sqlite3_open(DEFAULT_DATABASE_PATH, db);

    if(rc != SQLITE_OK) {
        sqlite3_close(*db);
        return -1;
    }
    return 0;
}

/*
 * Closes the database handle
 */

void closeDb(sqlite3 *db) {
    sqlite3_close(db);
}

/*
 * Searches database and returns the path of an image with given id
 * Warning: caller has to free resulting string
 */

char* queryDb(sqlite3 *db, int nr) {
    sqlite3_stmt *res; //statement to be sent to database
    char *retStr = NULL;
    int rc = 0;

    //Command wich will be executed
    //@num gets replaced with function attribute 'nr'
    char *sql = "SELECT NAME FROM CAT WHERE ID = @num;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL); //Convert statement to byte-code


    if(rc != SQLITE_OK) {
        logEventv2(ERROR,"Failed to fetch database data. Error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    else {
        int idx = sqlite3_bind_parameter_index(res, "@num"); //Get index of parameter ('@num')
        sqlite3_bind_int(res, idx, nr); //Set value of Parameter('@num') to nr
    }

    int step = sqlite3_step(res); //Execute the statement

    //Copy the image-path which is in column 0 in extra buffer
    if(step == SQLITE_ROW) {
        const char *pTemp = (const char*) sqlite3_column_text(res, 0);
        size_t len = strlen(pTemp);
        retStr = malloc((len + 1) * sizeof(char));
        strcpy(retStr, pTemp);
    }

    sqlite3_finalize(res);

    return retStr;
}

/*
 * Returns the number of cat pictures which are stored in the database
 */

uint32_t getCatCountDb(sqlite3 *db) {
    sqlite3_stmt *res;
    intmax_t iCount = -1;
    const char sql[] = "SELECT COUNT(Id) FROM Cat;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL); //Compile to byte-code

    if(rc != SQLITE_OK) {
        logEventv2(ERROR,"Failed to fetch cat count. Error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    //Run the statement
    rc = sqlite3_step(res); //Only one row --> need to run only once

    if(rc == SQLITE_ROW) {
        iCount = strtoimax((const char*) sqlite3_column_text(res,0), NULL, 10); //The number gets returned as string --> convert it to an int
        if(iCount > UINT32_MAX) { //Make sure that iCount(intmax_t) fits in uint32_t
            return 0;
        }
    }

    sqlite3_finalize(res);
    return (uint32_t)iCount;
}

void updateClicks(sqlite3 *db) {
    const char sql[] = "UPDATE Clicks SET Clicks = Clicks + 1 WHERE Name = \"cat\";";
    sqlite3_stmt *res;

    int rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL); //Compile to byte-code

    if(rc != SQLITE_OK) {
        logEventv2(ERROR,"Failed to prepare statement in updateClicks. Error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    //Run the statement
    rc = sqlite3_step(res);

    if(rc != SQLITE_DONE) {
        logEventv2(ERROR,"Failed to execute statement in updateClicks. Error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    sqlite3_finalize(res);
}
