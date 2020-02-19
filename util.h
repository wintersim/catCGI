//
// Created by simon on 13.02.20.
//

#ifndef CATAJAX_UTIL_H
#define CATAJAX_UTIL_H

#include <stdarg.h>
#include <sys/types.h>
#include <kcgi.h>

const char *getHeaderField(const struct kreq *r, const char *search);
void logClient(struct kreq *r);
char* genRandPath(const char* pathPrefix,const char* fileSuffix, size_t randLen);

#endif //CATAJAX_UTIL_H
