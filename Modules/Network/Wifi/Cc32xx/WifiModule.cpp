//AbstractionLayer
#include "WifiModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"

ErrorType Wifi::init() {
    constexpr Kilobytes stackSize = 1024;
    Id thread;
    ErrorType error = OperatingSystem::Instance().createThread(OperatingSystemConfig::Priority::High,
                                             std::string("wifiThread"),
                                             nullptr,
                                             stackSize,
                                             sl_Task,
                                             thread);

    if (ErrorType::Success != error) {
        return error;
    }

    PLT_LOGI(TAG, "Turning on radio");
    error = radioOn();
    if (ErrorType::Success != error) {
        return error;
    }


    PLT_LOGI(TAG, "Bringing up network");
    return networkUp();
}

ErrorType Wifi::networkUp() {
    assert(WifiConfig::Mode::Unknown != _mode);
    _u8 provisioningCommand;
    Seconds timeout = 1200;

    if (_mode == WifiConfig::Mode::AccessPointAndStation) {
        return ErrorType::NotSupported;
    }

    if (WifiConfig::Mode::AccessPoint == _mode) {
        provisioningCommand = SL_WLAN_PROVISIONING_CMD_START_MODE_APSC;
    }
    else if (WifiConfig::Mode::Station == _mode) {
        provisioningCommand = SL_WLAN_PROVISIONING_CMD_START_MODE_SC;

    }
    else {
        return ErrorType::NotSupported;
    }

    constexpr uint32_t flags = 0;
    ErrorType error;
    uint8_t role = toCc32xxRole(_mode, error);
    if (ErrorType::Success != error) {
        return error;
    }

    constexpr char *smartConfigKey = nullptr;
    return toPlatformError(sl_WlanProvisioning(provisioningCommand, role, timeout, smartConfigKey, flags));
}

ErrorType Wifi::networkDown() {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType Wifi::txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Wifi::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType Wifi::rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Wifi::getMacAddress(std::string &macAddress) {
    assert(macAddress.size() >= 6);
    uint16_t macAddressLen = macAddress.size();

    return toPlatformError(sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, NULL, &macAddressLen, reinterpret_cast<unsigned char *>(macAddress.data())));
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::mainLoop() {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::radioOn() {
    //Will fail if you have not initialized something for the network processor to send data on (SPI or UART not initialized)
    const _i16 mode = sl_Start(NULL, NULL, NULL);
    _mode = fromCc32xxRole(mode);

    if (_mode != WifiConfig::Mode::Unknown) {
        return ErrorType::Success;
    }
    else {
        return toPlatformError(mode);
    }
}

ErrorType Wifi::radioOff() {
    return toPlatformError(sl_Stop(0xFFFF));
}

ErrorType Wifi::setSsid(WifiConfig::Mode mode, std::string ssid) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::setPassword(WifiConfig::Mode mode, std::string password) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::setMode(WifiConfig::Mode mode) {
    _mode = mode;
    return ErrorType::Success;
}

ErrorType Wifi::setAuthMode(WifiConfig::AuthMode authMode) {
    _authMode = authMode;
    return ErrorType::Success;
}

#ifdef __cplusplus
extern "C" {

void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent) {
    return;
}
void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t *buffer) {
    return;
}
void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t *pNetAppRequest, SlNetAppResponse_t *pNetAppResponse) {
    return;
}
void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t *pHttpEvent, SlNetAppHttpServerResponse_t * pHttpResponse) {
    return;
}
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {
    return;
}
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {
    return;
}
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {
    return;
}
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent) {
    return;
}
#endif

#ifdef __cplusplus
}
#endif
