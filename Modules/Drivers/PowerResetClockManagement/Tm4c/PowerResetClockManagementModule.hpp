#ifndef __POWER_RESET_CLOCK_MANAGEMENT_MODULE_HPP__
#define __POWER_RESET_CLOCK_MANAGEMENT_MODULE_HPP__

#include "PowerResetClockManagementAbstraction.hpp"
//TI
#include "driverlib/sysctl.h"

class PowerResetClockManagement : public PowerResetClockManagementAbstraction {

    public:
    ErrorType init() override;
    ErrorType setClockFrequency(const Hertz frequency, const Hertz externalCrystalFrequency) override;

    private:
    uint32_t toTm4cSysCtlClockFrequency(const Hertz frequency, const bool usePll, const bool useMainOscillator);
    uint32_t toTm4cSysCtlExternalClockFrequency(const Hertz externalCrystalFrequency, const bool usePll);

    constexpr bool isValidFrequency(const Hertz frequency) {
#if defined(PART_TM4C1294NCPDT)
        return frequency >= 3.125E6 && frequency <= 120E6;
#elif defined(PART_TM4C123GH6PM)
        return frequency >= 3.125E6 && frequency <= 80E6;
#endif
    }

    constexpr uint32_t toTm4cSysCtlDivider(const uint32_t divider);
    constexpr uint32_t toTm4cSysCtlDividerPlusHalf(const uint32_t divider);
    constexpr uint32_t toTm4cSysCtlDividerMinusHalf(const uint32_t divider);
};

#endif //__POWER_RESET_CLOCK_MANAGEMENT_HPP__