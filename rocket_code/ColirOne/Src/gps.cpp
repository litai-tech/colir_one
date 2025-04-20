#include "gps.h"

GPS::GPS(void){
    myGPS = {0};
}

void GPS::gpsRun(void){
    nmea_parse(&myGPS, DataBuffer);
}

double GPS::getLongitude(void){
    return myGPS.longitude;
}

double GPS::getLatitude(void){
    return myGPS.latitude;
}

int GPS::getVisibleSatellites(void){
    return myGPS.satelliteCount;
}