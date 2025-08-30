//AbstractionLayer
#include "WifiModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Math.hpp"
//C++
#include <cstring>
//ESP
#include "esp_wifi_types.h"
#include "esp_mac.h"
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif
static void WifiEventHandler(void *eventHandleArg, esp_event_base_t event_base, int32_t event_id, void *event_data);
//Global so that the handler can access it. Otherwise would have to make it a public data member.
static EventGroupHandle_t wifiEventGroup;
//Access point started event.
constexpr unsigned int wifiApStartedBit = BIT0;
//Station started event.
constexpr unsigned int wifiStaStartedBit = BIT1;
//Wait for both to connect
constexpr unsigned int wifiApAndStaStartedBit = BIT2;
#ifdef __cplusplus
}
#endif

//Based off https://github.com/espressif/esp-idf/blob/c5865270b50529cd32353f588d8a917d89f3dba4/examples/wifi/softap_sta/main/softap_sta.c
//networkUp() is not called because it's functionality is implemented in the event handler.
ErrorType Wifi::init() {
    esp_err_t err;
    ErrorType error;

    if (WifiTypes::Mode::Unknown == mode() || WifiTypes::AuthMode::Unknown == authMode()) {
        return ErrorType::PrerequisitesNotMet;
    }

    wifiEventGroup = xEventGroupCreate();

    err = esp_netif_init();
    if (ESP_OK != err) {
        return fromPlatformError(err);
    }

    if (ESP_OK != (err = esp_event_loop_create_default())) {
        bool isCriticalErrror = !(err = ESP_ERR_INVALID_STATE);
        if (isCriticalErrror) {
            return fromPlatformError(err);
        }
    }

    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiEventHandler, this);
    if (ESP_OK != err) {
        return fromPlatformError(err);
    }
    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiEventHandler, this);
    if (ESP_OK != err) {
        return fromPlatformError(err);
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (ESP_OK != (err = esp_wifi_init(&cfg))) {
        return fromPlatformError(err);
    }

    esp_wifi_set_mode(toEspWifiMode(mode()));
    unsigned int eventBitsToWaitFor = 0;
    switch (mode()) {
        case WifiTypes::Mode::AccessPoint: {
            error = initAccessPoint();
            eventBitsToWaitFor |= wifiApStartedBit;
            break;
        }
        case WifiTypes::Mode::Station: {
            error = initStation();
            eventBitsToWaitFor |= wifiStaStartedBit;
            break;
        }
        case WifiTypes::Mode::AccessPointAndStation: {
            error = initStation();
            if (ErrorType::Success == error) {
                error = initAccessPoint();
            }
            eventBitsToWaitFor |= wifiApAndStaStartedBit;
            break;
        }
        default: {
            error = ErrorType::InvalidParameter;
            break;
        }
    }

    if (ErrorType::Success != error) {
        return error;
    }

    error = radioOn();
    if (ErrorType::Success != error) {
        return error;
    }

    constexpr Milliseconds timeout = 10000;
    PLT_LOGI(TAG, "Waiting at most %us for wifi to be ready", timeout / 1000);
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                          eventBitsToWaitFor,
                                          pdFALSE,
                                          pdFALSE,
                                          pdMS_TO_TICKS(timeout));

    if (bits & eventBitsToWaitFor) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::Timeout;
    }
}

ErrorType Wifi::initAccessPoint() {
    if (NULL == esp_netif_get_handle_from_ifkey("WIFI_AP_DEF")) {
        esp_netif_create_default_wifi_ap();
    }

    wifi_config_t wifiAccessPointConfig;
    memset(&wifiAccessPointConfig, 0, sizeof(wifi_config_t));

    memcpy(wifiAccessPointConfig.ap.ssid, accessPointSsid().data(), accessPointSsid().length());
    wifiAccessPointConfig.ap.ssid_len = accessPointSsid().length();
    wifiAccessPointConfig.ap.channel = 1;
    memcpy(wifiAccessPointConfig.ap.password, accessPointPassword().data(), accessPointPassword().length());
    wifiAccessPointConfig.ap.max_connection = 10;
    wifiAccessPointConfig.ap.authmode = toEspAuthMode(authMode());
    wifiAccessPointConfig.ap.pmf_cfg.required = false;

    esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &wifiAccessPointConfig);
    if (ESP_OK != err) {
        return fromPlatformError(err);
    }

    return ErrorType::Success;
}
ErrorType Wifi::initStation() {
    if (NULL == esp_netif_get_handle_from_ifkey("WIFI_STA_DEF")) {
        esp_netif_create_default_wifi_sta();
    }
    else {
        //Not a typo, stations connect to access points.
        PLT_LOGI(TAG, "Reconnecting to access point <ssid:%s, password:%s>", stationSsid().c_str(), stationPassword().c_str());
    }

    wifi_config_t wifiStationConfig;
    memset(&wifiStationConfig, 0, sizeof(wifi_config_t));

    memcpy(wifiStationConfig.sta.ssid, stationSsid().data(), stationSsid().length());
    memcpy(wifiStationConfig.sta.password, stationPassword().data(), stationPassword().length());
    wifiStationConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiStationConfig.sta.failure_retry_cnt = 5;
    //Minimum security that we will accept.
    wifiStationConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiStationConfig.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifiStationConfig);
    return fromPlatformError(err);
}

ErrorType Wifi::networkUp() {
    if (WifiTypes::Mode::AccessPointAndStation == mode()) {
        initAccessPoint();
        initStation();
    }
    else if (WifiTypes::Mode::AccessPoint == mode()) {
        initAccessPoint();
    }
    else if (WifiTypes::Mode::Station == mode()) {
        initStation();
    }

    esp_err_t err = esp_wifi_connect();
    if (ESP_OK == err) {
        NetworkAbstraction::_status.isUp = true;
    }
    else {
        PLT_LOGW(TAG, "Failed to bring up wifi network <Error:%s>", esp_err_to_name(err));
        return fromPlatformError(err);
    }

    return ErrorType::Success;
}

ErrorType Wifi::networkDown() {
    ErrorType error = fromPlatformError(esp_wifi_disconnect());
    NetworkAbstraction::_status.isUp = false;
    return error;
}

ErrorType Wifi::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;

    if (version == IpTypes::Version::IPv4) {
        ip_addr_t ip_addr;
        ip_addr.type = IPADDR_TYPE_V4;
        
        uint8_t google_dns_server_ip[] = {8,8,8,8};
        IP4_ADDR(&ip_addr.u_addr.ip4, google_dns_server_ip[0], google_dns_server_ip[1], google_dns_server_ip[2], google_dns_server_ip[3]);
        dns_setserver(0, &ip_addr);
        struct hostent *hent = gethostbyname(hostname.data());
        
        if (NULL == hent) {
            uint8_t cloudflare_dns_server_ip[] = {1,1,1,1};
            IP4_ADDR(&ip_addr.u_addr.ip4, cloudflare_dns_server_ip[0], cloudflare_dns_server_ip[1], cloudflare_dns_server_ip[2], cloudflare_dns_server_ip[3]);
            dns_setserver(0, &ip_addr);
            hent = gethostbyname(hostname.data());
        }

        if (NULL != hent) {
            struct in_addr **addr_list = (struct in_addr **)hent->h_addr_list;
            struct sockaddr_in dest_ip;
            dest_ip.sin_addr.s_addr = addr_list[0]->s_addr;
            dest_ip.sin_family = toPosixFamily(version);
            dest_ip.sin_port = htons(port);

            if (-1 != (sock = socket(toPosixFamily(version), toPosixSocktype(protocol), IPPROTO_IP))) {
                if (0 == connect(sock, (struct sockaddr *)&dest_ip, sizeof(dest_ip))) {
                    fd_set fdset;
                    FD_ZERO(&fdset);
                    FD_SET(sock, &fdset);

                    Microseconds tvUsec = timeout * 1000;
                    struct timeval timeoutval;
                    if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
                        PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
                        tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
                    }
                    //Try to use seconds if possible. Some systems don't like very large usec timeouts.
                    if (timeout >= 1000) {
                        timeoutval.tv_sec = timeout / 1000;
                        timeoutval.tv_usec = 0;
                    }
                    else {
                        timeoutval.tv_sec = 0;
                        timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);
                    }

                    // Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
                    int res = select(sock+1, NULL, &fdset, NULL, &timeoutval);
                    if (res < 0) {
                        PLT_LOGE(TAG, "Error during connection: select for socket to be writable %s", strerror(errno));
                        error = ErrorType::Failure;
                    }
                    else if (res == 0) {
                        PLT_LOGE(TAG, "Connection timeout: select for socket to be writable %s", strerror(errno));
                        error = ErrorType::Timeout;
                    }
                    else {
                        int sockerr;
                        socklen_t len = (socklen_t)sizeof(int);

                        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*)(&sockerr), &len) < 0) {
                            PLT_LOGE(TAG, "Error when getting socket error using getsockopt() %s", strerror(errno));
                            error = ErrorType::Failure;
                        }
                        else if (sockerr) {
                            PLT_LOGE(TAG, "Connection error %d", sockerr);
                            error = ErrorType::Failure;
                        }
                        else {
                            sock = sock;
                            error = ErrorType::Success;
                        }
                    }
                }
                else {
                    PLT_LOGW(TAG, "Failed to connect: %s", strerror(errno));
                    error = fromPlatformError(errno);
                }
            }
            else {
                PLT_LOGW(TAG, "Failed to create socket: %s", strerror(errno));
                error = fromPlatformError(errno);
            }
        }
        else {
            PLT_LOGW(TAG, "Failed to get host by name: %d", h_errno);
            error = fromPlatformError(h_errno);
        }
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType Wifi::disconnect(const Socket &socket) {
    ErrorType error = ErrorType::Failure;

    if (-1 != shutdown(socket, 0)) {
        if (-1 != close(socket)) {
            error = ErrorType::Success;
        }
        else {
            error = fromPlatformError(errno);
        }
    }
    
    return error;
}

ErrorType Wifi::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) {
    struct sockaddr_storage destinationAddress;
    int ipProtocol;
    ErrorType error = ErrorType::Failure;

    if (IpTypes::Version::IPv4 == version) {
        struct sockaddr_in *ipv4DestinationAddress = reinterpret_cast<struct sockaddr_in *>(&destinationAddress);
        ipv4DestinationAddress->sin_addr.s_addr = htonl(INADDR_ANY);
        ipv4DestinationAddress->sin_family = AF_INET;
        ipv4DestinationAddress->sin_port = htons(port);
        ipProtocol = IPPROTO_IP;

        listenerSocket = socket(AF_INET, toPosixSocktype(protocol), ipProtocol);
        if (listenerSocket < 0) {
            return fromPlatformError(errno);
        }
    }
    else if (IpTypes::Version::IPv6 == version) {
        struct sockaddr_in6 *ipv6DestinationAddress = reinterpret_cast<struct sockaddr_in6 *>(&destinationAddress);
        bzero(&ipv6DestinationAddress->sin6_addr, sizeof(ipv6DestinationAddress->sin6_addr));
        ipv6DestinationAddress->sin6_family = AF_INET6;
        ipv6DestinationAddress->sin6_port = htons(port);
        ipProtocol = IPPROTO_IPV6;

        listenerSocket = socket(AF_INET6, toPosixSocktype(protocol), ipProtocol);
        if (listenerSocket < 0) {
            return fromPlatformError(errno);
        }
    }

    int enable = 1;
    if (0 == setsockopt(listenerSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
        if (0 == bind(listenerSocket, reinterpret_cast<struct sockaddr *>(&destinationAddress), sizeof(destinationAddress))) {
            if (0 == listen(listenerSocket, 1)) {
                error = ErrorType::Success;
            }
            else {
                error = fromPlatformError(errno);
            }
        }
    }

    return error;
}

ErrorType Wifi::acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) {
    struct sockaddr_storage clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    ErrorType error = ErrorType::Failure;

    Microseconds tvUsec = timeout * 1000;
    struct timeval timeoutval;
    if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
        PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
        tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
    }
    timeoutval.tv_sec = 0;
    timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);

    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(listenerSocket, &readfds);

    int ret;
    ret = select(listenerSocket + 1, &readfds, NULL, NULL, &timeoutval);
    if (ret > 0) {
        if (FD_ISSET(listenerSocket, &readfds)) {
            if ((newSocket = accept(listenerSocket, (struct sockaddr *)&clientAddress, &clientAddressSize)) >= 0) {
                error = ErrorType::Success;
            }
            else {
                error = fromPlatformError(errno);
            }
        }
    }
    else if (0 == ret){
        error = ErrorType::Timeout;
    }
    else {
        error = fromPlatformError(errno);
    }

    return error;
}

ErrorType Wifi::closeConnection(const Socket socket) {
    if (-1 != close(socket)) {
        return ErrorType::Success;
    }
    else {
        return fromPlatformError(errno);
    }
}

ErrorType Wifi::radioOn() {
    return fromPlatformError(esp_wifi_start());
}

ErrorType Wifi::radioOff() {
    return fromPlatformError(esp_wifi_stop());
}

ErrorType Wifi::transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    Bytes sent = 0;
    Bytes remaining = frame.size();

    assert(remaining > 0);

    while (remaining > 0) {
        ssize_t bytesWritten = send(socket, &frame.at(sent), remaining, 0);
        if (bytesWritten < 0) {
            return fromPlatformError(errno);
        }

        sent += bytesWritten;
        remaining -= bytesWritten;
    }

    return ErrorType::Success;
}

ErrorType Wifi::receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    ErrorType error = ErrorType::Timeout;
    ssize_t bytesReceived = 0;

    Microseconds tvUsec = timeout * 1000;
    struct timeval timeoutval;
    if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
        PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
        tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
    }
    //There is some a limit on the amount of usec's that can be used for Darwin but the limit is not stated, so try to use seconds if possible
    if (timeout >= 1000) {
        timeoutval.tv_sec = timeout / 1000;
        timeoutval.tv_usec = 0;
    }
    else {
        timeoutval.tv_sec = 0;
        timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);
    }

    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

    int ret;
    ret = select(socket + 1, &readfds, NULL, NULL, &timeoutval);
    if (ret < 0) {
        error = fromPlatformError(errno);
    }
    else if (FD_ISSET(socket, &readfds)) {
        if (-1 == (bytesReceived = recv(socket, frameBuffer.data(), frameBuffer.size(), 0))) {
            error = fromPlatformError(errno);
        }
        else if (0 == bytesReceived) {
            //recv returns 0 if the connection is closed.
            error = ErrorType::PrerequisitesNotMet;
        }
        else {
            frameBuffer.resize(bytesReceived);
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType Wifi::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    uint8_t macAddressByteArray[6];
    esp_err_t err;

    err = esp_efuse_mac_get_default(macAddressByteArray);
    if (ESP_OK != err) {
        return fromPlatformError(err);
    }

    assert(snprintf(macAddress.data(), macAddress.size(), MACSTR, MAC2STR(macAddressByteArray)) > 0);

    return ErrorType::Success;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    int signalStrengthRssi;

    esp_wifi_sta_get_rssi(&signalStrengthRssi);

    signalStrength = DecibelMilliWatts(signalStrengthRssi);
 
    _status.signalStrength = signalStrength;

    return ErrorType::Success;
}

ErrorType Wifi::setSsid(WifiTypes::Mode mode, const std::string &ssid) {
    if (WifiTypes::Mode::Station == mode) {
        setStationSsid(ssid);
    }
    else if (WifiTypes::Mode::AccessPoint == mode) {
        setAccessPointSsid(ssid);
    }
    else if (WifiTypes::Mode::AccessPointAndStation == mode) {
        return ErrorType::InvalidParameter;
    }

    return ErrorType::Success; 
}

ErrorType Wifi::setStationSsid(const std::string &ssid) {
    _stationSsid = ssid;

    return ErrorType::Success;
}

ErrorType Wifi::setAccessPointSsid(const std::string &ssid) {
    _accessPointSsid = ssid;

    return ErrorType::Success;
}

ErrorType Wifi::setPassword(WifiTypes::Mode mode, const std::string &password) {
    ErrorType error = ErrorType::Success;

    if (WifiTypes::Mode::Station == mode) {
        _stationPassword = password;
    }
    else if (WifiTypes::Mode::AccessPoint == mode) {
        _accessPointPassword = password;
    }
    else if (WifiTypes::Mode::AccessPointAndStation == mode) {
        error = ErrorType::InvalidParameter;
    }

    return error;
}

#ifdef __cplusplus
extern "C" {
#endif

static void WifiEventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData) {
    //This is here just to inform you of what arg is.
    Wifi *self = reinterpret_cast<Wifi *>(arg);

    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_AP_START) {
        xEventGroupSetBits(wifiEventGroup, wifiApStartedBit);
        if (xEventGroupGetBits(wifiEventGroup) & wifiStaStartedBit) {
            xEventGroupSetBits(wifiEventGroup, wifiApAndStaStartedBit);
        }
    }
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) eventData;
        PLT_LOGI(Wifi::TAG, "Station " MACSTR " joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) eventData;
        PLT_LOGI(Wifi::TAG, "Station " MACSTR " left, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_CONNECTED) {
        PLT_LOGI(Wifi::TAG, "Station connected");
        const_cast<WifiTypes::Status &>(self->status()).isProvisioned = true;
    }
    else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        PLT_LOGI(Wifi::TAG, "Station disconnected");
        const_cast<WifiTypes::Status &>(self->status()).isProvisioned = false;
    }
    else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        self->networkUp();
        PLT_LOGI(Wifi::TAG, "Station started");
        xEventGroupSetBits(wifiEventGroup, wifiStaStartedBit);
        if (xEventGroupGetBits(wifiEventGroup) & wifiApStartedBit) {
            xEventGroupSetBits(wifiEventGroup, wifiApAndStaStartedBit);
        }
    }
    else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) eventData;
        PLT_LOGI(Wifi::TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

#ifdef __cplusplus
}
#endif
