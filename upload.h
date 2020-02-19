//
// Created by simon on 12.02.20.
//

#ifndef CATAJAX_UPLOAD_H
#define CATAJAX_UPLOAD_H

#include <glob.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>
#include <kcgi.h>

int upload(struct kreq* r);

int save_upload(const char *data, const char *filepath, size_t sz);

#endif //CATAJAX_UPLOAD_H
