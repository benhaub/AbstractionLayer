//AbstractionLayer
#include "GptmPwmModule.hpp"
//ESP
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_gen.h"
#include "hal/mcpwm_ll.h"

namespace {
    Count _GroupZeroActiveTimers = 0;
    Count _GroupOneActiveTimers = 0;
}

ErrorType GptmPwmModule::init() {
    assert(PeripheralNumber::Unknown != peripheralNumber());

    if (_GroupZeroActiveTimers < _MaxTimersPerGroup) {
        _groupId = 0;
    }
    else if (_GroupOneActiveTimers < _MaxTimersPerGroup) {
        _groupId = 1;
    }
    else {
        return ErrorType::NotAvailable;
    }

    const mcpwm_timer_config_t timerConfig = {
        .group_id = _groupId,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = _TimerResolution,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = microsecondsToEspTimerTicks(_period) 
    };

    esp_err_t err = ESP_FAIL;
    if (ESP_OK == (err = mcpwm_new_timer(&timerConfig, &_timer))) {
        const mcpwm_operator_config_t operatorConfig = {
            .group_id = _groupId
        };
        if (ESP_OK == (err = mcpwm_new_operator(&operatorConfig, &_operator))) {
            if (ESP_OK == (err = mcpwm_operator_connect_timer(_operator, _timer))) {
                const mcpwm_comparator_config_t comparatorConfig = {
                    .flags = {
                        .update_cmp_on_tez = true
                    }
                };
                if (ESP_OK == (err = mcpwm_new_comparator(_operator, &comparatorConfig, &_comparator))) {
                    const mcpwm_generator_config_t generatorConfig = {
                        .gen_gpio_num = outputPinConst()
                    };
                    if (ESP_OK == (err = mcpwm_new_generator(_operator, &generatorConfig, &_generator))) {
                        setDutyCycle(_dutyCycle);
                        //Compare event to cause the signal to go high
                        mcpwm_generator_set_action_on_timer_event(_generator, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
                        //Compare event to cause the signal to go low
                        mcpwm_generator_set_action_on_compare_event(_generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, _comparator, MCPWM_GEN_ACTION_LOW));
                        if (ESP_OK == (err = mcpwm_timer_enable(_timer))) {
                            if (0 == _groupId) {
                                _GroupZeroActiveTimers += 1;
                            }
                            else if (1 == _groupId) {
                                _GroupOneActiveTimers += 1;
                            }
                            return ErrorType::Success;
                        }
                    }
                }
            }
        }
    }

    return fromPlatformError(err);
}

ErrorType GptmPwmModule::deinit() {
    assert(_groupId != 1);
    assert(_GroupZeroActiveTimers >= 0);
    assert(_GroupOneActiveTimers >= 0);

    if (0 == _groupId) {
        _GroupZeroActiveTimers -= 1;
    }
    else if (1 == _groupId) {
        _GroupOneActiveTimers -= 1;
    }

    mcpwm_del_timer(_timer);
    mcpwm_del_comparator(_comparator);
    mcpwm_del_generator(_generator);
    mcpwm_del_operator(_operator);

    return ErrorType::Success;
}

ErrorType GptmPwmModule::start() {
    esp_err_t err;
    //See GptmPwmModule::stop().
    setDutyCycle(_dutyCycle);
    err = mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP);
    return fromPlatformError(err);
}

ErrorType GptmPwmModule::stop() {
    esp_err_t err;
    //The start_stop function does not support stopping immediately so the best way to do this is to zero out the duty cycle.
    err = mcpwm_comparator_set_compare_value(_comparator, microsecondsToEspTimerTicks(0));
    return fromPlatformError(err);
}

ErrorType GptmPwmModule::setDutyCycle(const Percent on) {
    _dutyCycle = on;
    esp_err_t err = ESP_FAIL;

    if (nullptr == _comparator) {
        err = ESP_OK;
    }
    else {
        //The compare events are set to go low when the compare value is reached, and then high again when the counter reaches max.
        //So if we had a 20ms period and a 10% duty cycle, we'd have the signal going low at 2ms and then high again at 20ms and then
        //restarting back to zero.
        const Microseconds dutyValue = (on / 100) * _period;
        err = mcpwm_comparator_set_compare_value(_comparator, microsecondsToEspTimerTicks(dutyValue));
    }

    return fromPlatformError(err);
}

ErrorType GptmPwmModule::setPeriod(const Microseconds period) {
    _period = period;
    esp_err_t err = ESP_FAIL;

    if (nullptr == _timer) {
        err = ESP_OK;
    }
    else {
        err = mcpwm_timer_set_period(_timer, microsecondsToEspTimerTicks(period));
    }

    return fromPlatformError(err);
}

constexpr uint32_t GptmPwmModule::microsecondsToEspTimerTicks(const Microseconds period) {
    const float periodToResolutionRatio = static_cast<float>(period) / _TimerResolution;
    return static_cast<uint32_t>(static_cast<float>(MCPWM_LL_MAX_COUNT_VALUE) * periodToResolutionRatio);
}