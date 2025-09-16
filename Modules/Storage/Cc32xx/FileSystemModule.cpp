//AbstractionLayer
#include "FileSystemModule.hpp"
#include "StorageAbstraction.hpp"
#include "OperatingSystemModule.hpp"

ErrorType FileSystem::mount() {
    _status.mounted = true;
    return ErrorType::NotAvailable;
}

ErrorType FileSystem::unmount() {
    _status.mounted = false;
    return ErrorType::NotAvailable;
}

ErrorType FileSystem::maxPartitionSize(Bytes &size) {
    bool maxStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto maxStorageQueryCallback = [&]() -> ErrorType {
        SlFsControlGetStorageInfoResponse_t storageInfo;
        _i32 slRetVal;

        slRetVal = sl_FsCtl((SlFsCtl_e)SL_FS_CTL_GET_STORAGE_INFO, 0, NULL, NULL, 0, reinterpret_cast<_u8 *>(&storageInfo), sizeof(storageInfo), NULL);
        if (0 != slRetVal) {
            callbackError = fromPlatformError(slRetVal);
        }

        size = storageInfo.DeviceUsage.DeviceBlocksCapacity * storageInfo.DeviceUsage.DeviceBlockSize;

        maxStorageQueryDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(maxStorageQueryCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!maxStorageQueryDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::availablePartition(Bytes &size) {
    bool availableStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto availableStorageQueryCallback = [&]() -> ErrorType {
        SlFsControlGetStorageInfoResponse_t storageInfo;
        _i32 slRetVal;

        slRetVal = sl_FsCtl((SlFsCtl_e)SL_FS_CTL_GET_STORAGE_INFO, 0, NULL, NULL, 0, reinterpret_cast<_u8 *>(&storageInfo), sizeof(storageInfo), NULL);
        if (0 != slRetVal) {
            callbackError = fromPlatformError(slRetVal);
        }

        size = storageInfo.DeviceUsage.NumOfAvailableBlocksForUserFiles * storageInfo.DeviceUsage.DeviceBlockSize;

        availableStorageQueryDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(availableStorageQueryCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!availableStorageQueryDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::erasePartition(){
    return ErrorType::NotSupported;
}

ErrorType FileSystem::open(std::string_view path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    ErrorType callbackError = ErrorType::Failure;
    bool openDone = false;
    assert(path.size() > 0);

    auto openCallback = [&]() -> ErrorType {
        _u32 token = 0;
        _i32 retval = 0;
        //Comes from the Host driver documentation. Not sure how to query for this or if there is a constant somewhere.
        constexpr Bytes maxFileSize = 62.5f * 1024;

        Bytes maxSize = SL_FS_CREATE_MAX_SIZE(path.size());
        assert(maxSize <= maxFileSize);

        const bool fileIsNotOpen = !openFiles.contains(path);
        if (fileIsNotOpen) {
            retval = sl_FsOpen(reinterpret_cast<const _u8 *>(path.data()), toCc32xxAccessMode(mode, callbackError) | maxSize, &token);
            if (retval >= 0) {
                file.path->assign(path);
                file.isOpen = true;
                file.openMode = mode;
                file.filePointer = 0;
                openFiles[path] = retval;
                _status.openedFiles = openFiles.size();
                if (fileShouldBeTruncated(mode)) {
                    file.size = 0;
                }
                callbackError = ErrorType::Success;
            }
            else {
                callbackError = fromPlatformError(retval);
            }
        }
        else {
            callbackError = ErrorType::Success;
        }

        openDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(openCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!openDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::close(FileSystemTypes::File &file) {
    ErrorType callbackError = ErrorType::Failure;
    bool closeDone = false;

    auto closeCallback = [&]() -> ErrorType {
        const bool fileIsOpen = openFiles.contains(file.path->c_str());
        if (fileIsOpen) {
            _i32 retval = sl_FsClose(openFiles[file.path->c_str()], NULL, NULL, 0);
            if (SL_FS_OK == retval) {
                openFiles.erase(file.path->c_str());
                file.isOpen = false;
                file.openMode = FileSystemTypes::OpenMode::Unknown;
                _status.openedFiles = openFiles.size();
            }

            callbackError = fromPlatformError(retval);
        }
        else {
            callbackError = ErrorType::Success;
        }

        closeDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(closeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!closeDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::remove(FileSystemTypes::File &file) {
    ErrorType callbackError = ErrorType::Failure;
    bool removeDone = false;

    auto removeCallback = [&]() -> ErrorType {
        if (file.isOpen) {
            if (ErrorType::Success == (callbackError = close(file))) {
                _i32 retval = sl_FsDel(reinterpret_cast<const unsigned char *>(file.path->c_str()), 0);
                if (SL_FS_OK == retval) {
                    removeDone = true;
                    callbackError = fromPlatformError(retval);
                    return callbackError;
                }
            }
        }

        removeDone = true;
        callbackError =  ErrorType::Success;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(removeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!removeDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::readBlocking(FileSystemTypes::File &file, std::string &buffer) {
    ErrorType callbackError = ErrorType::Failure;
    bool readDone = false;

    auto readCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsRead(openFiles[file.path->c_str()], file.filePointer, reinterpret_cast<_u8 *>(buffer.data()), buffer.size());
        if (retval > 0) {
            file.filePointer += static_cast<FileOffset>(retval);
            buffer.resize(retval);
            callbackError = ErrorType::Success;
        }
        else {
            callbackError = fromPlatformError(retval);
            buffer.resize(0);
        }

        readDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(readCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!readDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::readNonBlocking(FileSystemTypes::File &file, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {

    auto readCallback = [&, callback](std::shared_ptr<std::string> buffer) -> ErrorType {
        ErrorType error = readBlocking(file, *buffer);
        callback(error, buffer);
        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(readCallback, buffer));
    return _storage.addEvent(event);
}

ErrorType FileSystem::writeBlocking(FileSystemTypes::File &file, std::string_view data) {
    ErrorType callbackError = ErrorType::Failure;
    bool writeDone = false;

    //SimpleLink write function does not accept a constant string.
    std::string_view dataToWrite(data);

    auto writeCallback = [&]() -> ErrorType {
        //DO NOT try to edit data. Only casting away constness because FsWrite does not take a const parameter.
        //It's UB to edit a string_view.
        char *data = const_cast<char *>(dataToWrite.data());
        _i32 retval = sl_FsWrite(openFiles[file.path->c_str()], file.filePointer, reinterpret_cast<_u8 *>(data), dataToWrite.size());
        if (retval >= 0) {
            writeDone = true;
            callbackError = ErrorType::Success;
            file.size += dataToWrite.size();
        }
        else {
            callbackError = fromPlatformError(retval);
        }

        writeDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(writeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!writeDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return error;
}

ErrorType FileSystem::writeNonBlocking(FileSystemTypes::File &file, const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto writeCallback = [&, callback](std::shared_ptr<std::string> data) -> ErrorType {
        ErrorType error = writeBlocking(file, std::string_view(data->data(), data->size()));
        callback(error, data->size());
        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(writeCallback, data));
    return _storage.addEvent(event);
}

ErrorType FileSystem::synchronize(const FileSystemTypes::File &file) {
    return ErrorType::NotAvailable;
}

ErrorType FileSystem::size(FileSystemTypes::File &file) {
    return ErrorType::NotAvailable;
}