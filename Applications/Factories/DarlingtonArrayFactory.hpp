/***************************************************************************//**
* @author  Ben Haubrich
* @file    DarlingtonArrayFactory.hpp
* @details DarlingtonArray factory for creating various types of DarlingtonArrays
* @ingroup Applications
* @sa https://en.wikipedia.org/wiki/Factory_(object-oriented_programming)
*******************************************************************************/
#ifndef __DARLINGTON_ARRAY_FACTORY_HPP__
#define __DARLINGTON_ARRAY_FACTORY_HPP__

//AbstractionLayer
#include "StmUln2003/StmUln2003.hpp"
//C++
#include <variant>
#include <optional>

/**
 * @namespace DarlingtonArrayFactoryTypes
 * @brief Contains types and constants used by the DarlingtonArrayFactory.
 */
namespace DarlingtonArrayFactoryTypes {

    ///@brief The tag used for logging
    static constexpr char Tag[] = "DarlingtonArrayFactory";

    using DarlingtonArrayFactoryVariant = std::variant<StmUln2003>;

    /**
     * @enum PartNumber
     * @brief The part number of the darlington array to create.
     */
    enum class PartNumber : uint8_t {
        Unknown = 0,    ///< Unknown H-Bridge part number
        StmUln2003 = 1  ///< ST Microelectronics Transistor Array ULN2003
    };
}

/**
 * @namespace DarlingtonArrayFactory
 * @brief Functions and types that aid the creation of darlington array interfaces.
 */
namespace DarlingtonArrayFactory {

    /**
     * @brief Creates a darlington array for the type selected.
     * @param[in] partNumber The part number of the darlington array to create.
     * @param[out] darlingtonArray The hbridge that is output by the factory
     * @returns ErrorType::Success if the darlington array was created successfully.
     * @returns ErrorType::NotSupported if the part number is not recognized.
     */
    inline ErrorType Factory(const DarlingtonArrayFactoryTypes::PartNumber partNumber, std::optional<DarlingtonArrayFactoryTypes::DarlingtonArrayFactoryVariant> &darlingtonArray) {
        ErrorType error = ErrorType::Success;

        switch (partNumber) {
            case DarlingtonArrayFactoryTypes::PartNumber::StmUln2003:
                hBridge.emplace(std::in_place_type_t<StmUln2003>());
                break;
            default: [[unlikely]]
                error =  ErrorType::NotSupported;
        }

        return error;
    }
}

#endif // __DARLINGTON_ARRAY_FACTORY_HPP__