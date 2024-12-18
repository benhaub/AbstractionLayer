/***************************************************************************//**
* @author   Ben Haubrich
* @file     GptmAbstraction.hpp
* @details  Abstraction layer for GPTM
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __GPTM_ABSTRACTION_HPP__
#define __GPTM_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"

/**
 * @class GptmAbstraction
 * @brief Abstraction layer for GPTM.
*/
class GptmAbstraction {

    public:
    ///@brief Constructor
    GptmAbstraction() = default;
    ///@brief Destructor
    virtual ~GptmAbstraction() = default;

    virtual ErrorType init() = 0;
    virtual ErrorType deinit() = 0;
    virtual ErrorType start() = 0;
    virtual ErrorType stop() = 0;
};

#endif //__GPTM_ABSTRACTION_HPP__