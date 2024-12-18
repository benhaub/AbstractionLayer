//AbstractionLayer
#include "PwmModule.hpp"

ErrorType Pwm::init() {
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
    _pwmHandle = PWM_open(toCc32xxPwmPeripheralNumber(_peripheral, error), &pwmParams);
    if (error != ErrorType::Success) {
        return error;
    }
    if (nullptr == _pwmHandle) {
        return ErrorType::Failure;
    }

    return toPlatformError(PWM_setDuty(_pwmHandle, pwmParams.dutyValue));
}

ErrorType Pwm::setHardwareConfig(PeripheralNumber peripheral) {
    _peripheral = peripheral;

    return ErrorType::Success;
}

ErrorType Pwm::setDriverConfig(Percent duty, Milliseconds period) {
    assert(duty <= period);

    _dutyCycle = duty;
    _period = period;

    return ErrorType::Success;
}

ErrorType Pwm::setFirmwareConfig() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::deinit() {
    PWM_close(_pwmHandle);

    return ErrorType::Success;
}

ErrorType Pwm::start() {
    PWM_start(_pwmHandle);

    return ErrorType::Success;
}

ErrorType Pwm::stop() {
    PWM_stop(_pwmHandle);

    return ErrorType::Success;
}

ErrorType Pwm::setDutyCycle(Percent on) {
    const Milliseconds dutyValue = (on / 100) * _period;

    return toPlatformError(PWM_setDuty(_pwmHandle, dutyValue));
}

ErrorType Pwm::setPeriod(Milliseconds period) {
    return toPlatformError(PWM_setPeriod(_pwmHandle, period * 1E3));
}