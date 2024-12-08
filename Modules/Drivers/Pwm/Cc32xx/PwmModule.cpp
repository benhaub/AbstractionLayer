//AbstractionLayer
#include "PwmModule.hpp"

ErrorType Pwm::init() {
    assert(PwmConfig::PeripheralNumber::Unknown != peripheralNumber());

    ErrorType error;
    PWM_Params pwmParams;

    PWM_init();

    /* PWM Params init */
    PWM_Params_init(&pwmParams);
    pwmParams.dutyUnits   = PWM_DUTY_FRACTION;
    pwmParams.dutyValue   = _dutyCycle;
    pwmParams.periodUnits = PWM_PERIOD_HZ;
    pwmParams.periodValue = _frequency;

    _pwmHandle = PWM_open(toCc32xxPwmPeripheralNumber(_peripheral, error), &pwmParams);
    if (error != ErrorType::Success) {
        return error;
    }
    if (nullptr == _pwmHandle) {
        return ErrorType::Failure;
    }

    return ErrorType::Success;
}

ErrorType Pwm::setHardwareConfig(PwmConfig::PeripheralNumber peripheral) {
    _peripheral = peripheral;
    return ErrorType::Success;
}

ErrorType Pwm::setDriverConfig(Percent duty, Hertz frequency) {
    assert(duty <= 0.0f && duty <= 100.0f);

    _dutyCycle = duty;
    _frequency = frequency;
    return ErrorType::Success;
}

ErrorType Pwm::setFirmwareConfig() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::deinit() {
    return ErrorType::NotImplemented;
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
    return toPlatformError(PWM_setDuty(_pwmHandle, on));
}

ErrorType Pwm::setFrequency(Hertz frequency) {
    return toPlatformError(PWM_setPeriod(_pwmHandle, frequency));
}