/***************************************************************************//**
* @author   Ben Haubrich
* @file     Gptm/TimerAbstraction.hpp
* @details  Abstraction layer for GPTM Timer
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __GPTM_TIMER_ABSTRACTION_HPP__
#define __GPTM_TIMER_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
#include "GptmAbstraction.hpp"

/**
 * @class GptmTimerAbstraction
 * @brief Abstraction layer for GPTM Timer
*/
class GptmTimerAbstraction : public GptmAbstraction {

    public:
    ///@brief Constructor
    GptmTimerAbstraction() : GptmAbstraction() {}
    ///@brief Destructor
    virtual ~GptmTimerAbstraction() = default;
};

#endif //__GPTM_TIMER_ABSTRACTION_HPP__