//AbstractionLayer
#include "PwmModule.hpp"
#include "Log.hpp"
//TI
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

namespace {
    /**
     * @brief  The outputs available. 0 is avaible, 1 is used.
     * @details The 8 outputs are grouped in 4 generators. Index 0 and 1 are for generator 0, 2,3 for generator 1, etc.
     */
    std::array<uint8_t, 8> _PwmPeripheralZeroAvailableOutputs = {0,0,0,0,0,0,0,0};
    /**
     * @sa _PwmPeripheralZeroAvailableOutputs
     */
    std::array<uint8_t, 8> _PwmPeripheralOneAvailableOutputs = {0,0,0,0,0,0,0,0};
}

ErrorType Pwm::init() {
    ErrorType error = ErrorType::PrerequisitesNotMet;

    if (PeripheralNumber::Unknown != peripheralNumberConst()) {
        const uint32_t tm4c123PwmSysCtlPeripheralNumber = toTm4c123SysCtlPeripheralNumber(peripheralNumberConst(), error);
        if (ErrorType::Success == error) {
            SysCtlPeripheralEnable(tm4c123PwmSysCtlPeripheralNumber);
            while(!SysCtlPeripheralReady(tm4c123PwmSysCtlPeripheralNumber));
            Register tm4c123PwmBaseRegister = toTm4c123PeripheralBaseRegister(peripheralNumberConst(), error);
            if (ErrorType::Success == error) {
                _output = getOutputFromNextAvailableGenerator(error, _generator);
                PWMGenConfigure(reinterpret_cast<uint32_t>(tm4c123PwmBaseRegister), _generator, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
                if (ErrorType::Success == error) {
                    const uint32_t clockTicksInPeriod = static_cast<float>((SysCtlClockGet() / 1E6)) * _period;
                    //Each generator has two PWM outputs but the period of a generator is the same for both outputs while the pulsewidth can be varied.
                    //This class initializes PWMs per generator by using PWM0 first, then PWM1 next, then for generator 2 it's PWM2 and PWM3, etc.
                    //So if you have already initialized a PWM then the output chosen will not be divisible by 2.
                    //If you have hit this assert then chances are that what you really want
                    if (_output % 2 != 0 && clockTicksInPeriod != PWMGenPeriodGet(reinterpret_cast<uint32_t>(tm4c123PwmBaseRegister), _generator)) {
                        uint32_t newOutput = getOutputFromNextAvailableGenerator(error, _generator);
                        returnOutputToGenerator(_output);
                        _output = newOutput;
                        PLT_LOGW(TAG, "PWM instance was specified with a different period. Initializing on a new generator. Ensure your pinmux is set appropriately.");
                    }
                    PWMGenPeriodSet(reinterpret_cast<uint32_t>(tm4c123PwmBaseRegister), _generator, clockTicksInPeriod);
                    const uint32_t pulseWidth = (_dutyCycle / 100) * clockTicksInPeriod;
                    PWMPulseWidthSet(reinterpret_cast<uint32_t>(tm4c123PwmBaseRegister), _output, pulseWidth);
                    PWMGenEnable(reinterpret_cast<uint32_t>(tm4c123PwmBaseRegister), _generator);
                    error = ErrorType::Success;
                }
            }
        }
    }

    _isInitialized = true;
    return error;
}

ErrorType Pwm::deinit() {
    ErrorType error = ErrorType::Failure;

    //TODO: Need to return the output to the available outputs
    PWMGenDisable(reinterpret_cast<uint32_t>(toTm4c123PeripheralBaseRegister(peripheralNumberConst(), error)), _generator);
    returnOutputToGenerator(_output);
    _isInitialized = false;

    return error;
}

ErrorType Pwm::start() {
    ErrorType error = ErrorType::Failure;

    if (_isInitialized) {
        const uint32_t outputBits = toOutputBits(_output, error);
        if (ErrorType::Success == error) {
            PWMOutputState(reinterpret_cast<uint32_t>(toTm4c123PeripheralBaseRegister(peripheralNumberConst(), error)), outputBits, true);
        }
    }

    return error;
}

ErrorType Pwm::stop() {
    ErrorType error = ErrorType::Failure;

    if (_isInitialized) {
        const uint32_t outputBits = toOutputBits(_output, error);
        if (ErrorType::Success == error) {
            PWMOutputState(reinterpret_cast<uint32_t>(toTm4c123PeripheralBaseRegister(peripheralNumberConst(), error)), outputBits, false);
        }
    }

    return error;
}

ErrorType Pwm::setDutyCycle(Percent on) {
    ErrorType error = ErrorType::Success;
    _dutyCycle = on;

    if (_isInitialized) {
        const uint32_t pulseWidth = (on / 100) * _period;
        PWMPulseWidthSet(reinterpret_cast<uint32_t>(toTm4c123PeripheralBaseRegister(peripheralNumberConst(), error)), _output, pulseWidth);
    }

    return error;
}

ErrorType Pwm::setPeriod(const Microseconds period) {
    ErrorType error = ErrorType::Success;
    _period = period;

    if (_isInitialized) {
        const uint32_t clockTicksInPeriod = static_cast<float>((SysCtlClockGet() / 1E6)) * period;
        PWMGenPeriodSet(reinterpret_cast<uint32_t>(toTm4c123PeripheralBaseRegister(peripheralNumberConst(), error)), _generator, clockTicksInPeriod);
    }

    return error;
}

uint32_t Pwm::toTm4c123SysCtlPeripheralNumber(const PeripheralNumber peripheralNumber, ErrorType &error) {
    error = ErrorType::Success;

    switch (peripheralNumber) {
        case PeripheralNumber::Zero:
            return SYSCTL_PERIPH_PWM0;
        case PeripheralNumber::One:
            return SYSCTL_PERIPH_PWM1;
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }
}

Register Pwm::toTm4c123PeripheralBaseRegister(const PeripheralNumber peripheralNumber, ErrorType &error) {
    error = ErrorType::Success;

    switch (peripheralNumber) {
        case PeripheralNumber::Zero:
            return Register(PWM0_BASE);
        case PeripheralNumber::One:
            return Register(PWM1_BASE);
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }
}

uint32_t Pwm::getOutputFromNextAvailableGenerator(ErrorType &error, uint32_t &generator) {
    error = ErrorType::Success;

    auto availableOutputsIndexToTm4c123Generator = [](const uint8_t index, ErrorType &error) -> uint32_t {
        error = ErrorType::Success;
        uint32_t tm4c123PwmGenerator = 0;

        switch (index) {
            case 0:
            case 1:
                tm4c123PwmGenerator = PWM_GEN_0;
                break;
            case 2:
            case 3:
                tm4c123PwmGenerator = PWM_GEN_1;
                break;
            case 4:
            case 5:
                tm4c123PwmGenerator = PWM_GEN_2;
                break;
            case 6:
            case 7:
                tm4c123PwmGenerator = PWM_GEN_3;
                break;
            default:
                error = ErrorType::NotSupported;
        }

        return tm4c123PwmGenerator;
    };

    auto availableOutputsIndexToTm4c123Output = [](const uint8_t index, ErrorType &error) -> uint32_t {
        error = ErrorType::Success;
        uint32_t tm4c123PwmOutput = 0;

        switch (index) {
            case 0:
                tm4c123PwmOutput = PWM_OUT_0;
                break;
            case 1:
                tm4c123PwmOutput = PWM_OUT_1;
                break;
            case 2:
                tm4c123PwmOutput = PWM_OUT_2;
                break;
            case 3:
                tm4c123PwmOutput = PWM_OUT_3;
                break;
            case 4:
                tm4c123PwmOutput = PWM_OUT_4;
                break;
            case 5:
                tm4c123PwmOutput = PWM_OUT_5;
                break;
            case 6:
                tm4c123PwmOutput = PWM_OUT_6;
                break;
            case 7:
                tm4c123PwmOutput = PWM_OUT_7;
                break;
            default:
                error = ErrorType::NotSupported;
        }

        return tm4c123PwmOutput;
    };

    uint8_t i = 0;
    if (PeripheralNumber::Zero == peripheralNumberConst()) {
        for (auto &nextOutput : _PwmPeripheralZeroAvailableOutputs) {
            const bool outputIsUnused = nextOutput == 0;
            if (outputIsUnused) {
                generator = availableOutputsIndexToTm4c123Generator(i, error);
                nextOutput = 1;
                break;
            }
            else {
                i++;
            }
        }
    }
    else if (PeripheralNumber::One == peripheralNumberConst()) {
        for (auto &nextOutput : _PwmPeripheralOneAvailableOutputs) {
            const bool outputIsUnused = nextOutput == 0;
            if (outputIsUnused) {
                generator = availableOutputsIndexToTm4c123Generator(i, error);
                nextOutput = 1;
                break;
            }
            else {
                i++;
            }
        }
    }

    const uint32_t pwmOutput = availableOutputsIndexToTm4c123Output(i, error);
    return pwmOutput;
}

ErrorType Pwm::returnOutputToGenerator(const uint32_t output) {
    ErrorType error = ErrorType::Success;

    uint8_t i = 0;
    if (PeripheralNumber::Zero == peripheralNumberConst()) {
        for (auto &nextOutput : _PwmPeripheralZeroAvailableOutputs) {
            const bool outputFound = nextOutput == output;
            if (outputFound) {
                nextOutput = 0;
                break;
            }
            else {
                i++;
            }
        }
    }
    else if (PeripheralNumber::One == peripheralNumberConst()) {
        for (auto &nextOutput : _PwmPeripheralOneAvailableOutputs) {
            const bool outputFound = nextOutput == output;
            if (outputFound) {
                nextOutput = 0;
                break;
            }
            else {
                i++;
            }
        }
    }

    return error;
}

uint32_t Pwm::toOutputBits(const uint32_t generatorOutput, ErrorType &error) {
    error = ErrorType::Success;

    switch (generatorOutput) {
        case PWM_OUT_0:
            return PWM_OUT_0_BIT;
        case PWM_OUT_1:
            return PWM_OUT_1_BIT;
        case PWM_OUT_2:
            return PWM_OUT_2_BIT;
        case PWM_OUT_3:
            return PWM_OUT_3_BIT;
        case PWM_OUT_4:
            return PWM_OUT_4_BIT;
        case PWM_OUT_5:
            return PWM_OUT_5_BIT;
        case PWM_OUT_6:
            return PWM_OUT_6_BIT;
        case PWM_OUT_7:
            return PWM_OUT_7_BIT;
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }
}