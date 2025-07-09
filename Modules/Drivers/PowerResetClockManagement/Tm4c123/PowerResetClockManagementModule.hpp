#ifndef __POWER_RESET_CLOCK_MANAGEMENT_MODULE_HPP__
#define __POWER_RESET_CLOCK_MANAGEMENT_MODULE_HPP__

#include "PowerResetClockManagementAbstraction.hpp"
//TI
#include "driverlib/sysctl.h"

class PowerResetClockManagement : public PowerResetClockManagementAbstraction {
    public:
    PowerResetClockManagement() {
        //This must have something to do with how I'm linking libraries in CMake,
        //but TI libdriver code will not be linked to this libraary if I don't reference it somewhere in the header file.
        //This function is not meant to have any purpose other than to make a reference to the library so linking works.
        SysCtlClockGet();
    }

    ErrorType init() override;
    ErrorType setClockFrequency(const Hertz frequency, const Hertz externalCrystalFrequency) override;

    private:
    uint32_t toTm4c123SysCtlClockFrequency(const Hertz frequency, const bool usePll, const bool useMainOscillator);
    uint32_t toTm4c123SysCtlExternalClockFrequency(const Hertz externalCrystalFrequency, const bool usePll);

    constexpr bool isValidFrequency(const Hertz frequency) {
        return frequency >= 3.125E6 && frequency <= 80E6;
    }

    constexpr uint32_t toTm4c123SysCtlDivider(const uint32_t divider);
    constexpr uint32_t toTm4c123SysCtlDividerPlusHalf(const uint32_t divider);
    constexpr uint32_t toTm4c123SysCtlDividerMinusHalf(const uint32_t divider);
};

#endif //__POWER_RESET_CLOCK_MANAGEMENT_HPP__