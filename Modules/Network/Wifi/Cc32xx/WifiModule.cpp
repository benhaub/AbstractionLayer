//AbstractionLayer
#include "WifiModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
#include "SpiModule.hpp"
//TI
#include "ti/drivers/net/wifi/slnetifwifi.h"

ErrorType Wifi::init() {
    Spi spi;
    Id thread;
    ErrorType error = spi.init();
    if (ErrorType::Success == error) {
        constexpr Bytes kilobyte = 1024;
        //SIMPLELINK_THREAD_NAME is defined at compile time in cc32xx.cmake. Both Wifi and storage must create this thread in order to access the
        //API for Wifi and storage and may do so independently of each other. The Operating System must be check for this presence of this thread
        //before attempting to start it. Checking the return value of sl_start() is not sufficient since the wifi radio can be turned on or off at
        //any time.
        constexpr std::array<char, OperatingSystemTypes::MaxThreadNameLength> simplelinkThreadName = {SIMPLELINK_THREAD_NAME};
        if (ErrorType::NoData == OperatingSystem::Instance().threadId(simplelinkThreadName, thread)) {
            
            error = OperatingSystem::Instance().createThread(OperatingSystemTypes::Priority::High,
                                                    simplelinkThreadName,
                                                    nullptr,
                                                    2*kilobyte,
                                                    sl_Task,
                                                    thread);
            
            assert(ErrorType::Success == error);
        }

        if (ErrorType::Success == (error = radioOn())) {
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

            //Reconfigure
            configure(_params);

            //Turn off and on for changes to take affect.
            if (ErrorType::Success == (error = radioOff())) {
                error = radioOn();
            }
        }
    }

    return error;
}

ErrorType Wifi::networkUp() {
    assert(WifiTypes::Mode::Unknown != _params.mode);
    constexpr Seconds timeout = 60*5;
    ErrorType error = ErrorType::NotSupported;

    const SlWlanSecParams_t securityParameters = {
        .Type = toCc32xxSecurityType(_params.authMode, error),
        .Key = reinterpret_cast<_i8 *>(_params.stationPassword.data()),
        .KeyLen = static_cast<_u8>(_params.stationPassword.length())
    };

    if (_params.mode == WifiTypes::Mode::Station || _params.mode == WifiTypes::Mode::AccessPointAndStation) {
        const _i16 result = sl_WlanConnect(reinterpret_cast<_i8 *>(_params.stationSsid.data()), static_cast<_u8>(_params.stationSsid.length()), 0, &securityParameters, nullptr);

        if (0 == result) {
            PLT_LOGI(WifiAbstraction::TAG, "Connected <SSID:%s>", _params.stationSsid.c_str());

            sl_WlanProfileDel(0);

            constexpr SlWlanSecParamsExt_t * notUsingEnterpriseSecurity = nullptr;
            constexpr _u8 *notUsingBssid = nullptr;
            constexpr _u32 maxProfilePriority = 15;
            constexpr _u32 optionsAreNotSupported = 0;

            _i16 result = sl_WlanProfileAdd(reinterpret_cast<_i8 *>(_params.stationSsid.data()),
                                    _params.stationSsid.length(),
                                    notUsingBssid,
                                    &securityParameters,
                                    notUsingEnterpriseSecurity,
                                    maxProfilePriority,
                                    optionsAreNotSupported);

            if (result < 0) {
                PLT_LOGW(WifiAbstraction::TAG, "Failed to add profile <SSID:%s, error:%u>", _params.stationSsid.c_str(), result);
            }
            else {
                _status.isProvisioned = true;
            }

            error = fromPlatformError(result);
        }
        else {
            PLT_LOGW(WifiAbstraction::TAG, "Failed to connect to station <SSID:%s, error:%d>", _params.stationSsid.c_str(), result);

            NetworkAbstraction::_status.isUp = false;
            return fromPlatformError(sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_START_MODE_AP, ROLE_STA, timeout, nullptr, 0));
        }
    }

    NetworkAbstraction::_status.isUp = ErrorType::Success == error;

    return error;    
}

ErrorType Wifi::networkDown() {
    ErrorType error = ErrorType::Failure;

    //Don't check for errors. We may not have connected to an access point.
    sl_WlanDisconnect();

    error = fromPlatformError(sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_STOP, 0xFF, 0, nullptr, 0));
    if (error == ErrorType::Success) {
        NetworkAbstraction::_status.isUp = false;
        _status.isProvisioned = false;
    }

    return error;
}

ErrorType Wifi::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;

    if (version != IpTypes::Version::IPv4) {
        error = ErrorType::NotSupported;
    }
    else {
        const _i16 protocolFamily = toSimpleLinkProtocolFamily(version, error);
        if (ErrorType::Success == error) {
            constexpr void * notUsingInterfaceContext = nullptr;
            uint16_t destinationIpListSize = 1;
            uint32_t destinationIp;
            SlNetSock_AddrIn_t localAddress;

            error = fromPlatformError(SlNetIfWifi_getHostByName(notUsingInterfaceContext, const_cast<char *>(hostname.data()), hostname.length(), &destinationIp, &destinationIpListSize, protocolFamily));
            if (ErrorType::Success == error) {
                const _i16 domain = toSimplelinkDomain(version, error);

                if (ErrorType::Success == error) {
                    const _i16 slProtocol = toSimpleLinkProtocol(protocol, error);

                    if (ErrorType::Success == error) {
                        const _i16 type = toSimpleLinkType(protocol, error);

                        if (ErrorType::Success == error) {
                            error = ErrorType::Failure;
                            sock = SlNetIfWifi_socket(nullptr, domain, type, slProtocol, nullptr);

                            if (sock >= 0) {
                                localAddress.sin_family = protocolFamily;
                                localAddress.sin_addr.s_addr = SlNetUtil_htonl(destinationIp);
                                localAddress.sin_port = SlNetUtil_htons(port);
                                constexpr uint8_t notUsingFlags = 0;
                                constexpr void * notUsingSocketContext = nullptr;

                                const int32_t connectReturn = SlNetIfWifi_connect(sock, notUsingSocketContext, reinterpret_cast<SlNetSock_Addr_t *>(&localAddress), sizeof(localAddress), notUsingFlags);
                                if (connectReturn >= 0) {
                                    error = ErrorType::Success;
                                }
                                else {
                                    error = ErrorType::Failure;
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

ErrorType Wifi::disconnect(const Socket &socket) {
    ErrorType error = ErrorType::Success;

    if (-1 != socket) {
        error = fromPlatformError(SlNetIfWifi_close(socket, nullptr));
    }

    return error;
}

ErrorType Wifi::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::closeConnection(const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::transmit(std::string_view frame, const Socket socket, const Milliseconds timeout) {
    Bytes sent = 0;
    Bytes remaining = frame.size();

    assert(remaining > 0);

    while (remaining > 0) {
        ssize_t bytesWritten = SlNetIfWifi_send(socket, nullptr, &frame.at(sent), remaining, 0);
        if (bytesWritten < 0) {
            return fromPlatformError(errno);
        }

        sent += bytesWritten;
        remaining -= bytesWritten;
    }

    return ErrorType::Success;
}

ErrorType Wifi::receive(char *frameBuffer, const size_t bufferSize, const Socket socket, Bytes &read, const Milliseconds timeout) {
    ErrorType error = ErrorType::Timeout;
    read = 0;

    Microseconds tvUsec = timeout * 1000;
    SlNetSock_Timeval_t timeval;
#pragma GCC diagnostic push
//Comparing integers of different signedness is the entire point of this code.
#pragma GCC diagnostic ignored "-Wsign-compare"
    if (tvUsec > std::numeric_limits<decltype(timeval.tv_usec)>::max()) {
        PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
        tvUsec = std::numeric_limits<decltype(timeval.tv_usec)>::max();
    }
#pragma GCC diagnostic pop
    if (timeout >= 1000) {
        timeval.tv_sec = timeout / 1000;
        timeval.tv_usec = 0;
    }
    else {
        timeval.tv_sec = 0;
        timeval.tv_usec = static_cast<decltype(timeval.tv_usec)>(tvUsec);
    }

    SlNetSock_SdSet_t readSds;
    SlNetSock_sdsClrAll(&readSds);
    SlNetSock_sdsSet(socket, &readSds);

    const int32_t ret = SlNetIfWifi_select(nullptr, socket+1, &readSds, nullptr, nullptr, &timeval);
    if (ret < 0) {
        return fromPlatformError(errno);
    }

    if (SlNetSock_sdsIsSet(socket, &readSds)) {
        ssize_t bytesReceived = 0;

        if ((bytesReceived = SlNetIfWifi_recv(socket, nullptr, frameBuffer, bufferSize, 0)) < 0) {
            error = fromPlatformError(errno);
        }
        else if (0 == bytesReceived) {
            //recv returns 0 if the connection is closed.
            error = ErrorType::PrerequisitesNotMet;
        }
        else {
            read = bytesReceived;
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType Wifi::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    uint16_t macAddressLen = macAddress.size();

    return fromPlatformError(sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, NULL, &macAddressLen, reinterpret_cast<unsigned char *>(macAddress.data())));
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::radioOn() {
    //Will fail if you have not initialized something for the network processor to send data on (SPI or UART not initialized)
    const _i16 mode = sl_Start(NULL, NULL, NULL);

    if (fromCc32xxRole(mode) != WifiTypes::Mode::Unknown) {
        return ErrorType::Success;
    }
    else {
        return fromPlatformError(mode);
    }
}

ErrorType Wifi::radioOff() {
    signed short result = sl_Stop(0xFFFF);
    if (0 == result) {
        NetworkAbstraction::_status.isUp = false;
    }

    return fromPlatformError(result);
}

ErrorType Wifi::setSsid(WifiTypes::Mode mode, const StaticString::Data<WifiTypes::MaxSsidLength> &ssid) {
    ErrorType error = ErrorType::Success;

    _u16 result = 0;
    if (mode == WifiTypes::Mode::AccessPoint || mode == WifiTypes::Mode::AccessPointAndStation) {
        result = sl_WlanSet(SL_WLAN_CFG_AP_ID, SL_WLAN_AP_OPT_SSID, ssid.size(), reinterpret_cast<const unsigned char *>(ssid.c_str()));

        if (0 == result) {
            _params.accessPointSsid.assign(ssid);
        }

        error = fromPlatformError(result);
    }
    else if (mode == WifiTypes::Mode::Station || mode == WifiTypes::Mode::AccessPointAndStation) {
        _params.stationSsid.assign(ssid);
    }

    return error;
}

ErrorType Wifi::setPassword(WifiTypes::Mode mode, const StaticString::Data<WifiTypes::MaxPasswordLength> &password) {
    //Passwords have length requirements based on the authorization mode
    assert(WifiTypes::AuthMode::Unknown != _params.authMode);
    ErrorType error = ErrorType::Success;

    if (WifiTypes::AuthMode::Wpa == _params.authMode || WifiTypes::AuthMode::WpaWpa2 == _params.authMode) {
        if (password.size() < 8) {
            return ErrorType::InvalidParameter;
        }
    }

    if (password.size() > 64) {
        return ErrorType::InvalidParameter;
    }

    if (mode == WifiTypes::Mode::AccessPointAndStation) {
        return ErrorType::InvalidParameter;
    }

    _u16 result = 0;
    if (mode == WifiTypes::Mode::AccessPoint) {
        result = sl_WlanSet(SL_WLAN_CFG_AP_ID, SL_WLAN_AP_OPT_PASSWORD, password.size(), reinterpret_cast<const unsigned char *>(password.c_str()));

        if (0 == result) {
            _params.accessPointPassword = password;
        }

        error = fromPlatformError(result);
    }
    else if (mode == WifiTypes::Mode::Station) {
        _params.stationPassword = password;
    }

    return error;
}

ErrorType Wifi::setMode(WifiTypes::Mode mode) {
    ErrorType error = ErrorType::Failure;

    //TI uses mode and role interchangeably and I wish they wouldn't.
    _u8 cc32xxMode = toCc32xxRole(mode, error);
    if (ErrorType::Success != error) {
        return error;
    }
    _i16 result = sl_WlanSetMode(cc32xxMode);
    if (0 == result) {
        _params.mode = mode;
    }

    return fromPlatformError(result);
}

ErrorType Wifi::setAuthMode(WifiTypes::AuthMode authMode) {
    ErrorType error = ErrorType::Failure;

    _u8 securityType = toCc32xxSecurityType(authMode, error);
    if (ErrorType::Success != error) {
        return error;
    }

    _u16 result = sl_WlanSet(SL_WLAN_CFG_AP_ID, SL_WLAN_AP_OPT_SECURITY_TYPE, 1, (_u8 *)&securityType);
    if (0 == result) {
        _params.authMode = authMode;
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

//Handler is from the provisioning CCS example project.
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {
    switch(pWlanEvent->Id) {
        case SL_WLAN_EVENT_CONNECT:
            PLT_LOGI(WifiAbstraction::TAG, "STA connected to AP. <BSSID:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x>,",
                pWlanEvent->Data.Connect.Bssid[0],
                pWlanEvent->Data.Connect.Bssid[1],
                pWlanEvent->Data.Connect.Bssid[2],
                pWlanEvent->Data.Connect.Bssid[3],
                pWlanEvent->Data.Connect.Bssid[4],
                pWlanEvent->Data.Connect.Bssid[5]);

            pWlanEvent->Data.Connect.SsidName[pWlanEvent->Data.Connect.SsidLen] = '\0';
            PLT_LOGI(WifiAbstraction::TAG, "SSID:%s", pWlanEvent->Data.Connect.SsidName);
            break;

        case SL_WLAN_EVENT_DISCONNECT:
            PLT_LOGI(WifiAbstraction::TAG, "STA disconnected from AP. <Reason Code:%d>",
                        pWlanEvent->Data.Disconnect.ReasonCode);
            break;

        case SL_WLAN_EVENT_STA_ADDED:
            PLT_LOGI(WifiAbstraction::TAG, 
                "New STA Added. <MAC Address:%.2x:%.2x:%.2x:%.2x:%.2x>",
                pWlanEvent->Data.STAAdded.Mac[0],
                pWlanEvent->Data.STAAdded.Mac[1],
                pWlanEvent->Data.STAAdded.Mac[2],
                pWlanEvent->Data.STAAdded.Mac[3],
                pWlanEvent->Data.STAAdded.Mac[4],
                pWlanEvent->Data.STAAdded.Mac[5]);
            break;

        case SL_WLAN_EVENT_STA_REMOVED:
            PLT_LOGI(WifiAbstraction::TAG, 
                "STA Removed <MAC Address:%.2x:%.2x:%.2x:%.2x:%.2x>",
                pWlanEvent->Data.STAAdded.Mac[0],
                pWlanEvent->Data.STAAdded.Mac[1],
                pWlanEvent->Data.STAAdded.Mac[2],
                pWlanEvent->Data.STAAdded.Mac[3],
                pWlanEvent->Data.STAAdded.Mac[4],
                pWlanEvent->Data.STAAdded.Mac[5]);
            break;

        case SL_WLAN_EVENT_PROVISIONING_PROFILE_ADDED:
            PLT_LOGI(WifiAbstraction::TAG, "Profile Added <SSID:%s>",
                    pWlanEvent->Data.ProvisioningProfileAdded.Ssid);
            if(pWlanEvent->Data.ProvisioningProfileAdded.ReservedLen > 0) {
                PLT_LOGI(WifiAbstraction::TAG, "Profile Added <PrivateToken:%s>",
                        pWlanEvent->Data.ProvisioningProfileAdded.Reserved);
            }
            break;

        case SL_WLAN_EVENT_PROVISIONING_STATUS: {
            switch(pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus) {
                case SL_WLAN_PROVISIONING_GENERAL_ERROR:
                case SL_WLAN_PROVISIONING_ERROR_ABORT:
                case SL_WLAN_PROVISIONING_ERROR_ABORT_INVALID_PARAM:
                case SL_WLAN_PROVISIONING_ERROR_ABORT_HTTP_SERVER_DISABLED:
                case SL_WLAN_PROVISIONING_ERROR_ABORT_PROFILE_LIST_FULL:
                case SL_WLAN_PROVISIONING_ERROR_ABORT_PROVISIONING_ALREADY_STARTED:
                    PLT_LOGI(WifiAbstraction::TAG, "Provisioning Error <status=%d>",
                                pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus);
                    break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_FAIL_NETWORK_NOT_FOUND:
                    PLT_LOGW(WifiAbstraction::TAG, "Profile confirmation failed. network not found");
                    break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_FAIL_CONNECTION_FAILED:
                    PLT_LOGW(WifiAbstraction::TAG, "Profile confirmation failed. Connection failed");
                    break;

                case
                    SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_CONNECTION_SUCCESS_IP_NOT_ACQUIRED:
                    PLT_LOGW(WifiAbstraction::TAG, "Profile confirmation failed. IP address not acquired");
                    break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_SUCCESS_FEEDBACK_FAILED:
                    PLT_LOGW(WifiAbstraction::TAG, "Profile Confirmation failed (Connection Success, feedback to Smartphone app failed).");
                    break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_SUCCESS:
                    PLT_LOGI(WifiAbstraction::TAG, "Profile Confirmation Success!");
                    break;

                case SL_WLAN_PROVISIONING_AUTO_STARTED:
                    PLT_LOGI(WifiAbstraction::TAG, "Auto-Provisioning Started");
                    break;

                case SL_WLAN_PROVISIONING_STOPPED:
                    PLT_LOGI(WifiAbstraction::TAG, "Provisioning stopped <role:%d>", pWlanEvent->Data.ProvisioningStatus.Role);
                    if(ROLE_STA == pWlanEvent->Data.ProvisioningStatus.Role) {
                        PLT_LOGI(WifiAbstraction::TAG, "WLAN <Status:%s>",pWlanEvent->Data.ProvisioningStatus.WlanStatus);

                        if(SL_WLAN_STATUS_CONNECTED == pWlanEvent->Data.ProvisioningStatus.WlanStatus) {
                            PLT_LOGI(WifiAbstraction::TAG, "Connected <SSID:%s>", pWlanEvent->Data.ProvisioningStatus.Ssid);
                        }
                    }
                    break;

                case SL_WLAN_PROVISIONING_SMART_CONFIG_SYNCED:
                    PLT_LOGI(WifiAbstraction::TAG, "Smart Config Synced!");
                    break;

                case SL_WLAN_PROVISIONING_SMART_CONFIG_SYNC_TIMEOUT:
                    PLT_LOGW(WifiAbstraction::TAG, "Smart Config Sync Timeout!");
                    break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_WLAN_CONNECT:
                    PLT_LOGI(WifiAbstraction::TAG, "Profile confirmation: WLAN Connected!");
                    break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_IP_ACQUIRED:
                    PLT_LOGI(WifiAbstraction::TAG, "Profile confirmation: IP Acquired!");
                    break;

                case SL_WLAN_PROVISIONING_EXTERNAL_CONFIGURATION_READY:
                    PLT_LOGI(WifiAbstraction::TAG, "External configuration is ready! ");
                    break;

                default:
                    PLT_LOGW(WifiAbstraction::TAG, "Unknown Provisioning <Status:%d>",
                        pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus);
                    break;
            }

            break;
        }
        default:
            PLT_LOGE(WifiAbstraction::TAG, "WlanEventHandler has received %d !!!!",
            pWlanEvent->Id);
            break;
    }
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
