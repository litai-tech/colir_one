#ifndef __BAROMETER_H__
#define __BAROMETER_H__

#include "stdint.h"
#include "stdio.h"
#include "../../Module/Inc/bmp581.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class Barometer {
    public:
        Barometer();
        float getPressure(void);
        float getTemperature(void);
        float calculateAltitude(float pressure);;
        float calculateSeaLevelAltitude(float pressure);
        void resetZeroAltitude(void);
        uint8_t init(void);
    private :
        float zeroAltitude;
        bmp5_sensor_data bmpData;
};
#endif

#endif