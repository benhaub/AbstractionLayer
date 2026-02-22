#ifndef RIVERDI_EVE3_TFT_35_INCH_HPP__
#define RIVERDI_EVE3_TFT_35_INCH_HPP__

//AbstractionLayer
#include "Bridgetek/Bridgetek81x.hpp"

/**
 * @namespace RiverdiEve3Tft35InchTypes
 * @details Types for the Riverdi 3.5" display
 */
namespace RiverdiEve3Tft35InchTypes {

    /**
     * @struct Configuration
     * @brief Configuration for the Riverdi display
     */
    struct Configuration {
        SpiTypes::SpiParams spiParams; ///< SPI parameters
        PinNumber powerdown = -1; ///< Pin number for powerdown control
    };
}

/**
 * @class RiverdiEve3Tft35Inch
 * @details Riverdi Eve3 3.5 Inch LCD screen
 */
class RiverdiEve3Tft35Inch final : public LcdAbstraction {

    public:
    RiverdiEve3Tft35Inch() : LcdAbstraction() {}

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
    //https://github.com/riverdi/riverdi-eve/blob/master/riverdi_modules/modules.h
    //A lot of the values can not be found in the datasheet and only exist in the source code.
    constexpr LcdTypes::ScreenParameters screenParameters() override {
        return LcdTypes::ScreenParameters {
            .activeArea = {.origin = {0,0}, .width = 320, .height = 240},
            .horizontalOffset = 70,
            .verticalOffset = 13,
            .totalHorizontalLines = 408,
            .totalVerticalLines = 263,
            .horizontalFrontPorch = 0,
            .verticalFrontPorch = 0,
            .verticalPulseWidth = 2,
            .horizontalPulseWidth = 10,
            .swizzle = 2,
            //Supported pixel clock frequencies can be found on Pg. 27, Tabel 4-11, Bt81X datasheet.
            .pixelClockDivisor = 6,
            .pixelClockPolarity = 1,
            //The display has 24-bit colour but BT81x can improve half tones with dithering.
            .dithering = 1
        };
    }

    /// @brief Get a constant reference to the LCD parameters
    const RiverdiEve3Tft35InchTypes::Configuration &params() const { return _params; }


    private:
    RiverdiEve3Tft35InchTypes::Configuration _params;
    Bridgetek81x _bt815;
    Gpio _powerdown;
    /// @brief Set to true when a sketch, spinner or screensaver is started.
    bool _periodicOperationStarted = false;
};

#endif //RIVERDI_EVE3_TFT_35_INCH_HPP__