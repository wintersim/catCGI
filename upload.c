//
// Created by simon on 12.02.20.
//

#include "upload.h"
#include "util.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>

int save_upload(const char *data, const char *filepath, size_t sz) {
    int error = 0;
    FILE *file = NULL;

    if(data != NULL && filepath != NULL && sz > 0) {
        file = fopen(filepath,"wb");
        if (file != NULL) {
            fwrite(data, 1, sz, file);
        } else {
            error = -1;
        }
    } else {
        error = -1;
    }

    fclose(file);

    return error;
}

/*
 * Functions receives a request with a file attached and stores the attached file
 * Returns a status code
 */

int upload(struct kreq *r) {
    //Check if an image was sent

    int validRequest = -1;

    //r->fieldmap[1]->file --> filename
    //sr->fieldmap[1]->ctype --> mimetype



    if (!strncmp(r->fieldmap[1]->key, "img", 3)) { //fieldmap[1] is the image key, as specified in main.c
        validRequest = 0;

        char suffix[15] = "";
        strcpy(suffix, ksuffixes[r->fieldmap[1]->ctypepos]);

        char *rndPath = genRandPath("/tmp", suffix, 8);

        if(!save_upload(r->fieldmap[1]->val, rndPath, r->fieldmap[1]->valsz)) {
            khttp_head(r, kresps[KRESP_STATUS],
                       "%s", khttps[KHTTP_200]);
            khttp_head(r, kresps[KRESP_CONTENT_TYPE],
                       "%s", kmimetypes[KMIME_TEXT_HTML]);
            khttp_body(r);
            khttp_puts(r, "<h1>Upload successful!</h1>");
        } else {
            khttp_head(r, kresps[KRESP_STATUS],
                       "%s", khttps[KHTTP_500]);
            validRequest = -1;
        }

        free(rndPath);
    }
    return validRequest;
}
