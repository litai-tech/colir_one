/*
 * barometer.h
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */

#ifndef __BAROMETER_H__
#define __BAROMETER_H__

#include "stdint.h"
#include "stdio.h"
#include "../../Module/Inc/bmp581.h"
#include "app.h"

#ifdef __cplusplus
extern "C"{
#endif
class Barometer {
    public:
        Barometer();
        colirone_err_t init(void);
        float getPressure(void);
        float getTemperature(void);
        float calculateAltitude(float pressure);;
        float calculateSeaLevelAltitude(float pressure);
        void resetZeroAltitude(void);
    private :
        float zeroAltitude;
        bmp5_sensor_data bmpData;
};
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#endif

#endif