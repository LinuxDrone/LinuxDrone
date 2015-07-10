
#include <AP_HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_LINUX

#include "HAL_LDR_Class.h"
#include "AP_HAL_LDR_Private.h"

using namespace Ldr;

/*static LdrUARTDriver uartADriver;
static LdrUARTDriver uartBDriver;
static LdrUARTDriver uartCDriver;
static LdrSemaphore  i2cSemaphore;
static LdrI2CDriver  i2cDriver(&i2cSemaphore);
static LdrSPIDeviceManager spiDeviceManager;
static LdrAnalogIn analogIn;
static LdrStorage storageDriver;
static LdrGPIO gpioDriver;
static LdrRCInput rcinDriver;*/
static LdrRCOutput rcoutDriver;
/*static LdrScheduler schedulerInstance;
static LdrUtil utilInstance;*/

HAL_Ldr::HAL_Ldr() :
    AP_HAL::HAL(
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL, //&rcoutDriver,
        NULL,
        NULL)
{}

void HAL_Ldr::init(int argc,char* const argv[]) const {
    /* initialize all drivers and private members here.
     * up to the programmer to do this in the correct order.
     * Scheduler should likely come first. */
    //scheduler->init(NULL);
    //uartA->begin(115200);
    //_member->init();
}

const HAL_Ldr AP_HAL_Ldr;

#endif
