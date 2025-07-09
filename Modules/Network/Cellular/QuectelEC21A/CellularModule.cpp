//AbstractionLayer Modules
#include "CellularModule.hpp"
#include "OperatingSystemModule.hpp"
//AbstractionLayer Applications
#include "Log.hpp"
//C++
#include <cstring>

#define CELLULAR_MODULE_DEBUGGING_ON 0

ErrorType Cellular::init() {
    assert(nullptr != _ic);
    assert(-1 != _resetPin);
    assert(false == _accessPointName.empty());
    assert(CellularConfig::RadioAccessTechnology::Unknown != _radioAccessTechnology);
    assert(CellularConfig::AccessMode::Unknown != _accessMode);

    ErrorType error = ErrorType::Failure;

    _gpioReset = std::make_unique<Gpio>();
    assert(nullptr != _gpioReset);
    _gpioReset->configure(nullptr, _resetPin, GpioPinDirection::DigitalOutput, GpioInterruptMode::Disabled, false, false);

    PLT_LOGI(TAG, "Resetting modem. Waiting for at least 12 seconds for uart to become active.");
    error = reset();
    if (ErrorType::Success != error) {
        return error;
    }

    error = _ic->flushRxBuffer();
    if (ErrorType::Success != error) {
        return error;
    }

    std::string mfgName(32, 0);
    if (ErrorType::Success == getManufacturerName(mfgName)) {
        if (std::string::npos != mfgName.find("Quectel")) {
            _status.manufacturerName.assign("Quectel");
        }
    }
    else if (mfgName.empty()) {
        return ErrorType::PrerequisitesNotMet;
    }

    //Parsing of responses depends on echo mode being disabled.
    if (ErrorType::Success != echoMode(false)) {
        PLT_LOGE(TAG, "Failed to disable echo mode.");
        return error;
    }

    error = terminatingCharacter(_commandLineTerminationCharacter);
    if (ErrorType::Success != error) {
        return error;
    }

    error = responseFormattingCharacter(_responseFormattingCharacter);
    if (ErrorType::Success != error) {
        return error;
    }

    error = networkUp();
    if (ErrorType::Success != error) {
        return error;
    }

    return ErrorType::Success;
}

ErrorType Cellular::networkUp() {
    constexpr Milliseconds timeout = 1000;
    constexpr Count maxRetries = 10;
    ErrorType error = ErrorType::Failure;
    std::string responseBuffer(64, 0);

    error = simCardIsInserted();
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "SIM card not inserted.");
        return error;
    }

    error = sendCommand("AT+COPS=0", timeout, maxRetries);
    if (ErrorType::Success != error) {
        return error;
    }

    error = receiveCommand(responseBuffer, timeout, maxRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "AT command error: AT+COPS=0");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    //Quectel LTE Standard TCP/IP Application Note, Pg. 7.
    //It can take up to 90 seconds for the module to register to the network.
    {
    int registerToNetworkRetries = 0;
    do {
        OperatingSystem::Instance().delay(1000);
        error = sendCommand("AT+CREG?", timeout, 1);
        if (ErrorType::Success != error) {
            return error;
        }

        error = receiveCommand(responseBuffer, timeout, 1, "+CREG: 0,5");
        if (ErrorType::Success != error) {
            PLT_LOGW(TAG, "Not registered to network.");
            PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        }

    } while (registerToNetworkRetries < 90 && ErrorType::Success != error);
    }

    if (ErrorType::Success != error) {
        //If it does not register in 90s, then reboot the module.
        reset();
        return error;
    }

    assert(false == accessPointNameConst().empty());
    assert(snprintf(responseBuffer.data(), responseBuffer.size(), "AT+CGDCONT=%d,\"IP\",\"%s\"", _IpContext, accessPointNameConst().c_str()));
    error = sendCommand(responseBuffer, timeout, maxRetries);
    if (ErrorType::Success != error) {
        return error;
    }
    error = receiveCommand(responseBuffer, timeout, maxRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "AT command error: AT+CGDCONT");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    error = sendCommand("AT+CGATT=1", timeout, maxRetries);
    if (ErrorType::Success != error) {
        return error;
    }
    error = receiveCommand(responseBuffer, timeout, maxRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "AT command error: AT+CGATT");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    _status.isUp = true;
    return error;
}

ErrorType Cellular::networkDown() {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) {
    return ErrorType::NotImplemented;
}

//https://iot.stackexchange.com/questions/5705/getting-ip-and-mac-address-of-lte-modem-using-at-commands
//https://networkengineering.stackexchange.com/questions/77606/mobile-cellular-networks-gsm-lte-5g-and-mac-addresses#:~:text=Mobile%20devices%20don't%20use,service%2C%20they%20use%20IMEI%20addresses./
//Cell networks don't use a MAC address.
ErrorType Cellular::getMacAddress(std::string &macAddress) {
    return ErrorType::NotAvailable;
}

//Pg. 78, sect. 6.3,Quectel AT command manual.
ErrorType Cellular::getSignalStrength(DecibelMilliWatts &signalStrength) {
    constexpr Milliseconds timeout = 1000;
    constexpr Count maxRetries = 10;
    std::string responseBuffer(64, 0);

    ErrorType error = sendCommand("AT+CSQ", timeout, maxRetries);
    if (ErrorType::Success != error) {
        return error;
    }

    error = receiveCommand(responseBuffer, timeout, maxRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "AT command error: AT+CSQ");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    for (size_t i = 0; i < responseBuffer.size(); i++) {
        if (',' == responseBuffer.at(i)) {
            responseBuffer.at(i) = '\0';
            const size_t sizeOfSignalQualityResponsePreamble = sizeof(_commandLineTerminationCharacter) + sizeof(_responseFormattingCharacter) + sizeof("+CSQ: ")-1;
            signalStrength = strtod(responseBuffer.c_str() + sizeOfSignalQualityResponsePreamble, nullptr);
            break;
        }
    }

    if (signalStrength != 199 && signalStrength != 99) {
        if (signalStrength <= 31) {
            signalStrength = (signalStrength * 2) - 113;
        }
        else if (signalStrength >= 100) {
            signalStrength -= 216;
        }
    }

    _status.signalStrength = signalStrength;

    return ErrorType::Success;
}

ErrorType Cellular::mainLoop() {
    ErrorType error;

    if (ErrorType::NoData == (error = runNextEvent())) {
        return error;
    }
    else if (ErrorType::Success == error) {
        return error;
    }

    return ErrorType::Failure;
}

//See Reset, Section 3.8, Quectel hardware design
ErrorType Cellular::reset() {
    assert(nullptr != _gpioReset);
    _gpioReset->pinWrite(GpioLogicLevel::High);
    OperatingSystem::Instance().delay(Milliseconds(151));
    _gpioReset->pinWrite(GpioLogicLevel::Low);

    //See Note 1, Pg. 43, Quectel hardware design.
    //Wait 12 seconds for the uart to become active.
    OperatingSystem::Instance().delay(Milliseconds(12000));

    return ErrorType::Success;
}

ErrorType Cellular::sendCommand(const std::string &atCommand, const Milliseconds timeout, const Count maxRetries) {
    std::string commandBuffer(atCommand.size() + sizeof(_commandLineTerminationCharacter), 0);
    ErrorType error = ErrorType::Failure;
    Count retries = 0;

    do {
        commandBuffer.assign(atCommand + _commandLineTerminationCharacter);
        error = _ic->txBlocking(commandBuffer, timeout);
#if CELLULAR_MODULE_DEBUGGING_ON
        PLT_LOGI(TAG, "Sending...");
        PLT_LOG_BUFFER_HEXDUMP(TAG, commandBuffer.data(), commandBuffer.size(), LogType::Info);
#endif
        if (ErrorType::Success == error) {
            return error;
        }
        else {
            retries++;
        }
    } while (retries < maxRetries);

    if (retries >= maxRetries) {
        error = ErrorType::Timeout;
    }
    else {
        error = ErrorType::Success;
    }

    return error;
}

ErrorType Cellular::receiveCommand(std::string &responseBuffer, const Milliseconds timeout, const Count maxRetries, const std::string expectedResponse) {
    ErrorType error = ErrorType::Failure;
    Count retries = 0;

    responseBuffer.clear();
    responseBuffer.resize(0);
    std::string bytesRead(16, 0);
    bool endOfResponse = false;
    const bool specialCaseQiSend = 0 == expectedResponse.compare(">");

    do {
        bytesRead.clear();
        bytesRead.resize(bytesRead.capacity());

        error = _ic->rxBlocking(bytesRead, timeout);
#if CELLULAR_MODULE_DEBUGGING_ON
        PLT_LOGI(TAG, "Receiving...");
#endif
        if (ErrorType::Success != error) {
            retries++;
        }
        else {
#if CELLULAR_MODULE_DEBUGGING_ON
            PLT_LOGI(TAG, "Partial response...");
            PLT_LOG_BUFFER_HEXDUMP(TAG, bytesRead.data(), bytesRead.size(), LogType::Info);
#endif
            responseBuffer.append(bytesRead);
        }

        //Special case - There is no response formatting character for AT+QISEND
        if (specialCaseQiSend) {
            endOfResponse = std::string::npos != responseBuffer.find(">");
        }
        else {
            endOfResponse = responseBuffer.back() == _responseFormattingCharacter || specialCaseQiSend;
        }

    } while (retries < maxRetries && !endOfResponse);

#if CELLULAR_MODULE_DEBUGGING_ON
    PLT_LOGI(TAG, "Full response...");
    PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Info);
#endif

    if (retries >= maxRetries) {
        error = ErrorType::Timeout;
    }
    else if (!specialCaseQiSend && responseBuffer.back() != _responseFormattingCharacter) {
        //You can try increasing the buffer size in case it wasn't big enough to read all the data until the response formatting character was found.
        PLT_LOGE(TAG, "Response formatting character not found.");
        error = ErrorType::Failure;
    }
    else {
        error = ErrorType::Success;
    }

    if (!expectedResponse.empty() && std::string::npos != responseBuffer.find(expectedResponse)) {
        error = ErrorType::Success;
    }
    else {
        error = ErrorType::Failure;
    }

    return error;
}

ErrorType Cellular::getManufacturerName(std::string &mfgName) {
    ErrorType error;
    error = sendCommand("ATI", 1000, 10);
    if (ErrorType::Success == error) {
        error = receiveCommand(mfgName, 1000, 10, "OK");
    }

    return error;
}

ErrorType Cellular::simCardIsInserted() {
    std::string commandBuffer(64, 0);
    constexpr int numRetries = 10;
    constexpr Milliseconds timeout = 1000;
    ErrorType error = ErrorType::Failure;

    error = sendCommand("AT+QSIMSTAT?", timeout, numRetries);
    if (ErrorType::Success != error) {
        return error;
    }

    std::string &responseBuffer = commandBuffer;
    error = receiveCommand(responseBuffer, timeout, numRetries, "OK");
    if (ErrorType::Success != error) {
        return error;
    }

    responseBuffer.erase(std::remove(responseBuffer.begin(), responseBuffer.end(), '\r'), responseBuffer.end());
    responseBuffer.erase(std::remove(responseBuffer.begin(), responseBuffer.end(), '\n'), responseBuffer.end());
    responseBuffer.replace(responseBuffer.find("OK"), 2, "");
    responseBuffer.replace(responseBuffer.find("+QSIMSTAT:"), 2, "");

    const char *delim = ",";
    char *save_ptr = responseBuffer.data();
    char *token = strtok_r(save_ptr, delim, &save_ptr);

    bool cardDetectedUrcEnabled = strtoul(token, nullptr, 10);
    token = strtok_r(save_ptr, delim, &save_ptr);
    bool isInserted = strtoul(token, nullptr, 10);

    if (isInserted) {
        error = ErrorType::Success;
    }
    else {
        error = ErrorType::Failure;
    }

    return error;
}

ErrorType Cellular::pdpContextIsActive(const PdpContext context) {
    std::string commandBuffer(128, 0);
    const std::string expectedResponse = std::string("+QIACT: ").append(std::to_string(context));
    constexpr int numRetries = 10;
    constexpr Milliseconds timeout = 1000;
    ErrorType error = ErrorType::Failure;

    error = sendCommand("AT+QIACT?", timeout, numRetries);
    if (ErrorType::Success != error) {
        return error;
    }

    std::string &responseBuffer = commandBuffer;
    error = receiveCommand(responseBuffer, timeout, numRetries, expectedResponse);
    if (ErrorType::Success != error) {
        return error;
    }

    return ErrorType::Success;
}

ErrorType Cellular::dataIsAvailable(const Socket socket) {
    constexpr Count maxRetries = 10;
    constexpr Milliseconds timeout = 1000;

    std::string commandQueryAvailableBytes("AT+QIRD=");
    commandQueryAvailableBytes.append(std::to_string(socket));
    commandQueryAvailableBytes.append(",");
    commandQueryAvailableBytes.append(std::to_string(0));

    std::string receiveBuffer(64, 0);

    ErrorType error = sendCommand(commandQueryAvailableBytes, timeout, maxRetries);
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "AT command error:");
        PLT_LOG_BUFFER_HEXDUMP(TAG, commandQueryAvailableBytes.data(), commandQueryAvailableBytes.size(), LogType::Warning);
        return error;
    }

    error = receiveCommand(receiveBuffer, timeout, maxRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "Receive error:");
        PLT_LOG_BUFFER_HEXDUMP(TAG, receiveBuffer.data(), receiveBuffer.size(), LogType::Warning);
        return error;
    }

    const char *delim = ",";
    char *save_ptr = receiveBuffer.data();
    char *token = strtok_r(save_ptr, delim, &save_ptr);
    token = strtok_r(save_ptr, delim, &save_ptr);
    token = strtok_r(save_ptr, delim, &save_ptr);

    if (nullptr == token) {
        return ErrorType::NoData;
    }

    const Bytes unread = strtoul(token, nullptr, 10);

    if (unread > 0) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::NoData;
    }
}

ErrorType Cellular::activatePdpContext(const PdpContext context, const Socket socket, const ContextType contextType, const std::string &accessPointName) {
    constexpr Milliseconds timeout = 1000;
    constexpr Count numRetries = 10;
    std::string responseBuffer(128, 0);

    std::string configurePdpContextCommand("AT+QICSGP=");
    configurePdpContextCommand.append(std::to_string(context));
    configurePdpContextCommand.append(",").append(std::to_string(static_cast<uint8_t>(contextType)));
    configurePdpContextCommand.append(",").append("\"").append(accessPointName).append("\"");

    std::string activatePdpContextCommand("AT+QIACT=");
    activatePdpContextCommand.append(std::to_string(context));

    ErrorType error = sendCommand(configurePdpContextCommand, timeout, numRetries);
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "Error sending command:");
        PLT_LOG_BUFFER_HEXDUMP(TAG, configurePdpContextCommand.data(), configurePdpContextCommand.size(), LogType::Warning);
        return error;
    }
    error = receiveCommand(responseBuffer, timeout, numRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "Error receiving command");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    error = sendCommand(activatePdpContextCommand, timeout, numRetries);
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "Error sending command:");
        PLT_LOG_BUFFER_HEXDUMP(TAG, activatePdpContextCommand.data(), activatePdpContextCommand.size(), LogType::Warning);
        return error;
    }
    error = receiveCommand(responseBuffer, timeout, numRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "Error receiving command");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    if (ErrorType::Success != error) {
        sendCommand("AT+QIGETERROR", 1000, 10);
        receiveCommand(responseBuffer, 1000, 10, "OK");
        PLT_LOGW(TAG, "Error activating PDP context");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    return ErrorType::Success;
}

ErrorType Cellular::deactivatePdpContext(const PdpContext context) {
    constexpr Milliseconds timeout = 1000;
    constexpr Count numRetries = 10;
    ErrorType error = ErrorType::Failure;

    std::string deavtivatePdpContextCommand("AT+QIDEACT=");
    deavtivatePdpContextCommand.append(std::to_string(context));
    error = sendCommand(deavtivatePdpContextCommand, timeout, numRetries);
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "Error sending command:");
        PLT_LOG_BUFFER_HEXDUMP(TAG, deavtivatePdpContextCommand.data(), deavtivatePdpContextCommand.size(), LogType::Warning);
        return error;
    }

    std::string &responseBuffer = deavtivatePdpContextCommand;
    error = receiveCommand(responseBuffer, timeout, numRetries, "OK");
    if (ErrorType::Success != error) {
        PLT_LOGW(TAG, "Error receiving command");
        PLT_LOG_BUFFER_HEXDUMP(TAG, responseBuffer.data(), responseBuffer.size(), LogType::Warning);
        return error;
    }

    return error;
}

ErrorType Cellular::echoMode(const bool enable) {
    ErrorType error = ErrorType::Failure;
    std::string commandBuffer(64, 0);
    commandBuffer.assign("ATE");
    commandBuffer.append(enable ? "1" : "0");
    error = sendCommand(commandBuffer, 1000, 10);

    std::string &responseBuffer = commandBuffer;

    if (ErrorType::Success == error) {
        error = receiveCommand(responseBuffer, 1000, 10, "OK");
    }

    return error;
}

ErrorType Cellular::terminatingCharacter(char &terminatingCharacter) {
    std::string responseBuffer(16, 0);
    ErrorType error;
    error = sendCommand("ATS3?", 1000, 10);
    if (ErrorType::Success == error) {
        error = receiveCommand(responseBuffer, 1000, 10, "OK");
    }
    else {
        PLT_LOGW(TAG, "Error sending to <command:ATS3?>");
    }

    responseBuffer.erase(std::remove(responseBuffer.begin(), responseBuffer.end(), _commandLineTerminationCharacter), responseBuffer.end());
    responseBuffer.erase(std::remove(responseBuffer.begin(), responseBuffer.end(), _responseFormattingCharacter), responseBuffer.end());
    responseBuffer.replace(responseBuffer.find("OK"), sizeof("OK") - 1, "");

    terminatingCharacter = strtoul(responseBuffer.c_str(), nullptr, 10);

    return error;
}

ErrorType Cellular::responseFormattingCharacter(char &responseFormattingCharacter) {
    std::string responseBuffer(16, 0);
    ErrorType error;
    error = sendCommand("ATS4?", 500, 10);
    if (ErrorType::Success == error) {
        error = receiveCommand(responseBuffer, 500, 10, "OK");
    }
    else {
        PLT_LOGW(TAG, "Error sending to <command:ATS4?>");
    }

    responseBuffer.erase(std::remove(responseBuffer.begin(), responseBuffer.end(), _commandLineTerminationCharacter), responseBuffer.end());
    responseBuffer.erase(std::remove(responseBuffer.begin(), responseBuffer.end(), _responseFormattingCharacter), responseBuffer.end());
    responseBuffer.replace(responseBuffer.find("OK"), sizeof("OK") - 1, "");

    responseFormattingCharacter = strtoul(responseBuffer.c_str(), nullptr, 10);

    return error;
}
