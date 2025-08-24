#ifndef __STM_ULN_2003_HPP__
#define __STM_ULN_2003_HPP__

#include "DarlingtonArrayAbstraction.hpp"
#include "GptmPwmAbstraction.hpp"
#include "PwmAbstraction.hpp"
#include "GpioAbstraction.hpp"

/**
 * @namespace StmUln2003Types
 * @brief Contains types related to the ST Microelectronics ULN2003 Darlington Array.
 */
namespace StmUln2003Types {
    constexpr Count InputPins = 8; ///< The number of input pins on the ULN2003
}

/**
 * @class StmUln2003
 * @brief ST Microelectronics ULN2003 Darlington Array
 */
class StmUln2003 final : public DarlingtonArrayAbstraction<StmUln2003Types::InputPins> {

    public:
    /// @brief Constructor
    StmUln2003() : DarlingtonArrayAbstraction<StmUln2003Types::InputPins>() {}

    ErrorType init() override;

    /**
     * @brief Drive the input pin specified on the darlington array to the specified state.
     * @details When drive high, a darlington array will turn on the BJT and drive the output to ground.
     *          The output of a darlington is therefore either low (input logic high) or high impedance (input logic low).
     * @param pinNumber The pin number on the darlington array.
     * @param on True to drive the pin to the high state, false to drive the pin to the low state.
     * @returns ErrorType::Success if the pin was driven
     * @returns ErrorType::Failure otherwise
     */
    ErrorType togglePin(Count pinNumber, bool on) override;
};

#endif //__STM_ULN_2003_HPP__