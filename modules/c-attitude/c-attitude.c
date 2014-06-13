//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// This work is based on the original code from the OpenPilot project
// which is licensed under CC-BY-SA terms. See http://www.openpilot.org
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#include "c-attitude.h"
#include "c-attitude.helper.h"
#include "quaternion-functions.h"
//#include "math/quaternion.h"
#include "math.h"


void rotateBoard(attitude_data_t *p_ad);
void correctBias(attitude_data_t *p_ad);
void calcAttitude(attitude_data_t *p_ad);
void settingsLoad(attitude_data_t *p_ad, params_c_attitude_t *params);
void init_attitude(attitude_data_t *p_ad);

void accelFilter(const float *raw, float *filtered, attitude_data_t *p_ad);
static inline void apply_accel_filter(const float *raw, float *filtered);

static bool printEnable = false;

/**
 * @brief c_attitude_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_attitude_run (module_c_attitude_t *module)
{
    // Указатель на структуру с входными данными модуля.
    input_t* input;

    long last_print_time = (long)rt_timer_read();
    long print_period = rt_timer_ns2ticks(1000000000);

    static SRTIME  timeTick, timeTickold;
    timeTickold = timeTick = (SRTIME)rt_timer_read();

    static attitude_data_t *p_ad, attitude_data;
    p_ad = &attitude_data;

    p_ad->attitude_initialized = false;

    while(1) {
        get_input_data((module_t *)module);

        if(rt_timer_read() - last_print_time > print_period)
        {
            printEnable = true;
            printf("\ntimeElapsed      = %f\n", p_ad->timeElapsed);

            last_print_time = rt_timer_read();
        }

        if(!p_ad->attitude_initialized)
        {
            init_attitude(p_ad);

            // Read settings in MongoDB
            settingsLoad(p_ad, &module->params_c_attitude);
        }

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            input = (input_t*)module->module_info.input_data;

            p_ad->accels[0]   = input->accel_x;
            p_ad->accels[1]   = -input->accel_y;
            p_ad->accels[2]   = -input->accel_z;
            p_ad->gyros[0]    = input->gyro_x;
            p_ad->gyros[1]    = -input->gyro_y;
            p_ad->gyros[2]    = -input->gyro_z;

            //rt_task_sleep(module->module_info.queue_timeout);
            timeTick = (SRTIME)rt_timer_read();
            p_ad->dT = ((float)((rt_timer_ticks2ns(timeTick  - timeTickold)))/1000000000.0f);
            p_ad->timeElapsed += p_ad->dT;
            timeTickold = timeTick;

            //if(el < 0.0018f) printf("\nDT_time      = %f\n", el);

            if (p_ad->timeElapsed < TIMESTART && p_ad->timeElapsed > 1.0f) {
                // Use accels to initialise attitude and calculate gyro bias
                p_ad->accelKp     = 1.0f;
                p_ad->accelKi     = 0.0f;
                p_ad->yawBiasRate = 0.01f;
                p_ad->rollPitchBiasRate    = 0.01f;
                p_ad->accel_filter_enabled = false;
                p_ad->initSeting = 0;
                p_ad->bias_correct_gyro=true;
                p_ad->samplesCalib += 1;
            } else if (p_ad->zero_during_arming ) {
                p_ad->accelKp     = 1.0f;
                p_ad->accelKi     = 0.0f;
                p_ad->yawBiasRate = 0.01f;
                p_ad->rollPitchBiasRate    = 0.01f;
                p_ad->accel_filter_enabled = false;
                p_ad->initSeting = 0;
            } else if (p_ad->initSeting == 0) {
                // Reload settings (all the rates)
                p_ad->accelKi = p_ad->accelKi_copy;
                p_ad->accelKp = p_ad->accelKp_copy;
                p_ad->yawBiasRate = p_ad->yawBiasRate_copy;
                p_ad->rollPitchBiasRate = 0.0f;
                if (p_ad->accel_alpha > 0.0f) {
                    p_ad->accel_filter_enabled = true;
                }
                p_ad->initSeting = 1;
                printf("\nCall frequency module = %fHz\n",p_ad->samplesCalib/TIMESTART);
                printf("gyro_correct_int2     = %f\n",p_ad->gyro_correct_int[2]);
            }

            rotateBoard(p_ad);
            correctBias(p_ad);
            calcAttitude(p_ad);

            // Во время калибровки обнулять выходы
            if(p_ad->initSeting==0)
            {
                p_ad->rpy[0]= 0.0f;
                p_ad->rpy[1]= 0.0f;
                p_ad->rpy[2]= 0.0f;
                p_ad->q[0]  = 1.0f;
                p_ad->q[1]  = 0.0f;
                p_ad->q[2]  = 0.0f;
                p_ad->q[3]  = 0.0f;
            }

            // Вывод данных из модуля
            qrpy_out_t* qrpy_out;
            checkout_qrpy_out(module, &qrpy_out);
            qrpy_out->roll  = p_ad->rpy[0];
            qrpy_out->pitch = p_ad->rpy[1];
            qrpy_out->yaw   = p_ad->rpy[2];
            qrpy_out->q1    = p_ad->q[0];
            qrpy_out->q2    = p_ad->q[1];
            qrpy_out->q3    = p_ad->q[2];
            qrpy_out->q4    = p_ad->q[3];
            checkin_qrpy_out(module, &qrpy_out);

        }
        else
        {
            // вышел таймаут
        }

        // Эти данные следует добыть из разделяемой памяти, если они не придут через трубу
        //module->module_info.refresh_input_mask = accel_x | accel_y | accel_z |
        //                                         gyro_x  | gyro_y  | gyro_z;

        //if(printEnable) printf("p_ad->samplesCalib = %f\n",p_ad->samplesCalib);
        //if(printEnable) printf("gyro_correct_int2   = %f\n",p_ad->gyro_correct_int[2]);
        //if(printEnable) printf("p_ad->gyros[2]c   = %f\n",p_ad->gyros[2]);

        printEnable = false;
    }
}


/* ---------------  FUNCTIONS -----------------------*/

/**
 * @brief Инициализация переменных модуля
 * @return
 */

void init_attitude(attitude_data_t *p_ad)
{
    memset(p_ad, 0, sizeof(attitude_data_t));

    p_ad->q[0] = 1;
    p_ad->q[1] = 0;
    p_ad->q[2] = 0;
    p_ad->q[3] = 0;

    p_ad->bias_correct_gyro     = true;
    p_ad->trimFlight            = TRIMFLIGHT_NORMAL;
    p_ad->attitude_initialized  = true;
    return;
}


/**
 * @brief Читает данные из базы MongoDB
 * И сохраняет в структуре attitude_data_t
 */

void settingsLoad(attitude_data_t *p_ad, params_c_attitude_t *params)
{
        p_ad->accelKp_copy = p_ad->accelKp = params->AccelKp;
        p_ad->accelKi_copy = p_ad->accelKi = params->AccelKi;
        p_ad->yawBiasRate_copy = p_ad->yawBiasRate = params->YawBiasRate;
        p_ad->gyroGain = params->GyroGain;

        float accelTau = 0;
        const float fakeDt = 0.0025f;
        accelTau = params->AccelTau;
        // Calculate accel filter alpha, in the same way as for gyro data in stabilization module.
        if (accelTau < 0.0001f) {
            p_ad->accel_alpha = 0; // not trusting this to resolve to 0
            p_ad->accel_filter_enabled = false;
        } else {
            p_ad->accel_alpha = expf(-fakeDt / accelTau);
            p_ad->accel_filter_enabled = true;
        }

        //zero_during_arming = ZeroDuringArming;
        //bias_correct_gyro  = BiasCorrectGyro;

        p_ad->accelbias[0] = params->AccelBias_X;
        p_ad->accelbias[1] = params->AccelBias_Y;
        p_ad->accelbias[2] = params->AccelBias_Z;

        p_ad->gyro_correct_int[0] = params->GyroBias_X;
        p_ad->gyro_correct_int[1] = params->GyroBias_Y;
        p_ad->gyro_correct_int[2] = params->GyroBias_Z;

        p_ad->boardRotation[0] = params->BoardRotation_Roll;
        p_ad->boardRotation[1] = params->BoardRotation_Pitch;
        p_ad->boardRotation[2] = params->BoardRotation_Yaw;


        // Indicates not to expend cycles on rotation
        if (p_ad->boardRotation[0] == 0 && p_ad->boardRotation[1] == 0 && p_ad->boardRotation[2] == 0) {
            p_ad->rotate = 0;

            // Shouldn't be used but to be safe
            float rotationQuat[4] = { 1, 0, 0, 0 };
            Quaternion2R(rotationQuat, p_ad->R);
        } else {
            float rotationQuat[4];
            RPY2Quaternion(p_ad->boardRotation, rotationQuat);
            Quaternion2R(rotationQuat, p_ad->R);
            p_ad->rotate = 1;
        }

        if (p_ad->trimFlight == TRIMFLIGHT_START) {
            p_ad->trim_accels[0] = 0;
            p_ad->trim_accels[1] = 0;
            p_ad->trim_accels[2] = 0;
            p_ad->trim_samples   = 0;
            p_ad->trim_requested = true;
        } else if (p_ad->trimFlight == TRIMFLIGHT_LOAD) {
            p_ad->trim_requested = false;
            p_ad->accelbias[0] = p_ad->trim_accels[0] / p_ad->trim_samples;
            p_ad->accelbias[1] = p_ad->trim_accels[1] / p_ad->trim_samples;
            // Z should average -grav
            p_ad->accelbias[2] = p_ad->trim_accels[2] / p_ad->trim_samples + GRAV / ACCEL_SCALE;
            p_ad->trimFlight  = TRIMFLIGHT_NORMAL;
        } else {
            p_ad->trim_requested = false;
        }
}

void rotateBoard(attitude_data_t *p_ad)
{
    if (p_ad->rotate) {
        float vec_out[3];
        rot_mult(p_ad->R, p_ad->accels, vec_out);
        p_ad->accels[0] = vec_out[0];
        p_ad->accels[1] = vec_out[1];
        p_ad->accels[2] = vec_out[2];
        rot_mult(p_ad->R, p_ad->gyros, vec_out);
        p_ad->gyros[0]  = vec_out[0];
        p_ad->gyros[1]  = vec_out[1];
        p_ad->gyros[2]  = vec_out[2];
    }
}


void correctBias(attitude_data_t *p_ad)
{
    p_ad->accels[0] -= p_ad->accelbias[0] * ACCEL_SCALE;
    p_ad->accels[1] -= p_ad->accelbias[1] * ACCEL_SCALE;
    p_ad->accels[2] -= p_ad->accelbias[2] * ACCEL_SCALE;

    if (p_ad->bias_correct_gyro) {
        // Applying integral component here so it can be seen on the gyros and correct bias
        p_ad->gyros[0] += p_ad->gyro_correct_int[0];
        p_ad->gyros[1] += p_ad->gyro_correct_int[1];
        p_ad->gyros[2] += p_ad->gyro_correct_int[2];
    }

    // Force the roll & pitch gyro rates to average to zero during initialisation
    p_ad->gyro_correct_int[0] += -p_ad->gyros[0] * p_ad->rollPitchBiasRate;
    p_ad->gyro_correct_int[1] += -p_ad->gyros[1] * p_ad->rollPitchBiasRate;
    p_ad->gyro_correct_int[2] += -p_ad->gyros[2] * p_ad->yawBiasRate;
}


void accelFilter(const float *raw, float *filtered, attitude_data_t *p_ad)
{
    if (p_ad->accel_filter_enabled) {
        filtered[0] = filtered[0] * p_ad->accel_alpha + raw[0] * (1 - p_ad->accel_alpha);
        filtered[1] = filtered[1] * p_ad->accel_alpha + raw[1] * (1 - p_ad->accel_alpha);
        filtered[2] = filtered[2] * p_ad->accel_alpha + raw[2] * (1 - p_ad->accel_alpha);
    } else {
        filtered[0] = raw[0];
        filtered[1] = raw[1];
        filtered[2] = raw[2];
    }
}


void calcAttitude(attitude_data_t *p_ad)
{
    float grot[3];
    float accel_err[3];

    // Apply smoothing to accel values, to reduce vibration noise before main calculations.
    accelFilter(p_ad->accels, p_ad->accels_filtered, p_ad);

    // Rotate gravity unit vector to body frame, filter and cross with accels
    grot[0] = -(2 * (p_ad->q[1] * p_ad->q[3] - p_ad->q[0] * p_ad->q[2]));
    grot[1] = -(2 * (p_ad->q[2] * p_ad->q[3] + p_ad->q[0] * p_ad->q[1]));
    grot[2] = -(p_ad->q[0] * p_ad->q[0] - p_ad->q[1] * p_ad->q[1] - p_ad->q[2] * p_ad->q[2] + p_ad->q[3] * p_ad->q[3]);

    accelFilter(grot, p_ad->grot_filtered, p_ad);

    CrossProduct((const float *)p_ad->accels_filtered, (const float *)p_ad->grot_filtered, accel_err);

    // Account for accel magnitude
    float accel_mag = sqrtf(p_ad->accels_filtered[0] * p_ad->accels_filtered[0] +
                            p_ad->accels_filtered[1] * p_ad->accels_filtered[1] +
                            p_ad->accels_filtered[2] * p_ad->accels_filtered[2]);
    if (accel_mag < 1.0e-3f) {
        return;
    }

    // Account for filtered gravity vector magnitude
    float grot_mag;

    if (p_ad->accel_filter_enabled) {
        grot_mag = sqrtf(p_ad->grot_filtered[0] * p_ad->grot_filtered[0] +
                         p_ad->grot_filtered[1] * p_ad->grot_filtered[1] +
                         p_ad->grot_filtered[2] * p_ad->grot_filtered[2]);
    } else {
        grot_mag = 1.0f;
    }

    if (grot_mag < 1.0e-3f) {
        return;
    }

    accel_err[0] /= (accel_mag * grot_mag);
    accel_err[1] /= (accel_mag * grot_mag);
    accel_err[2] /= (accel_mag * grot_mag);

    // Accumulate integral of error.  Scale here so that units are (deg/s) but Ki has units of s
    p_ad->gyro_correct_int[0] += accel_err[0] * p_ad->accelKi;
    p_ad->gyro_correct_int[1] += accel_err[1] * p_ad->accelKi;

    // p_ad->gyro_correct_int[2] += accel_err[2] * p_ad->accelKi;

    // Correct rates based on error, integral component dealt with in updateSensors
    p_ad->gyros[0] += accel_err[0] * p_ad->accelKp / p_ad->dT;
    p_ad->gyros[1] += accel_err[1] * p_ad->accelKp / p_ad->dT;
    p_ad->gyros[2] += accel_err[2] * p_ad->accelKp / p_ad->dT;

    { // scoping variables to save memory
      // Work out time derivative from INSAlgo writeup
      // Also accounts for the fact that gyros are in deg/s
        float qdot[4];
        qdot[0] = (-p_ad->q[1] * p_ad->gyros[0] - p_ad->q[2] * p_ad->gyros[1] - p_ad->q[3] * p_ad->gyros[2]) * p_ad->dT * (M_PI_F / 180.0f / 2.0f);
        qdot[1] = (p_ad->q[0] * p_ad->gyros[0] - p_ad->q[3] * p_ad->gyros[1] + p_ad->q[2] * p_ad->gyros[2]) * p_ad->dT * (M_PI_F / 180.0f / 2.0f);
        qdot[2] = (p_ad->q[3] * p_ad->gyros[0] + p_ad->q[0] * p_ad->gyros[1] - p_ad->q[1] * p_ad->gyros[2]) * p_ad->dT * (M_PI_F / 180.0f / 2.0f);
        qdot[3] = (-p_ad->q[2] * p_ad->gyros[0] + p_ad->q[1] * p_ad->gyros[1] + p_ad->q[0] * p_ad->gyros[2]) * p_ad->dT * (M_PI_F / 180.0f / 2.0f);

        // Take a time step
        p_ad->q[0]  += qdot[0];
        p_ad->q[1]  += qdot[1];
        p_ad->q[2]  += qdot[2];
        p_ad->q[3]  += qdot[3];

        if (p_ad->q[0] < 0) {
            p_ad->q[0] = -p_ad->q[0];
            p_ad->q[1] = -p_ad->q[1];
            p_ad->q[2] = -p_ad->q[2];
            p_ad->q[3] = -p_ad->q[3];
        }
    }

    // Renomalize
    float qmag = sqrtf(p_ad->q[0] * p_ad->q[0] +
                       p_ad->q[1] * p_ad->q[1] +
                       p_ad->q[2] * p_ad->q[2] +
                       p_ad->q[3] * p_ad->q[3]);

    //if(printEnable) printf("qmag = %f\n",qmag);
    //float qmagt;
    //QUATERNION qt = quaternionMake(p_ad->q[1], p_ad->q[2], p_ad->q[3], p_ad->q[0]);
    //qmagt = quaternionLength(qt);
    //if(printEnable & (qmagt > 0)) printf("qmag = %f\n",qmagt);

    // If quaternion has become inappropriately short or is nan reinit.
    // THIS SHOULD NEVER ACTUALLY HAPPEN
    if ((fabsf(qmag) < 1e-3f) || isnan(qmag)) {
        p_ad->q[0] = 1;
        p_ad->q[1] = 0;
        p_ad->q[2] = 0;
        p_ad->q[3] = 0;
    } else {
        p_ad->q[0] = p_ad->q[0] / qmag;
        p_ad->q[1] = p_ad->q[1] / qmag;
        p_ad->q[2] = p_ad->q[2] / qmag;
        p_ad->q[3] = p_ad->q[3] / qmag;
    }

    // Convert into eueler degrees (makes assumptions about RPY order)
    Quaternion2RPY(p_ad->q, p_ad->rpy);

    return;
}

