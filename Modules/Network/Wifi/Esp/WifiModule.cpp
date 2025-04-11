//Common
#include "Log.hpp"
//Modules
#include "WifiModule.hpp"
#include "OperatingSystemModule.hpp"
//C++
#include <cstring>
//ESP
#include "esp_wifi_types.h"
#include "esp_mac.h" //For setting the Access Point ssid
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif
static void WifiEventHandler(void *eventHandleArg, esp_event_base_t event_base, int32_t event_id, void *event_data);
//Global so that the handler can access it. Otherwise would have to make it a public data member.
static EventGroupHandle_t wifiEventGroup;
//Only wait for the AP to connect.
constexpr unsigned int wifiApStartedBit = BIT0;
//Only wait for the Station to connect.
constexpr unsigned int wifiStaStartedBit = BIT1;
//Wait for both to connect
constexpr unsigned int wifiApAndStaStartedBit = BIT2;
#ifdef __cplusplus
}
#endif

//Based off https://github.com/espressif/esp-idf/blob/c5865270b50529cd32353f588d8a917d89f3dba4/examples/wifi/softap_sta/main/softap_sta.c
ErrorType Wifi::init() {
    esp_err_t err;
    ErrorType error;
    _status.isUp = false;

    if (WifiConfig::Mode::Unknown == mode() || WifiConfig::AuthMode::Unknown == authMode()) {
        return ErrorType::PrerequisitesNotMet;
    }

    wifiEventGroup = xEventGroupCreate();

    if (ErrorType::Success != (error = networkUp())) {
        return error;
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
        case WifiConfig::Mode::AccessPoint: {
            error = initAccessPoint();
            eventBitsToWaitFor |= wifiApStartedBit;
            break;
        }
        case WifiConfig::Mode::Station: {
            error = initStation();
            eventBitsToWaitFor |= wifiStaStartedBit;
            break;
        }
        case WifiConfig::Mode::AccessPointAndStation: {
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
    PLT_LOGI(TAG, "Waiting at most %us for interface to come up", timeout / 1000);
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                          eventBitsToWaitFor,
                                          pdFALSE,
                                          pdFALSE,
                                          pdMS_TO_TICKS(timeout));

    if (bits & eventBitsToWaitFor) {
        _status.isUp = true;
        return ErrorType::Success;
    } else {
        return ErrorType::Timeout;
    }
}

ErrorType Wifi::initAccessPoint() {
    esp_netif_create_default_wifi_ap();

    wifi_config_t wifiAccessPointConfig;
    memcpy(wifiAccessPointConfig.ap.ssid, accessPointSsid().data(), accessPointSsid().length());
    wifiAccessPointConfig.ap.ssid_len = accessPointSsid().length();
    wifiAccessPointConfig.ap.channel = 1;
    memcpy(wifiAccessPointConfig.ap.password, accessPointPassword().data(), accessPointPassword().length());
    //Wifi driver only knows how to verify the password if it's properly terminated.
    wifiAccessPointConfig.ap.password[accessPointPassword().length()] = '\0';
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
    esp_netif_create_default_wifi_sta();

    wifi_config_t wifiStationConfig;
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
    esp_err_t err;

    err = esp_netif_init();
    if (ESP_OK != err) {
        return fromPlatformError(err);
    }

    return ErrorType::Success;
}

ErrorType Wifi::networkDown() {
    ErrorType error = fromPlatformError(esp_wifi_stop());
    _ipAddress.clear();
    _status.isUp = false;
    return error;
}

ErrorType Wifi::radioOn() {
    return fromPlatformError(esp_wifi_start());
}

ErrorType Wifi::radioOff() {
    return fromPlatformError(esp_wifi_deinit());
}

ErrorType Wifi::txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    Bytes sent = 0;
    Bytes remaining = frame.size();

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

ErrorType Wifi::txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callbackr) {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    ErrorType error = ErrorType::Timeout;
    ssize_t bytesReceived = 0;

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
    FD_SET(socket, &readfds);

    //Wait for input from the socket until the timeout
    {
    int ret;
    ret = select(socket + 1, &readfds, NULL, NULL, &timeoutval);
    if (ret < 0) {
        frameBuffer.resize(0);
        return fromPlatformError(errno);
    }
    }

    if (FD_ISSET(socket, &readfds)) {
        if (-1 == (bytesReceived = recv(socket, frameBuffer.data(), frameBuffer.size(), 0))) {
            frameBuffer.resize(0);
            error = fromPlatformError(errno);
        }
        else if (0 == bytesReceived) {
            //recv returns 0 if the connection is closed.
            frameBuffer.resize(0);
            error = ErrorType::PrerequisitesNotMet;
        }
        else {
            frameBuffer.resize(bytesReceived);
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType Wifi::rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::getMacAddress(std::string &macAddress) {
    uint8_t macAddressByteArray[6];
    constexpr Count macAddressStringSize = 17;
    esp_err_t err;
    assert(macAddress.size() >= macAddressStringSize);

    err = esp_read_mac(macAddressByteArray, ESP_MAC_WIFI_STA);
    if (ESP_OK != err) {
        return fromPlatformError(err);
    }

    assert(snprintf(macAddress.data(), macAddress.size(), MACSTR, MAC2STR(macAddressByteArray)) > 0);
    macAddress.resize(macAddressStringSize);

    return ErrorType::Success;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    int signalStrengthRssi;
    esp_wifi_sta_get_rssi(&signalStrengthRssi);
    signalStrength = DecibelMilliWatts(signalStrengthRssi);

    _status.signalStrength = signalStrength;

    return ErrorType::Success;
}

ErrorType Wifi::mainLoop() {
    return runNextEvent();
}

ErrorType Wifi::setSsid(WifiConfig::Mode mode, const std::string &ssid) {
    if (WifiConfig::Mode::Station == mode) {
        setStationSsid(ssid);
    }
    else if (WifiConfig::Mode::AccessPoint == mode) {
        setAccessPointSsid(ssid);
    }
    else if (WifiConfig::Mode::AccessPointAndStation == mode) {
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

ErrorType Wifi::setPassword(WifiConfig::Mode mode, const std::string &password) {
    ErrorType error = ErrorType::Success;

    if (WifiConfig::Mode::Station == mode) {
        _stationPassword = password;
    }
    else if (WifiConfig::Mode::AccessPoint == mode) {
        _accessPointPassword = password;
    }
    else if (WifiConfig::Mode::AccessPointAndStation == mode) {
        error = ErrorType::InvalidParameter;
    }

    return error;
}

#ifdef __cplusplus
extern "C" {
#endif

static void WifiEventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData) {
    static int s_retry_num = 0;

    //This is here just to inform you of what arg is.
    __attribute__((unused)) Wifi *self = reinterpret_cast<Wifi *>(arg);

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
    }
    else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        PLT_LOGI(Wifi::TAG, "Station started");
        xEventGroupSetBits(wifiEventGroup, wifiStaStartedBit);
        if (xEventGroupGetBits(wifiEventGroup) & wifiApStartedBit) {
            xEventGroupSetBits(wifiEventGroup, wifiApAndStaStartedBit);
        }
    }
    else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) eventData;
        PLT_LOGI(Wifi::TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }

    s_retry_num++;
}

#ifdef __cplusplus
}
#endif
