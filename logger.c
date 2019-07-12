//
// Created by simon on 5/12/18.
//

#include <string.h>
#include <malloc.h>
#include "logger.h"


/*
 * Writes a message with date and time to a logfile
 */
void logEvent(const char *logStr, logType type){
    const char rawLog[]  = "[%s] <%s> - %s"; //Log format
    const char warning[] = "Warning";
    const char error[]   = "Error";

    char timestamp[MAX_TIMESTAMP_BUFFER] = ""; //Buffer for timestamp
    char *log; //log string
    char typeBuffer[MAX_LOGTYPE_STRLEN]; //pointer to logtype-string for easier use
    size_t logSize; //size of log string

    FILE* logFile;
    time_t t;
    struct tm *timeInfo;


    //Open log file in append mode
    logFile = fopen(DEFAULT_LOG_PATH, "a");

    if(logFile == NULL) {
        return;
    }

    //Generate date and time string based on current time
    t = time(NULL);
    timeInfo = localtime(&t);
    strftime(timestamp, MAX_TIMESTAMP_BUFFER,"%x at %X", timeInfo);

    //Set log type
    if(type == WARNING) {
        strcpy(typeBuffer, warning);
    } else if(type == ERROR) {
        strcpy(typeBuffer, error);
    }

    //Allocate memory for log string
    logSize = strlen(rawLog) + strlen(timestamp) + strlen(typeBuffer) + strlen(logStr) + 2; //+ '\n'

    log = malloc(logSize * sizeof(char));

    //format log string (date - message) and write it to file
    sprintf(log, rawLog, timestamp, typeBuffer, logStr);
    fprintf(logFile, "%s\n", log);

    fclose(logFile);
    free(log);
}

/*
 * Writes formatted message to a logfile
 */

void logEventv2(logType type, const char* format, ...) {
    va_list list;
    int totalLen;
    char str[MAX_LOG_REASON_LEN];

    va_start(list, format);
    vsnprintf(str,MAX_LOG_REASON_LEN,format, list);
    va_end(list);
    logEvent(str, type);
}
