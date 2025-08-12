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

/**
 * @namespace AdcTypes
 * @brief Namespace for ADC types
 */
namespace AdcTypes {

    /**
     * @enum Channel
     * @brief The channel of the ADC
     * @details The channel of an ADC will be tied to a physical pin and you will have to appeal to the documentation
     *          to determine which pins are measured by which channels.
     * @note Because we have ventured into the realm of analog, we now refer to inputs as channels.
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

    /**
     * @struct Parameters
     * @brief Contains the parameters for the ADC.
     */
    struct Parameters {
        AdcTypes::Channel channel = AdcTypes::Channel::Unknown; ///< The channel of the ADC
        PeripheralNumber peripheralNumber = PeripheralNumber::Unknown; ///< The peripheral number of the ADC
    };
}

/**
 * @class AdcAbstraction
 * @brief Abstraction for ADC
 */
class AdcAbstraction {
    public:
    /// @brief Constructor
    AdcAbstraction() = default;
    /// @brief Destructor
    virtual ~AdcAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "AdcAbstraction";

    /**
     * @brief Configure the ADC.
     * @param params The parameters to configure the ADC with.
     * @returns ErrorType::Success if the ADC was configured
     * @returns ErrorType::Failure if the ADC was not configured
     */
    virtual ErrorType configure(const AdcTypes::Parameters &params) {
        _params = params;
        return ErrorType::Success;
    }

    /**
     * @brief Initialize the ADC
     * @returns ErrorType::Success if the initialization succeeded.
     * @returns ErrorType::PrerequisitesNotMet if the ADC was not configured properly before init was called
     * @sa channel()
     * @sa peripheralNumber()
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief De-initialize the ADC
     * @returns ErrorType::Success if the de-initialization succeeded.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType deinit() = 0;
    /**
     * @brief Convert an analog signal into a discretized value.
     * @param rawValue The discretized value
     * @returns ErrorType::Success if the conversion succeeded.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType convert(Count &rawValue) = 0;
    /**
     * @brief Convert a discretized value into a voltage.
     * @param rawValue The discretized value
     * @param volts The voltage
     * @returns ErrorType::Success if the conversion succeeded.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType rawToVolts(const Count rawValue, Volts &volts) = 0;

    const AdcTypes::Parameters &params() const { return _params; }

    private:
    /// @brief The parameters for the ADC
    AdcTypes::Parameters _params;
};

#endif //__ADC_ABSTRACTION_HPP__