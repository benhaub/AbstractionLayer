#include "DarlingtonArrayAbstraction.hpp"
#include "GptmPwmAbstraction.hpp"
#include "PwmAbstraction.hpp"
#include "GpioAbstraction.hpp"

template <Count _numberOfPins>
class DarlingtonArray final : public DarlingtonArrayAbstraction<_numberOfPins> {
    public:
    DarlingtonArray() : DarlingtonArrayAbstraction<_numberOfPins>() {}

    /**
     * @brief Drive the input pin specified on the darlington array to the specified state.
     * @details When drive high, a darlington array will turn on the BJT and drive the output to ground.
     *          The output of a darlington is therefore either low (input logic high) or high impedance (input logic low).
     * @param pinNumber The pin number on the darlington array.
     * @param on True to drive the pin to the high state, false to drive the pin to the low state.
     * @returns ErrorType::Success if the pin was driven
     * @returns ErrorType::Failure otherwise
     */
    ErrorType togglePin(Count pinNumber, bool on) override {
        if (pinNumber >= _numberOfPins) {
            return ErrorType::InvalidParameter;
        }

        if (this->isDrivenByGptmPwm()) {
            return on ? this->_gptPwms[pinNumber]->start() : this->_gptPwms[pinNumber]->stop();
        }
        else if (this->isDrivenByStandalonePwm()) {
            return on ? this->_pwms[pinNumber]->start() : this->_pwms[pinNumber]->stop();
        }
        else if (this->isDrivenByGpio()) {
            return this->_gpios[pinNumber]->pinWrite(on ? GpioTypes::LogicLevel::High : GpioTypes::LogicLevel::Low);
        }
        else {
            return ErrorType::NotSupported;
        }
    }
};