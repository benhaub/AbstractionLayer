/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Sm10001.hpp
* @details \b Synopsis: \n Driver for the slide potentiometer by adafruit
* @see https://www.adafruit.com/product/5466
* @ingroup Applications
*******************************************************************************/
#ifndef __SM10001_HPP__
#define __SM10001_HPP__

//AbstractionLayer
#include "GptmPwmModule.hpp"
#include "PwmModule.hpp"
//C++
#include <vector>

class Sm10001 {

    public:
    Sm10001() = default;
    ~Sm10001() = default;

    ErrorType init();

    private:
    std::vector<GptmPwmModule> _gptPwms;
    std::vector<Pwm> _pwms;
    bool _pwmIsImplementedByGptm = false;
    bool _pwmIsStandaloneDriver = false;
};

#endif //__SM100001_HPP__