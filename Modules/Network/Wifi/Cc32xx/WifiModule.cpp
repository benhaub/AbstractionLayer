//AbstractionLayer
#include "WifiModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
#include "SpiModule.hpp"

Wifi::Wifi() : WifiAbstraction() {
    _status.isUp = false;

    Spi spi;
    Id thread;
    ErrorType error = spi.init();
    if (ErrorType::Success != error) {
        return;
    }

    constexpr Bytes kilobyte = 1024;
    error = OperatingSystem::Instance().createThread(OperatingSystemConfig::Priority::High,
                                            std::string("wifiNetworkThread"),
                                            nullptr,
                                            2*kilobyte,
                                            sl_Task,
                                            thread);

    radioOn();

    //Enable DHCP client
    sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE, SL_NETCFG_ADDR_DHCP, 0, 0);

    //Disable IPV6
    uint16_t ifBitmap = 0;
    sl_NetCfgSet(SL_NETCFG_IF, SL_NETCFG_IF_STATE, sizeof(ifBitmap), (uint8_t *)&ifBitmap);

    //Disable scan
    uint8_t ucConfigOpt = SL_WLAN_SCAN_POLICY(0, 0);
    sl_WlanPolicySet(SL_WLAN_POLICY_SCAN, ucConfigOpt, NULL, 0);

    //Set Tx power level for station mode. Number between 0-15, as dB offset from max power - 0 will set max power
    uint8_t ucPower = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, SL_WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, sizeof(ucPower), (uint8_t *)&ucPower);

    //Set PM policy to normal
    sl_WlanPolicySet(SL_WLAN_POLICY_PM, SL_WLAN_NORMAL_POLICY, NULL, 0);

    //Unregister mDNS services
    sl_NetAppMDNSUnRegisterService(0, 0, 0);

    //Remove all 64 filters (8*8)
    SlWlanRxFilterOperationCommandBuff_t rxFilterIdMask;
    memset(rxFilterIdMask.FilterBitmap, 0xFF, 8);
    sl_WlanSet(SL_WLAN_RX_FILTERS_ID, SL_WLAN_RX_FILTER_REMOVE, sizeof(rxFilterIdMask), (uint8_t *)&rxFilterIdMask);
}

ErrorType Wifi::init() {
    ErrorType error = ErrorType::Success;

    Id thread;
    const bool wifiNetworkThreadHasNotBeenCreated = ErrorType::NoData == OperatingSystem::Instance().threadId("wifiNetworkThread", thread);
    if (wifiNetworkThreadHasNotBeenCreated) {
        return ErrorType::PrerequisitesNotMet;
    }

    //Turn off and on for changes to take affect.
    error = radioOff();
    if (ErrorType::Success != error) {
        return error;
    }

    error = radioOn();
    if (ErrorType::Success != error) {
        return error;
    }

    return networkUp();

    return error;
}

ErrorType Wifi::networkUp() {
    assert(WifiConfig::Mode::Unknown != _mode);
    Seconds timeout = 1200;

    if (_mode == WifiConfig::Mode::AccessPointAndStation) {
        return ErrorType::NotSupported;
    }

    constexpr uint32_t flags = 0;
    ErrorType error;
    uint8_t role = toCc32xxRole(_mode, error);
    if (ErrorType::Success != error) {
        return error;
    }

    constexpr char *smartConfigKey = nullptr;
    signed short result;
    result = sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_START_MODE_AP, role, timeout, smartConfigKey, flags);
    if (0 == result) {
        _status.isUp = true;
    }

    return fromPlatformError(result);
}

ErrorType Wifi::networkDown() {
    signed short result;
    result = sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_STOP, 0xFF, 0, NULL, 0x0);
    if (0 == result) {
        _status.isUp = false;
    }

    return fromPlatformError(result);
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

    return fromPlatformError(sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, NULL, &macAddressLen, reinterpret_cast<unsigned char *>(macAddress.data())));
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::mainLoop() {
    return runNextEvent();
}

ErrorType Wifi::radioOn() {
    //Will fail if you have not initialized something for the network processor to send data on (SPI or UART not initialized)
    const _i16 mode = sl_Start(NULL, NULL, NULL);
    _mode = fromCc32xxRole(mode);

    if (_mode != WifiConfig::Mode::Unknown) {
        return ErrorType::Success;
    }
    else {
        return fromPlatformError(mode);
    }
}

ErrorType Wifi::radioOff() {
    signed short result = sl_Stop(0xFFFF);
    if (0 == result) {
        _status.isUp = false;
    }

    return fromPlatformError(result);
}

ErrorType Wifi::setSsid(WifiConfig::Mode mode, std::string ssid) {
    if (mode != WifiConfig::Mode::AccessPoint) {
        return ErrorType::InvalidParameter;
    }

    signed short result = sl_WlanSet(SL_WLAN_CFG_AP_ID, SL_WLAN_AP_OPT_SSID, ssid.size(), reinterpret_cast<const unsigned char *>(ssid.c_str()));
    if (0 == result) {
        _ssid = ssid;
    }

    return fromPlatformError(result);
}

ErrorType Wifi::setPassword(WifiConfig::Mode mode, std::string password) {
    //Passwords have length requirements based on the authorization mode
    assert(WifiConfig::AuthMode::Unknown != _authMode);

    if (WifiConfig::Mode::AccessPoint != mode) {
        return ErrorType::InvalidParameter;
    }

    if (WifiConfig::AuthMode::Wpa == _authMode ||WifiConfig::AuthMode::WpaWpa2 == _authMode) {
        if (password.size() < 8 || password.size() > 63) {
            return ErrorType::InvalidParameter;
        }
    }

    signed short result = sl_WlanSet(SL_WLAN_CFG_AP_ID, SL_WLAN_AP_OPT_PASSWORD, password.size(), reinterpret_cast<const unsigned char *>(password.c_str()));

    if (0 == result) {
        _password = password;
    }

    return fromPlatformError(result);
}

ErrorType Wifi::setMode(WifiConfig::Mode mode) {
    ErrorType error = ErrorType::Failure;

    //TI uses mode and role interchangeably and I wish they wouldn't.
    _u8 cc32xxMode = toCc32xxRole(mode, error);
    if (ErrorType::Success != error) {
        return error;
    }
    _i16 result = sl_WlanSetMode(cc32xxMode);
    if (0 == result) {
        _mode = mode;
    }

    return fromPlatformError(result);
}

ErrorType Wifi::setAuthMode(WifiConfig::AuthMode authMode) {
    ErrorType error = ErrorType::Failure;

    unsigned short securityType = toCc32xxSecurityType(authMode, error);
    if (ErrorType::Success != error) {
        return error;
    }

    signed short result = sl_WlanSet(SL_WLAN_CFG_AP_ID, SL_WLAN_AP_OPT_SECURITY_TYPE, 1, (_u8 *)&securityType);
    if (0 == result) {
        _authMode = authMode;
    }

    return fromPlatformError(result);
}

#ifdef __cplusplus
extern "C" {

void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent) {
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