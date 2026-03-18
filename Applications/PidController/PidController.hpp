/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   PidController.hpp
* @details \b Synopsis: \n Proporional-Integral-Derivative controller
* @see https://en.wikipedia.org/wiki/Proportional%E2%80%93integral%E2%80%93derivative_controller
* @ingroup Applications
*******************************************************************************/
#ifndef __PID_CONTROLLER_HPP
#define __PID_CONTROLLER_HPP

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
/**
 * @class PidController
 * @brief Proporional-Integral-Derivative controller
 */                       
class PidController {
    public:
    /// @brief Constructor
    PidController() = default;

    /**
     * @brief Calculate the control variable
     * @param controlVariable The control variable is used to adjust process parameters like a new duty cycle to increase or reduce power output.
     * @param processVariable The process variable is the difference between the desired setpoint and the measure setpoint. For a tank of water
     *        filled by a pump, the desired setpoint might be 1L, and the measure set point is 800mL, so the processVariable is 200mL.
     * @sa https://en.wikipedia.org/wiki/Proportional%E2%80%93integral%E2%80%93derivative_controller#Discrete_implementation
     */
    ErrorType calculateControlVariable(const float processVariable, float &controlVariable);

    private:
    /// @brief Integral term
    float _integralTermKi;
    /// @brief Derivative term
    float _derivativeTermKd;
    /// @brief Proportional term
    float _proportionalTermKp;
    /// @brief Set point
    float _setPoint;
    /// @brief previous error from T-1
    float _previousErrorTMinusOne;
    /// @brief previous control variable from T-2
    float _previousControlVariableTMinusTwo;
};

#endif /* __PID_CONTROLLER_HPP */