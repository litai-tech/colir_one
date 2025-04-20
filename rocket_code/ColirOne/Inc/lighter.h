#ifndef __LIGHTER_H__
#define __LIGHTER_H__

#include "stdint.h"
#include "stdio.h"

class Lighter {
    public: 
        Lighter();
        void fireLighter(uint8_t lighterNumber);
};

#endif