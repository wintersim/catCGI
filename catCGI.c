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
#include "rndcat.h"
#include "upload.h"


int kvalid_everything(struct kpair *p) {
    return 1;
}

int run() {
    //Initialize KCGI
    struct kreq r;
    const char *page = "index";
    struct kvalid catKey[] = {
            {kvalid_stringne, "type"},
            {kvalid_everything, "img"} //Validator accepts everything
    };

    if (KCGI_OK != khttp_parse(&r, catKey, 2, &page, 1, 0)) {
        logEvent("khttp_parse failed!", ERROR);
        return -1;
    }

    // Only HTTP GET & POST allowed
    if (!(r.method == KMETHOD_POST || r.method == KMETHOD_GET)) {
        khttp_head(&r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_405]);
        khttp_free(&r);
        return -1;
    }

    //If the method is POST, we assume it's an upload
    //If it's GET we assume it's a random cat request
    //Content is of course checked

    if (r.method == KMETHOD_GET) {
        randcat(&r);
    } else if (r.method == KMETHOD_POST) {
        upload(&r);
    }

    khttp_free(&r);
    return EXIT_SUCCESS;
}


