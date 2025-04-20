#ifndef __GPS_H__
#define __GPS_H__

#include "stdint.h"
#include "stdio.h"
#include "../../Module/Inc/nmea_parse.h"

#ifdef __cplusplus
extern "C"{
#endif
class GPS{
    public:
        GPS();
        double getLongitude();
        double getLatitude();
        int getVisibleSatellites();
        void gpsRun();
    private:
        GPS_t myGPS;
        uint8_t DataBuffer[512];
};
#ifdef __cplusplus
}
#endif

#endif