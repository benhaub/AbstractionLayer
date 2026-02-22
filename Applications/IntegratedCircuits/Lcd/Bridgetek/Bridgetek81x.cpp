//AbstractionLayer
#include "Bridgetek81x.hpp"
#include "OperatingSystemModule.hpp"

#ifndef BT81X_CHIP_SELECT_PERIPHERAL_NUMBER
#error "Bridgetek uses a GPIO chip select to communicate with the device. Please define the peripheral number"
#endif

#ifndef BT81X_CHIP_SELECT_PIN_NUMBER
#error "Bridgetek uses a GPIO chip select to communicate with the device. Please define the pin number"
#endif

ErrorType Bridgetek81x::init(const SpiTypes::SpiParams params, bool externalClock, const Bridgetek81xTypes::SystemClockFrequency systemClockFrequency, const LcdTypes::ScreenParameters &screenParams) {
    ErrorType error = ErrorType::InvalidParameter;

    if (params.driverConfig.clockFrequency <= 30E6 && params.driverConfig.clockFrequency <= static_cast<Hertz>(systemClockFrequency) / 2) {
        SpiTypes::SpiParams initParams = params;
        //Pg. 17, BT81x programming guide. During initialization, the SPI clock must not exceed 11MHz.
        initParams.driverConfig.clockFrequency = 10E6;

        if (ErrorType::Success == (error = _spi.configure(initParams))) {

            if (ErrorType::Success == (error = _spi.init())) {
                GpioTypes::GpioParams chipSelectParams;
                chipSelectParams.hardwareConfig.peripheralNumber = BT81X_CHIP_SELECT_PERIPHERAL_NUMBER;
                chipSelectParams.hardwareConfig.pinNumber = BT81X_CHIP_SELECT_PIN_NUMBER;
                chipSelectParams.hardwareConfig.pullUpEnable = true;
                chipSelectParams.hardwareConfig.driveStrength = GpioTypes::DriveStrength::TwoMilliAmps;
                chipSelectParams.hardwareConfig.driveType = GpioTypes::DriveType::PushPull;

                _chipSelect.configure(chipSelectParams);

                if (ErrorType::Success == (error = _chipSelect.init())) {
                    _chipSelect.pinWrite(GpioTypes::LogicLevel::High);

                    if (externalClock) {
                        error = sendHostCommand(Bridgetek81xTypes::HostCommands::ClockExternal, 0);
                        const Hertz lcdSystemClockFrequency = static_cast<Hertz>(systemClockFrequency);
                        const uint8_t clockSelectParameter = toHostCommandFrequency(lcdSystemClockFrequency, error);

                        if (ErrorType::Success == error) {
                            error = sendHostCommand(Bridgetek81xTypes::HostCommands::ClockSelect, clockSelectParameter);
                        }
                    }

                    if (ErrorType::Success == (error = sendHostCommand(Bridgetek81xTypes::HostCommands::ResetPulse, 0))) {
                        
                        if (ErrorType::Success == (error = sendHostCommand(Bridgetek81xTypes::HostCommands::Active, 0))) {
                            //Sect. 4.9.4 BT81x Datasheet, May take up to 300ms before software can access registers or RAM from the
                            //sleep state.
                            OperatingSystem::Instance().delay(Milliseconds(300));

                            constexpr Count maxRetries = 50000;

                            if (ErrorType::Success == (error = readRegId(maxRetries))) {
                                //At startup the chip ID is located in RAM and is readable until the application overwrites it.
                                uint32_t chipId = 0;
                                if (ErrorType::Success == (error = readChipId(chipId))) {

                                    if (((chipId >> 8) & 0xFF) >= 0x15 && ((chipId >> 8) & 0xFF) <= 0x16) {

                                        if (ErrorType::Success == (error = readResetStatus())) {
                                            const Hertz lcdSystemClockFrequency = static_cast<Hertz>(systemClockFrequency);
                                            constexpr uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::Frequency);
                                            StaticString::Container dataBytes = StaticString::Data<sizeof(uint32_t)>();
                                            dataBytes->assign(reinterpret_cast<const char*>(&lcdSystemClockFrequency), sizeof(lcdSystemClockFrequency));

                                            if (ErrorType::Success == (error = hostMemoryWrite(address, dataBytes, Milliseconds(500), 10))) {
                                                error = setScreenParameters(screenParams);

                                                if (ErrorType::Success == error) {
                                                    _pixelClockDivisor = screenParams.pixelClockDivisor;

                                                    error = _spi.deinit();

                                                    if (ErrorType::Success == error) {
                                                        error = _spi.configure(params);

                                                        if (ErrorType::Success == error) {
                                                            error = _spi.init();
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        PLT_LOGE(TAG, "Detected incompatible chip <chipId: %u>", chipId);
                                        error = ErrorType::NotSupported;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return error;
}

ErrorType Bridgetek81x::reset(const Gpio &powerdown) {
    ErrorType error = powerdown.pinWrite(GpioTypes::LogicLevel::Low);

    if (ErrorType::Success == error) {
        //Minimum time for PD to be held low is 5ms. Sect. 4.9.2 BT81x Datasheet
        OperatingSystem::Instance().delay(Milliseconds(6));

        error = powerdown.pinWrite(GpioTypes::LogicLevel::High);
    }

    return error;
}

ErrorType Bridgetek81x::startFreeHandSketch(const Area &area, const HexCodeColour brushColour, const HexCodeColour paperColour, const Bridgetek81xTypes::BitmapPixelFormat pixelFormat) {
    ErrorType error = ErrorType::InvalidParameter;

    if (Bridgetek81xTypes::BitmapPixelFormat::L1 == pixelFormat || Bridgetek81xTypes::BitmapPixelFormat::L8 == pixelFormat) {
        error = setDrawColour(paperColour);

        if (ErrorType::Success == error) {
            error = writeToCommandBuffer(Bridgetek81xTypes::Commands::Sketch,
            {
                static_cast<uint32_t>((area.origin.y << 16) | area.origin.x),
                static_cast<uint32_t>((area.height << 16) | area.width),
                static_cast<uint32_t>(Bridgetek81xTypes::BaseAddresses::GeneralPurposeGraphicsRam),
                static_cast<uint32_t>(pixelFormat) 
            });

            if (ErrorType::Success == error) {
                error = writeToCommandBuffer(Bridgetek81xTypes::Commands::Memzero,
                {
                    static_cast<uint32_t>(Bridgetek81xTypes::BaseAddresses::GeneralPurposeGraphicsRam),
                    static_cast<uint32_t>(area.size())
                });

                if (ErrorType::Success == error) {
                    //TODO: There are 32 possible bitmap handles. We should be tracking an managing their use.
                    error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::BitmapHandle(1));

                    if (ErrorType::Success == error) {
                        constexpr uint32_t bitmapSourceAddress =
                            static_cast<uint32_t>(Bridgetek81xTypes::BaseAddresses::GeneralPurposeGraphicsRam);
                        constexpr Bridgetek81xTypes::DisplayListCommand bitmapSource =
                            Bridgetek81xTypes::DisplayListCommands::BitmapSourceAddress(bitmapSourceAddress);
                        error = sendDisplayListCommand(bitmapSource);

                        if (ErrorType::Success == error) {
                            const uint32_t linestride = toLineStride(pixelFormat, area.width, error);
                            const Bridgetek81xTypes::DisplayListCommand bitmapLayout =
                                Bridgetek81xTypes::DisplayListCommands::BitmapLayout(pixelFormat, linestride, area.height);
                            error = sendDisplayListCommand(bitmapLayout);

                            if (ErrorType::Success == error) {
                                constexpr Bridgetek81xTypes::DisplayListCommands::BitmapFilteringMode filterMode =
                                    Bridgetek81xTypes::DisplayListCommands::BitmapFilteringMode::Nearest;
                                constexpr Bridgetek81xTypes::DisplayListCommands::BitmapWrapMode wrapModeX =
                                    Bridgetek81xTypes::DisplayListCommands::BitmapWrapMode::Border;
                                constexpr Bridgetek81xTypes::DisplayListCommands::BitmapWrapMode wrapModeY =
                                    Bridgetek81xTypes::DisplayListCommands::BitmapWrapMode::Border;
                                const Bridgetek81xTypes::DisplayListCommand bitmapSize =
                                    Bridgetek81xTypes::DisplayListCommands::BitmapSize(filterMode,
                                                                                        wrapModeX,
                                                                                        wrapModeY,
                                                                                        area);

                                error = sendDisplayListCommand(bitmapSize);
                            }
                        }
                    }
                }
            }
        }
    }

    if (ErrorType::Success == error) {
        sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::LineWidth(16));

        if (ErrorType::Success == error) {
            error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::BeginGraphicsPrimitive(Bridgetek81xTypes::DisplayListCommands::GraphicsPrimitive::Rectangles));

            if (ErrorType::Success == error) {
                error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::Vertex2F(0, 10*16));

                if (ErrorType::Success == error) {
                    error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::Vertex2F(area.width*16, area.height*16));

                    if (ErrorType::Success == error) {
                        error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::ColorRgb(brushColour));

                        if (ErrorType::Success == error) {
                            error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::BeginGraphicsPrimitive(Bridgetek81xTypes::DisplayListCommands::GraphicsPrimitive::Bitmaps));

                            if (ErrorType::Success == error) {
                                //TODO: This should be using the bitmap handle that was set earlier the the BitmapHandle DL command.
                                error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::Vertex2II(0, 0, 1, 0));

                                if (ErrorType::Success == error) {
                                    error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::EndGraphicsPrimitive());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return error;
}

ErrorType Bridgetek81x::saveScreenToRamG(const uint32_t address, const Area &area) {
    constexpr uint32_t ramG = static_cast<uint32_t>(Bridgetek81xTypes::BaseAddresses::GeneralPurposeGraphicsRam);
    constexpr uint32_t ramGEnd = ramG + static_cast<uint32_t>(Bridgetek81xTypes::AddressSpace::GeneralPurposeGraphicsRam);
    ErrorType error = ErrorType::InvalidParameter;

    if (address >= ramG && address + area.size() < ramGEnd) {
        error = writeToCommandBuffer(Bridgetek81xTypes::Commands::Snapshot2,
            {
                static_cast<uint32_t>(Bridgetek81xTypes::BitmapPixelFormat::Argb4),
                address,
                static_cast<uint32_t>(area.origin.y) << 16 | area.origin.x,
                static_cast<uint32_t>(area.height) << 16 | area.width,
            }, true);
    }

    return error;
}

ErrorType Bridgetek81x::clearScreen(const uint32_t hexCodeColour) {
    ErrorType error = startDisplayList();

    if (ErrorType::Success == error) {
        error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::ClearColorRgb(hexCodeColour));

        if (ErrorType::Success == error) {
            error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::Clear(1,1,1));
        }
    }

    if (ErrorType::Success == error) {
        error = commitDisplayList();
    }

    return error;
}

ErrorType Bridgetek81x::setForegroundColour(const HexCodeColour colour) {
    return writeToCommandBuffer(Bridgetek81xTypes::Commands::FgColor, {static_cast<uint32_t>(colour)});
}

ErrorType Bridgetek81x::setBackgroundColour(const HexCodeColour colour) {
    return writeToCommandBuffer(Bridgetek81xTypes::Commands::BgColor, {static_cast<uint32_t>(colour)});
}

ErrorType Bridgetek81x::setDrawColour(const HexCodeColour colour) {
    return sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::ColorRgb(static_cast<uint32_t>(colour)));
}

ErrorType Bridgetek81x::toggleBacklight(const bool on, const Percent brightness) {
    constexpr Milliseconds timeout = 500;
    constexpr Count maxRetries = 10;
    ErrorType error;
    StaticString::Container dataBytes = StaticString::Data<sizeof(uint32_t)>();

    constexpr uint32_t gpioXDirectionAddress = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::GpioXDirection);
    uint16_t value = on ? 0x8000 : 0;
    dataBytes->assign(reinterpret_cast<const char*>(&value), sizeof(value));

    if (ErrorType::Success == (error = hostMemoryWrite(gpioXDirectionAddress, dataBytes, timeout, maxRetries))) {
        constexpr uint32_t setBacklightAddress = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::GpioXParameters);
        value = on ? 0x8000 : 0;
        dataBytes->assign(reinterpret_cast<const char*>(&value), sizeof(value));

        if (ErrorType::Success == (error = hostMemoryWrite(setBacklightAddress, dataBytes, timeout, maxRetries))) {
            constexpr uint32_t pwmEnableAddress = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::PwmHz);
            value = on ? 250 : 0;
            dataBytes->assign(reinterpret_cast<const char*>(&value), sizeof(value));

            if (ErrorType::Success == (error = hostMemoryWrite(pwmEnableAddress, dataBytes, timeout, maxRetries))) {
                constexpr uint32_t pwmDutyAddress = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::PwmDuty);
                value = on ? (brightness / 100.0f) * 128 : 0;
                dataBytes->assign(reinterpret_cast<const char*>(&value), sizeof(value));
                error = hostMemoryWrite(pwmDutyAddress, dataBytes, timeout, maxRetries);
            }
        }
    }

    return error;
}

ErrorType Bridgetek81x::toggleDisplay(const bool on) {
    constexpr Count maxRetries = 10;
    constexpr Milliseconds timeout = 500;
    constexpr uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::PixelClock);
    const uint8_t data = on ? _pixelClockDivisor : 0;
    StaticString::Container dataBytes = StaticString::Data<sizeof(uint8_t)>();
    dataBytes->assign(reinterpret_cast<const char*>(&data), sizeof(data));
    return hostMemoryWrite(address, dataBytes, timeout, maxRetries);
}

ErrorType Bridgetek81x::setTouchThreshold(const uint16_t threshold) {
    constexpr Count maxRetries = 10;
    constexpr Milliseconds timeout = 500;
    constexpr uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::ResistiveTouchEngineRegisters::TouchRzThresh);
    StaticString::Container dataBytes = StaticString::Data<sizeof(uint16_t)>();
    dataBytes->assign(reinterpret_cast<const char*>(&threshold), sizeof(threshold));
    return hostMemoryWrite(address, dataBytes, timeout, maxRetries);
}

ErrorType Bridgetek81x::calibrate(const Coordinate &instructionTextLocation) {
    //It's not a bug that commitDisplayList is not called.
    //Pg. 159, Sect. 5.60 CMD_CALIBRATE, Bt81X Programming Guide.
    ErrorType error = startDisplayList();

    if (ErrorType::Success == error) {
        error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::ClearColorRgb(0x00000000));

        if (ErrorType::Success == error) {
            error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::Clear(1,1,1));

            if (ErrorType::Success == error) {
                error = drawText<26>(instructionTextLocation, Bridgetek81xTypes::Font::Font11, 0x00FFFFFF, Bridgetek81xTypes::Options::Centered, "Tap the dots to calibrate");

                if (ErrorType::Success == error) {
                    error = writeToCommandBuffer(Bridgetek81xTypes::Commands::Calibrate, {0}, true);
                }
            }
        }

        //while the programming guide does say that "The completion of this function [calibration] is detected when
        //the value of REG_CMD_READ is equal to REG_CMD_WRITE", from testing it appears that they simply mean the completion of
        //writing the command to the LCD screen, not the completion of the actual calibration process.
        constexpr uint32_t regTouchDirectXyAddress = static_cast<uint32_t>(Bridgetek81xTypes::ResistiveTouchEngineRegisters::TouchDirectXy);
        constexpr Milliseconds timeout = 500;
        uint32_t isScreenTouched = 0;
        int numberOfTouchesDetected = 0;
        
        while (numberOfTouchesDetected < 3) {
            error = hostMemoryRead(regTouchDirectXyAddress, isScreenTouched, timeout);
            bool touchDetected = ErrorType::Success == error && (isScreenTouched & 0x80000000) == 0;

            if (touchDetected) {
                numberOfTouchesDetected++;
            }

            //Make sure we don't count holding touches on the screen as a touch.
            while (touchDetected) {
                error = hostMemoryRead(regTouchDirectXyAddress, isScreenTouched, timeout);
                touchDetected = ErrorType::Success == error && (isScreenTouched & 0x80000000) == 0;
            }
        }
    }

    return error;
}

ErrorType Bridgetek81x::startDisplayList() {
    ErrorType error = writeToCommandBuffer(Bridgetek81xTypes::Commands::DisplayListStart, {});

    if (ErrorType::Success == error) {
        //Pg. 23, Code Snippet 10 Note, BT81x Programming Guide. Command clear is recommened to be used before
        //any other drawing operation to put the graphics engine in a known state.
        error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::Clear(1,1,1));
    }

    return error;
}

ErrorType Bridgetek81x::commitDisplayList() {
    ErrorType error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::Display());

    if (ErrorType::Success == error) {
        error = writeToCommandBuffer(Bridgetek81xTypes::Commands::Swap, {}, true);
    }

    return error;
}

ErrorType Bridgetek81x::enableTouchTag(const uint8_t tag) {
    ErrorType error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::GraphicsObjectTagMask(1));

    if (ErrorType::Success == error) {
        error = sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::GraphicsObjectTag(tag));
    }

    return error;
}

ErrorType Bridgetek81x::disableTouchTag() {
    return sendDisplayListCommand(Bridgetek81xTypes::DisplayListCommands::GraphicsObjectTagMask(0));
}

ErrorType Bridgetek81x::checkForScreenTouches(const uint8_t tag, Coordinate &touchedAt) {
    constexpr uint32_t tagAddress = static_cast<uint32_t>(Bridgetek81xTypes::ResistiveTouchEngineRegisters::TouchTag);
    constexpr uint32_t tagXyAddress = static_cast<uint32_t>(Bridgetek81xTypes::ResistiveTouchEngineRegisters::TouchTagXy);
    constexpr Milliseconds timeout = 500;
    uint8_t tagValue = 0;
    uint32_t tagXyValue = 0;

    ErrorType error = hostMemoryRead(tagAddress, tagValue, timeout);

    if (ErrorType::Success == error) {
        error = hostMemoryRead(tagXyAddress, tagXyValue, timeout);

        if (ErrorType::Success == error) {
            if (tagValue == tag) {
                touchedAt.y = tagXyValue & 0xFFFF;
                touchedAt.x = tagXyValue >> 16;
                error = ErrorType::Success;
            }
            else {
                touchedAt.x = 0;
                touchedAt.y = 0;
                error = ErrorType::Negative;
            }
        }
    }
    else {
        error = ErrorType::Negative;
    }

    return error;
}

ErrorType Bridgetek81x::memoryCopy(const PixelFormat pixelFormat, const uint32_t address, char *buffer, const size_t bufferSize, size_t &bytesCopied) {
    uint32_t dataBuffer = 0;
    ErrorType error = ErrorType::InvalidParameter;

    //TODO: Support more formats.
    //TODO: Assumes we saved in Argb4 format.
    //if (PixelFormat::Greyscale == pixelFormat) {

    //    for (size_t i = 0, j = 0; i < bufferSize*2; i += sizeof(dataBuffer), j += 2) {
    //        error = hostMemoryRead(address + i, dataBuffer, Milliseconds(500));

    //        if (ErrorType::Success == error) {
    //            const uint16_t grey = ToGreyscale(PixelFormat::Argb4, dataBuffer);
    //            const uint8_t grey1 = grey >> 8;
    //            const uint8_t grey2 = grey & 0xFF;

    //            memcpy(&buffer[j], &grey1, sizeof(grey1));
    //            bytesCopied += sizeof(grey1);
    //            memcpy(&buffer[j+1], &grey2, sizeof(grey2));
    //            bytesCopied += sizeof(grey2);
    //        }
    //        else {
    //            break;
    //        }
    //    }
    //}

    //TODO: Assumes to we saved in grey scale and want greyscale back.
    for (size_t i = 0; i < bufferSize; i = i + sizeof(dataBuffer)) {
        error = hostMemoryRead(address + i, dataBuffer, Milliseconds(500));

        if (ErrorType::Success == error) {
            memcpy(&buffer[i], &dataBuffer, sizeof(dataBuffer));
            bytesCopied += sizeof(dataBuffer);
        }
        else {
            break;
        }
    }

    return error;
}

ErrorType Bridgetek81x::stopPeriodicOperation() {
    return writeToCommandBuffer(Bridgetek81xTypes::Commands::Stop, {}, true);
}

ErrorType Bridgetek81x::sendDisplayListCommand(const Bridgetek81xTypes::DisplayListCommand command) {
    return writeToCommandBuffer(static_cast<Bridgetek81xTypes::Commands>(command), {});
}

ErrorType Bridgetek81x::sendHostCommand(const Bridgetek81xTypes::HostCommands hostCommand, const uint8_t parameter) {
    constexpr Bytes dummy = 0x00;
    StaticString::Container hostCommandBytes = StaticString::Data<sizeof(hostCommand) + sizeof(parameter) + sizeof(dummy)>();
    ErrorType error;

    hostCommandBytes->push_back(static_cast<uint8_t>(hostCommand));
    hostCommandBytes->push_back(parameter);
    hostCommandBytes->push_back(dummy);

    if (ErrorType::Success == (error = _chipSelect.pinWrite(GpioTypes::LogicLevel::Low))) {
        IcCommunicationProtocolTypes::AdditionalCommunicationParameters additionalParams;
        error = _spi.txBlocking(hostCommandBytes, Milliseconds(500), additionalParams);
    }

    _chipSelect.pinWrite(GpioTypes::LogicLevel::High);

    return error;
}

ErrorType Bridgetek81x::writeToCommandBuffer(const Bridgetek81xTypes::Commands command, std::initializer_list<uint32_t> parameters, bool flush) {
    constexpr uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::CoprocessorEngineRegisters::WriteDataToCommandBuffer);
    constexpr Milliseconds timeout = 500;
    //const uint16_t maxCommandBufferSize = 4096 - 4;
    ErrorType error = ErrorType::LimitReached;

    std::string_view commandBytes = std::string_view(reinterpret_cast<const char *>(&command), sizeof(command));

    if (_commandBuffer->size() + parameters.size()*4 + commandBytes.size() <= _commandBuffer->capacity()) {
        _commandBuffer->append(commandBytes);

        for (const auto param : parameters) {
            std::string_view paramBytes = std::string_view(reinterpret_cast<const char *>(&param), sizeof(param));
            _commandBuffer->append(paramBytes);
        }
        
        if (flush) {
            error = hostMemoryWrite(address, _commandBuffer, timeout, 0);
            _commandBuffer->clear();
            //uint16_t freeSpaceInCommandBuffer = 0;

            //TODO: It would probably be better to use the interrupts for this instead.
            //while (ErrorType::Success == error && freeSpaceInCommandBuffer != maxCommandBufferSize) {
            //    freeSpaceInCommandBuffer = commandBufferSpace(error);
            //}
        }
        else {
            error = ErrorType::Success;
        }
    }

    return error;
}

uint16_t Bridgetek81x::commandBufferSpace(ErrorType &error) {
    error = ErrorType::Success;
    constexpr Milliseconds timeout = 500;
    constexpr Count maxRetries = 10;
    //Pg. 107, Sect. 5.7 Coprocessor Faults
    constexpr uint16_t coprocessorFaultCode = 0xfff;
    const uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::CoprocessorEngineRegisters::FreeSpaceInCommandBuffer);

    uint16_t freeSpaceInCommandBuffer = 0;
    error = hostMemoryRead(address, freeSpaceInCommandBuffer, timeout);

    //Pg. 107 Sect. 5.7 Coprocessor Faults
    if (freeSpaceInCommandBuffer == coprocessorFaultCode) {
        uint32_t patchAddress = 0;
        constexpr uint32_t coprocessorPatchPointer = static_cast<uint32_t>(Bridgetek81xTypes::SpecialRegisters::CoprocessorPatchPointer);
        error = hostMemoryRead(coprocessorPatchPointer, patchAddress, timeout);

        if (ErrorType::Success == error) {
            constexpr uint32_t cpuResetAddress = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::CpuReset);
            StaticString::Container dataBytes = StaticString::Data<sizeof(uint32_t)>();
            dataBytes->push_back(0x1);
            error = hostMemoryWrite(cpuResetAddress, dataBytes, timeout, maxRetries);

            if (ErrorType::Success == error) {
                constexpr uint32_t readCommandFifoOffset = static_cast<uint32_t>(Bridgetek81xTypes::CoprocessorEngineRegisters::ReadFifoOffset);
                constexpr uint32_t writeCommandFifoOffset = static_cast<uint32_t>(Bridgetek81xTypes::CoprocessorEngineRegisters::WriteFifoOffset);
                constexpr uint32_t displayListFifoOffset = static_cast<uint32_t>(Bridgetek81xTypes::CoprocessorEngineRegisters::DisplayListFifoOffset);
                dataBytes->clear();
                dataBytes->push_back(0x0);
                error = hostMemoryWrite(readCommandFifoOffset, dataBytes, timeout, maxRetries);

                if (ErrorType::Success == error) {
                    dataBytes->clear();
                    dataBytes->push_back(0x0);
                    error = hostMemoryWrite(writeCommandFifoOffset, dataBytes, timeout, maxRetries);

                    if (ErrorType::Success == error) {
                        dataBytes->clear();
                        dataBytes->push_back(0x0);
                        uint32_t displayListFifo = 0;
                        error = hostMemoryRead(displayListFifoOffset, displayListFifo, timeout);

                        if (ErrorType::Success == error) {
                            error = hostMemoryWrite(cpuResetAddress, dataBytes, timeout, maxRetries);

                            if (ErrorType::Success == error) {
                                dataBytes->assign(reinterpret_cast<const char*>(&patchAddress), sizeof(patchAddress));
                                error = hostMemoryWrite(coprocessorPatchPointer, dataBytes, timeout, maxRetries);

                                if (ErrorType::Success == error) {
                                    error = writeToCommandBuffer(Bridgetek81xTypes::Commands::Flashattach, {}, true);

                                    if (ErrorType::Success == error) {
                                        error = writeToCommandBuffer(Bridgetek81xTypes::Commands::Flashfast, {}, true);

                                        if (ErrorType::Success == error) {
                                            dataBytes->assign(reinterpret_cast<const char*>(&_pixelClockDivisor), sizeof(_pixelClockDivisor));
                                            error = hostMemoryWrite(address, dataBytes, Milliseconds(500), maxRetries);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return freeSpaceInCommandBuffer;
}

ErrorType Bridgetek81x::readRegId(const Count maxRetries) {
    constexpr uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::Id);
    constexpr uint8_t expectedRegId = 0x7C;
    constexpr Milliseconds timeout = 500;
    ErrorType error;
    Count retries = 0;
    uint8_t data = 0;

    while ((data & 0xFF) != expectedRegId && retries < maxRetries) {

        error = hostMemoryRead(address, data, timeout);
        retries++;

        if (retries >= maxRetries) {
            error = ErrorType::Timeout;
        }
    }

    return error;
}

ErrorType Bridgetek81x::readChipId(uint32_t &chipId) {
    constexpr uint32_t address = 0xC0000;
    constexpr Milliseconds timeout = 500;

    return hostMemoryRead(address, chipId, timeout);
}

ErrorType Bridgetek81x::readResetStatus() {
    constexpr uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::CpuReset);
    constexpr Count maxRetries = 500;
    constexpr Milliseconds timeout = 500;
    uint8_t resetStatus = 0;
    Count retries = 0;
    ErrorType error;

    do {
        error = hostMemoryRead(address, resetStatus, timeout);
        retries++;

        if (ErrorType::Success != error && retries >= maxRetries) {
            error = ErrorType::Timeout;
        }

    } while ((resetStatus & 0x7) != 0 && retries < maxRetries);

    return error;
}

ErrorType Bridgetek81x::setScreenParameters(const LcdTypes::ScreenParameters &screenParams) {
    constexpr Milliseconds timeout = 500;
    constexpr Count maxRetries = 10;
    ErrorType error = ErrorType::Success;
    StaticString::Container dataBytes = StaticString::Data<sizeof(uint32_t)>();

    constexpr uint32_t hSizeAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Hsize);
    dataBytes->assign(reinterpret_cast<const char*>(&screenParams.activeArea.width), sizeof(screenParams.activeArea.width));
    error = hostMemoryWrite(hSizeAddress, dataBytes, timeout, maxRetries);

    if (ErrorType::Success == error) {
        constexpr uint32_t hCycleAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Hcycle);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.totalHorizontalLines), sizeof(screenParams.totalHorizontalLines));
        error = hostMemoryWrite(hCycleAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t hOffsetAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Hoffset);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.horizontalOffset), sizeof(screenParams.horizontalOffset));
        error = hostMemoryWrite(hOffsetAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t hSync0Address = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Hsync0);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.horizontalFrontPorch), sizeof(screenParams.horizontalFrontPorch));
        error = hostMemoryWrite(hSync0Address, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t hSync1Address = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Hsync1);
        const uint16_t hsync = screenParams.horizontalFrontPorch + screenParams.horizontalPulseWidth;
        dataBytes->assign(reinterpret_cast<const char*>(&hsync), sizeof(hsync));
        error = hostMemoryWrite(hSync1Address, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t vSizeAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Vsize);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.activeArea.height), sizeof(screenParams.activeArea.height));
        error = hostMemoryWrite(vSizeAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t vCycleAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Vcycle);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.totalVerticalLines), sizeof(screenParams.totalVerticalLines));
        error = hostMemoryWrite(vCycleAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t vOffsetAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Voffset);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.verticalOffset), sizeof(screenParams.verticalOffset));
        error = hostMemoryWrite(vOffsetAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t vSync0Address = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Vsync0);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.verticalFrontPorch), sizeof(screenParams.verticalFrontPorch));
        error = hostMemoryWrite(vSync0Address, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t vSync1Address = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Vsync1);
        const uint16_t vsync = screenParams.verticalFrontPorch + screenParams.verticalPulseWidth;
        dataBytes->assign(reinterpret_cast<const char*>(&vsync), sizeof(vsync));
        error = hostMemoryWrite(vSync1Address, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t swizzleAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Swizzle);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.swizzle), sizeof(screenParams.swizzle));
        error = hostMemoryWrite(swizzleAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        const uint8_t cspread = 0;
        constexpr uint32_t cspreadAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Cspread);
        dataBytes->assign(reinterpret_cast<const char*>(&cspread), sizeof(cspread));
        error = hostMemoryWrite(cspreadAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t pclkPolarityAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::PixelClockPolarity);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.pixelClockPolarity), sizeof(screenParams.pixelClockPolarity));
        error = hostMemoryWrite(pclkPolarityAddress, dataBytes, timeout, maxRetries);
    }
    if (ErrorType::Success == error) {
        constexpr uint32_t ditherAddress = static_cast<uint32_t>(Bridgetek81xTypes::GraphicsEngineRegisters::Dither);
        dataBytes->assign(reinterpret_cast<const char*>(&screenParams.dithering), sizeof(screenParams.dithering));
        error = hostMemoryWrite(ditherAddress, dataBytes, timeout, maxRetries);
    }

    return error;
}

ErrorType Bridgetek81x::resetEngines(const bool resetAudioEngine, const bool resetTouchEngine, const bool resetCoprocessorEngine) {
    constexpr uint32_t address = static_cast<uint32_t>(Bridgetek81xTypes::MiscellaneousRegisters::CpuReset);
    constexpr Milliseconds timeout = 500;
    StaticString::Container dataBytes = StaticString::Data<sizeof(uint32_t)>();
    uint8_t resetBits = 0;

    if (resetCoprocessorEngine) {
        resetBits |= 0x1;
    }

    if (resetTouchEngine) {
        resetBits |= 0x2;
    }

    if (resetAudioEngine) {
        resetBits |= 0x4;
    }

    dataBytes->push_back(resetBits);
    ErrorType error = hostMemoryWrite(address, dataBytes, timeout, 10);

    if (ErrorType::Success == error) {
        dataBytes->clear();
        dataBytes->push_back(0x0);
        error = hostMemoryWrite(address, dataBytes, timeout, 10);
    }

    return error;
}

ErrorType Bridgetek81x::checkForScreenTouches(uint32_t &touchTag) {
    constexpr uint32_t touchTagAddress = static_cast<uint32_t>(Bridgetek81xTypes::ResistiveTouchEngineRegisters::TouchTag);
    constexpr uint32_t touchTagRawAddress = static_cast<uint32_t>(Bridgetek81xTypes::ResistiveTouchEngineRegisters::TouchRawXy);
    constexpr Milliseconds timeout = 500;
    uint32_t tag = 0;
    uint32_t touchCoordinates = 0;

    ErrorType error = hostMemoryRead(touchTagAddress, tag, timeout);

    if (ErrorType::Success == error) {
        error = hostMemoryRead(touchTagRawAddress, touchCoordinates, timeout);

        if (ErrorType::Success == error) {

            if (touchCoordinates & 0x8000) {
                touchTag = tag;
            }
            else {
                error = ErrorType::Negative;
            }
        }
    }

    return error;
}