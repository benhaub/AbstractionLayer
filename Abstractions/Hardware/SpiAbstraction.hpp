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

    /**
     * @brief Set the hardware configuration for SPI
     * @return ErrorType::Success if the configuration was set successfully
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType setHardwareConfig() = 0;
    /**
     * @brief Set the driver configuration for SPI
     * @return ErrorType::Success if the configuration was set successfully
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType setDriverConfig() = 0;
    /**
     * @brief Set the firmware configuration for SPI
     * @return ErrorType::Success if the configuration was set successfully
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType setFirmwareConfig() = 0;
};

#endif //__GPTM_TIMER_ABSTRACTION_HPP__