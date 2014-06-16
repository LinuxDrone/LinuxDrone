#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "c-attitude.helper.h"

enum TrimFlight {TRIMFLIGHT_START,TRIMFLIGHT_LOAD,TRIMFLIGHT_NORMAL};

#define GRAV            9.81f
#define ACCEL_SCALE     (GRAV * 0.004f)
#define M_PI_F          3.14159265358979323846264338328f

// Время на калибровку гироскопов после запуска, в секундах
#define TIMESTART       7.0f

class CAttitude
{
public:
    CAttitude();
    ~CAttitude();

    void run(module_c_attitude_t *module);

private:
    float rpy[3];
    float accels[3], gyros[3];

    float gyro_correct_int[3];
    float accelKp, accelKp_copy;
    float accelKi, accelKi_copy;

    float accel_alpha;
    bool accel_filter_enabled;

    float accels_filtered[3];
    float grot_filtered[3];

    float yawBiasRate, yawBiasRate_copy;
    float rollPitchBiasRate;
    float gyroGain;
    int16_t accelbias[3];
    float q[4];
    float R[3][3];
    int8_t rotate;
    bool zero_during_arming;
    bool bias_correct_gyro;
    float boardRotation[3];
    char  trimFlight;
    uint8_t initSeting;
    float dT;                   // in seconds
    float timeElapsed;          // in miliseconds
    bool attitude_initialized;
    float samplesCalib;

    // For running trim flights
    volatile bool trim_requested;
    volatile int32_t trim_accels[3];
    volatile int32_t trim_samples;

    bool printEnable;

    void init();
    void settingsLoad(params_c_attitude_t *params);
    void rotateBoard();
    void correctBias();
    void accelFilter(const float *raw, float *filtered);
    void calcAttitude();
};
