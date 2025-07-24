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
 * @namespace SpiTypes
 * @brief Spi configuration types
*/
namespace SpiTypes {
    /**
     * @struct SpiParams
     * @brief Contains the parameters used to configure the SPI.
     */
    struct SpiParams final : public IcCommunicationProtocolTypes::ConfigurationParameters {
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Spi; }
    };
}

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

    ErrorType configure(const IcCommunicationProtocolTypes::ConfigurationParameters &params) override {
        _spiParams = static_cast<const SpiTypes::SpiParams &>(params);
        return ErrorType::Success;
    }

    const SpiTypes::SpiParams &spiParams() const { return _spiParams; }

    private:
    /// @brief The UART parameters.
    SpiTypes::SpiParams _spiParams;
};

#endif //__GPTM_TIMER_ABSTRACTION_HPP__