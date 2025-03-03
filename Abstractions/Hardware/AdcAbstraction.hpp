/***************************************************************************//**
* @author  Ben Haubrich
* @file    AdcAbstraction.hpp
* @details Abstraction for ADC
* @ingroup Abstractions
*******************************************************************************/
#ifndef __ADC_ABSTRACTION_HPP__
#define __ADC_ABSTRACTION_HPP__

#include "Types.hpp"
#include "Error.hpp"

namespace AdcTypes {

    /**
     * @enum Channel
     * @brief The channel of the ADC
     * @details The channel of an ADC will be tied to a physical pin and you will have to appeal to the documentation
     *          to determine which pins are measured by which channels.
     */
    enum class Channel : uint8_t {
        Zero = 0,   ///< Channel 0
        One,        ///< Channel 1
        Two,        ///< Channel 2
        Three,      ///< Channel 3
        Four,       ///< Channel 4
        Five,       ///< Channel 5
        Six,        ///< Channel 6
        Seven,      ///< Channel 7
        Eight,      ///< Channel 8
        Nine,       ///< Channel 9
        Ten,        ///< Channel 10
        Eleven,     ///< Channel 11
        Twelve,     ///< Channel 12
        Unknown     ///< Unknown channel
    };
}

class AdcAbstraction {
    public:
    AdcAbstraction() = default;
    ~AdcAbstraction() = default;

    static constexpr char TAG[] = "AdcAbstraction";

    virtual ErrorType init() = 0;
    virtual ErrorType deinit() = 0;
    virtual ErrorType convert(Count &rawValue) = 0;
    virtual ErrorType rawToVolts(const Count rawValue, Volts &volts) = 0;

    /// @brief Get the channel as a const reference
    const AdcTypes::Channel &channelConst() const { return _channel; }
    /// @brief Get the channel as a mutable reference
    AdcTypes::Channel &channel() { return _channel; }
    /// @brief Get the peripheral number as a const reference
    const PeripheralNumber &peripheralNumberConst() const { return _peripheralNumber; }
    /// @brief Get the peripheral number as a mutable reference
    PeripheralNumber &peripheralNumber() { return _peripheralNumber; }

    protected:
    /// @brief The peripheral number
    PeripheralNumber _peripheralNumber = PeripheralNumber::Unknown;
    /// @brief The channel
    AdcTypes::Channel _channel = AdcTypes::Channel::Unknown;
};

#endif //__ADC_ABSTRACTION_HPP__