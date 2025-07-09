#include "GptmPwmModule.hpp"
//C++
#include <cassert>

ErrorType GptmPwmModule::init() {
    assert(PeripheralNumber::Unknown != peripheralNumber());

    PWM_init();

    /* PWM Params init */
    PWM_Params pwmParams;
    PWM_Params_init(&pwmParams);
    pwmParams.idleLevel = PWM_IDLE_LOW;
    pwmParams.periodUnits = PWM_PERIOD_US;
    pwmParams.periodValue = _period;
    pwmParams.dutyUnits   = PWM_DUTY_US;
    pwmParams.dutyValue   = (_dutyCycle / 100) * pwmParams.periodValue;

    ErrorType error;
    _pwmHandle = PWM_open(toCc32xxPwmPeripheralNumber(peripheralNumber(), error), &pwmParams);
    if (error != ErrorType::Success) {
        return error;
    }
    if (nullptr == _pwmHandle) {
        return ErrorType::Failure;
    }

    return ErrorType::Success;
}

ErrorType GptmPwmModule::deinit() {
    PWM_close(_pwmHandle);

    return ErrorType::Success;
}

ErrorType GptmPwmModule::start() {
    PWM_start(_pwmHandle);

    return ErrorType::Success;
}

ErrorType GptmPwmModule::stop() {
    PWM_stop(_pwmHandle);

    return ErrorType::Success;
}

ErrorType GptmPwmModule::setDutyCycle(const Percent on) {
    _dutyCycle = on;

    if (nullptr == _pwmHandle) {
        return ErrorType::Success;
    }
    else {
        const Microseconds dutyValue = (on / 100) * _period;
        return fromPlatformError(PWM_setDuty(_pwmHandle, dutyValue));
    }
}

ErrorType GptmPwmModule::setPeriod(const Microseconds period) {
    _period = period;

    if (nullptr == _pwmHandle) {
        return ErrorType::Success;
    }
    else {
        return fromPlatformError(PWM_setPeriod(_pwmHandle, period));
    }
}