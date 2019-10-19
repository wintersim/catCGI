//
// Created by simon on 5/10/18.
//

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h> /* KCGI */
#include <stdio.h> /* printf,etc */
#include <stdlib.h> /* malloc */
#include <unistd.h> /* sleep */
#include <string.h> /* strlen, strcmp */
#include <math.h> /* log10 */
#include <sodium.h> /* random number */


#include "database.h" /* Sqlite database */
#include "logger.h" /* Event logger */

void logClient(struct kreq *r);
uint32_t getRandomNumber(uint32_t rangeStart, uint32_t rangeEnd);
int getMimeType(char *path);
char *openImage(sqlite3 *db, int nr, long *pInt, int *pInt1);

//#define DEBUG

#ifdef DEBUG

/*
 * Used to attach gdb to current process; for debugging
 */
//sudo gdb catAjax $(pgrep catAjax)

int wait = 1;

void wait_for_gdb_to_attach() { //DEBUG ONLY!
    while (wait) {
        sleep(1); // sleep for 1 second
    }
}
#endif


int main() {
#ifdef DEBUG
    wait_for_gdb_to_attach(); //DEBUG
#endif

    //Initialize KCGI
    struct kreq r;
    const char *page = "index";
    struct kvalid catKey = {kvalid_stringne, "type"};
    if (KCGI_OK != khttp_parse(&r, &catKey, 1, &page, 1, 0)) {
        logEvent("khttp_parse failed!", ERROR);
        return -1;
    }

    //Open Database
    sqlite3 *db;

    if(openDb(&db) != 0) {
        logEventv2(ERROR,"Failed to open databse");
        return -1;
    }

    uint32_t catCount = getCatCountDb(db); //Number of pictures stored in database

    if(catCount <= 0) {
        logEventv2(ERROR, "getCatCountDb failed. Catcount: %d", catCount);
        return -1;
    }

    // Only HTTP GET allowed
    if(r.method != KMETHOD_GET) {
        khttp_puts(&r, "Only HTTP-GET request allowed");
        khttp_free(&r);
        return -1;
    }


    //Check content, which is sent with GET and deliver image

    long fileLen = 0;
    int mimeType = 0;
    int validRequest = 0;

    struct kpair *p = NULL;

    if((p = r.fieldmap[0])) {
        if(!strncmp(p->parsed.s, "cat",3)) {
            validRequest = 1;
        }
    }

    if(validRequest) {
        size_t rand = getRandomNumber(1, catCount); //Random Number
#ifdef DEBUG
        khttp_puts(&r, "<pre>");
            khttp_puts(&r, "Random Number: ");
            char tmp[40] = {};
            sprintf(tmp,"%d", (int)rand);
            khttp_puts(&r, tmp);
            khttp_puts(&r, "</pre>");
#endif
        char *data = openImage(db, (int) rand, &fileLen, &mimeType); //get image-tag based on random number

        khttp_head(&r, kresps[KRESP_STATUS], //200 OK
                   "%s", khttps[KHTTP_200]);
        khttp_head(&r, kresps[KRESP_CONTENT_TYPE],
                   "%s", kmimetypes[mimeType]);
        khttp_head(&r, kresps[KRESP_CACHE_CONTROL], "no-store");
        khttp_body(&r);

        khttp_write(&r, data, fileLen);
        updateClicks(db);
        free(data);

    } else {
        khttp_write(&r, "\0",1);
        logClient(&r);
    }

#ifdef DEBUG
    for(int i = 0; i < r.reqsz; i++) {
        khttp_puts(&r, r.reqs[i].key);
        khttp_puts(&r, "=");
        khttp_puts(&r, r.reqs[i].val);
        khttp_puts(&r, "<br>");
    }
#endif

    closeDb(db);
    khttp_free(&r);
    return EXIT_SUCCESS;
}



/*
 * Determines the mime type of an image based on it's file ending
 * Returns the index of the mime string in the kmimetypes[] array
 */

//TODO Probably needs a rework, since it's mostly hacked together
//TODO Maybe use File desriptors, instead of file extension
int getMimeType(char *path) {
    char *delim = ".";
    char *ptr = NULL;
    char *cpy = NULL;
    enum kmime ret = -100;

    size_t len = strlen(path);
    cpy = malloc((len + 1) * sizeof(char));
    strncpy(cpy, path, len);

    ptr = strtok(cpy, delim);

    while(ptr != NULL) {
        ptr = strtok(NULL, delim);
        if(strncmp(ptr, "jpg", 3) == 0) {
            ret = KMIME_IMAGE_JPEG;
            break;
        } else if(strncmp(ptr, "png", 3) == 0) {
            ret = KMIME_IMAGE_PNG;
            break;
        }
    }
    free(cpy);
    return ret;
}

void logClient(struct kreq *r){
    char *ipAddr = 0;
    char *userAgent = 0;

    //Get Remote Address
    ipAddr = r->remote;

    //Get User-Agent
    for(int i = 0; i < r->reqsz; i++) {
        if(strcmp("User-Agent", r->reqs[i].key) == 0) {
            userAgent = r->reqs[i].val;
        }
    }

    logEventv2(WARNING, "%s - %s sent invalid request.", ipAddr, userAgent);
}


/*
 * Generates a random number using sodium library
 */
uint32_t getRandomNumber(uint32_t rangeStart, uint32_t rangeEnd){
    return randombytes_uniform(rangeEnd) + rangeStart;
}

char *openImage(sqlite3 *db, int nr, long *pInt, int *mimeType) {
    char *fileBuffer;

    char *path = queryDb(db, nr);

    if(path == NULL) {
        logEventv2(ERROR, "Failed to load path from db");
        return NULL;
    }

    *mimeType = getMimeType(path);

    FILE *file = fopen(path, "rb"); //Open file in binary read mode

    if(file == NULL) { //Check if file exists
        logEventv2(ERROR,"File could not be opened[imgToBase64] File: %s", path);
        return NULL;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    *pInt = ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    fileBuffer=(char *)malloc((size_t) ((*pInt)+1l));

    if (!fileBuffer)
    {
        logEventv2(ERROR,"Memory Error[openImage]");
        fclose(file);
        return NULL;
    }

    //Read file contents into buffer
    fread(fileBuffer, (size_t) *pInt, 1, file);
    fclose(file);

    free(path);
    return fileBuffer;
}
