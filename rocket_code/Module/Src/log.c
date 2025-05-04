/*
 * log.c
 *
 *  Created on: April 28, 2025
 *      Author: Dat Hoang
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"
#include "util.h"
#include "device.h"

#if DEBUG_LEVEL > 0
static uint32_t LOGMASK = TAG_FILENO;


void set_logging_mask(uint32_t mask)
{
    LOGMASK = mask;
}


struct logtag
{
    uint32_t tagn;
    const char * tag;
};

struct logtag tagtable[] = {
    {TAG_ERR,"ERR"},
    {TAG_USB,"USB"},
    {TAG_GREEN,"[1;32mDEBUG[0m"},
    {TAG_RED,"[1;31mDEBUG[0m"}
};


__attribute__((weak)) void set_logging_tag(uint32_t tag)
{
    // nothing
}

void LOG(uint32_t tag, const char * filename, int num, const char * fmt, ...)
{
    unsigned int i;

    if (((tag & 0x7fffffff) & LOGMASK) == 0)
    {
        return;
    }
    for (i = 0; i < sizeof(tagtable)/sizeof(struct logtag); i++)
    {
        if (tag & tagtable[i].tagn)
        {
            if (tagtable[i].tag[0] && !(tag & TAG_NO_TAG)) printf("[%s] ", tagtable[i].tag);
            i = 0;
            break;
        }
    }
    if (i != 0)
    {
        printf2(TAG_ERR,"INVALID LOG TAG\n");
        exit(1);
    }
    set_logging_tag(tag);
#ifdef ENABLE_FILE_LOGGING
    if (tag & TAG_FILENO)
    {
        printf("%s:%d: ", filename, num);
    }
#endif
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void LOG_HEX(uint32_t tag, uint8_t * data, int length)
{
    if (((tag & 0x7fffffff) & LOGMASK) == 0)
    {
        return;
    }
    set_logging_tag(tag);
    dump_hex(data,length);
}

uint32_t timestamp()
{
    static uint32_t t1 = 0;
    uint32_t t2 = millis();
    uint32_t diff = t2 - t1;
    t1 = t2;
    return diff;
}
#endif

