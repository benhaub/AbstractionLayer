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
        toggleOn ? _wifi = dynamic_cast<WifiAbstraction *>(network) : _wifi = nullptr;
    }
    else if (dynamic_cast< CellularAbstraction *>(network)) {
        toggleOn ? _cellular = dynamic_cast< CellularAbstraction *>(network) : _cellular = nullptr;
    }
    else {
        error = ErrorType::InvalidParameter;
    }

    return error;
}

ErrorType StatusLogger::toggleLoggingFor(IpClientAbstraction *ipClient, bool toggleOn) {
    toggleOn ? _ipClient = ipClient : _ipClient = nullptr;
    return ErrorType::Success;
}

ErrorType StatusLogger::toggleLoggingFor(IpServerAbstraction *ipServer, bool toggleOn) {
    toggleOn ? _ipServer = ipServer : _ipServer = nullptr;
    return ErrorType::Success;
}

ErrorType StatusLogger::toggleLoggingFor(OperatingSystemAbstraction *operatingSystem, bool toggleOn) {
    toggleOn ? _operatingSystem = operatingSystem : _operatingSystem = nullptr;
    return ErrorType::Success;
}

ErrorType StatusLogger::toggleLoggingFor(FileSystemAbstraction *filesystem, bool toggleOn) {
    toggleOn ? _filesystem = filesystem : _filesystem = nullptr;
    return ErrorType::Success;
}

ErrorType StatusLogger::toggleLoggingFor(StorageAbstraction *storage, bool toggleOn) {
    toggleOn ? _storage = storage : _storage = nullptr;
    return ErrorType::Success;
}

void StatusLogger::printLog(void) {
#ifdef configTIMER_TASK_STACK_DEPTH
    static_assert(configTIMER_TASK_STACK_DEPTH >= 256, "StatusLogger: Timer stack too small");
#endif
    if (nullptr != _ipClient) {
        const IpClientTypes::ClientStatus &status = _ipClient->statusConst();
        PLT_LOGI(TAG, "<IpClientStatus> <Connected:%s>", (status.connected ? "true" : "false"));
    }
    if (nullptr != _ipServer) {
        const IpServerTypes::ServerStatus &status = _ipServer->statusConst();
        PLT_LOGI(TAG, "<IpServerStatus> <Listening:%s, Active Connections:%u>",
        (status.listening ? "true" : "false"), status.activeConnections);
    }
    if (nullptr != _wifi) {
        const NetworkTypes::Status &status = _wifi->statusConst();
        PLT_LOGI(TAG, "<WifiStatus> <Connected:%s, Signal Strength (dBm):%d>", (status.isUp ? "true" : "false"), status.signalStrength);
    }
    if (nullptr != _cellular) {
        const NetworkTypes::Status &status = _cellular->statusConst();
        PLT_LOGI(TAG, "<CellularStatus> <Connected:%s, Signal Strength (dBm):%d>", (status.isUp ? "true" : "false"), status.signalStrength);
    }
    if (nullptr != _operatingSystem) {
        const OperatingSystemConfig::Status &status = _operatingSystem->statusConst();
        PLT_LOGI(TAG, "<OperatingSystemStatus> <Thread Count:%d, Idle (%%):%.1f, Up Time:%d, Free Heap (%%):%.1f>", status.threadCount, status.idle, status.upTime, status.freeHeap);
    }
    if (nullptr != _filesystem) {
        const FileSystemTypes::Status &status = _filesystem->statusConst();
        PLT_LOGI(TAG, "<FileSystemStatus> <Mounted:%s, Open Files:%u, Free (%%):%.1f>", (status.mounted ? "true" : "false"), status.openedFiles, status.freeSpace);
    }
    if (nullptr != _storage) {
        const StorageTypes::Status &status = _storage->statusConst();
        PLT_LOGI(TAG, "<StorageStatus> <Initialized:%s>", (status.isInitialized ? "true" : "false"));
    }
}