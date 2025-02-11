#ifndef __SPIFFS_HPP__
#define __SPIFFS_HPP__

//TI driverlib
#include "ti/drivers/net/wifi/fs.h"
//C++
#include <cassert>

namespace Spiffs {

    ErrorType maxSize(Bytes &size) {
        SlFsControlGetStorageInfoResponse_t storageInfo;
        _i32 slRetVal;

        slRetVal = sl_FsCtl((SlFsCtl_e)SL_FS_CTL_GET_STORAGE_INFO, 0, NULL, NULL, 0, reinterpret_cast<_u8 *>(&storageInfo), sizeof(storageInfo), NULL);
        if (0 != slRetVal) {
            return fromPlatformError(slRetVal);
        }

        size = storageInfo.DeviceUsage.DeviceBlocksCapacity * storageInfo.DeviceUsage.DeviceBlockSize;

        assert(ErrorType::Success == OperatingSystem::Instance().incrementSemaphore(std::string(SIMPLELINK_THREAD_NAME)));

        return ErrorType::Success;
    }

    ErrorType available(Bytes &size) {
        SlFsControlGetStorageInfoResponse_t storageInfo;
        _i32 slRetVal;

        slRetVal = sl_FsCtl((SlFsCtl_e)SL_FS_CTL_GET_STORAGE_INFO, 0, NULL, NULL, 0, reinterpret_cast<_u8 *>(&storageInfo), sizeof(storageInfo), NULL);
        if (0 != slRetVal) {
            return fromPlatformError(slRetVal);
        }

        size = storageInfo.DeviceUsage.NumOfAvailableBlocksForUserFiles * storageInfo.DeviceUsage.DeviceBlockSize;

        assert(ErrorType::Success == OperatingSystem::Instance().incrementSemaphore(std::string(SIMPLELINK_THREAD_NAME)));

        return ErrorType::Success;
    }

    ErrorType erasePartition(const std::string &partitionName) {
        return ErrorType::NotImplemented;
    }
}

#endif //__SPIFFS_HPP__