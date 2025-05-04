/*
 * util.c
 *
 *  Created on: April 28, 2025
 *      Author: Dat Hoang
 */
#include <stdint.h>
#include <stdio.h>

void dump_hex(uint8_t * buf, int size)
{
    while(size--)
    {
        printf("%02x ", *buf++);
    }
    printf("\n");
}