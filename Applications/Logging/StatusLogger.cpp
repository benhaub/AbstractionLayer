#include "StatusLogger.hpp"
//AbstractionLayer
#include "HttpServerAbstraction.hpp"
#include "IpClientAbstraction.hpp"
#include "IpServerAbstraction.hpp"
#include "CellularAbstraction.hpp"
#include "WifiAbstraction.hpp"
#include "OperatingSystemAbstraction.hpp"
#include "FileSystemAbstraction.hpp"
#include "StorageAbstraction.hpp"

ErrorType StatusLogger::toggleLoggingFor(NetworkAbstraction *network, bool toggleOn) {
    ErrorType error = ErrorType::Success;

    if (dynamic_cast<WifiAbstraction *>(network)) {
        if (toggleOn) {
            _wifiNetworks.push_back(static_cast<WifiAbstraction *>(network));
        }
        else {
            _wifiNetworks.erase(std::remove(_wifiNetworks.begin(), _wifiNetworks.end(), static_cast<WifiAbstraction *>(network)), _wifiNetworks.end());
        }
    }
    else if (dynamic_cast<CellularAbstraction *>(network)) {
        if (toggleOn) {
            _cellularNetworks.push_back(static_cast<CellularAbstraction *>(network));
        }
        else {
            _cellularNetworks.erase(std::remove(_cellularNetworks.begin(), _cellularNetworks.end(), static_cast<CellularAbstraction *>(network)), _cellularNetworks.end());
        }
    }
    else {
        error = ErrorType::InvalidParameter;
    }

    return error;
}

ErrorType StatusLogger::toggleLoggingFor(IpClientAbstraction *ipClient, bool toggleOn) {
    if (nullptr != ipClient) {
        if (toggleOn) {
            _ipClients.push_back(ipClient);
        }
        else {
            _ipClients.erase(std::remove(_ipClients.begin(), _ipClients.end(), ipClient), _ipClients.end());
        }

        return ErrorType::Success;
    }
    else {
        return ErrorType::InvalidParameter;
    }
}

ErrorType StatusLogger::toggleLoggingFor(IpServerAbstraction *ipServer, bool toggleOn) {
    if (nullptr != ipServer) {
        if (toggleOn) {
            _ipServers.push_back(ipServer);
        }
        else {
            _ipServers.erase(std::remove(_ipServers.begin(), _ipServers.end(), ipServer), _ipServers.end());
        }

        return ErrorType::Success;
    }
    else {
        return ErrorType::InvalidParameter;
    }
}

ErrorType StatusLogger::toggleLoggingFor(OperatingSystemAbstraction *operatingSystem, bool toggleOn) {
    toggleOn ? _operatingSystem = operatingSystem : _operatingSystem = nullptr;
    return ErrorType::Success;
}

ErrorType StatusLogger::toggleLoggingFor(FileSystemAbstraction *filesystem, bool toggleOn) {
    if (nullptr != filesystem) {
        if (toggleOn) {
            _filesystems.push_back(filesystem);
        }
        else {
            _filesystems.erase(std::remove(_filesystems.begin(), _filesystems.end(), filesystem), _filesystems.end());
        }

        return ErrorType::Success;
    }
    else {
        return ErrorType::InvalidParameter;
    }
}

ErrorType StatusLogger::toggleLoggingFor(StorageAbstraction *storage, bool toggleOn) {
    if (nullptr != storage) {
        if (toggleOn) {
            _storageMediums.push_back(storage);
        }
        else {
            _storageMediums.erase(std::remove(_storageMediums.begin(), _storageMediums.end(), storage), _storageMediums.end());
        }

        return ErrorType::Success;
    }
    else {
        return ErrorType::InvalidParameter;
    }
}

void StatusLogger::printLog(void) {
#ifdef configTIMER_TASK_STACK_DEPTH
    static_assert(configTIMER_TASK_STACK_DEPTH >= 256, "StatusLogger: Timer stack too small");
#endif
    for (const auto &client : _ipClients) {
        const IpClientTypes::ClientStatus &status = client->statusConst();
        PLT_LOGI(TAG, "<IpClientStatus> <Connected:%s>", (status.connected ? "true" : "false"));
    }
    for (const auto &server : _ipServers) {
        const IpServerTypes::ServerStatus &status = server->statusConst();
        PLT_LOGI(TAG, "<IpServerStatus> <Listening:%s, Active Connections:%u>",
        (status.listening ? "true" : "false"), status.activeConnections);
    }
    for (const auto &wifi : _wifiNetworks) {
        const NetworkTypes::Status &status = wifi->statusConst();
        PLT_LOGI(TAG, "<WifiStatus> <Connected:%s, Signal Strength (dBm):%d>", (status.isUp ? "true" : "false"), status.signalStrength);
    }
    for (const auto &cellular : _cellularNetworks) {
        const NetworkTypes::Status &status = cellular->statusConst();
        PLT_LOGI(TAG, "<CellularStatus> <Connected:%s, Signal Strength (dBm):%d>", (status.isUp ? "true" : "false"), status.signalStrength);
    }
    if (nullptr != _operatingSystem) {
        const OperatingSystemConfig::Status &status = _operatingSystem->statusConst();
        PLT_LOGI(TAG, "<OperatingSystemStatus> <Thread Count:%d, Idle (%%):%.1f, Up Time:%d>", status.threadCount, status.idle, status.upTime);
        for (const auto &memoryRegion : status.memoryRegion) {
            PLT_LOGI(TAG, "<Memory Region:%s> <Free (%%):%.1f>", memoryRegion.name.c_str(), memoryRegion.free);
        }
    }
    for (const auto &filesystem : _filesystems) {
        const FileSystemTypes::Status &status = filesystem->statusConst();
        PLT_LOGI(TAG, "<FileSystem:%s> <Mounted:%s, Open Files:%u, Free (%%):%.1f>", status.partitionName.c_str(), (status.mounted ? "true" : "false"), status.openedFiles, status.freeSpace);
    }
    for (const auto &storage : _storageMediums) {
        const StorageTypes::Status &status = storage->statusConst();
        PLT_LOGI(TAG, "<StorageStatus> <Initialized:%s>", (status.isInitialized ? "true" : "false"));
    }
}