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

char* calcImg(sqlite3 *db, int nr);
void logClient(struct kreq *r);
uint32_t getRandomNumber(uint32_t rangeStart, uint32_t rangeEnd);
char *imgToB64(const char *path, size_t *len);
int getMimeType(char *path);

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
    if (KCGI_OK != khttp_parse(&r, NULL, 0, &page, 1, 0)) {
        logEvent("khttp_parse failed!", ERROR);
        return -1;
    }

    khttp_head(&r, kresps[KRESP_STATUS], //200 OK
               "%s", khttps[KHTTP_200]);
    khttp_head(&r, kresps[KRESP_CONTENT_TYPE],
               "%s", kmimetypes[KMIME_TEXT_HTML]); //text/html
    khttp_body(&r);


    // Only HTTP Post allowed
    if(r.method != KMETHOD_POST) {
        khttp_puts(&r, "Only HTTP-POST request allowed");
        khttp_free(&r);
        return -1;
    }


    //Open Database
    sqlite3 *db; //Database handle

    if(openDb(&db) != 0) {
        logEventv2(ERROR,"Failed to open databse");
        return -1;
    }

    uint32_t catCount = getCatCountDb(db); //Number of pictures stored in database

    if(catCount <= 0) {
        logEventv2(ERROR, "getCatCountDb failed. Catcount: %d", catCount);
        return -1;
    }

    //Check content, which is sent with POST and deliver image tag

    for(int i = 0; i < r.fieldsz; i++) {
        if(strncmp(r.fields[i].val, "cat", 3) == 0) { //strncmp to prevent buffer-overflow
            size_t rand = getRandomNumber(1, catCount); //Random Number
#ifdef DEBUG
            khttp_puts(&r, "<pre>");
            khttp_puts(&r, "Random Number: ");
            char tmp[40] = {};
            sprintf(tmp,"%d", (int)rand);
            khttp_puts(&r, tmp);
            khttp_puts(&r, "</pre>");
#endif
            char* str = calcImg(db, (int)rand); //get image-tag based on random number
            khttp_puts(&r, str);
            updateClicks(db);
            free(str);
       } else {
           logClient(&r);
      }
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
 * Opens a image file and converts its data to base64
 * Warning: caller has to free resulting string
 */

char *imgToB64(const char *path, size_t *len) {
    char *buffer;

    FILE *file = fopen(path, "rb"); //Open file in binary read mode

    if(file == NULL) { //Check if file exists
        char tmp[512] = "";
        sprintf(tmp, "File: %s\n", path);
        logEventv2(ERROR,"File could not be opened[imgToBase64] File: %s", tmp);
        return NULL;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    long fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    buffer=(char *)malloc((size_t) (fileLen+1l));

    if (!buffer)
    {
        logEventv2(ERROR,"Memory Error[imgToBase64]");
        fclose(file);
        return NULL;
    }

    //Read file contents into buffer
    fread(buffer, (size_t) fileLen, 1, file);
    fclose(file);

    //Allocate Memory for b64 buffer

    *len = sodium_base64_ENCODED_LEN((size_t)fileLen, sodium_base64_VARIANT_ORIGINAL);

    char *b64 = malloc(*len * sizeof(char));

    //Convert to B64
    sodium_bin2base64(b64, sodium_base64_ENCODED_LEN((size_t)fileLen, sodium_base64_VARIANT_ORIGINAL), buffer, (size_t)fileLen, sodium_base64_VARIANT_ORIGINAL);

    free(buffer);
    return b64;

}

/*
 * Determines the mime type of an image based on it's file ending
 * Returns the index of the mime string in the kmimetypes[] array
 */

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

/*
 * Generates the whole image tag which can be sent to client
 * Warning: caller has to free resulting string
 */

char* calcImg(sqlite3 *db,int nr) {
    const char *rawTag = "<img src=\"data:%s;base64,%s\">";
    size_t len;

    //Get path string from database
    char *imgPath = queryDb(db, nr);

    //Convert image to base64
    size_t b64len;
    char *b64 = imgToB64(imgPath, &b64len);

    //Get mime type
    int mime = getMimeType(imgPath);

    if(mime < 0) {
        logEventv2(ERROR, "Could not determine mime type[calcImg]");
        return NULL;
    }

    size_t mimeLen = strlen(kmimetypes[mime]);
    char *mimeStr = malloc((mimeLen + 1) * sizeof(char));

    //Copy mime String from kcgi variable to local buffer
    strncpy(mimeStr,kmimetypes[mime], mimeLen);
    mimeStr[mimeLen] = '\0';

    //Calculate final length
    len = strlen(rawTag) + b64len + mimeLen;

    //Allocate Memory for the resulting String
    char *str = malloc((len + 1) * sizeof(char));

    //Format the String
    sprintf(str, rawTag, mimeStr ,b64);

    //Free all data and return final image-tag-string
    free(imgPath);
    free(b64);
    free(mimeStr);
    return str;
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
