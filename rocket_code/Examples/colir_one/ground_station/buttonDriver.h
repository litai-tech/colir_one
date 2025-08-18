#ifndef __BUTTON_DRIVER_H__
#define __BUTTON_DRIVER_H__

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

void buttonInit(void);
void buttonProcess(unsigned long now);

#ifdef __cplusplus
}
#endif

#endif