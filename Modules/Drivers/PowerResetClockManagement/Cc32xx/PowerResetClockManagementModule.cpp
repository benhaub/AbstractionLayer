//AbstractionLayer
#include "PowerResetClockManagementModule.hpp"
//TI driverlib
#include "ti/devices/cc32xx/inc/hw_types.h"
#include "ti/devices/cc32xx/driverlib/prcm.h"
#include "ti/drivers/power/PowerCC32XX.h"

ErrorType PowerResetClockManagement::init() {
    PRCMCC3200MCUInit();
    return fromPlatformError(Power_init());
}

ErrorType PowerResetClockManagement::setClockFrequency(const Hertz frequency, const Hertz externalCrystalFrequency) {
    return ErrorType::NotImplemented;
}