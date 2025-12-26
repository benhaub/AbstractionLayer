/***************************************************************************//**
* @author   Ben Haubrich
* @file     LcdFactory.hpp
* @details  Lcd abstraction factory for creating various types of LCD screens 
* @ingroup Applications
*******************************************************************************/
#ifndef __LCD_FACTORY_HPP__
#define __LCD_FACTORY_HPP__

//AbstractionLayer
#include "MatrixOrbital/Eve3Tft3_5Inch.hpp"
#include "Riverdi/Eve3Tft3_5Inch.hpp"
//C++
#include <variant>
#include <optional>

/**
 * @namespace LcdFactoryTypes
 * @brief Contains types and constants used by the LcdFactory.
 */
namespace LcdFactoryTypes {

    ///@brief The tag used for logging
    static constexpr char Tag[] = "LcdFactory";

    /**
     * @typedef LcdFactoryVariant
     * @brief The variant type that can hold any Lcd type created by the factory.
     */
    using LcdFactoryVariant = std::variant<RiverdiEve3Tft35Inch, MatrixOrbitalEve3Tft35Inch>;
    /**
     * @enum PartNumber
     * @brief The part number of the H-Bridge
     */
    enum class PartNumber : uint8_t {
        Unknown = 0,          ///< Unknown LCD part number
        Rvt35AHBNWC00,        ///< Riverdi Eve 3.5" Capactive touch screen (BT815)
        MatrixOrbitalEve3_35A ///< Matrix Orbital Eve 3.5" Thin Film Transistor (TFT) screen (BT815)
    };
}

/**
 * @namespace LcdFactory
 * @brief Functions and types that aid the creation of lcd interfaces.
 */
namespace LcdFactory {

    /**
     * @brief Creates a lcd interface for the type selected.
     * @details The output lcd is passed by reference as a parameter because the atomics in EventQueue delete
     *          the move assignment.
     * @param[in] partNumber The part number of the lcd to create
     * @param[out] lcd The lcd that is output by the factory
     * @returns ErrorType::Success if the lcd was created successfully
     * @returns ErrorType::NotSupported if the technology is not supported
     */
    template <LcdFactoryTypes::PartNumber _PartNumber>
    inline constexpr ErrorType Factory(std::optional<LcdFactoryTypes::LcdFactoryVariant> &lcd) {
        ErrorType error = ErrorType::Success;

        if constexpr (_PartNumber == LcdFactoryTypes::PartNumber::Rvt35AHBNWC00) {
            lcd.emplace(std::in_place_type_t<RiverdiEve3Tft35Inch>());
        }
        else if constexpr (_PartNumber == LcdFactoryTypes::PartNumber::MatrixOrbitalEve3_35A) {
            lcd.emplace(std::in_place_type_t<MatrixOrbitalEve3Tft35Inch>());
            error = ErrorType::NotSupported;
        }
        else { [[unlikely]]
            error =  ErrorType::NotSupported;
        }

        return error;
    }
}

#endif // __LCD_FACTORY_HPP__