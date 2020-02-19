//
// Created by simon on 12.02.20.
//

#include <string.h>
#include <sqlite3.h>
#include <sodium/randombytes.h>
#include "rndcat.h"
#include "database.h"
#include "logger.h"
#include "util.h"

int getMimeType(char *path);

uint32_t getRandomNumber(uint32_t rangeStart, uint32_t rangeEnd);

char *openImage(sqlite3 *db, int nr, long *fileLen, int *mimeType);



/*
 * Function sends a random cat picture from the database to the client
 * Returns a status code
 */

int randcat(struct kreq *req) {
    struct kreq r = *req;

    long fileLen = 0;
    int mimeType = 0;
    int validRequest = 0;

    struct kpair *p;
    if ((p = r.fieldmap[0])) {
        if (!strncmp(p->parsed.s, "cat", 3)) {
            validRequest = 1;
        }
    }

    sqlite3 *db = NULL;

    if (validRequest) {

        //Open Database
        if (openDb(&db) != 0) {
            logEventv2(ERROR, "Failed to open database");
            return -1;
        }

        uint32_t catCount = getCatCountDb(db); //Number of pictures stored in database

        if (catCount <= 0) {
            logEventv2(ERROR, "getCatCountDb failed. Catcount: %d", catCount);
            closeDb(db);
            return -1;
        }

        size_t rand = getRandomNumber(1, catCount); //Random Number

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
        khttp_write(&r, "\0", 1);
        logClient(&r);
        closeDb(db);
        return -1;
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
    return 0;
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

    while (ptr != NULL) {
        ptr = strtok(NULL, delim);
        if (strncmp(ptr, "jpg", 3) == 0) {
            ret = KMIME_IMAGE_JPEG;
            break;
        } else if (strncmp(ptr, "png", 3) == 0) {
            ret = KMIME_IMAGE_PNG;
            break;
        }
    }
    free(cpy);
    return ret;
}


/*
 * Generates a random number using sodium library
 */

uint32_t getRandomNumber(uint32_t rangeStart, uint32_t rangeEnd) {
    return randombytes_uniform(rangeEnd) + rangeStart;
}

/*
 * Gets the filepath from the image with id 'nr' from the database then loads the file into memory
 * Additionally the mime-type is determined
 */

char *openImage(sqlite3 *db, int nr, long *fileLen, int *mimeType) {
    char *fileBuffer;

    char *path = queryDb(db, nr);

    if (path == NULL) {
        logEventv2(ERROR, "Failed to load path from db");
        return NULL;
    }

    *mimeType = getMimeType(path);

    FILE *file = fopen(path, "rb"); //Open file in binary read mode

    if (file == NULL) { //Check if file exists
        logEventv2(ERROR, "File could not be opened[imgToBase64] File: %s", path);
        return NULL;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    *fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    fileBuffer = (char *) malloc((size_t) ((*fileLen) + 1l));

    if (!fileBuffer) {
        logEventv2(ERROR, "Memory Error[openImage]");
        fclose(file);
        return NULL;
    }

    //Read file contents into buffer
    fread(fileBuffer, (size_t) *fileLen, 1, file);
    fclose(file);

    free(path);
    return fileBuffer;
}



