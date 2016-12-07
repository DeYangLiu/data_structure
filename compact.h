#ifndef _COMPACT_LAYER_H_
#define _COMPACT_LAYER_H_

typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned long   U32;
typedef unsigned long long U64;

#include <stdio.h>
#define LOG(fmt, ...) fprintf(stdout, "[%s %d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#endif
