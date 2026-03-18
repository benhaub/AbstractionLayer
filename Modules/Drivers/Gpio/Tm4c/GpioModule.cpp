#include "GpioModule.hpp"
//TI sysconfig
#include "pinout.h"
//TI driverlib
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

ErrorType Gpio::init() {
    ErrorType error;
    //static because the PinoutSet function initializes all GPIOs.
    static bool notInitialized = true;

    //TI Sysconfig PinoutSet calls ALL gpio pin initializations.
    if (notInitialized) {
        PinoutSet();
        notInitialized = false;
    }

    unsigned long portNumber = toTm4cPortNumber(gpioParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        unsigned char pinNumber = toTm4cPinNumber(gpioParams().hardwareConfig.pinNumber, error);

        if (ErrorType::Success == error) {
            unsigned long driveStrength = toTm4cDriveStrength(gpioParams().hardwareConfig.driveStrength, error);

            if (ErrorType::Success == error) {
                unsigned long pinType = toTm4cPinType(gpioParams().hardwareConfig.driveType, error);

                if (ErrorType::Success == error) {
                    GPIOPadConfigSet(portNumber, pinNumber, driveStrength, pinType);
                }
            }
        }
    }

    return error;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) const {
    ErrorType error;
    unsigned long portNumber = toTm4cPortNumber(gpioParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        unsigned char pinNumber = toTm4cPinNumber(gpioParams().hardwareConfig.pinNumber, error);

        if (logicLevel == GpioTypes::LogicLevel::High) {
            GPIOPinWrite(portNumber, pinNumber, pinNumber);
        }
        else {
            GPIOPinWrite(portNumber, pinNumber, 0x0);
        }
    }

    return error;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) const {
    return ErrorType::NotImplemented;
}

unsigned long Gpio::toTm4cPortNumber(const PeripheralNumber number, ErrorType &error) const {
    error = ErrorType::Success;

    switch (number) {
        case PeripheralNumber::Zero:
            return GPIO_PORTA_BASE;
        case PeripheralNumber::One:
            return GPIO_PORTB_BASE;
        case PeripheralNumber::Two:
            return GPIO_PORTC_BASE;
        case PeripheralNumber::Three:
            return GPIO_PORTD_BASE;
        case PeripheralNumber::Four:
            return GPIO_PORTE_BASE;
        case PeripheralNumber::Five:
            return GPIO_PORTF_BASE;
#if defined(PART_TM4C1294NCPDT)
        case PeripheralNumber::Six:
            return GPIO_PORTG_BASE;
        case PeripheralNumber::Seven:
            return GPIO_PORTH_BASE;
#endif
        default:
            error = ErrorType::NotSupported;
            return GPIO_PORTA_BASE;
    }
}
unsigned char Gpio::toTm4cPinNumber(const PinNumber pin, ErrorType &error) const {
    const unsigned char tm4cPinNumber =  1 << pin;

    if (tm4cPinNumber == GPIO_PIN_0 ||
        tm4cPinNumber == GPIO_PIN_1 ||
        tm4cPinNumber == GPIO_PIN_2 ||
        tm4cPinNumber == GPIO_PIN_3 ||
        tm4cPinNumber == GPIO_PIN_4 ||
        tm4cPinNumber == GPIO_PIN_5 ||
        tm4cPinNumber == GPIO_PIN_6 ||
        tm4cPinNumber == GPIO_PIN_7
    ) {
        error = ErrorType::Success;
    }
    else {
        error = ErrorType::NotSupported;
    }

    return tm4cPinNumber;
}

unsigned long Gpio::toTm4cDriveStrength(const GpioTypes::DriveStrength driveStrength, ErrorType &error) {
    error = ErrorType::Success;
    unsigned long tm4cStrength = 0;
    
    switch (driveStrength) {
        case GpioTypes::DriveStrength::TwoMilliAmps:
            return GPIO_STRENGTH_2MA;
        case GpioTypes::DriveStrength::FourMilliAmps:
            return GPIO_STRENGTH_4MA;
        case GpioTypes::DriveStrength::EightMilliAmps:
            return GPIO_STRENGTH_8MA;
        case GpioTypes::DriveStrength::TwelveMilliAmps:
            return GPIO_STRENGTH_12MA;
        default:
            error = ErrorType::InvalidParameter;
            return GPIO_STRENGTH_2MA;
    }

    return tm4cStrength;
}

unsigned long Gpio::toTm4cPinType(const GpioTypes::DriveType driveType, ErrorType &error) {
    error = ErrorType::Success;
    unsigned long tm4cPinType = 0;
    
    switch (driveType) {
        case GpioTypes::DriveType::OpenDrain:
            return GPIO_PIN_TYPE_OD;
        case GpioTypes::DriveType::PushPull:
            return GPIO_PIN_TYPE_STD;
        default:
            error = ErrorType::InvalidParameter;
            return GPIO_PIN_TYPE_OD;
    }

    return tm4cPinType;
}