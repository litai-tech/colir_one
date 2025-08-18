#include "basic.h"

static float verticalVelocity = 0;
static float altitude = 0;

float getVerticalVelocity(){
  return verticalVelocity;
}

void setVerticalVelocity(float velocity){
  verticalVelocity = velocity;
}

float getAltitude(){
  return altitude;
}

void setAltitude(float alt){
  altitude = alt;
}