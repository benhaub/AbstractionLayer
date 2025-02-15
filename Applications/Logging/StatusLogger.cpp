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

ErrorType StatusLogger::toggleLoggingFor(HttpServerAbstraction *httpServer, bool toggleOn) {
    toggleOn ? _httpServer = httpServer : _httpServer = nullptr;
    return ErrorType::Success;
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
    if (nullptr != _httpServer) {
        PLT_LOGI(TAG, "<HttpServerStatus> <Listening:%s, Active Connections:%u>",
        (_httpServer->statusConst().listening ? "true" : "false"), _httpServer->statusConst().activeConnections);
    }
    if (nullptr != _ipClient) {
        PLT_LOGI(TAG, "<IpClientStatus> <Connected:%s>", (_ipClient->statusConst().connected ? "true" : "false"));
    }
    if (nullptr != _ipServer) {
        PLT_LOGI(TAG, "<IpServerStatus> <Listening:%s, Active Connections:%u>",
        (_ipServer->statusConst().listening ? "true" : "false"), _ipServer->statusConst().activeConnections);
    }
    if (nullptr != _wifi) {
        PLT_LOGI(TAG, "<WifiStatus> <Connected:%s, Signal Strength (dBm):%d>", (_wifi->statusConst().isUp ? "true" : "false"), _wifi->statusConst().signalStrength);
    }
    if (nullptr != _cellular) {
        PLT_LOGI(TAG, "<CellularStatus> <Connected:%s, Signal Strength (dBm):%d>", (_cellular->statusConst().isUp ? "true" : "false"), _cellular->statusConst().signalStrength);
    }
    if (nullptr != _operatingSystem) {
        PLT_LOGI(TAG, "<OperatingSystemStatus> <Thread Count:%d, Idle (%%):%.1f, Up Time:%d, Free Heap (%%):%.1f>", _operatingSystem->statusConst().threadCount, _operatingSystem->statusConst().idle, _operatingSystem->statusConst().upTime, _operatingSystem->statusConst().freeHeap);
    }
    if (nullptr != _filesystem) {
        PLT_LOGI(TAG, "<FileSystemStatus> <Mounted:%s, Open Files:%u, Free (%%):%.1f>", (_filesystem->statusConst().mounted ? "true" : "false"), _filesystem->statusConst().openedFiles, _filesystem->statusConst().freeSpace);
    }
    if (nullptr != _storage) {
        PLT_LOGI(TAG, "<StorageStatus> <Initialized:%s>", (_storage->statusConst().isInitialized ? "true" : "false"));
    }
}