#include "28byj485V.hpp"
#include "OperatingSystemModule.hpp"

ErrorType Mikroe28byj485V::init() {
    ErrorType error = ErrorType::PrerequisitesNotMet;

    if (_darlingtonArray->isDrivenByGpio()) {
        error = ErrorType::NotSupported;
    }
    else if (_darlingtonArray->isDrivenByGptmPwm()) {
        for (auto &gptPwm : _darlingtonArray->gptPwms()) {
            if (-1 != gptPwm->outputPinConst()) {
                if (PeripheralNumber::Unknown != gptPwm->peripheralNumber()) {
                    gptPwm->setPeriod(Microseconds(200000));
                    gptPwm->setDutyCycle(Percent(100));
                    error = gptPwm->init();
                    if (ErrorType::Success == error) {
                        error = gptPwm->stop();
                    }
                }
            }
        }
    }
    else if (_darlingtonArray->isDrivenByStandalonePwm()) {
        for (auto &pwm : _darlingtonArray->standalonePwms()) {
            if (-1 != pwm->outputPinConst()) {
                if (PeripheralNumber::Unknown != pwm->peripheralNumber()) {
                    pwm->setPeriod(Microseconds(200000));
                    pwm->setDutyCycle(Percent(100));
                    error = pwm->init();
                    if (ErrorType::Success == error) {
                        error = pwm->stop();
                    }
                }
            }
        }
    }

    return error;
}

//https://components101.com/motors/28byj-48-stepper-motor
//https://www.python-exemplarisch.ch/index_en.php?inhalt_links=navigation_en.inc.php&inhalt_mitte=raspi/en/steppermotors.inc.php
ErrorType Mikroe28byj485V::rotateForward() {
    constexpr uint8_t orange = 3;
    constexpr uint8_t pink = 2;
    constexpr uint8_t yellow = 1;
    constexpr uint8_t blue = 0;

    //Forward
    for (int i = 0; i < 2048; i++) {
        _darlingtonArray->togglePin(pink, true);
        _darlingtonArray->togglePin(orange, false);
        _darlingtonArray->togglePin(yellow, false);
        _darlingtonArray->togglePin(blue, false);
        OperatingSystem::Instance().delay(Milliseconds(3));
        _darlingtonArray->togglePin(pink, false);
        _darlingtonArray->togglePin(orange, true);
        _darlingtonArray->togglePin(yellow, false);
        _darlingtonArray->togglePin(blue, false);
        OperatingSystem::Instance().delay(Milliseconds(3));
        _darlingtonArray->togglePin(pink, false);
        _darlingtonArray->togglePin(orange, false);
        _darlingtonArray->togglePin(yellow, true);
        _darlingtonArray->togglePin(blue, false);
        OperatingSystem::Instance().delay(Milliseconds(3));
        _darlingtonArray->togglePin(pink, false);
        _darlingtonArray->togglePin(orange, false);
        _darlingtonArray->togglePin(yellow, false);
        _darlingtonArray->togglePin(blue, true);
        OperatingSystem::Instance().delay(Milliseconds(3));
    }

    return ErrorType::Success;
}