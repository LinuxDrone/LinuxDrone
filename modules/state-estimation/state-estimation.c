#include "state-estimation.helper.h"
#include "inc/stateestimation.h"


#define CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(shortname, a1, a2, a3) \
    if (IS_SET(states.updated, a1)) { \
        if (IS_REAL(input->a1) && IS_REAL(input->a2) && IS_REAL(input->a3)) { \
            states.shortname[0] = input->a1; \
            states.shortname[1] = input->a2; \
            states.shortname[2] = input->a3; \
        } \
        else { \
            UNSET_MASK(states.updated, a1); \
        } \
    }

#define CHECK_AND_LOAD_TO_STATE_1_DIMENSION(shortname, a1) \
    if (IS_SET(states.updated, a1)) { \
        if (IS_REAL(input->a1)) { \
            states.shortname[0] = input->a1; \
        } \
        else { \
            UNSET_MASK(states.updated, a1); \
        } \
    }

#define EXPORT_STATE_IF_UPDATED_3_DIMENSIONS(statename, shortname, a1, a2, a3) \
    if (IS_SET(states.updated, a1)) { \
        statename##_t* obj##statename; \
        checkout_##statename(module, &obj##statename); \
        obj##statename->a1 = states.shortname[0]; \
        obj##statename->a2 = states.shortname[1]; \
        obj##statename->a3 = states.shortname[2]; \
        checkin_##statename(module, &obj##statename); \
    }

#define EXPORT_STATE_IF_UPDATED_2_DIMENSIONS(statename, shortname, a1, a2) \
    if (IS_SET(states.updated, a1)) { \
        statename##_t* obj##statename; \
        checkout_##statename(module, &obj##statename); \
        obj##statename->a1 = states.shortname[0]; \
        obj##statename->a2 = states.shortname[1]; \
        checkin_##statename(module, &obj##statename); \
    }

struct filterPipelineStruct;

typedef const struct filterPipelineStruct {
    const stateFilter *filter;
    const struct filterPipelineStruct *next;
} filterPipeline;


// different filters available to state estimation
static stateFilter magFilter;
static stateFilter baroFilter;
static stateFilter altitudeFilter;
static stateFilter airFilter;
static stateFilter stationaryFilter;
static stateFilter llaFilter;
static stateFilter cfFilter;
static stateFilter cfmFilter;
static stateFilter ekf13iFilter;
static stateFilter ekf13Filter;

// preconfigured filter chains selectable via revoSettings.FusionAlgorithm
static filterPipeline *cfQueue = &(filterPipeline) {
    .filter = &magFilter,
    .next   = &(filterPipeline) {
        .filter = &airFilter,
        .next   = &(filterPipeline) {
            .filter = &llaFilter,
            .next   = &(filterPipeline) {
                .filter = &baroFilter,
                .next   = &(filterPipeline) {
                    .filter = &altitudeFilter,
                    .next   = &(filterPipeline) {
                        .filter = &cfFilter,
                        .next   = NULL,
                    }
                }
            }
        }
    }
};

static const filterPipeline *cfmQueue = &(filterPipeline) {
    .filter = &magFilter,
    .next   = &(filterPipeline) {
        .filter = &airFilter,
        .next   = &(filterPipeline) {
            .filter = &llaFilter,
            .next   = &(filterPipeline) {
                .filter = &baroFilter,
                .next   = &(filterPipeline) {
                    .filter = &altitudeFilter,
                    .next   = &(filterPipeline) {
                        .filter = &cfmFilter,
                        .next   = NULL,
                    }
                }
            }
        }
    }
};

static const filterPipeline *ekf13iQueue = &(filterPipeline) {
    .filter = &magFilter,
    .next   = &(filterPipeline) {
        .filter = &airFilter,
        .next   = &(filterPipeline) {
            .filter = &llaFilter,
            .next   = &(filterPipeline) {
                .filter = &baroFilter,
                .next   = &(filterPipeline) {
                    .filter = &stationaryFilter,
                    .next   = &(filterPipeline) {
                        .filter = &ekf13iFilter,
                        .next   = NULL,
                    }
                }
            }
        }
    }
};

static const filterPipeline *ekf13Queue = &(filterPipeline) {
    .filter = &magFilter,
    .next   = &(filterPipeline) {
        .filter = &airFilter,
        .next   = &(filterPipeline) {
            .filter = &llaFilter,
            .next   = &(filterPipeline) {
                .filter = &baroFilter,
                .next   = &(filterPipeline) {
                    .filter = &ekf13Filter,
                    .next   = NULL,
                }
            }
        }
    }
};



int32_t StateEstimationInitialize(void)
{
/*
    RevoSettingsInitialize();

    GyroSensorInitialize();
    MagSensorInitialize();
    BaroSensorInitialize();
    AirspeedSensorInitialize();
    GPSVelocitySensorInitialize();
    GPSPositionSensorInitialize();

    GyroStateInitialize();
    AccelStateInitialize();
    MagStateInitialize();
    AirspeedStateInitialize();
    PositionStateInitialize();
    VelocityStateInitialize();
*/

    // Initialize Filters
    ///filterMagInitialize(&magFilter);
    filterBaroInitialize(&baroFilter);
    filterAltitudeInitialize(&altitudeFilter);
    filterAirInitialize(&airFilter);
    ///filterStationaryInitialize(&stationaryFilter);
    ///filterLLAInitialize(&llaFilter);
    filterCFInitialize(&cfFilter);
    ///filterCFMInitialize(&cfmFilter);
    ///filterEKF13iInitialize(&ekf13iFilter);
    ///filterEKF13Initialize(&ekf13Filter);

    return 0;
}


static int32_t fusionAlgorithm     = -1;
static filterPipeline *filterChain = NULL;

void state_estimation_run (module_state_estimation_t *module)
{
    ///static enum { RUNSTATE_LOAD = 0, RUNSTATE_FILTER = 1, RUNSTATE_SAVE = 2 } runState = RUNSTATE_LOAD;
    static int8_t alarm     = 0;
    static int8_t lastAlarm = -1;
    static uint16_t alarmcounter = 0;
    static filterPipeline *current;
    static stateEstimation states;
    ///static uint32_t last_time;
    ///static uint16_t bootDelay = 30;


    int cycle=0;
    while(1) {
        get_input_data((module_t*)module);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties==0)
        {
            // вышел таймаут а данных все нет во входной очереди
            continue;
        }

        input_t* input = (input_t*)module->module_info.input_data;



        // check if a new filter chain should be initialized
        if (fusionAlgorithm != module->params_state_estimation.FusionAlgorithm) {
            ///FlightStatusData fs;
            ///FlightStatusGet(&fs);
            ///if (fs.Armed == FLIGHTSTATUS_ARMED_DISARMED || fusionAlgorithm == -1) {
            if (fusionAlgorithm == -1) {
                const filterPipeline *newFilterChain;
                switch (module->params_state_estimation.FusionAlgorithm) {
                case FUSIONALGORITHM_COMPLEMENTARY:
                    newFilterChain = cfQueue;
                    break;
                case FUSIONALGORITHM_COMPLEMENTARYMAG:
                    newFilterChain = cfmQueue;
                    break;
                case FUSIONALGORITHM_INS13INDOOR:
                    newFilterChain = ekf13iQueue;
                    break;
                case FUSIONALGORITHM_INS13OUTDOOR:
                    newFilterChain = ekf13Queue;
                    break;
                default:
                    newFilterChain = NULL;
                }

                // initialize filters in chain
                current = (filterPipeline *)newFilterChain;
                bool error = 0;
                while (current != NULL) {
                    int32_t result = current->filter->init((stateFilter *)current->filter);
                    if (result != 0) {
                        error = 1;
                        break;
                    }
                    current = current->next;
                }
                if (error) {
                    ///AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_ERROR);
                    return;
                } else {
                    // set new fusion algortithm
                    filterChain     = (filterPipeline *)newFilterChain;
                    fusionAlgorithm = module->params_state_estimation.FusionAlgorithm;
                }
            }
        }

        // read updated sensor UAVObjects and set initial state
        states.updated = module->module_info.updated_input_properties;

        // fetch sensors, check values, and load into state struct
        CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(gyro, gyro_x, gyro_y, gyro_z);
        CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(accel, accel_x, accel_y, accel_z);
        CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(mag, mag_x, mag_y, mag_z);
        CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(vel, in_vel_north, in_vel_east, in_vel_down);
        CHECK_AND_LOAD_TO_STATE_1_DIMENSION(baro, baroAltitude);
        CHECK_AND_LOAD_TO_STATE_1_DIMENSION(airspeed, in_calibratedAirspeed);
        states.airspeed[1] = 0.0f; // sensor does not provide true airspeed, needs to be calculated by filter, set to zero for now
        // GPS position data (LLA) is not fetched here since it does not contain floats. The filter must do all checks itself

        // at this point sensor state is stored in "states" with some rudimentary filtering applied

        // apply all filters in the current filter chain
        current  = (filterPipeline *)filterChain;


        while (current != NULL) {
            int32_t result = current->filter->filter((stateFilter *)current->filter, &states);
            if (result > alarm) {
                alarm = result;
            }
            current = current->next;
        }


        // the final output of filters is saved in state variables
        EXPORT_STATE_IF_UPDATED_3_DIMENSIONS(GyroState, gyro, gyroX, gyroY, gyroZ);
        EXPORT_STATE_IF_UPDATED_3_DIMENSIONS(AccelState, accel, accelX, accelY, accelZ);
        EXPORT_STATE_IF_UPDATED_3_DIMENSIONS(MagState, mag, magX, magY, magZ);
        EXPORT_STATE_IF_UPDATED_3_DIMENSIONS(PositionState, pos, PosNorth, PosEast, PosDown);
        EXPORT_STATE_IF_UPDATED_3_DIMENSIONS(VelocityState, vel, VelNorth, VelEast, VelDown);
        EXPORT_STATE_IF_UPDATED_2_DIMENSIONS(AirspeedState, airspeed, CalibratedAirspeed, TrueAirspeed);

        // attitude nees manual conversion from quaternion to euler
        if (IS_SET(states.updated, in_q1)) {
            AttitudeState_t* objAttitudeState;
            checkout_AttitudeState(module, &objAttitudeState);
            objAttitudeState->Q1 = states.attitude[0];
            objAttitudeState->Q2 = states.attitude[1];
            objAttitudeState->Q3 = states.attitude[2];
            objAttitudeState->Q4 = states.attitude[3];
            Quaternion2RPY(&objAttitudeState->Q1, &objAttitudeState->Roll);
            checkin_AttitudeState(module, &objAttitudeState);
        }


        // throttle alarms, raise alarm flags immediately
        // but require system to run for a while before decreasing
        // to prevent alarm flapping
        ///if (alarm >= lastAlarm) {
        ///    lastAlarm    = alarm;
        ///    alarmcounter = 0;
        ///} else {
        ///    if (alarmcounter < 100) {
        ///        alarmcounter++;
        ///    } else {
        ///        lastAlarm    = alarm;
        ///        alarmcounter = 0;
        ///    }
        ///}

        // clear alarms if everything is alright, then schedule callback execution after timeout
        ///if (lastAlarm == 1) {
            ///AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_WARNING);
        ///} else if (lastAlarm == 2) {
            ///AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_ERROR);
        ///} else if (lastAlarm >= 3) {
            ///AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_CRITICAL);
        ///} else {
            ///AlarmsClear(SYSTEMALARMS_ALARM_ATTITUDE);
        ///}

        cycle++;
    }
}

void state_estimation_command (state_estimation_command_t type_command, void* params)
{
    switch (type_command)
    {
        case cmd_command0:
        break;

        case cmd_command1:
        break;

        case cmd_command2:
        break;

        default:
            printf("state_estimation_command. Unknown command: %i.\n", type_command);
    }
}
