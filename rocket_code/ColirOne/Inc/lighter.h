#ifndef __LIGHTER_H__
#define __LIGHTER_H__

#include "stdint.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"{
#endif
class Lighter {
    public: 
        Lighter();
        void fireLighter(uint8_t lighterNumber);
};

#ifdef __cplusplus
}
#endif


#endif