//AbstractionLayer
#include "GptmPwmModule.hpp"
//ESP
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_gen.h"

ErrorType GptmPwmModule::init() {
    assert(PeripheralNumber::Unknown != peripheralNumber());

    const mcpwm_timer_config_t timerConfig = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = millisecondsToEspTimerTicks(_period) 
    };

    esp_err_t err;
    if (ESP_OK == (err = mcpwm_new_timer(&timerConfig, &_timer))) {
        mcpwm_oper_handle_t oper = NULL;
        const mcpwm_operator_config_t operatorConfig = {
            .group_id = 0
        };
        if (ESP_OK == (err = mcpwm_new_operator(&operatorConfig, &oper))) {
            if (ESP_OK == (err = mcpwm_operator_connect_timer(oper, _timer))) {
                const mcpwm_comparator_config_t comparatorConfig = {
                    .flags = {
                        .update_cmp_on_tez = true
                    }
                };
                if (ESP_OK == (err = mcpwm_new_comparator(oper, &comparatorConfig, &_comparator))) {
                    mcpwm_gen_handle_t generator = NULL;
                    const mcpwm_generator_config_t generatorConfig = {
                        .gen_gpio_num = outputPinConst()
                    };
                    if (ESP_OK == (err = mcpwm_new_generator(oper, &generatorConfig, &generator))) {
                        setDutyCycle(_dutyCycle);
                        //Compare event to cause the signal to go high
                        mcpwm_generator_set_action_on_timer_event(generator, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
                        //Compare event to cause the signal to go low
                        mcpwm_generator_set_action_on_compare_event(generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, _comparator, MCPWM_GEN_ACTION_LOW));
                        if (ESP_OK == (err = mcpwm_timer_enable(_timer))) {
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
    return ErrorType::NotImplemented;
}

ErrorType GptmPwmModule::start() {
    esp_err_t err;
    err = mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP);
    return fromPlatformError(err);
}

ErrorType GptmPwmModule::stop() {
    esp_err_t err;
    err = mcpwm_timer_start_stop(_timer, MCPWM_TIMER_STOP_FULL);
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
        const Milliseconds dutyValue = (on / 100) * _period;
        err = mcpwm_comparator_set_compare_value(_comparator, millisecondsToEspTimerTicks(dutyValue));
    }

    return fromPlatformError(err);
}

ErrorType GptmPwmModule::setPeriod(const Milliseconds period) {
    _period = period;
    esp_err_t err = ESP_FAIL;

    if (nullptr == _timer) {
        err = ESP_OK;
    }
    else {
        err = mcpwm_timer_set_period(_timer, millisecondsToEspTimerTicks(period));
    }

    return fromPlatformError(err);
}