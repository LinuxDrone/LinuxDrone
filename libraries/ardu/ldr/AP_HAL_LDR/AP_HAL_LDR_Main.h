

#ifndef __AP_HAL_LDR_MAIN_H__
#define __AP_HAL_LDR_MAIN_H__

#if CONFIG_HAL_BOARD == HAL_BOARD_LINUX
#define AP_HAL_MAIN() extern "C" {\
    int main (void) {\
	hal.init(0, NULL);			\
        setup();\
        hal.scheduler->system_initialized(); \
        for(;;) loop();\
        return 0;\
    }\
    }
#endif // HAL_BOARD_LDR

#endif // __AP_HAL_LDR_MAIN_H__
