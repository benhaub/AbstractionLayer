#ifndef __POWER_RESET_CLOCK_MANAGEMENT_MODULE_HPP__
#define __POWER_RESET_CLOCK_MANAGEMENT_MODULE_HPP__

#include "PowerResetClockManagementAbstraction.hpp"

class PowerResetClockManagement : public PowerResetClockManagementAbstraction {
    public:
    PowerResetClockManagement() = default;

    ErrorType init() override;
    ErrorType setClockFrequency(const Hertz frequency, const Hertz externalCrystalFrequency) override;
};

#endif //__POWER_RESET_CLOCK_MANAGEMENT_HPP__