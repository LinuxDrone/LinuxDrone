//#include "ardu.helper.h"


/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 *  ArduCopter Version 3.0
 *  Creator:        Jason Short
 *  Lead Developer: Randy Mackay
 *  Lead Tester:    Marco Robustini
 *  Based on code and ideas from the Arducopter team: Leonard Hall, Andrew Tridgell, Robert Lefebvre, Pat Hickey, Michael Oborne, Jani Hirvinen,
                                                      Olivier Adler, Kevin Hester, Arthur Benemann, Jonathan Challinger, John Arne Birkeland,
                                                      Jean-Louis Naudin, Mike Smith, and more
 *  Thanks to:	Chris Anderson, Jordi Munoz, Jason Short, Doug Weibel, Jose Julio
 *
 *  Special Thanks to contributors (in alphabetical order by first name):
 *
 *  Adam M Rivera       :Auto Compass Declination
 *  Amilcar Lucas       :Camera mount library
 *  Andrew Tridgell     :General development, Mavlink Support
 *  Angel Fernandez     :Alpha testing
 *  AndreasAntonopoulous:GeoFence
 *  Arthur Benemann     :DroidPlanner GCS
 *  Benjamin Pelletier  :Libraries
 *  Bill King           :Single Copter
 *  Christof Schmid     :Alpha testing
 *  Craig Elder         :Release Management, Support
 *  Dani Saez           :V Octo Support
 *  Doug Weibel	        :DCM, Libraries, Control law advice
 *  Emile Castelnuovo   :VRBrain port, bug fixes
 *  Gregory Fletcher    :Camera mount orientation math
 *  Guntars             :Arming safety suggestion
 *  HappyKillmore       :Mavlink GCS
 *  Hein Hollander      :Octo Support, Heli Testing
 *  Igor van Airde      :Control Law optimization
 *  Jack Dunkle         :Alpha testing
 *  James Goppert       :Mavlink Support
 *  Jani Hiriven        :Testing feedback
 *  Jean-Louis Naudin   :Auto Landing
 *  John Arne Birkeland	:PPM Encoder
 *  Jose Julio          :Stabilization Control laws, MPU6k driver
 *  Julien Dubois       :PosHold flight mode
 *  Julian Oes          :Pixhawk
 *  Jonathan Challinger :Inertial Navigation, CompassMot, Spin-When-Armed
 *  Kevin Hester        :Andropilot GCS
 *  Max Levine          :Tri Support, Graphics
 *  Leonard Hall        :Flight Dynamics, Throttle, Loiter and Navigation Controllers
 *  Marco Robustini     :Lead tester
 *  Michael Oborne      :Mission Planner GCS
 *  Mike Smith          :Pixhawk driver, coding support
 *  Olivier Adler       :PPM Encoder, piezo buzzer
 *  Pat Hickey          :Hardware Abstraction Layer (HAL)
 *  Robert Lefebvre     :Heli Support, Copter LEDs
 *  Roberto Navoni      :Library testing, Porting to VRBrain
 *  Sandro Benigno      :Camera support, MinimOSD
 *  Sandro Tognana      :PosHold flight mode
 *  ..and many more.
 *
 *  Code commit statistics can be found here: https://github.com/diydrones/ardupilot/graphs/contributors
 *  Wiki: http://copter.ardupilot.com/
 *  Requires modified version of Arduino, which can be found here: http://ardupilot.com/downloads/?category=6
 *
 */

#include "Copter.h"

#define SCHED_TASK(func) FUNCTOR_BIND(&copter, &Copter::func, void)

/*
  scheduler table for fast CPUs - all regular tasks apart from the fast_loop()
  should be listed here, along with how often they should be called
  (in 2.5ms units) and the maximum time they are expected to take (in
  microseconds)
  1    = 400hz
  2    = 200hz
  4    = 100hz
  8    = 50hz
  20   = 20hz
  40   = 10hz
  133  = 3hz
  400  = 1hz
  4000 = 0.1hz

 */
const AP_Scheduler::Task Copter::scheduler_tasks[] PROGMEM = {
    { SCHED_TASK(rc_loop),               4,    130 },   // 0
    { SCHED_TASK(throttle_loop),         8,     75 },   // 1
    { SCHED_TASK(update_GPS),            8,    200 },   // 2
#if OPTFLOW == ENABLED
    { SCHED_TASK(update_optical_flow),   2,    160 },   // 3
#endif
    { SCHED_TASK(update_batt_compass),  40,    120 },   // 4
    { SCHED_TASK(read_aux_switches),    40,     50 },   // 5
    { SCHED_TASK(arm_motors_check),     40,     50 },   // 6
    { SCHED_TASK(auto_trim),            40,     75 },   // 7
    { SCHED_TASK(update_altitude),      40,    140 },   // 8
    { SCHED_TASK(run_nav_updates),       8,    100 },   // 9
    { SCHED_TASK(update_thr_average),    4,     90 },   // 10
    { SCHED_TASK(three_hz_loop),       133,     75 },   // 11
    { SCHED_TASK(compass_accumulate),    8,    100 },   // 12
    { SCHED_TASK(barometer_accumulate),  8,     90 },   // 13
#if FRAME_CONFIG == HELI_FRAME
    { SCHED_TASK(check_dynamic_flight),  8,     75 },
#endif
    { SCHED_TASK(update_notify),         8,     90 },   // 14
    { SCHED_TASK(one_hz_loop),         400,    100 },   // 15
    { SCHED_TASK(ekf_check),            40,     75 },   // 16
    { SCHED_TASK(crash_check),          40,     75 },   // 17
    { SCHED_TASK(landinggear_update),   40,     75 },   // 18
    { SCHED_TASK(lost_vehicle_check),   40,     50 },   // 19
    { SCHED_TASK(gcs_check_input),       1,    180 },   // 20
    { SCHED_TASK(gcs_send_heartbeat),  400,    110 },   // 21
    { SCHED_TASK(gcs_send_deferred),     8,    550 },   // 22
    { SCHED_TASK(gcs_data_stream_send),  8,    550 },   // 23
    { SCHED_TASK(update_mount),          8,     75 },   // 24
    { SCHED_TASK(ten_hz_logging_loop),  40,    350 },   // 25
    { SCHED_TASK(fifty_hz_logging_loop), 8,    110 },   // 26
    { SCHED_TASK(full_rate_logging_loop),1,    100 },   // 27
    { SCHED_TASK(perf_update),        4000,     75 },   // 28
    { SCHED_TASK(read_receiver_rssi),   40,     75 },   // 29
#if FRSKY_TELEM_ENABLED == ENABLED
    { SCHED_TASK(frsky_telemetry_send), 80,     75 },   // 30
#endif
#if EPM_ENABLED == ENABLED
    { SCHED_TASK(epm_update),           40,     75 },   // 31
#endif
#ifdef USERHOOK_FASTLOOP
    { SCHED_TASK(userhook_FastLoop),     4,     75 },
#endif
#ifdef USERHOOK_50HZLOOP
    { SCHED_TASK(userhook_50Hz),         8,     75 },
#endif
#ifdef USERHOOK_MEDIUMLOOP
    { SCHED_TASK(userhook_MediumLoop),  40,     75 },
#endif
#ifdef USERHOOK_SLOWLOOP
    { SCHED_TASK(userhook_SlowLoop),    120,    75 },
#endif
#ifdef USERHOOK_SUPERSLOWLOOP
    { SCHED_TASK(userhook_SuperSlowLoop),400,   75 },
#endif
};


void Copter::setup()
{
    cliSerial = hal.console;

    // Load the default values of variables listed in var_info[]s
    AP_Param::setup_sketch_defaults();

    // setup storage layout for copter
    StorageManager::set_layout_copter();

    init_ardupilot();

    // initialise the main loop scheduler
    scheduler.init(&scheduler_tasks[0], sizeof(scheduler_tasks)/sizeof(scheduler_tasks[0]));

    // setup initial performance counters
    perf_info_reset();
    fast_loopTimer = hal.scheduler->micros();
}

/*
  if the compass is enabled then try to accumulate a reading
 */
void Copter::compass_accumulate(void)
{
    if (g.compass_enabled) {
        compass.accumulate();
    }
}

/*
  try to accumulate a baro reading
 */
void Copter::barometer_accumulate(void)
{
    barometer.accumulate();
}

void Copter::perf_update(void)
{
    if (should_log(MASK_LOG_PM))
        Log_Write_Performance();
    if (scheduler.debug()) {
        gcs_send_text_fmt(PSTR("PERF: %u/%u %lu %lu\n"),
                          (unsigned)perf_info_get_num_long_running(),
                          (unsigned)perf_info_get_num_loops(),
                          (unsigned long)perf_info_get_max_time(),
                          (unsigned long)perf_info_get_min_time());
    }
    perf_info_reset();
    pmTest1 = 0;
}

void Copter::loop()
{
    // wait for an INS sample
    ins.wait_for_sample();

    uint32_t timer = micros();

    // check loop time
    perf_info_check_loop_time(timer - fast_loopTimer);

    // used by PI Loops
    G_Dt                    = (float)(timer - fast_loopTimer) / 1000000.0f;
    fast_loopTimer          = timer;

    // for mainloop failure monitoring
    mainLoop_count++;

    // Execute the fast loop
    // ---------------------
    fast_loop();

    // tell the scheduler one tick has passed
    scheduler.tick();

    // run all the tasks that are due to run. Note that we only
    // have to call this once per loop, as the tasks are scheduled
    // in multiples of the main loop tick. So if they don't run on
    // the first call to the scheduler they won't run on a later
    // call until scheduler.tick() is called again
    uint32_t time_available = (timer + MAIN_LOOP_MICROS) - micros();
    scheduler.run(time_available);
}


// Main loop - 400hz
void Copter::fast_loop()
{

    // IMU DCM Algorithm
    // --------------------
    read_AHRS();

    // run low level rate controllers that only require IMU data
    attitude_control.rate_controller_run();

#if FRAME_CONFIG == HELI_FRAME
    update_heli_control_dynamics();
#endif //HELI_FRAME

    // send outputs to the motors library
    motors_output();

    // Inertial Nav
    // --------------------
    read_inertia();

    // run the attitude controllers
    update_flight_mode();

    // update home from EKF if necessary
    update_home_from_EKF();

    // check if we've landed
    update_land_detector();
}

// rc_loops - reads user input from transmitter/receiver
// called at 100hz
void Copter::rc_loop()
{
    // Read radio and 3-position switch on radio
    // -----------------------------------------
    read_radio();
    read_control_switch();
}

// throttle_loop - should be run at 50 hz
// ---------------------------
void Copter::throttle_loop()
{
    // get altitude and climb rate from inertial lib
    read_inertial_altitude();

    // update throttle_low_comp value (controls priority of throttle vs attitude control)
    update_throttle_thr_mix();

    // check auto_armed status
    update_auto_armed();

#if FRAME_CONFIG == HELI_FRAME
    // update rotor speed
    heli_update_rotor_speed_targets();

    // update trad heli swash plate movement
    heli_update_landing_swash();
#endif
}

// update_mount - update camera mount position
// should be run at 50hz
void Copter::update_mount()
{
#if MOUNT == ENABLED
    // update camera mount's position
    camera_mount.update();
#endif

#if CAMERA == ENABLED
    camera.trigger_pic_cleanup();
#endif
}

// update_batt_compass - read battery and compass
// should be called at 10hz
void Copter::update_batt_compass(void)
{
    // read battery before compass because it may be used for motor interference compensation
    read_battery();

    if(g.compass_enabled) {
        // update compass with throttle value - used for compassmot
        compass.set_throttle(motors.get_throttle()/1000.0f);
        compass.read();
        // log compass information
        if (should_log(MASK_LOG_COMPASS)) {
            DataFlash.Log_Write_Compass(compass);
        }
    }
}

// ten_hz_logging_loop
// should be run at 10hz
void Copter::ten_hz_logging_loop()
{
    // log attitude data if we're not already logging at the higher rate
    if (should_log(MASK_LOG_ATTITUDE_MED) && !should_log(MASK_LOG_ATTITUDE_FAST)) {
        Log_Write_Attitude();
        Log_Write_Rate();
        if (should_log(MASK_LOG_PID)) {
            DataFlash.Log_Write_PID(LOG_PIDR_MSG, g.pid_rate_roll.get_pid_info() );
            DataFlash.Log_Write_PID(LOG_PIDP_MSG, g.pid_rate_pitch.get_pid_info() );
            DataFlash.Log_Write_PID(LOG_PIDY_MSG, g.pid_rate_yaw.get_pid_info() );
            DataFlash.Log_Write_PID(LOG_PIDA_MSG, g.pid_accel_z.get_pid_info() );
        }
    }
    if (should_log(MASK_LOG_MOTBATT)) {
        Log_Write_MotBatt();
    }
    if (should_log(MASK_LOG_RCIN)) {
        DataFlash.Log_Write_RCIN();
    }
    if (should_log(MASK_LOG_RCOUT)) {
        DataFlash.Log_Write_RCOUT();
    }
    if (should_log(MASK_LOG_NTUN) && (mode_requires_GPS(control_mode) || landing_with_GPS())) {
        Log_Write_Nav_Tuning();
    }
    if (should_log(MASK_LOG_IMU) || should_log(MASK_LOG_IMU_FAST) || should_log(MASK_LOG_IMU_RAW)) {
        DataFlash.Log_Write_Vibration(ins);
    }
}

// fifty_hz_logging_loop
// should be run at 50hz
void Copter::fifty_hz_logging_loop()
{
#if HIL_MODE != HIL_MODE_DISABLED
    // HIL for a copter needs very fast update of the servo values
    gcs_send_message(MSG_RADIO_OUT);
#endif

#if HIL_MODE == HIL_MODE_DISABLED
    if (should_log(MASK_LOG_ATTITUDE_FAST)) {
        Log_Write_Attitude();
        Log_Write_Rate();
        if (should_log(MASK_LOG_PID)) {
            DataFlash.Log_Write_PID(LOG_PIDR_MSG, g.pid_rate_roll.get_pid_info() );
            DataFlash.Log_Write_PID(LOG_PIDP_MSG, g.pid_rate_pitch.get_pid_info() );
            DataFlash.Log_Write_PID(LOG_PIDY_MSG, g.pid_rate_yaw.get_pid_info() );
            DataFlash.Log_Write_PID(LOG_PIDA_MSG, g.pid_accel_z.get_pid_info() );
        }
    }

    // log IMU data if we're not already logging at the higher rate
    if (should_log(MASK_LOG_IMU) && !should_log(MASK_LOG_IMU_FAST)) {
        DataFlash.Log_Write_IMU(ins);
    }
#endif
}

// full_rate_logging_loop
// should be run at the MAIN_LOOP_RATE
void Copter::full_rate_logging_loop()
{
    if (should_log(MASK_LOG_IMU_FAST)) {
        DataFlash.Log_Write_IMU(ins);
    }
}

// three_hz_loop - 3.3hz loop
void Copter::three_hz_loop()
{
    // check if we've lost contact with the ground station
    failsafe_gcs_check();

#if AC_FENCE == ENABLED
    // check if we have breached a fence
    fence_check();
#endif // AC_FENCE_ENABLED

#if SPRAYER == ENABLED
    sprayer.update();
#endif

    update_events();

    // update ch6 in flight tuning
    tuning();
}

// one_hz_loop - runs at 1Hz
void Copter::one_hz_loop()
{
    if (should_log(MASK_LOG_ANY)) {
        Log_Write_Data(DATA_AP_STATE, ap.value);
    }

    // perform pre-arm checks & display failures every 30 seconds
    static uint8_t pre_arm_display_counter = 15;
    pre_arm_display_counter++;
    if (pre_arm_display_counter >= 30) {
        pre_arm_checks(true);
        pre_arm_display_counter = 0;
    }else{
        pre_arm_checks(false);
    }

    // auto disarm checks
    auto_disarm_check();

    if (!motors.armed()) {
        // make it possible to change ahrs orientation at runtime during initial config
        ahrs.set_orientation();

        // check the user hasn't updated the frame orientation
        motors.set_frame_orientation(g.frame_orientation);

        // set all throttle channel settings
        motors.set_throttle_range(g.throttle_min, channel_throttle->radio_min, channel_throttle->radio_max);
    }

    // update assigned functions and enable auxiliar servos
    RC_Channel_aux::enable_aux_servos();

    check_usb_mux();

#if AP_TERRAIN_AVAILABLE
    terrain.update();

    // tell the rangefinder our height, so it can go into power saving
    // mode if available
#if CONFIG_SONAR == ENABLED
    float height;
    if (terrain.height_above_terrain(height, true)) {
        sonar.set_estimated_terrain_height(height);
    }
#endif
#endif

    // update position controller alt limits
    update_poscon_alt_max();

    // enable/disable raw gyro/accel logging
    ins.set_raw_logging(should_log(MASK_LOG_IMU_RAW));
}

// called at 50hz
void Copter::update_GPS(void)
{
    static uint32_t last_gps_reading[GPS_MAX_INSTANCES];   // time of last gps message
    bool gps_updated = false;

    gps.update();

    // log after every gps message
    for (uint8_t i=0; i<gps.num_sensors(); i++) {
        if (gps.last_message_time_ms(i) != last_gps_reading[i]) {
            last_gps_reading[i] = gps.last_message_time_ms(i);

            // log GPS message
            if (should_log(MASK_LOG_GPS)) {
                DataFlash.Log_Write_GPS(gps, i, current_loc.alt);
            }

            gps_updated = true;
        }
    }

    if (gps_updated) {
        // set system time if necessary
        set_system_time_from_GPS();

        // checks to initialise home and take location based pictures
        if (gps.status() >= AP_GPS::GPS_OK_FIX_3D) {

#if CAMERA == ENABLED
            if (camera.update_location(current_loc) == true) {
                do_take_picture();
            }
#endif
        }
    }
}

void Copter::init_simple_bearing()
{
    // capture current cos_yaw and sin_yaw values
    simple_cos_yaw = ahrs.cos_yaw();
    simple_sin_yaw = ahrs.sin_yaw();

    // initialise super simple heading (i.e. heading towards home) to be 180 deg from simple mode heading
    super_simple_last_bearing = wrap_360_cd(ahrs.yaw_sensor+18000);
    super_simple_cos_yaw = simple_cos_yaw;
    super_simple_sin_yaw = simple_sin_yaw;

    // log the simple bearing to dataflash
    if (should_log(MASK_LOG_ANY)) {
        Log_Write_Data(DATA_INIT_SIMPLE_BEARING, ahrs.yaw_sensor);
    }
}

// update_simple_mode - rotates pilot input if we are in simple mode
void Copter::update_simple_mode(void)
{
    float rollx, pitchx;

    // exit immediately if no new radio frame or not in simple mode
    if (ap.simple_mode == 0 || !ap.new_radio_frame) {
        return;
    }

    // mark radio frame as consumed
    ap.new_radio_frame = false;

    if (ap.simple_mode == 1) {
        // rotate roll, pitch input by -initial simple heading (i.e. north facing)
        rollx = channel_roll->control_in*simple_cos_yaw - channel_pitch->control_in*simple_sin_yaw;
        pitchx = channel_roll->control_in*simple_sin_yaw + channel_pitch->control_in*simple_cos_yaw;
    }else{
        // rotate roll, pitch input by -super simple heading (reverse of heading to home)
        rollx = channel_roll->control_in*super_simple_cos_yaw - channel_pitch->control_in*super_simple_sin_yaw;
        pitchx = channel_roll->control_in*super_simple_sin_yaw + channel_pitch->control_in*super_simple_cos_yaw;
    }

    // rotate roll, pitch input from north facing to vehicle's perspective
    channel_roll->control_in = rollx*ahrs.cos_yaw() + pitchx*ahrs.sin_yaw();
    channel_pitch->control_in = -rollx*ahrs.sin_yaw() + pitchx*ahrs.cos_yaw();
}

// update_super_simple_bearing - adjusts simple bearing based on location
// should be called after home_bearing has been updated
void Copter::update_super_simple_bearing(bool force_update)
{
    // check if we are in super simple mode and at least 10m from home
    if(force_update || (ap.simple_mode == 2 && home_distance > SUPER_SIMPLE_RADIUS)) {
        // check the bearing to home has changed by at least 5 degrees
        if (labs(super_simple_last_bearing - home_bearing) > 500) {
            super_simple_last_bearing = home_bearing;
            float angle_rad = radians((super_simple_last_bearing+18000)/100);
            super_simple_cos_yaw = cosf(angle_rad);
            super_simple_sin_yaw = sinf(angle_rad);
        }
    }
}

void Copter::read_AHRS(void)
{
    // Perform IMU calculations and get attitude info
    //-----------------------------------------------
#if HIL_MODE != HIL_MODE_DISABLED
    // update hil before ahrs update
    gcs_check_input();
#endif

    ahrs.update();
}

// read baro and sonar altitude at 10hz
void Copter::update_altitude()
{
    // read in baro altitude
    read_barometer();

    // read in sonar altitude
    sonar_alt           = read_sonar();

    // write altitude info to dataflash logs
    if (should_log(MASK_LOG_CTUN)) {
        Log_Write_Control_Tuning();
    }
}

/*
  compatibility with old pde style build
 */
void setup(void);
void loop(void);

void setup(void)
{
    copter.setup();
}
void loop(void)
{
    copter.loop();
}

AP_HAL_MAIN();



/*
void ardu_run (module_ardu_t *module)
{
    // ldr hal.init(0, NULL);			\ // Херь всякая инициализарующая периыерию ardupilot\libraries\AP_HAL_AVR\HAL_AVR_APM2_Class.cpp
    setup();
    //ldr hal.scheduler->system_initialized(); \
    //ldr for(;;) loop();\



    int cycle=0;
    while(1) {
        get_input_data((module_t*)module);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
        }
        else
        {
            // вышел таймаут
        }

        input_t* input = (input_t*)module->module_info.input_data;

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

        // Принудительное считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}

void ardu_command (ardu_command_t type_command, void* params)
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
            printf("ardu_command. Unknown command: %i.\n", type_command);
    }
}
*/
