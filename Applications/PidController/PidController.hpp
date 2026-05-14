/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   PidController.hpp
* @details Proporional-Integral-Derivative controller
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
 * @tparam T The type of the process variable.
 * @brief Proporional-Integral-Derivative controller
 */                       
template <typename T>
class PidController {

    public:
    /**
     * @brief Calculate the control variable
     * @pre Call this function in regular intervals.
     * @param[in] processVariable The last value received from the process output.
     * @param[in] setPoint The desired target value
     * @param[in] proportionalTermKp The control variable is increased or decreased in proportion to this term. Often used to convert the output to the units needed to control it via the input. 
     * @param[in] integralTermKi Keeps track of accumulated error. Should be a multiple of the frequency at which you call this function.
     * @param[in] derivativeTermKd Helps to control damping and setpoint overshoot. Should be a multiple of the frequency at which you call this function.
     * @param[out] controlVariable The output control variable value which is fed to the process input for corrections.
     * @sa https://en.wikipedia.org/wiki/Proportional%E2%80%93integral%E2%80%93derivative_controller#Discrete_implementation
     * @sa https://youtu.be/tFVAaUcOm4I
     */
    ErrorType calculateControlVariable(const T processVariable,
                                                    const T setPoint,
                                                    const T proportionalTermKp,
                                                    const T integralTermKi,
                                                    const T derivativeTermKd,
                                                    T &controlVariable) {
        const T error = setPoint - processVariable;

        _integralAccumulator += error;
        const T derivative = error - _previousErrorTMinusOne;

        controlVariable = (proportionalTermKp * error) +
                        (integralTermKi * _integralAccumulator) +
                        (derivativeTermKd * derivative);

        _previousErrorTMinusOne = error;
        return ErrorType::Success;
    }

    void reset() {
        _integralAccumulator = 0.0f;
        _previousErrorTMinusOne = 0.0f;
    }

    private:
    /// @brief Accumulated integral error
    T _integralAccumulator = 0.0f;
    /// @brief Previous error from T-1
    T _previousErrorTMinusOne = 0.0f;
};

#endif /* __PID_CONTROLLER_HPP */