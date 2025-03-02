#include "28byj485V.hpp"
#include "OperatingSystemModule.hpp"

ErrorType Mikroe28byj485V::init() {
    PeripheralNumber peripheral = PeripheralNumber::Unknown;
    constexpr uint8_t numberOfCoils = 4;
    constexpr Milliseconds pwmPeriod = Milliseconds(200);
    constexpr Percent pwmDutyCycle = Percent(100);

    for (int i = 0; i < numberOfCoils; i++) {
        //TODO These vectors need to have a size
        assert(false);
        //TODO: Should also have a constructor similar to Sm10001.
        _pwmIsImplementedByGptm = _gptPwms[i].init() != ErrorType::NotImplemented;
        _pwmIsStandaloneDriver = _pwms[i].init() != ErrorType::NotImplemented;

        switch (i) {
            case 0: peripheral = PeripheralNumber::Zero;
                break;
            case 1: peripheral = PeripheralNumber::One;
                break;
            case 2: peripheral = PeripheralNumber::Two;
                break;
            case 3: peripheral = PeripheralNumber::Three;
                break;
        }

        if (_pwmIsStandaloneDriver) {
            _pwms[i].peripheralNumber() = peripheral;
            _pwms[i].setPeriod(pwmPeriod);
            _pwms[i].setDutyCycle(pwmDutyCycle);
            ErrorType error = _pwms[i].init();
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error || ErrorType::NotImplemented == error);
                if (isCriticalError) {
                    return error;
                }
            }
            else {
                _pwms[i].stop();
            }
        }
        else if (_pwmIsImplementedByGptm) {
            _gptPwms[i].peripheralNumber() = peripheral;
            _gptPwms[i].setPeriod(pwmPeriod);
            _gptPwms[i].setDutyCycle(pwmDutyCycle);
            ErrorType error = _gptPwms[i].init();
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error || ErrorType::NotImplemented == error);
                if (isCriticalError) {
                    return error;
                }
            }
            else {
                _gptPwms[i].stop();
            }
        }
    }

    return ErrorType::Success;
}

ErrorType Mikroe28byj485V::start() {
    constexpr uint8_t orangeWireInput = 3;
    constexpr uint8_t pinkWireInput = 2;
    constexpr uint8_t yellowWireInput = 1;
    constexpr uint8_t blueWireInput = 0;

    //Forward
    for (int i = 0; i < 512; i++) {
        if (_pwmIsStandaloneDriver) {
            _pwms[pinkWireInput].start(); _pwms[orangeWireInput].stop(); _pwms[yellowWireInput].stop(); _pwms[blueWireInput].stop();
            OperatingSystem::Instance().delay(3);
            _pwms[pinkWireInput].stop(); _pwms[orangeWireInput].start(); _pwms[yellowWireInput].stop(); _pwms[blueWireInput].stop();
            OperatingSystem::Instance().delay(3);
            _pwms[pinkWireInput].stop(); _pwms[orangeWireInput].stop(); _pwms[yellowWireInput].start(); _pwms[blueWireInput].stop();
            OperatingSystem::Instance().delay(3);
            _pwms[pinkWireInput].stop(); _pwms[orangeWireInput].stop(); _pwms[yellowWireInput].stop(); _pwms[blueWireInput].start();
            OperatingSystem::Instance().delay(3);
        }
        else {
            _gptPwms[pinkWireInput].start(); _gptPwms[orangeWireInput].stop(); _gptPwms[yellowWireInput].stop(); _gptPwms[blueWireInput].stop();
            OperatingSystem::Instance().delay(3);
            _gptPwms[pinkWireInput].stop(); _gptPwms[orangeWireInput].start(); _gptPwms[yellowWireInput].stop(); _gptPwms[blueWireInput].stop();
            OperatingSystem::Instance().delay(3);
            _gptPwms[pinkWireInput].stop(); _gptPwms[orangeWireInput].stop(); _gptPwms[yellowWireInput].start(); _gptPwms[blueWireInput].stop();
            OperatingSystem::Instance().delay(3);
            _gptPwms[pinkWireInput].stop(); _gptPwms[orangeWireInput].stop(); _gptPwms[yellowWireInput].stop(); _gptPwms[blueWireInput].start();
            OperatingSystem::Instance().delay(3);
        }
    }

    return ErrorType::Success;
}