/***************************************************************************//**
* @author  Ben Haubrich
* @file    SpiAbstraction.hpp
* @details Abstraction for Serial Peripheral Interface
* @ingroup Abstractions
*******************************************************************************/
#ifndef __SPI_ABSTRACTION_HPP__
#define __SPI_ABSTRACTION_HPP__

//AbstractionLayer
#include "IcCommunicationProtocol.hpp"

/**
 * @class SpiAbstraction
 * @brief Abstraction layer for Serial Peripheral Interface
*/
class SpiAbstraction : public IcCommunicationProtocol {

    public:
    ///@brief Constructor
    SpiAbstraction() : IcCommunicationProtocol() {}
    ///@brief Destructor
    virtual ~SpiAbstraction() = default;

    virtual ErrorType setHardwareConfig() = 0;
    virtual ErrorType setDriverConfig() = 0;
    virtual ErrorType setFirmwareConfig() = 0;
};

#endif //__GPTM_TIMER_ABSTRACTION_HPP__