/*
 * app.h
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */

#ifndef __APP_H__
#define __APP_H__

#include "stdint.h"

#define COLIRONE_OK                        0
#define COLIRONE_ERROR                     1
#define COLIRONE_CHECK_ERROR(x)            if(x != COLIRONE_OK) return COLIRONE_ERROR;

typedef uint8_t colirone_err_t;

#endif