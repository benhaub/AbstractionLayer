#ifndef __BRIDGETEK_81X_HPP__
#define __BRIDGETEK_81X_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Math.hpp"
#include "SpiModule.hpp"
#include "GpioModule.hpp"
#include "LcdAbstraction.hpp"
#include "Bridgetek81xTypes.hpp"
//C++
#include <string_view>
#include <cstring>

namespace {
    constexpr inline uint32_t To4ByteBlock(const size_t offset, std::string_view text) {
        uint32_t fourByteBlock = 0;

        if (offset < text.size()) {
            const size_t avail = std::min<size_t>(4, text.size() - offset);
            memcpy(&fourByteBlock, &text[offset], avail);
        }

        return fourByteBlock;
    };
}

/**
 * @class Bridgetek81x series graphics controller
 * @details Bridgtek graphics controller for LCD screens
 */
class Bridgetek81x {

    public:
    static constexpr char TAG[] = "Bridgetek81x";

    /**
     * @brief Initialize the BT81x.
     * @param[in] params The SPI parameters to configure the driver with.
     * @param[in] externalClock True if the clock source is external
     * @param[in] systemClockFrequency If using the external crystal as PLL input, this is the desired LCD system clock frequency. If set to zero then the frequency is 60MHz
     * @sa BT81X Series Programming Guide Sect. 2.4
     */
    ErrorType init(const SpiTypes::SpiParams params, bool externalClock, const Bridgetek81xTypes::SystemClockFrequency systemClockFrequency, const LcdTypes::ScreenParameters &screenParams);
    /**
     * @brief Reset the BT81x core
     * @param[in] powerdown The gpio connected to the powerdown signal
     * @returns Anything returned by GpioAbstraction:pinWrite
     */
    ErrorType reset(const Gpio &powerdown);
    /**
     * @brief Draw a string of text to the screen.
     * @tparam _TextSize The maximum size of the text to draw
     * @param[in] location The location to draw the text
     * @param[in] fontSize The size of the font to use
     * @param[in] options The options to use when drawing the text
     * @param[in] colour The colour of the text
     * @param[in] text The text to draw
     * @returns Anything returned by writeToCommandBuffer
     */
    template <size_t _TextSize>
    ErrorType drawText(const Coordinate location, const Bridgetek81xTypes::Font font, const HexCodeColour colour, const Bridgetek81xTypes::Options options, std::string_view text) {
        ErrorType error = ErrorType::InvalidParameter;
        constexpr Count fourByteBlocks = std::ceil(_TextSize / 4.0f);

        if (font > static_cast<Bridgetek81xTypes::Font>(Bridgetek81xTypes::Font::Unknown) &&
            font < static_cast<Bridgetek81xTypes::Font>(Bridgetek81xTypes::Font::Font18)) {
            error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::ColorRgb(colour));
            const uint32_t optionValue = options == Bridgetek81xTypes::Options::NoOptions ? 0 : static_cast<uint32_t>(options);

            if (ErrorType::Success == error) {
                return [&]<size_t... Is>(std::index_sequence<Is...>) -> ErrorType {
                    return writeToCommandBuffer(Bridgetek81xTypes::Commands::Text,
                        {
                            (static_cast<uint32_t>(location.y) << 16) | location.x,
                            (static_cast<uint32_t>(optionValue) << 16) | static_cast<uint32_t>(font),
                            To4ByteBlock(Is * 4, text)...
                        });
                }(std::make_index_sequence<fourByteBlocks>{});
            }
        }

        return error;
    }
    /**
     * @brief Draw a button with a label
     * @tparam _TextSize The maximum size of the text to draw
     * @param[in] area The area of the button which spans from the origin specified.
     * @param[in] font The font to use
     * @param[in] options The options to use when drawing the button
     * @param[in] text The text to draw on the button
     * @returns Anything returned by writeToCommandBuffer
     */
    template <size_t _TextSize>
    ErrorType drawButton(const Area &area, const Bridgetek81xTypes::Font font, const Bridgetek81xTypes::Options options, std::string_view text) {

        constexpr Count fourByteBlocks = _TextSize / 4;
        const uint32_t optionValue = options == Bridgetek81xTypes::Options::NoOptions ? 0 : static_cast<uint32_t>(options);

        return [&]<size_t... Is>(std::index_sequence<Is...>) -> ErrorType {
            return writeToCommandBuffer(Bridgetek81xTypes::Commands::Button,
                {
                    (static_cast<uint32_t>(area.origin.y) << 16) | area.origin.x,
                    (static_cast<uint32_t>(area.height) << 16) | area.width,
                    (static_cast<uint32_t>(optionValue) << 16) | static_cast<uint32_t>(font),
                    To4ByteBlock(Is * 4, text)...
                });
        }(std::make_index_sequence<fourByteBlocks>{});
    }
    /**
     * @brief Start a free hand sketch
     * @param[in] area The area of the sketch which spans from the origin specified.
     * @param[in] bitmapBaseAddress The base address of the bitmap
     * @param[in] pixelFormat The pixel format of the bitmap
     * @post Call displayFreeHandSketch to display the current contents of the sketch.
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType startFreeHandSketch(const Area &area, const HexCodeColour brushColour, const HexCodeColour paperColour, const Bridgetek81xTypes::BitmapPixelFormat pixelFormat);
    /**
     * @brief clear the screen
     */
    ErrorType clearScreen(const uint32_t hexCodeColour);
    /**
     * @brief Set the foreground colour
     * @param[in] colour The colour to set the foreground to
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType setForegroundColour(const HexCodeColour colour);
    /**
     * @brief Set the background colour
     * @param[in] colour The colour to set the background to
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType setBackgroundColour(const HexCodeColour colour);
    /**
     * @brief Set the colour that will apply to the next draw operation
     * @param[in] colour The colour to set the brush to
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType setDrawColour(const HexCodeColour colour);
    /**
     * @brief Take a snapshot of the screen and save it to RAMG
     * @param[in] address The address to save the screen to. Must be within the RAMG address space
     * @param[in] area The screen area to save to RAMG
     */
    ErrorType saveScreenToRamG(const uint32_t address, const Area &area);
    /**
     * @brief Toggle on/off the backlight
     * @param[in] on Whether to turn off or on the backlight.
     * @param[in] brightness The screen brightness
     */
    ErrorType toggleBacklight(const bool on, const Percent brightness);
    /**
     * @brief Toggle this display on/off
     * @param[in] on Whether to turn off or on the display
     * @param[in] brightness The screen brightness
     */
    ErrorType toggleDisplay(const bool on);
    /**
     * @brief Set the touch threshold
     */
    ErrorType setTouchThreshold(const uint16_t threshold);
    /**
     * @brief Calibrate
     * @param instructionTextLocation The location to draw the instruction text
     * @returns Anything returned by writeToCommandBuffer
     * @returns Anything returned by drawText
     * @returns Anything returned by startDisplayList
     * @returns Anything returned by sendDisplayListCommand
     */
    ErrorType calibrate(const Coordinate &instructionTextLocation);
    /**
     * @brief Start a new display list
     * @returns Anything retruned by writeToCommandBuffer
     */
    ErrorType startDisplayList();
    /**
     * @brief Commit the commands in the display list and show them on the screen
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType commitDisplayList();
    /**
     * @brief Enable the touch tag to be set for all following graphics objects.
     * @param[in] tag The tag to set for all following graphics objects
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType enableTouchTag(const uint8_t tag);
    /**
     * @brief Disable the touch tag to be set for all following graphics objects.
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType disableTouchTag();
    /**
     * @brief Check for screen touches on the provided tag (i.e check that a graphics object was touched)
     * @param[in] tag The tag to check for
     * @param[out] touchedAt The location of the touch. Valid only if the return value is ErrorType::Success
     * @returns ErrorType::Success if the tag was touched
     * @returns ErrorType::Negative if the tag was not touched
     * @returns Anything returned by hostMemoryRead
     */
    ErrorType checkForScreenTouches(const uint8_t tag, Coordinate &touchedAt);
    /**
     * @brief Copy from the address specified to the buffer provided
     * @pre The buffers size determines how many bytes to read
     * @param[in] address The address to copy from
     * @param[in] pixelFormat The pixel format to copy into the buffer
     * @param[in] buffer The buffer to copy the screen contents to
     * @param[in] bufferSize The size of the buffer
     * @param[out] bytesCopied The number of bytes to copy
     * @returns Anything returned by hostMemoryRead
     */
    ErrorType memoryCopy(const PixelFormat pixelFormat, const uint32_t address, char *buffer, const size_t bufferSize, size_t &bytesCopied);
    /**
     * @brief Stop the periodic operation such as sketch, spinner or screensaver
     * @returns Anything returned by writeToCommandBuffer
     */
    ErrorType stopPeriodicOperation();

    private:
    /// @brief The SPI interface to the BT81x
    Spi _spi;
    /// @brief The chip select GPIO for the BT81x
    Gpio _chipSelect;
    /// @brief The pixel clock divsor save from config so that the screen can be enabled/disabled later.
    uint8_t _pixelClockDivisor = 0;
    /// @brief The buffer of commands to write to the LCD display
    StaticString::Container _commandBuffer = StaticString::Data<512>();

    /**
     * @brief Read data from the address specified
     * @tparam _ReadType Any of uint32_t, uint16_t, or uint8_t depending on the amount of bytes you want to read
     * @param[in] address the address to read the data from.
     * @param[out] buffer The buffer to store data that was read
     * @param[in] timeout The amount of time to wait for the read to complete
     * @returns Anything returned by IcCommunicationProtocol::rxBlocking
     * @returns Anything returned by IcCommunicationProtocol::txBlocking
     * @sa Bt81x Datasheet, Sect. 4.1.3 - Host Memory Read
     */
    template <typename _ReadType>
    requires std::is_same_v<_ReadType, uint8_t> || std::is_same_v<_ReadType, uint16_t> || std::is_same_v<_ReadType, uint32_t>
    ErrorType hostMemoryRead(const uint32_t address, _ReadType &buffer, const Milliseconds timeout) {
        ErrorType error = ErrorType::Failure;
        constexpr uint8_t dummyByteValue = 0;
        StaticString::Container readTransactionAddressBytes = StaticString::Data<Bridgetek81xTypes::AddressSize + sizeof(dummyByteValue)>();

        readTransactionAddressBytes->push_back((address >> 16) & 0x3F);
        readTransactionAddressBytes->push_back((address >> 8) & 0xFF);
        readTransactionAddressBytes->push_back((address) & 0xFF);
        readTransactionAddressBytes->push_back(dummyByteValue);

        StaticString::Container byteBuffer = StaticString::Data<sizeof(uint8_t)>();
        //Dummy byte is an alias to byteBuffer, so they are sharing the same memory but named differently depending on usage
        StaticString::Container &dummyByte = byteBuffer;

        byteBuffer->resize(sizeof(uint8_t));
        buffer = 0;
        IcCommunicationProtocolTypes::AdditionalCommunicationParameters additionalParams;

        if (ErrorType::Success == (error = _chipSelect.pinWrite(GpioTypes::LogicLevel::Low))) {
            error = _spi.txBlocking(readTransactionAddressBytes, timeout, additionalParams);
        }

        //BT81x sends data whenever you set CS low and run the clock, so clear the data that was sent while transmitting
        //the address because we aren't interested in it.
        _spi.flushRxBuffer();

        if (ErrorType::Success == error) {

            for (size_t nextByte = 0; nextByte < sizeof(_ReadType); nextByte++) {

                dummyByte[0] = dummyByteValue;
                error = _spi.txBlocking(dummyByte, 0, additionalParams);

                if (ErrorType::Success == error) {
                    error = _spi.rxBlocking(byteBuffer, 0, additionalParams);

                    if (ErrorType::Success == error) {
                        buffer |= byteBuffer[0] << ToBits(nextByte);
                    }
                }
                else {
                    break;
                }
            }
        }

        _chipSelect.pinWrite(GpioTypes::LogicLevel::High);
        _spi.flushRxBuffer();

        return error;
    }

    /**
     * @brief Write data to the address specified
     * @tparam _WriteType Any of uint32_t, uint16_t, or uint8_t depending on the amount of bytes you want to write
     * @param[in] writeTransaction A transaction with the address and data
     * @sa Bridgetek81xTpyes::MemoryWriteTransaction
     * @param[in] timeout The amount of time to wait for the write to complete
     * @param[in] maxRetries The maximum number of times to retry the write
     * @param[in] mask The mask to apply to the value read back for comparison agains the value written.
     *                 Useful if you only care about some of the bits instead of the whole number. Defaults
     *                 to comparing the whole, exact value.
     * @returns Anything returned by IcCommunicationProtocol::txBlocking
     * @returns ErrorType::LimitReached if the data could not be written within the maximum number of retries
     */
    ErrorType hostMemoryWrite(const uint32_t address, const StaticString::Container &data, const Milliseconds timeout, const Count maxRetries) {
        StaticString::Container writeTransactionBytes = StaticString::Data<Bridgetek81xTypes::AddressSize>();
        ErrorType error = ErrorType::Failure;
        Count currentRetries = 0;

        writeTransactionBytes->push_back(((address >> 16) & 0xBF) | 0x80);
        writeTransactionBytes->push_back((address >> 8) & 0xFF);
        writeTransactionBytes->push_back(address & 0xFF);

        while (ErrorType::Success != error) {

            if (ErrorType::Success == (error = _chipSelect.pinWrite(GpioTypes::LogicLevel::Low))) {
                const IcCommunicationProtocolTypes::AdditionalCommunicationParameters additionalParams;
                error = _spi.txBlocking(writeTransactionBytes, timeout, additionalParams);

                if (ErrorType::Success == error) {
                    error = _spi.txBlocking(data, timeout, additionalParams);
                }
            }

            _chipSelect.pinWrite(GpioTypes::LogicLevel::High);

            constexpr uint32_t registersStart = static_cast<uint32_t>(Bridgetek81xTypes::BaseAddresses::Registers);
            constexpr uint32_t registersEnd = registersStart + static_cast<uint32_t>(Bridgetek81xTypes::AddressSpace::Registers);

            if (currentRetries < maxRetries && address >= registersStart && address < registersEnd) {
                uint32_t readBack = 0;
                error = hostMemoryRead(address, readBack, timeout);
                const uint32_t valueWritten = *((uint32_t *)&(data[0]));

                if (ErrorType::Success == error && (readBack & valueWritten) != valueWritten) {
                    error = ErrorType::Failure;
                    currentRetries++;
                }
            }
        }

        return maxRetries == 0 || currentRetries < maxRetries ? error : ErrorType::LimitReached;
    }

    /**
     * @brief Send a display list command to the BT81x
     * @param[in] command The display list command to send
     * @sa Bridgetek81xTyes::DisplayListCommands
     * @post After calling this function, call commitDisplayList to show the commands on the screen
     */
    ErrorType sendDisplayListCommand(const Bridgetek81xTypes::DisplayListCommand command);
    /**
     * @brief Send a host command to the BT81x
     * @param[in] hostCommand The host command to send
     * @param[in] parameter An optional parameter for the host command
     * @returns Anything returned by IcCommunicationProtocol::txBlocking
     * @returns Anything returned by GpioAbstraction::pinWrite
     */
    ErrorType sendHostCommand(const Bridgetek81xTypes::HostCommands hostCommand, const uint8_t parameter);
    /**
     * @brief Write data to the command buffer FIFO
     * @param[in] command The command to write to the command buffer
     * @param parameters The 4 byte parameters to write to the command buffer
     * @sa Bt81X Series Programming Guide, Sect. 5.1 Command FIFO - Fifo entries are always 4 bytes wide.
     * @param[in] flush True if the command buffer should be flushed to the LCD and displayed
     * @returns Anything returned by hostMemoryWrite
     */
    ErrorType writeToCommandBuffer(const Bridgetek81xTypes::Commands command, std::initializer_list<uint32_t> parameters, bool flush = false);
    /**
     * @brief True if there is space in the command buffer and bytes can be written.
     * @param[in] error The error that occured while qeurying the number of bytes available
     * @returns The number of bytes available
     * @post The return value is valid if, and only if, error == ErrorType::Success
     */
    uint16_t commandBufferSpace(ErrorType &error);
    /**
     * @brief Read the register ID
     * @param[in] maxRetries The maximum number of retries to attempt if the read fails
     * @returns Anything returned by hostMemoryRead
     * @returns Anything returned by startReadTransaction
     */
    ErrorType readRegId(const Count maxRetries);
    /**
     * @brief Read the chip ID
     * @pre This call only returns the valid chip ID if the location in RAM has not been overwritten.
     *      This is typically only true right after the device exits the powerdown state.
     * @param[out] chipId The chip ID read from the device
     * @returns Anything returned by hostMemoryRead
     * @returns Anything returned by startReadTransaction
     */
    ErrorType readChipId(uint32_t &chipId);
    /**
     * @brief Read the reset status register
     * @returns Anything returned by hostMemoryRead
     * @returns Anything returned by startReadTransaction
     */
    ErrorType readResetStatus();
    /**
     * @brief Set the screen parameters such as size, sync timings, etc.
     * @param[in] screenParams The screen parameters to set
     * @returns Anything returned by hostMemoryWrite
     * @sa Bt81X Datasheet Sect. 4.5 Parallel RGB interface, table 4-13
     */
    ErrorType setScreenParameters(const LcdTypes::ScreenParameters &screenParams);
    /**
     * @brief Reset the engines in the BT81x
     * @param[in] resetAudioEngine True if the audio engine should be reset
     * @param[in] resetTouchEngine True if the touch engine should be reset
     * @param[in] resetCoprocessorEngine True if the coprocessor engine should be reset
     */
    ErrorType resetEngines(const bool resetAudioEngine, const bool resetTouchEngine, const bool resetCoprocessorEngine);
    /**
     * @brief Check for screen touches
     * @param[out] touchTag The tag associated with the x and y coordinates of the touch
     * @returns ErrorType::Success if there are no touches, ErrorType::Negative if there are touches
     */
    ErrorType checkForScreenTouches(uint32_t &touchTag);

    /**
     * @brief Convert a frequency in Hertz to the corresponding host command value
     * @param[in] frequency The frequency to convert
     * @param[out] error Set to Success if the conversion was successful, InvalidParameter if the frequency is not supported
     * @returns The host command value for the frequency specified
     */
    constexpr uint8_t toHostCommandFrequency(const Hertz frequency, ErrorType &error) {
        error = ErrorType::Success;

        switch (frequency) {
            case Hertz(0):
                return 0;
            case Hertz(12E6):
                return 0x02;
            case Hertz(24E6):
                return 0x03;
            case Hertz(36E6):
                return 0x04;
            case Hertz(48E6):
                return 0x44;
            case Hertz(60E6):
                return 0x45;
            case Hertz(72E6):
                return 0x46;
            default:
                error = ErrorType::InvalidParameter;
                return 0;
        }
    }

    /**
     * @brief Convert a pixel format to a linestride
     * @param[in] pixelFormat The pixel format to convert
     * @returns The line stride for the pixel format
     */
    constexpr uint32_t toLineStride(const Bridgetek81xTypes::BitmapPixelFormat pixelFormat, const uint16_t width, ErrorType &error) {
        error = ErrorType::Success;

        constexpr auto bytesPerPixel = [](const uint32_t bitsPerPixel) -> float {
            constexpr float oneByteForEvery8Bits = 1/8.0f;
            return bitsPerPixel * oneByteForEvery8Bits;
        };

        switch (pixelFormat) {
            case Bridgetek81xTypes::BitmapPixelFormat::L1:
                return width * bytesPerPixel(1);
            case Bridgetek81xTypes::BitmapPixelFormat::L2:
                return width * bytesPerPixel(2);
            case Bridgetek81xTypes::BitmapPixelFormat::L4:
                return width * bytesPerPixel(4);
            case Bridgetek81xTypes::BitmapPixelFormat::L8:
            case Bridgetek81xTypes::BitmapPixelFormat::Rgb332:
            case Bridgetek81xTypes::BitmapPixelFormat::Argb2:
            case Bridgetek81xTypes::BitmapPixelFormat::Paletted565:
            case Bridgetek81xTypes::BitmapPixelFormat::Paletted4444:
            case Bridgetek81xTypes::BitmapPixelFormat::Paletted8:
                return width * bytesPerPixel(8);
            case Bridgetek81xTypes::BitmapPixelFormat::Argb1555:
            case Bridgetek81xTypes::BitmapPixelFormat::Argb4:
            case Bridgetek81xTypes::BitmapPixelFormat::Rgb565:
                return width * bytesPerPixel(16);
            default:
                error = ErrorType::InvalidParameter;
                return 0;
        }
    }
};

#endif //__BRIDGETEK_81X_HPP__