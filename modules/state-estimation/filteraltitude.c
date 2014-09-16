/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filteraltitude.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Barometric altitude filter, calculates vertical speed and true
 *             altitude based on Barometric altitude and accelerometers
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdbool.h>
#include <stdlib.h>

#include "inc/stateestimation.h"


///#include <attitudestate.h>
///#include <altitudefiltersettings.h>
///#include <CoordinateConversions.h>

// Private constants

#define STACK_REQUIRED 128

#define DT_ALPHA       1e-3f

// Private types
typedef struct  {
    float   state[4]; // state = altitude,velocity,accel_offset,accel
    float   pos[3]; // position updates from other filters
    float   vel[3]; // position updates from other filters
    float   dTA;
    float   dTA2;
    int lastTime;
    float   accelLast;
    float   baroLast;
    int baroLastTime;
    bool    first_run;
} data_t;

data_t data;

// Private variables

// Private functions

static int init_filter(stateFilter *self);
static int filter(stateFilter *self, stateEstimation *state);


int filterAltitudeInitialize(stateFilter *handle)
{
    handle->init      = &init_filter;
    handle->filter    = &filter;
    handle->localdata = calloc(1, sizeof(data_t));
    AttitudeStateInitialize();
    AltitudeFilterSettingsInitialize();
    return STACK_REQUIRED;
}

static int init_filter(stateFilter *self)
{
    data_t *this = (data_t *)self->localdata;

    this->state[0]  = 0.0f;
    this->state[1]  = 0.0f;
    this->state[2]  = 0.0f;
    this->state[3]  = 0.0f;
    this->pos[0]    = 0.0f;
    this->pos[1]    = 0.0f;
    this->pos[2]    = 0.0f;
    this->vel[0]    = 0.0f;
    this->vel[1]    = 0.0f;
    this->vel[2]    = 0.0f;
    this->dTA = -1.0f;
    this->dTA2      = -1.0f;
    this->baroLast  = 0.0f;
    this->accelLast = 0.0f;
    this->first_run = 1;
    return 0;
}

static int filter(stateFilter *self, stateEstimation *state)
{
    input_t* input = (input_t*)self->module->module_info.input_data;

    data_t *this = (data_t *)self->localdata;

    if (this->first_run) {
        // Initialize to current altitude reading at initial location
        if (IS_SET(state->updated, accel_x)) {
            this->lastTime = PIOS_DELAY_GetRaw();
        }
        if (IS_SET(state->updated, BaroAltitude)) {
            this->first_run    = 0;
            this->baroLastTime = PIOS_DELAY_GetRaw();
        }
    } else {
        // save existing position and velocity updates so GPS will still work
        if (IS_SET(state->updated, GpsLatitude)) {
            this->pos[0]  = state->pos[0];
            this->pos[1]  = state->pos[1];
            this->pos[2]  = state->pos[2];
            state->pos[2] = -this->state[0];
        }
        if (IS_SET(state->updated, North)) {
            this->vel[0]  = state->vel[0];
            this->vel[1]  = state->vel[1];
            this->vel[2]  = state->vel[2];
            state->vel[2] = -this->state[1];
        }
        if (IS_SET(state->updated, accel_x)) {
            // rotate accels into global coordinate frame
            float Rbe[3][3];
            Quaternion2R(input->q1 , Rbe);
            float current = -(Rbe[0][2] * state->accel[0] + Rbe[1][2] * state->accel[1] + Rbe[2][2] * state->accel[2] + 9.81f);

            // low pass filter accelerometers
            this->state[3] = (1.0f - self->module->params_state_estimation.AccelLowPassKp) * this->state[3] + self->module->params_state_estimation.AccelLowPassKp * current;

            // correct accel offset (low pass zeroing)
            this->state[2] = (1.0f - self->module->params_state_estimation.AccelDriftKi) * this->state[2] + self->module->params_state_estimation.AccelDriftKi * this->state[3];

            // correct velocity and position state (integration)
            // low pass for average dT, compensate timing jitter from scheduler
            float dT = PIOS_DELAY_DiffuS(this->lastTime) / 1.0e6f;
            this->lastTime = PIOS_DELAY_GetRaw();
            if (dT < 0.001f) {
                dT = 0.001f;
            }
            if (this->dTA < 0) {
                this->dTA = dT;
            } else {
                this->dTA = this->dTA * (1.0f - DT_ALPHA) + dT * DT_ALPHA;
            }
            float speedLast = this->state[1];

            this->state[1] += 0.5f * (this->accelLast + (this->state[3] - this->state[2])) * this->dTA;
            this->accelLast = this->state[3] - this->state[2];

            this->state[0] += 0.5f * (speedLast + this->state[1]) * this->dTA;


            state->pos[0]   = this->pos[0];
            state->pos[1]   = this->pos[1];
            state->pos[2]   = -this->state[0];
            state->updated |= GpsLatitude;

            state->vel[0]   = this->vel[0];
            state->vel[1]   = this->vel[1];
            state->vel[2]   = -this->state[1];
            state->updated |= North;
        }
        if (IS_SET(state->updated, BaroAltitude)) {
            // correct the altitude state (simple low pass)
            this->state[0] = (1.0f - self->module->params_state_estimation.BaroKp) * this->state[0] + self->module->params_state_estimation.BaroKp * state->baro[0];

            // correct the velocity state (low pass differentiation)
            // low pass for average dT, compensate timing jitter from scheduler
            float dT = PIOS_DELAY_DiffuS(this->baroLastTime) / 1.0e6f;
            this->baroLastTime = PIOS_DELAY_GetRaw();
            if (dT < 0.001f) {
                dT = 0.001f;
            }
            if (this->dTA2 < 0) {
                this->dTA2 = dT;
            } else {
                this->dTA2 = this->dTA2 * (1.0f - DT_ALPHA) + dT * DT_ALPHA;
            }
            this->state[1]  = (1.0f - (self->module->params_state_estimation.BaroKp * self->module->params_state_estimation.BaroKp)) * this->state[1] + (self->module->params_state_estimation.BaroKp * self->module->params_state_estimation.BaroKp) * (state->baro[0] - this->baroLast) / this->dTA2;
            this->baroLast  = state->baro[0];

            state->pos[0]   = this->pos[0];
            state->pos[1]   = this->pos[1];
            state->pos[2]   = -this->state[0];
            state->updated |= GpsLatitude;

            state->vel[0]   = this->vel[0];
            state->vel[1]   = this->vel[1];
            state->vel[2]   = -this->state[1];
            state->updated |= North;
        }
    }

    return 0;
}


/**
 * @}
 * @}
 */
