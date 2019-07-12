//
// Created by simon on 5/11/18.
//

#ifndef CATAJAX_DATABASE_H
#define CATAJAX_DATABASE_H

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>


#define DEFAULT_DATABASE_PATH "/var/lib/sqlite/cat.db"

int openDb(sqlite3**);
void closeDb(sqlite3*);
char* queryDb(sqlite3*,int);
uint32_t getCatCountDb(sqlite3*);
void updateClicks(sqlite3*);

#endif //CATAJAX_DATABASE_H
