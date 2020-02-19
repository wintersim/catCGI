//
// Created by simon on 13.02.20.
//

#include <string.h>
#include <stdint.h>
#include <malloc.h>
#include <sodium/randombytes.h>
#include "util.h"
#include "logger.h"


/*
 * Function logs IP address and User-Agent from the client
 */

void logClient(struct kreq *r) {
    const char *ipAddr = 0;
    const char *userAgent = 0;

    //Get Remote Address
    ipAddr = r->remote;

    userAgent = getHeaderField(r, "User-Agent");

    logEventv2(WARNING, "%s - %s sent invalid request.", ipAddr, userAgent);
}

/*
 * Function returns the value of the requested HTTP header field
 */

const char *getHeaderField(const struct kreq *r, const char *search) {
    const char *ret = NULL;
    for (int i = 0; i < r->reqsz; i++) {
        if (strcmp(search, r->reqs[i].key) == 0) {
            ret = r->reqs[i].val;
        }
    }
    return ret;
}

/*
 * Constructs a random Path: <pathPrefix>/(random).<fileSuffix>
 * Warning: Caller has to free() resulting string
 */

char *genRandPath(const char *pathPrefix, const char *fileSuffix, size_t randLen) {
    const char format[] = "%s/%s.%s"; //prefix/random.suffix
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    char *randBuffer = NULL;
    char *pathBuffer = NULL;
    long totalLen = 0;

    //generate random string
    randBuffer = malloc((randLen + 1) * sizeof(char));

    for (int i = 0; i < randLen; ++i) {
        randBuffer[i] = alphanum[randombytes_random() % (sizeof(alphanum) - 1)];
    }

    randBuffer[randLen] = 0;

    totalLen = strlen(format) + randLen + strlen(pathPrefix) + strlen(fileSuffix) + 1;
    pathBuffer = malloc(totalLen * sizeof(char));

    sprintf(pathBuffer, format, pathPrefix, randBuffer, fileSuffix);
    free(randBuffer);

    return pathBuffer;
}
