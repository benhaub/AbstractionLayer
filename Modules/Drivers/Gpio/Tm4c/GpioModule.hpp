#ifndef __GPIO_MODULE_HPP__
#define __GPIO_MODULE_HPP__

//AbstractionLayer
#include "GpioAbstraction.hpp"

class Gpio final : public GpioAbstraction {

    public:
    Gpio() : GpioAbstraction() {}

    ErrorType init() override;
    ErrorType pinWrite(const GpioTypes::LogicLevel &logicLevel) const override;
    ErrorType pinRead(GpioTypes::LogicLevel &logicLevel) const override;

    private:
    unsigned long toTm4cPortNumber(const PeripheralNumber number, ErrorType &error) const;
    unsigned char toTm4cPinNumber(const PinNumber pin, ErrorType &error) const;
    unsigned long toTm4cDriveStrength(const GpioTypes::DriveStrength driveStrength, ErrorType &error);
    unsigned long toTm4cPinType(const GpioTypes::DriveType driveType, ErrorType &error);
};

#endif // __GPIO_MODULE_HPP__