#include "state-estimation.helper.h"
#include "inc/stateestimation.h"

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
        //FETCH_SENSOR_FROM_UAVOBJECT_CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(GyroSensor, gyro, x, y, z);
        states.gyro[0] = input->gyro_x;
        states.gyro[1] = input->gyro_y;
        states.gyro[2] = input->gyro_z;
        //FETCH_SENSOR_FROM_UAVOBJECT_CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(AccelSensor, accel, x, y, z);
        states.accel[0] = input->accel_x;
        states.accel[1] = input->accel_y;
        states.accel[2] = input->accel_z;
        //FETCH_SENSOR_FROM_UAVOBJECT_CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(MagSensor, mag, x, y, z);
        states.mag[0] = input->mag_x;
        states.mag[1] = input->mag_y;
        states.mag[2] = input->mag_z;
        //FETCH_SENSOR_FROM_UAVOBJECT_CHECK_AND_LOAD_TO_STATE_3_DIMENSIONS(GPSVelocitySensor, vel, North, East, Down);
        states.vel[0] = input->North;
        states.vel[1] = input->East;
        states.vel[2] = input->Down;
        //FETCH_SENSOR_FROM_UAVOBJECT_CHECK_AND_LOAD_TO_STATE_1_DIMENSION_WITH_CUSTOM_EXTRA_CHECK(BaroSensor, baro, Altitude, true);
        states.baro[0] = input->BaroAltitude;


        //FETCH_SENSOR_FROM_UAVOBJECT_CHECK_AND_LOAD_TO_STATE_1_DIMENSION_WITH_CUSTOM_EXTRA_CHECK(AirspeedSensor, airspeed, CalibratedAirspeed, s.SensorConnected == AIRSPEEDSENSOR_SENSORCONNECTED_TRUE);
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
        ///EXPORT_STATE_TO_UAVOBJECT_IF_UPDATED_3_DIMENSIONS(GyroState, gyro, x, y, z);
        ///EXPORT_STATE_TO_UAVOBJECT_IF_UPDATED_3_DIMENSIONS(AccelState, accel, x, y, z);
        ///EXPORT_STATE_TO_UAVOBJECT_IF_UPDATED_3_DIMENSIONS(MagState, mag, x, y, z);
        //EXPORT_STATE_TO_UAVOBJECT_IF_UPDATED_3_DIMENSIONS(PositionState, pos, North, East, Down);
        //EXPORT_STATE_TO_UAVOBJECT_IF_UPDATED_3_DIMENSIONS(VelocityState, vel, North, East, Down);
        //EXPORT_STATE_TO_UAVOBJECT_IF_UPDATED_2_DIMENSIONS(AirspeedState, airspeed, CalibratedAirspeed, TrueAirspeed);
        // attitude nees manual conversion from quaternion to euler
        ///if (IS_SET(states.updated, SENSORUPDATES_attitude)) { \
           /// AttitudeStateData s;
            ///AttitudeStateGet(&s);
            ///s.q1 = states.attitude[0];
            ///s.q2 = states.attitude[1];
            ///s.q3 = states.attitude[2];
            ///s.q4 = states.attitude[3];
            ///Quaternion2RPY(&s.q1, &s.Roll);
            ///AttitudeStateSet(&s);
        ///}

        // throttle alarms, raise alarm flags immediately
        // but require system to run for a while before decreasing
        // to prevent alarm flapping
        if (alarm >= lastAlarm) {
            lastAlarm    = alarm;
            alarmcounter = 0;
        } else {
            if (alarmcounter < 100) {
                alarmcounter++;
            } else {
                lastAlarm    = alarm;
                alarmcounter = 0;
            }
        }

        // clear alarms if everything is alright, then schedule callback execution after timeout
        if (lastAlarm == 1) {
            ///AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_WARNING);
        } else if (lastAlarm == 2) {
            ///AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_ERROR);
        } else if (lastAlarm >= 3) {
            ///AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_CRITICAL);
        } else {
            ///AlarmsClear(SYSTEMALARMS_ALARM_ATTITUDE);
        }


/*
        Output1_t* objOutput1;
        checkout_Output1(module, &objOutput1);
        objOutput1->char_out = input->in1*2+cycle;
        objOutput1->short_out = input->in1*3+cycle;
        objOutput1->int_out = input->in1*4+cycle;
        objOutput1->long_out = input->in1*5+cycle;
        objOutput1->long_long_out = input->in1*6+cycle;
        objOutput1->float_out = input->in1*7+cycle;
        objOutput1->double_out = input->in1*8+cycle;
        char buffer_string_out [32];
        snprintf(buffer_string_out, 32, "data: %d", cycle);
        objOutput1->string_out = buffer_string_out;
        checkin_Output1(module, &objOutput1);

        Output2_t* objOutput2;
        checkout_Output2(module, &objOutput2);
        objOutput2->out3 = input->in1*2+cycle;
        checkin_Output2(module, &objOutput2);

        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask = in1 | in2;
*/
        // Принудительное считывание данных из разделяемой памяти
        //int res = refresh_input(module);

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
