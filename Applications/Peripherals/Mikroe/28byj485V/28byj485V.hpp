/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   28byj485V.hpp
* @details \b Synopsis: \n Driver for stepper motor driver by Mikroe
* @see https://www.mikroe.com/step-motor-5v
* @ingroup Applications
*******************************************************************************/
#ifndef __28BYJ48_5V_HPP__
#define __28BYJ48_5V_HPP__

//AbstractionLayer
#include "DarlingtonArrayAbstraction.hpp"

class Mikroe28byj485V {

    public:
    static constexpr Count _NumberOfCoils = 4;
    Mikroe28byj485V(std::unique_ptr<DarlingtonArrayAbstraction<_NumberOfCoils>> &darlingtonArray) {
        _darlingtonArray = std::move(darlingtonArray);
    }
    ~Mikroe28byj485V() = default;

    static constexpr char TAG[] = "28byj485V";

    ErrorType init();
    ErrorType rotateForward();

    private:
    std::unique_ptr<DarlingtonArrayAbstraction<_NumberOfCoils>> _darlingtonArray;
};

#endif // __28BYJ48_5V_HPP__