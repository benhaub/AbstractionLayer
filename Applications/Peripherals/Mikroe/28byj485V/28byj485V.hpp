#ifndef __28BYJ48_5V_HPP__
#define __28BYJ48_5V_HPP__

//AbstractionLayer
#include "GptmPwmModule.hpp"
#include "PwmModule.hpp"
//C++
#include <vector>

class Mikroe28byj485V {
    Mikroe28byj485V() = default;
    ~Mikroe28byj485V() = default;

    static constexpr char TAG[] = "28byj485V";

    ErrorType init();

    ErrorType start();

    private:
    std::vector<GptmPwmModule> _gptPwms;
    std::vector<Pwm> _pwms;
    bool _pwmIsImplementedByGptm = false;
    bool _pwmIsStandaloneDriver = false;
};

#endif // __28BYJ48_5V_HPP__