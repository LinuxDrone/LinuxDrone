/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filterair.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Airspeed filter, calculates true airspeed based on indicated
 *             airspeed and uncorrected barometric altitude
 *             NOTE: This Sensor uses UNCORRECTED barometric altitude for
 *             correction --  run before barometric bias correction!
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

#include "inc/stateestimation.h"

// Private constants

#define STACK_REQUIRED 64

// simple IAS to TAS aproximation - 2% increase per 1000ft
// since we do not have flowing air temperature information
#define IAS2TAS(alt) (1.0f + (0.02f * (alt) / 304.8f))

// Private types
typedef struct  {
    float altitude;
}data_t;

data_t data;

// Private functions

static int init_filter(stateFilter *self);
static int filter(stateFilter *self, stateEstimation *state);


int32_t filterAirInitialize(stateFilter *handle)
{
    handle->init      = &init_filter;
    handle->filter    = &filter;
    handle->localdata = calloc(1, sizeof(data_t));
    return STACK_REQUIRED;
}

static int init_filter(stateFilter *self)
{
    data_t *this = (data_t *)self->localdata;

    this->altitude = 0.0f;
    return 0;
}

static int filter(stateFilter *self, stateEstimation *state)
{
    data_t *this = (data_t *)self->localdata;

    // take static pressure altitude estimation for
    if (IS_SET(state->updated, baroAltitude)) {
        this->altitude = state->baro[0];
    }
    // calculate true airspeed estimation
    if (IS_SET(state->updated, in_calibratedAirspeed)) {
        state->airspeed[1] = state->airspeed[0] * IAS2TAS(this->altitude);
    }

    return 0;
}


/**
 * @}
 * @}
 */
