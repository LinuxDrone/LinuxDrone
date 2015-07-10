
#ifndef __AP_HAL_LDR_CLASS_H__
#define __AP_HAL_LDR_CLASS_H__

#include <AP_HAL.h>

#include "AP_HAL_Ldr_Namespace.h"

class HAL_Ldr : public AP_HAL::HAL {
public:
    HAL_Ldr();
    void init(int argc, char * const * argv) const;
};

extern const HAL_Ldr AP_HAL_Ldr;

#endif // __AP_HAL_LDR_CLASS_H__

