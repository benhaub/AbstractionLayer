/***************************************************************************//**
* @author   Ben Haubrich
* @file     NetworkFactory.hpp
* @details  Network abstraction factory for creating various types of network interfaces
* @ingroup Applications
* @sa https://en.wikipedia.org/wiki/Factory_(object-oriented_programming)
* @sa https://medium.com/pranayaggarwal25/unnamed-namespaces-static-f1498741c527
* @sa https://cs.smu.ca/~porter/csc/common_341_342/notes/storage_linkage.html
*******************************************************************************/
#ifndef __NETWORK_FACTORY_HPP__
#define __NETWORK_FACTORY_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
#include "WifiModule.hpp"
#include "Log.hpp"
//C++
#include <cassert>

///@brief The tag used for logging
static constexpr char NetworkFactoryTag[] = "NetworkFactory";

/**
 * @namespace NetworkFactoryTypes
 * @brief Contains types and constants used by the NetworkFactory.
 */
namespace NetworkFactoryTypes {

    /**
     * @struct FactoryParams
     * @brief Contains the parameters used to configure the network.
     */
    struct FactoryParams {
        public:
        /// @brief The technology type these parameters are meant for
        virtual NetworkTypes::Technology technology() const = 0;
    };

    /**
     * @struct WifiParams
     * @brief Contains the parameters used to configure the wifi.
     */
    struct WifiParams final : public FactoryParams {
        /// @brief The technology type these parameters are meant for
        NetworkTypes::Technology technology() const override { return NetworkTypes::Technology::Wifi; }

        char accessPointSsid[32];     ///< The SSID of the access point (Connecting to this device)
        char accessPointPassword[64]; ///< The password of the access point
        char stationSsid[32];         ///< The SSID of the station (Wifi that this device connects to)
        char stationPassword[64];     ///< The password of the station
        WifiTypes::AuthMode authMode; ///< Password authentication
        WifiTypes::Mode mode;         ///< Station, access point, or both
    };

    /**
     * @struct EthernetParams
     * @brief Contains the parameters used to configure the ethernet.
     */
    struct EthernetParams final : public FactoryParams {
        /// @brief The technology type these parameters are meant for
        NetworkTypes::Technology technology() const override { return NetworkTypes::Technology::Ethernet; }
    };
}

//Anonymous namespace.
namespace {
    ErrorType WifiConfigure(Wifi &wifi, const NetworkFactoryTypes::WifiParams &params) {
        ErrorType error = ErrorType::Success;

        if (WifiTypes::Mode::AccessPointAndStation == params.mode) {
            error = wifi.setSsid(WifiTypes::Mode::AccessPoint, params.accessPointSsid);
            if (ErrorType::Success == error) {
                error = wifi.setSsid(WifiTypes::Mode::Station, params.stationSsid);
            }
        }
        else if (WifiTypes::Mode::AccessPoint == params.mode) {
            error = wifi.setSsid(WifiTypes::Mode::AccessPoint, params.accessPointSsid);
        }
        else if (WifiTypes::Mode::Station == params.mode) {
            error = wifi.setSsid(WifiTypes::Mode::Station, params.stationSsid);
        }
        if (ErrorType::NotImplemented == error || ErrorType::NotAvailable == error) {
            PLT_LOGW(NetworkFactoryTag, "Setting wifi SSID is not allowed on this platform <error:%u>", (uint8_t)error);
        }
        else if (ErrorType::Success != error) {
            PLT_LOGE(NetworkFactoryTag, "Failed to set ssid <error:%u>", (uint8_t)error);
        }

        error = wifi.setAuthMode(params.authMode);
        if (ErrorType::NotImplemented == error || ErrorType::NotAvailable == error) {
            PLT_LOGW(NetworkFactoryTag, "Setting authorization modes is not allowed on this platform <error:%u>", (uint8_t)error);
        }
        else if (ErrorType::Success != error) {
            PLT_LOGE(NetworkFactoryTag, "Failed to set atuhorization mode <error:%u>", (uint8_t)error);
        }

        if (WifiTypes::Mode::AccessPointAndStation == params.mode) {
            error = wifi.setPassword(WifiTypes::Mode::AccessPoint, params.accessPointPassword);
            if (ErrorType::Success == error) {
                error = wifi.setPassword(WifiTypes::Mode::Station, params.stationPassword);
            }
        }
        else if (WifiTypes::Mode::AccessPoint == params.mode) {
            error = wifi.setPassword(WifiTypes::Mode::AccessPoint, params.accessPointPassword);
        }
        else if (WifiTypes::Mode::Station == params.mode) {
            error = wifi.setPassword(WifiTypes::Mode::Station, params.stationPassword);
        }
        if (ErrorType::NotImplemented == error || ErrorType::NotAvailable == error) {
            PLT_LOGW(NetworkFactoryTag, "Setting wifi password is not allowed on this platform <error:%u>", (uint8_t)error);
        }
        else if (ErrorType::Success != error) {
            PLT_LOGE(NetworkFactoryTag, "Failed to set password <error:%u>", (uint8_t)error);
        }

        error = wifi.setMode(params.mode);
        if (ErrorType::Success != error) {
            const bool isCriticalErrror = !((ErrorType::NotImplemented == error) || (ErrorType::NotAvailable == error));
            if (isCriticalErrror) {
                PLT_LOGE(NetworkFactoryTag, "Failed to set wifi mode <error:%u>", (uint8_t)error);
                return error;
            }
            else {
                PLT_LOGW(NetworkFactoryTag, "Setting wifi modes is not allowed on this platform <error:%u>", (uint8_t)error);
            }
        }

        return error;
    }
}

/**
 * @namespace NetworkFactory
 * @brief Functions and types that aid the creation of network interfaces.
 */
namespace NetworkFactory {

    /**
     * @brief Creates a network interface for the type selected.
     * @param technology The technology to use for the network interface.
     * @sa NetworkTypes::Technology
     * @param params The parameters to use for the network interface.
     * @param error The error code following the return of this function.
     * @returns Pointer to a NetworkAbstraction that contains the network of the type selected.
     */
    inline std::unique_ptr<NetworkAbstraction> Factory(NetworkTypes::Technology technology, const NetworkFactoryTypes::FactoryParams &params, ErrorType &error) {
        error = ErrorType::Success;

        switch (technology) {
        case NetworkTypes::Technology::Wifi: {
            auto wifi = std::make_unique<Wifi>();
            assert(nullptr != wifi.get());
            assert(params.technology() == NetworkTypes::Technology::Wifi);

            //TODO: Could this function go in the WifiAbstraction?
            error = WifiConfigure(*(wifi.get()), static_cast<const NetworkFactoryTypes::WifiParams &>(params));
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error);
                if (isCriticalError) {
                    PLT_LOGE(NetworkFactoryTag, "Wifi configuration failed <error:%u>", (uint8_t)error);
                    return nullptr;
                }
            }

            error = wifi->init();
            //If NotAvailable is returned it just means that initializing Wifi doesn't make sense on the current platform. For desktop
            //platforms the wifi is proabably already initialized for you.
            const bool isCriticalError = !(ErrorType::NotAvailable == error);
            if (ErrorType::Success != error) {
                if (isCriticalError) {
                    PLT_LOGE(NetworkFactoryTag, "Failed to initialize wifi <error:%u>", (uint8_t)error);
                    return nullptr;
                }
            }

            if (ErrorType::Success == error || !isCriticalError) {
                PLT_LOGI(NetworkFactoryTag, "Wifi network initialized <ssid (AP):%s, password (AP):%s, authMode (AP):%u, mode:%u>",
                    wifi->accessPointSsid().c_str(), wifi->accessPointPassword().c_str(), (uint8_t)wifi->authMode(), (uint8_t)wifi->mode());
                return wifi;
            }

            break;
        }
        case NetworkTypes::Technology::Zigbee:
        case NetworkTypes::Technology::Ethernet:
        case NetworkTypes::Technology::Cellular:
        default:
            error = ErrorType::NotSupported;
            break;
        }

        return nullptr;
    }
}

#endif // __NETWORK_FACTORY_HPP__
