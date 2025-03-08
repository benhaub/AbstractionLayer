#ifndef __GPTM_PWM_MODULE_HPP__
#define __GPTM_PWM_MODULE_HPP__

#include "GptmPwmAbstraction.hpp"
//ESP
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_cmpr.h"

class GptmPwmModule : public GptmPwmAbstraction {

    public:
    GptmPwmModule() : GptmPwmAbstraction() {}
    ~GptmPwmModule() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(const Percent on) override;
    ErrorType setPeriod(const Microseconds period) override;

    private:
    mcpwm_timer_handle_t _timer = nullptr;
    mcpwm_cmpr_handle_t _comparator = nullptr;

    uint32_t microsecondsToEspTimerTicks(const Microseconds period) {
        return (period);
    }
};

#endif //__GPTM_PWM_MODULE_HPP__