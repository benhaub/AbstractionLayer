#ifndef __MATRIXORBITAL_EVE3_TFT_35_INCH_HPP__
#define __MATRIXORBITAL_EVE3_TFT_35_INCH_HPP__

#include "LcdAbstraction.hpp"

/**
 * @class MatrixOrbitalEve3Tft35Inch
 * @details MatrixOrbital Eve3 3.5 Inch LCD screen
 */
class MatrixOrbitalEve3Tft35Inch final : public LcdAbstraction {

    public:
    MatrixOrbitalEve3Tft35Inch() : LcdAbstraction() {}

    ErrorType configure() override;
    ErrorType init() override;
    ErrorType reset() override;
    ErrorType startDesign() override;
    ErrorType addDesignElement(const LcdTypes::DesignElement &element) override;
    ErrorType endDesign() override;
    ErrorType copyScreen(StaticString::Container &buffer, const Area &area, const PixelFormat pixelFormat) override;
    ErrorType copyScreen(std::string &buffer, const Area &area, const PixelFormat pixelFormat) override;
    ErrorType clearScreen(const HexCodeColour hexCodeColour) override;
    ErrorType waitForTouches(std::initializer_list<Id> designElements, Milliseconds timeout) override;
    constexpr LcdTypes::ScreenParameters screenParameters() override {
        return LcdTypes::ScreenParameters {};
    }
};

#endif //MATRIXORBITAL_EVE3_TFT_35_INCH_HPP__