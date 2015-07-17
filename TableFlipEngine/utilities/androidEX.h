#ifndef _ANDROID_EX_H
#define _ANDROID_EX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
FILE *fmemopen(void *buf, size_t size, const char *mode);

#ifdef __cplusplus
} // extern "C"
#endif
#endif