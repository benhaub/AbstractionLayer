//AbstractionLayer
#include "Eve3Tft3_5Inch.hpp"
#include "OperatingSystemModule.hpp"

ErrorType RiverdiEve3Tft35Inch::configure() {
    _params.powerdown = APP_LCD_POWERDOWN_PIN_NUMBER;
    _params.spiParams.hardwareConfig.peripheral = APP_LCD_SPI_PERIPHERAL_NUMBER;
    _params.spiParams.hardwareConfig.chipSelect = APP_CHIP_SELECT_PIN_NUMBER;
    _params.spiParams.hardwareConfig.clock = APP_CLOCK_PIN_NUMBER;
    _params.spiParams.hardwareConfig.periperhalOutControllerIn = APP_POCI_PIN_NUMBER;
    _params.spiParams.hardwareConfig.perpheralInControllerOut = APP_PICO_PIN_NUMBER;
    _params.spiParams.driverConfig.clockFrequency = 10E6;
    _params.spiParams.driverConfig.format = SpiTypes::FrameFormat::Mode0;
    _params.spiParams.driverConfig.isController = true;
    _params.spiParams.driverConfig.dataSize = SpiTypes::DataSize::EightBits;
    _params.spiParams.driverConfig.channels = SpiTypes::Channels::Single;

    GpioTypes::GpioParams powerdownParams;
    powerdownParams.hardwareConfig.peripheralNumber = APP_LCD_POWERDOWN_PERIPHERAL_NUMBER;
    powerdownParams.hardwareConfig.pinNumber = APP_LCD_POWERDOWN_PIN_NUMBER;
    powerdownParams.hardwareConfig.driveType = GpioTypes::DriveType::PushPull;
    powerdownParams.hardwareConfig.driveStrength = GpioTypes::DriveStrength::EightMilliAmps;

    ErrorType error = _powerdown.configure(powerdownParams);

    if (ErrorType::Success == error) {
        error = _powerdown.init();
    }

    return error;
}

ErrorType RiverdiEve3Tft35Inch::init() {
    ErrorType error = reset();

    if (ErrorType::Success == error) {
        constexpr Bridgetek81xTypes::SystemClockFrequency lcdSystemClock = Bridgetek81xTypes::SystemClockFrequency::Frequency72MHz;
        constexpr bool hasExternalCrystal = true;

        if (ErrorType::Success == (error = _bt815.init(params().spiParams, hasExternalCrystal, lcdSystemClock, screenParameters()))) {

            if (ErrorType::Success == (error = _bt815.toggleBacklight(true, Percent(5)))) {
                error = _bt815.toggleDisplay(true);

                if (ErrorType::Success == error) {
                    error = _bt815.setTouchThreshold(1200);

                    if (ErrorType::Success == error) {
                        _bt815.calibrate({screenParameters().activeArea.width/2U, screenParameters().activeArea.height/2U});
                    }
                }
            }
        }
    }

    return error;
}

ErrorType RiverdiEve3Tft35Inch::reset() {
    return _bt815.reset(_powerdown);
}

ErrorType RiverdiEve3Tft35Inch::startDesign() {
    if (_periodicOperationStarted) {
        _bt815.stopPeriodicOperation();
        _periodicOperationStarted = false;
    }

    return _bt815.startDisplayList();
}

ErrorType RiverdiEve3Tft35Inch::addDesignElement(const LcdTypes::DesignElement &element) {
    ErrorType error = ErrorType::Success;

    switch (element.type) {
        case LcdTypes::DesignElementType::ClearScreen: {
            const LcdTypes::ClearScreen &clearScreen = static_cast<const LcdTypes::ClearScreen &>(element);
            error = _bt815.clearScreen(clearScreen.colour);
            break;
        }
        case LcdTypes::DesignElementType::FreehandSketch: {
            const LcdTypes::FreehandSketch &freehandSketch = static_cast<const LcdTypes::FreehandSketch &>(element);

            error = _bt815.startFreeHandSketch(freehandSketch.area, freehandSketch.brushColour, freehandSketch.paperColour,
                                               Bridgetek81xTypes::BitmapPixelFormat::L8
            );

            if (ErrorType::Success == error) {
                _periodicOperationStarted = true;
            }

            break;
        }
        case LcdTypes::DesignElementType::Text: {
            constexpr Count maxTextSize = 256;
            const LcdTypes::Text &text = static_cast<const LcdTypes::Text &>(element);

            if (static_cast<uint32_t>(text.font) >= static_cast<uint32_t>(Bridgetek81xTypes::Font::Font0) && static_cast<uint32_t>(text.font) <= static_cast<uint32_t>(Bridgetek81xTypes::Font::Font18)) {
                error = _bt815.drawText<maxTextSize>(text.location,
                                            static_cast<Bridgetek81xTypes::Font>(text.font),
                                            text.colour,
                                            Bridgetek81xTypes::Options::NoOptions,
                                            std::string_view(text.text->c_str(), text.text->size())
                );
            }
            else {
                error = ErrorType::InvalidParameter;
            }

            break;
        }
        case LcdTypes::DesignElementType::Button: {
            const LcdTypes::Button &button = static_cast<const LcdTypes::Button &>(element);

            if (static_cast<uint32_t>(button.font) >= static_cast<uint32_t>(Bridgetek81xTypes::Font::Font0) && static_cast<uint32_t>(button.font) <= static_cast<uint32_t>(Bridgetek81xTypes::Font::Font18)) {

                if (0 != button.id && button.id <= UINT8_MAX) {
                    error = _bt815.enableTouchTag(button.id);

                    if (ErrorType::Success == error) {
                        error = _bt815.drawButton<32>(button.area,
                                                    static_cast<Bridgetek81xTypes::Font>(button.font),
                                                    Bridgetek81xTypes::Options::NoOptions,
                                                    std::string_view(button.text->c_str(), button.text->size())
                        );

                        _bt815.disableTouchTag();
                    }
                }
            }
            else {
                error = ErrorType::InvalidParameter;
            }

            break;
        }
        default:
            error = ErrorType::NotSupported;
            break;
    }

    return error;
}

ErrorType RiverdiEve3Tft35Inch::endDesign() {
    return _bt815.commitDisplayList();
}

template <typename Buffer>
static ErrorType copyScreenImplementation(Buffer &&buffer, const Area &area, Bridgetek81x &bt815, const PixelFormat pixelFormat) {
    //TODO: This class should be able to manage the memory of RAMG and choose an appropriate address.
    uint32_t ramG;
    ErrorType error = ErrorType::Success;
    //I don't know if something like this could work or not, but could we know where to copy data from based
    //on the pixel format? Managing memory by saving certain pixel formats in certain start addresses? Could they
    //still vary in size by not allowing more than one format to be saved at once?
    //For a sketch, if you draw with lots of colours, then it might be beneficial to save the screen because it
    //will convert from L1/L8 for you.
    if (PixelFormat::Greyscale == pixelFormat) {
        //TODO: The address should not be hardcoded.
        ramG = static_cast<uint32_t>(Bridgetek81xTypes::BaseAddresses::GeneralPurposeGraphicsRam);
    }
    else {
        ramG = static_cast<uint32_t>(Bridgetek81xTypes::BaseAddresses::GeneralPurposeGraphicsRam) + area.size();
        error = bt815.saveScreenToRamG(ramG, area);
    }

    size_t bytesCopied = 0;

    if (ErrorType::Success == error) {
        buffer->resize(area.size());
        error = bt815.memoryCopy(pixelFormat, ramG, buffer->data(), buffer->size(), bytesCopied);

        if (ErrorType::Success == error) {
            buffer->resize(bytesCopied);
        }
    }

    return error;
}

ErrorType RiverdiEve3Tft35Inch::copyScreen(StaticString::Container &buffer, const Area &area, const PixelFormat pixelFormat) {
    return copyScreenImplementation(buffer, area, _bt815, pixelFormat);
}

ErrorType RiverdiEve3Tft35Inch::copyScreen(std::string &buffer, const Area &area, const PixelFormat pixelFormat) {
    return copyScreenImplementation(&buffer, area, _bt815, pixelFormat);
}

ErrorType RiverdiEve3Tft35Inch::clearScreen(const HexCodeColour hexCodeColour) {
    return _bt815.clearScreen(hexCodeColour);
}

ErrorType RiverdiEve3Tft35Inch::waitForTouches(std::initializer_list<Id> designElements, Milliseconds timeout) {
    Coordinate touchedAt;
    ErrorType error = ErrorType::Timeout;

    if (designElements.size() != 0) {

        for (Milliseconds i = 0; i < timeout; i++) {

            for (const Id &designElementId : designElements) {
                if (ErrorType::Success == _bt815.checkForScreenTouches(designElementId, touchedAt)) {
                    return ErrorType::Success;
                }
            }

            OperatingSystem::Instance().delay(Milliseconds(1));
        }
    }
    else {
        error = ErrorType::NoData;
    }

    return error;
}