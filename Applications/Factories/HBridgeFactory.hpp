/***************************************************************************//**
* @author  Ben Haubrich
* @file    HBridgeFactory.hpp
* @details HBridge factory for creating various types of HBridges
* @ingroup Applications
* @sa https://en.wikipedia.org/wiki/Factory_(object-oriented_programming)
*******************************************************************************/
#ifndef __HBRIDGE_FACTORY_HPP__
#define __HBRIDGE_FACTORY_HPP__

//AbstractionLayer
#include "TiDrv8872/TiDrv8872.hpp"
//C++
#include <variant>
#include <optional>

/**
 * @namespace HBridgeFactoryTypes
 * @brief Contains types and constants used by the HBridgeFactory.
 */
namespace HBridgeFactoryTypes {

    ///@brief The tag used for logging
    static constexpr char Tag[] = "HBridgeFactory";
    /**
     * @typedef HBridgeFactoryVariant
     * @brief The variant type that can hold any H-Bridge type created by the factory
     */
    using HBridgeFactoryVariant = std::variant<TiDrv8872>;
    /**
     * @enum PartNumber
     * @brief The part number of the H-Bridge
     */
    enum class PartNumber : uint8_t {
        Unknown = 0,   ///< Unknown H-Bridge part number
        TiDrv8872 = 1  ///< Texas Instruments DRV8872
    };
}

/**
 * @namespace HBridgeFactory
 * @brief Functions and types that aid the creation of HBridge interfaces.
 */
namespace HBridgeFactory {

    /**
     * @brief Creates an H-Bridge for the type selected.
     * @param[in] partNumber The part number of the H-Bridge to create.
     * @param[out] hBridge The H-Bridge that is output by the factory
     * @returns ErrorType::Success if the H-Bridge was created successfully.
     * @returns ErrorType::NotSupported if the part number is not recognized.
     */
    template <HBridgeFactoryTypes::PartNumber _PartNumber>
    inline constexpr ErrorType Factory(std::optional<HBridgeFactoryTypes::HBridgeFactoryVariant> &hBridge) {
        ErrorType error = ErrorType::Success;

        switch (_PartNumber) {
            case HBridgeFactoryTypes::PartNumber::TiDrv8872:
                hBridge.emplace(std::in_place_type_t<TiDrv8872>());
                break;
            default: [[unlikely]]
                error =  ErrorType::NotSupported;
        }

        return error;
    }
}

#endif // __HBRIDGE_FACTORY_HPP__