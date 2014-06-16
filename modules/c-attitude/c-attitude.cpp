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

#include <native/timer.h>
#include "c-attitude.h"
#include "c-attitude.helper.h"
#include "quaternion-functions.h"
//#include "math/quaternion.h"
#include "math.h"

CAttitude::CAttitude()
{
    init();
    printEnable = false;
}

CAttitude::~CAttitude()
{
}

void CAttitude::run(module_c_attitude_t *module)
{
    // Указатель на структуру с входными данными модуля.
    input_t* input;

    SRTIME last_print_time = (SRTIME)rt_timer_read();
    SRTIME print_period = rt_timer_ns2ticks(1000000000);

    static SRTIME  timeTick, timeTickold;
    timeTickold = timeTick = (SRTIME)rt_timer_read();

    attitude_initialized = false;

    while(1) {
        get_input_data((module_t *)module);

        if(rt_timer_read() - last_print_time > print_period)
        {
            printEnable = true;
            printf("\ntimeElapsed      = %f\n", timeElapsed);

            last_print_time = rt_timer_read();
        }

        if(!attitude_initialized)
        {
            init();

            // Read settings in MongoDB
            settingsLoad(&module->params_c_attitude);
        }

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            input = (input_t*)module->module_info.input_data;

            accels[0]   = input->accel_x;
            accels[1]   = -input->accel_y;
            accels[2]   = -input->accel_z;
            gyros[0]    = input->gyro_x;
            gyros[1]    = -input->gyro_y;
            gyros[2]    = -input->gyro_z;

            //rt_task_sleep(module->module_info.queue_timeout);
            timeTick = (SRTIME)rt_timer_read();
            dT = ((float)((rt_timer_ticks2ns(timeTick  - timeTickold)))/1000000000.0f);
            timeElapsed += dT;
            timeTickold = timeTick;

            //if(el < 0.0018f) printf("\nDT_time      = %f\n", el);

            if (timeElapsed < TIMESTART && timeElapsed > 1.0f) {
                // Use accels to initialise attitude and calculate gyro bias
                accelKp     = 1.0f;
                accelKi     = 0.0f;
                yawBiasRate = 0.01f;
                rollPitchBiasRate    = 0.01f;
                accel_filter_enabled = false;
                initSeting = 0;
                bias_correct_gyro=true;
                samplesCalib += 1;
            } else if (zero_during_arming ) {
                accelKp     = 1.0f;
                accelKi     = 0.0f;
                yawBiasRate = 0.01f;
                rollPitchBiasRate    = 0.01f;
                accel_filter_enabled = false;
                initSeting = 0;
            } else if (initSeting == 0) {
                // Reload settings (all the rates)
                accelKi = accelKi_copy;
                accelKp = accelKp_copy;
                yawBiasRate = yawBiasRate_copy;
                rollPitchBiasRate = 0.0f;
                if (accel_alpha > 0.0f) {
                    accel_filter_enabled = true;
                }
                initSeting = 1;
                printf("\nCall frequency module = %fHz\n", samplesCalib/TIMESTART);
                printf("gyro_correct_int2     = %f\n", gyro_correct_int[2]);
            }

            rotateBoard();
            correctBias();
            calcAttitude();

            // Во время калибровки обнулять выходы
            if(initSeting==0)
            {
                rpy[0]= 0.0f;
                rpy[1]= 0.0f;
                rpy[2]= 0.0f;
                q[0]  = 1.0f;
                q[1]  = 0.0f;
                q[2]  = 0.0f;
                q[3]  = 0.0f;
            }

            // Вывод данных из модуля
            qrpy_out_t* qrpy_out;
            checkout_qrpy_out(module, &qrpy_out);
            qrpy_out->roll  = rpy[0];
            qrpy_out->pitch = rpy[1];
            qrpy_out->yaw   = rpy[2];
            qrpy_out->q1    = q[0];
            qrpy_out->q2    = q[1];
            qrpy_out->q3    = q[2];
            qrpy_out->q4    = q[3];
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

        // TODO: убрать!
//        break;
    }
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

/**
* @brief Инициализация переменных модуля
* @return
*/

void CAttitude::init()
{
    for (int i = 0;i<3;i++) {
        rpy[i] = 0.0f;
        accels[i] = 0.0f;
        gyros[i] = 0.0f;
        gyro_correct_int[i] = 0.0f;
        accels_filtered[i] = 0.0f;
        grot_filtered[i] = 0.0f;
        accelbias[i] = 0;
        boardRotation[i] = 0.0f;
    }

    accelKp = accelKp_copy = 0.0f;
    accelKi = accelKi_copy = 0.0f;
    accel_alpha = 0.0f;
    accel_filter_enabled = false;
    yawBiasRate = yawBiasRate_copy = 0.0f;
    rollPitchBiasRate = 0.0f;
    gyroGain = 0.0f;
    rotate = 0;
    zero_during_arming = false;
    initSeting = 0;
    dT = 0.0f;
    timeElapsed = 0.0f;
    samplesCalib = 0.0f;

    q[0] = 1;
    q[1] = 0;
    q[2] = 0;
    q[3] = 0;
    memset(R, 0, sizeof(R));

    bias_correct_gyro     = true;
    trimFlight            = TRIMFLIGHT_NORMAL;
    attitude_initialized  = true;
}


/**
* @brief Читает данные из базы MongoDB
* И сохраняет в структуре attitude_data_t
*/

void CAttitude::settingsLoad(params_c_attitude_t *params)
{
    accelKp_copy = accelKp = params->AccelKp;
    accelKi_copy = accelKi = params->AccelKi;
    yawBiasRate_copy = yawBiasRate = params->YawBiasRate;
    gyroGain = params->GyroGain;

    float accelTau = 0;
    const float fakeDt = 0.0025f;
    accelTau = params->AccelTau;
    // Calculate accel filter alpha, in the same way as for gyro data in stabilization module.
    if (accelTau < 0.0001f) {
        accel_alpha = 0; // not trusting this to resolve to 0
        accel_filter_enabled = false;
    } else {
        accel_alpha = expf(-fakeDt / accelTau);
        accel_filter_enabled = true;
    }

    //zero_during_arming = ZeroDuringArming;
    //bias_correct_gyro  = BiasCorrectGyro;

    accelbias[0] = params->AccelBias_X;
    accelbias[1] = params->AccelBias_Y;
    accelbias[2] = params->AccelBias_Z;

    gyro_correct_int[0] = params->GyroBias_X;
    gyro_correct_int[1] = params->GyroBias_Y;
    gyro_correct_int[2] = params->GyroBias_Z;

    boardRotation[0] = params->BoardRotation_Roll;
    boardRotation[1] = params->BoardRotation_Pitch;
    boardRotation[2] = params->BoardRotation_Yaw;


    // Indicates not to expend cycles on rotation
    if (boardRotation[0] == 0 && boardRotation[1] == 0 && boardRotation[2] == 0) {
        rotate = 0;

        // Shouldn't be used but to be safe
        float rotationQuat[4] = { 1, 0, 0, 0 };
        Quaternion2R(rotationQuat, R);
    } else {
        float rotationQuat[4];
        RPY2Quaternion(boardRotation, rotationQuat);
        Quaternion2R(rotationQuat, R);
        rotate = 1;
    }

    if (trimFlight == TRIMFLIGHT_START) {
        trim_accels[0] = 0;
        trim_accels[1] = 0;
        trim_accels[2] = 0;
        trim_samples   = 0;
        trim_requested = true;
    } else if (trimFlight == TRIMFLIGHT_LOAD) {
        trim_requested = false;
        accelbias[0] = trim_accels[0] / trim_samples;
        accelbias[1] = trim_accels[1] / trim_samples;
        // Z should average -grav
        accelbias[2] = trim_accels[2] / trim_samples + GRAV / ACCEL_SCALE;
        trimFlight  = TRIMFLIGHT_NORMAL;
    } else {
        trim_requested = false;
    }
}

void CAttitude::rotateBoard()
{
    if (rotate) {
        float vec_out[3];
        rot_mult(R, accels, vec_out);
        accels[0] = vec_out[0];
        accels[1] = vec_out[1];
        accels[2] = vec_out[2];
        rot_mult(R, gyros, vec_out);
        gyros[0]  = vec_out[0];
        gyros[1]  = vec_out[1];
        gyros[2]  = vec_out[2];
    }
}


void CAttitude::correctBias()
{
    accels[0] -= accelbias[0] * ACCEL_SCALE;
    accels[1] -= accelbias[1] * ACCEL_SCALE;
    accels[2] -= accelbias[2] * ACCEL_SCALE;

    if (bias_correct_gyro) {
        // Applying integral component here so it can be seen on the gyros and correct bias
        gyros[0] += gyro_correct_int[0];
        gyros[1] += gyro_correct_int[1];
        gyros[2] += gyro_correct_int[2];
    }

    // Force the roll & pitch gyro rates to average to zero during initialisation
    gyro_correct_int[0] += -gyros[0] * rollPitchBiasRate;
    gyro_correct_int[1] += -gyros[1] * rollPitchBiasRate;
    gyro_correct_int[2] += -gyros[2] * yawBiasRate;
}


void CAttitude::accelFilter(const float *raw, float *filtered)
{
    if (accel_filter_enabled) {
        filtered[0] = filtered[0] * accel_alpha + raw[0] * (1 - accel_alpha);
        filtered[1] = filtered[1] * accel_alpha + raw[1] * (1 - accel_alpha);
        filtered[2] = filtered[2] * accel_alpha + raw[2] * (1 - accel_alpha);
    } else {
        filtered[0] = raw[0];
        filtered[1] = raw[1];
        filtered[2] = raw[2];
    }
}


void CAttitude::calcAttitude()
{
    float grot[3];
    float accel_err[3];

    // Apply smoothing to accel values, to reduce vibration noise before main calculations.
    accelFilter(accels, accels_filtered);

    // Rotate gravity unit vector to body frame, filter and cross with accels
    grot[0] = -(2 * (q[1] * q[3] - q[0] * q[2]));
    grot[1] = -(2 * (q[2] * q[3] + q[0] * q[1]));
    grot[2] = -(q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);

    accelFilter(grot, grot_filtered);

    CrossProduct((const float *)accels_filtered, (const float *)grot_filtered, accel_err);

    // Account for accel magnitude
    float accel_mag = sqrtf(accels_filtered[0] * accels_filtered[0] +
            accels_filtered[1] * accels_filtered[1] +
            accels_filtered[2] * accels_filtered[2]);
    if (accel_mag < 1.0e-3f) {
        return;
    }

    // Account for filtered gravity vector magnitude
    float grot_mag;

    if (accel_filter_enabled) {
        grot_mag = sqrtf(grot_filtered[0] * grot_filtered[0] +
                grot_filtered[1] * grot_filtered[1] +
                grot_filtered[2] * grot_filtered[2]);
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
    gyro_correct_int[0] += accel_err[0] * accelKi;
    gyro_correct_int[1] += accel_err[1] * accelKi;

    // p_ad->gyro_correct_int[2] += accel_err[2] * p_ad->accelKi;

    // Correct rates based on error, integral component dealt with in updateSensors
    gyros[0] += accel_err[0] * accelKp / dT;
    gyros[1] += accel_err[1] * accelKp / dT;
    gyros[2] += accel_err[2] * accelKp / dT;

    { // scoping variables to save memory
        // Work out time derivative from INSAlgo writeup
        // Also accounts for the fact that gyros are in deg/s
        float qdot[4];
        qdot[0] = (-q[1] * gyros[0] - q[2] * gyros[1] - q[3] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);
        qdot[1] =  (q[0] * gyros[0] - q[3] * gyros[1] + q[2] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);
        qdot[2] =  (q[3] * gyros[0] + q[0] * gyros[1] - q[1] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);
        qdot[3] = (-q[2] * gyros[0] + q[1] * gyros[1] + q[0] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);

        // Take a time step
        q[0]  += qdot[0];
        q[1]  += qdot[1];
        q[2]  += qdot[2];
        q[3]  += qdot[3];

        if (q[0] < 0) {
            q[0] = -q[0];
            q[1] = -q[1];
            q[2] = -q[2];
            q[3] = -q[3];
        }
    }

    // Renomalize
    float qmag = sqrtf(q[0] * q[0] +
            q[1] * q[1] +
            q[2] * q[2] +
            q[3] * q[3]);

    //if(printEnable) printf("qmag = %f\n",qmag);
    //float qmagt;
    //QUATERNION qt = quaternionMake(p_ad->q[1], p_ad->q[2], p_ad->q[3], p_ad->q[0]);
    //qmagt = quaternionLength(qt);
    //if(printEnable & (qmagt > 0)) printf("qmag = %f\n",qmagt);

    // If quaternion has become inappropriately short or is nan reinit.
    // THIS SHOULD NEVER ACTUALLY HAPPEN
    if ((fabsf(qmag) < 1e-3f) || isnan(qmag)) {
        q[0] = 1;
        q[1] = 0;
        q[2] = 0;
        q[3] = 0;
    } else {
        q[0] = q[0] / qmag;
        q[1] = q[1] / qmag;
        q[2] = q[2] / qmag;
        q[3] = q[3] / qmag;
    }

    // Convert into eueler degrees (makes assumptions about RPY order)
    Quaternion2RPY(q, rpy);

    return;
}


extern "C" {
/**
 * @brief c_attitude_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_attitude_run (module_c_attitude_t *module)
{
    CAttitude attitude;
    attitude.run(module);
}

} // end of extern "C"

/* ---------------  FUNCTIONS -----------------------*/
