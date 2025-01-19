/***************************************************************************//**
* @author  Ben Haubrich
* @file    GptmTimerAbstraction.hpp
* @details Abstraction for GPTM Timer
* @ingroup Abstractions
*******************************************************************************/
#ifndef __GPTM_TIMER_ABSTRACTION_HPP__
#define __GPTM_TIMER_ABSTRACTION_HPP__

//AbstractionLayer
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