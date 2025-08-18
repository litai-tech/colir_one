#ifndef __BASIC_H__
#define __BASIC_H__

#include "config.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

float getVerticalVelocity();
void setVerticalVelocity(float velocity);
float getAltitude();
void setAltitude(float alt);

#ifdef __cplusplus
}
#endif

#endif
