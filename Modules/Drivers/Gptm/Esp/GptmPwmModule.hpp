#ifndef __GPTM_PWM_MODULE_HPP__
#define __GPTM_PWM_MODULE_HPP__

#include "GptmPwmAbstraction.hpp"
//ESP
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_oper.h"

class GptmPwmModule final : public GptmPwmAbstraction {

    public:
    GptmPwmModule() : GptmPwmAbstraction() {}

    /**
     * @brief ESP PWM hardware limit on the amount of timers per PWM block.
     */
    static constexpr Count _MaxTimersPerGroup = 2;
    /**
     * @brief The timer resolution in ticks per second.
     * @details The higher the resolution, the less the period can be since it
     *          must be between 0 and 2^16
     */
    static constexpr uint32_t _TimerResolution = static_cast<uint32_t>(10E6f);

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(const Percent on) override;
    ErrorType setPeriod(const Microseconds period) override;

    private:
    /// @brief The group ID of the timer.
    int _groupId = -1;
    /// @brief The timer handle.
    mcpwm_timer_handle_t _timer = nullptr;
    /// @brief The comparator handle.
    mcpwm_cmpr_handle_t _comparator = nullptr;
    /// @brief The generator handle.
    mcpwm_gen_handle_t _generator = nullptr;
    /// @brief The operator handle.
    mcpwm_oper_handle_t _operator = nullptr;

    /// @brief Converts a period in microseconds to ticks.
    /// @param[in] period The period in microseconds.
    /// @return The period in ticks.
    constexpr uint32_t microsecondsToEspTimerTicks(const Microseconds period);
};

#endif //__GPTM_PWM_MODULE_HPP__