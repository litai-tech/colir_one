#ifndef __IMU_H__
#define __IMU_H__
#ifdef __cplusplus
extern "C"{
#endif
typedef struct {
    double x;
    double y;
    double z;
} XYZ_t;

typedef struct {
    double w;
    double x;
    double y;
    double z;
} Quaternion_t;

class IMU {
    public: 
        IMU();
        void init(void);
        XYZ_t getOrientation();
        XYZ_t getAcceleration();
        XYZ_t getGyroscope();
        Quaternion_t getQuaternion();
};

#ifdef __cplusplus
}
#endif

#endif