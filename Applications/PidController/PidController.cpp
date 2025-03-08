#include "PidController.hpp"

ErrorType PidController::calculateControlVariable(const float processVariable, float &controlVariable) {
    //e(t)
    const float error = _setPoint - processVariable;

    const float term1 = _previousErrorTMinusOne;
    const float term2 = 1 + error * ((_previousControlVariableTMinusTwo - _previousControlVariableTMinusTwo) / (_proportionalTermKp / _integralTermKi));
}