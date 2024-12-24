#include "GptmPwmModule.hpp"

ErrorType GptmPwmModule::init() {
    assert(PeripheralNumber::Unknown != peripheralNumber());

    PWM_init();

    /* PWM Params init */
    PWM_Params pwmParams;
    PWM_Params_init(&pwmParams);
    pwmParams.idleLevel = PWM_IDLE_LOW;
    pwmParams.periodUnits = PWM_PERIOD_US;
    pwmParams.periodValue = _period * 1E3;
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

    return toPlatformError(PWM_setDuty(_pwmHandle, pwmParams.dutyValue));
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

ErrorType GptmPwmModule::setDutyCycle(Percent on) {
    if (nullptr == _pwmHandle) {
        _dutyCycle = on;
        return ErrorType::Success;
    }

    const Milliseconds dutyValue = (on / 100) * _period;

    return toPlatformError(PWM_setDuty(_pwmHandle, dutyValue));
}

ErrorType GptmPwmModule::setPeriod(Milliseconds period) {
    if (nullptr == _pwmHandle) {
        _period = period;
        return ErrorType::Success;
    }

    return toPlatformError(PWM_setPeriod(_pwmHandle, period * 1E3));
}