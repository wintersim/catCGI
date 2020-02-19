//
// Created by simon on 19.02.20.
//

#include <zconf.h>
#include <kcgiregress.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <string.h>
#include "catCGI.h"

//#define DEBUG

#ifdef DEBUG

/*
 * Used to attach gdb to current process; for debugging
 */
//sudo gdb catAjax $(pgrep catAjax)

static int wait = 1;

void wait_for_gdb_to_attach() { //DEBUG ONLY!
    fprintf(stderr, "WAITING FOR GDB TO ATTACH\n");
    while (wait) {
        sleep(1); // sleep for 1 second
    }
}

#endif

//#define RUN_TEST

#ifdef RUN_TEST

#define UPLOAD_TST_IMAGE_PATH "/tmp/image.jpg"
#define TEST_URL_UPLOAD "http://localhost:17123/cgi-bin/catAjax"
#define TEST_URL_RANDOM "http://localhost:17123/cgi-bin/catAjax?type=cat"

/*
 * All functions that work with cURL are mostly copied together from various tutorials :^)
 */

int parent_upload() {
    struct curl_httppost *post, *last;
    int rc;
    post = last = NULL;

    CURL *curl = curl_easy_init();

    if (curl) {
        curl_formadd(&post, &last, CURLFORM_COPYNAME,
                     "img", CURLFORM_FILE, UPLOAD_TST_IMAGE_PATH,
                     CURLFORM_END);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_URL,
                         TEST_URL_UPLOAD);
        rc = curl_easy_perform(curl);

        if (rc != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(rc));
        }

        curl_formfree(post);
        return (CURLE_OK == rc);
    }
    return 0;
}

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int parent_rndCat() {
    CURL *curl_handle;
    CURLcode res;
    int nerr = 0;

    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, TEST_URL_RANDOM);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        nerr = 0;
    }
    else {
        printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
        chunk.memory[chunk.size] = 0;
        printf("Data: %s\n", chunk.memory);
        nerr = 1;
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
    return nerr;
}


#endif

int main(int argc, char **argv) {
#ifdef DEBUG
    wait_for_gdb_to_attach(); //DEBUG
#endif

#ifdef RUN_TEST
    return kcgi_regress_cgi(parent_rndCat, NULL, run, NULL) ? EXIT_SUCCESS : EXIT_FAILURE;
#else
    return run();
#endif
}
