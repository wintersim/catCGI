//
// Created by simon on 5/12/18.
//

#ifndef CATAJAX_LOGGER_H
#define CATAJAX_LOGGER_H

#include <stdio.h>
#include <time.h>
#include <errno.h>

#define DEFAULT_LOG_PATH "/var/log/caturbate/cgi.log"
#define MAX_TIMESTAMP_BUFFER 100
#define MAX_LOGTYPE_STRLEN 15

typedef enum {
    WARNING, ERROR
} logType;

void logEvent(const char *logStr, logType type);

#endif //CATAJAX_LOGGER_H
