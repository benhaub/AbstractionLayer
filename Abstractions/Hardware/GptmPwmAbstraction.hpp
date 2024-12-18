/***************************************************************************//**
* @author   Ben Haubrich
* @file     Gptm/PwmAbstraction.hpp
* @details  Abstraction layer for GPTM PWM
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __GPTM_PWM_ABSTRACTION_HPP__
#define __GPTM_PWM_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
#include "GptmAbstraction.hpp"

/**
 * @class GptmPwmAbstraction
 * @brief Abstraction layer for GPTM.
*/
class GptmPwmAbstraction : public GptmAbstraction {

    public:
    ///@brief Constructor
    GptmPwmAbstraction() : GptmAbstraction() {}
    ///@brief Destructor
    virtual ~GptmPwmAbstraction() = default;
};

#endif //__GPTM_PWM_ABSTRACTION_HPP__