#ifndef __LOG_H__
#define __LOG_H__

#include "app.h"
#include "stdint.h"

#define ENABLE_FILE_LOGGING

void LOG(uint32_t tag, const char * filename, int num, const char * fmt, ...);
void LOG_HEX(uint32_t tag, uint8_t * data, int length);

void set_logging_tag(uint32_t tag);

typedef enum
{
    TAG_ERR      = (1 << 0),
    TAG_GREEN    = (1 << 1),
    TAG_RED      = (1 << 2),
    TAG_USB      = (1 << 3),

    TAG_NO_TAG   = (1UL << 30),
    TAG_FILENO   = (1UL << 31)
} LOG_TAG;

#if defined(DEBUG_LEVEL) && DEBUG_LEVEL > 0

void set_logging_mask(uint32_t mask);
#define printf1(tag,fmt, ...) LOG(tag & ~(TAG_FILENO), NULL, 0, fmt, ##__VA_ARGS__)
#define printf2(tag,fmt, ...) LOG(tag | TAG_FILENO,__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define printf3(tag,fmt, ...) LOG(tag | TAG_FILENO,__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define dump_hex1(tag,data,len) LOG_HEX(tag,data,len)

uint32_t timestamp();

#else

#define set_logging_mask(mask)
#define printf1(tag,fmt, ...)
#define printf2(tag,fmt, ...)
#define printf3(tag,fmt, ...)
#define dump_hex1(tag,data,len)
#define timestamp()

#endif

#endif