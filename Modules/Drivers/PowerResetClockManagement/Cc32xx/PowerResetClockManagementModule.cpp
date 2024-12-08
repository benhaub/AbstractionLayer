//AbstractionLayer
#include "PowerResetClockManagementModule.hpp"
//TI driverlib
#include "ti/devices/cc32xx/inc/hw_types.h"
#include "ti/devices/cc32xx/driverlib/prcm.h"
#include "ti/drivers/power/PowerCC32XX.h"

ErrorType PowerResetClockManagement::init() {
    PRCMCC3200MCUInit();
    return toPlatformError(Power_init());
}