//AbstractionLayer
#include "FileSystemModule.hpp"
#include "StorageAbstraction.hpp"
#include "OperatingSystemModule.hpp"
//Cc32xx Storage module
#include "Spiffs.hpp"

ErrorType FileSystem::mount() {
    return ErrorType::NotAvailable;
}

ErrorType FileSystem::unmount() {
    return ErrorType::NotAvailable;
}

ErrorType FileSystem::maxPartitionSize(Bytes &size) {
    bool maxStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto maxStorageQueryCallback = [&]() -> ErrorType {
        switch (_implementation) {
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::maxSize(size);
                break;
            default:
                break;
        }

        maxStorageQueryDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(maxStorageQueryCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!maxStorageQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::availablePartition(Bytes &size) {
    bool availableStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto availableStorageQueryCallback = [&]() -> ErrorType {
        switch (_implementation) {
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::available(size);
                break;
            default:
                break;
        }

        availableStorageQueryDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(availableStorageQueryCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!availableStorageQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::erasePartition(){
    bool erasePartitionDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto erasePartitionCallback = [&]() -> ErrorType {
        callbackError = ErrorType::NotSupported;

        switch (_implementation) {
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::erasePartition(nameConst());
                break;
            default:
                break;
        }

        erasePartitionDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(erasePartitionCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!erasePartitionDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::open(const std::string &path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    ErrorType callbackError = ErrorType::Failure;
    bool openDone = false;

    auto openCallback = [&]() -> ErrorType {
        _u32 token = 0;
        //Comes from the Host driver documentation. Not sure how to query for this or if there is a constant somewhere.
        constexpr Bytes maxFileSize = 62.5f * 1024;

        Bytes maxSize = SL_FS_CREATE_MAX_SIZE(path.size());
        assert(maxSize <= maxFileSize);

        _deviceFileHandle = sl_FsOpen(reinterpret_cast<const _u8 *>(path.c_str()), toCc32xxAccessMode(mode, callbackError) | maxSize, &token);
        if (_deviceFileHandle == SL_FS_OK) {
            file.path.assign(path);
            file.isOpen = true;
            file.openMode = mode;
            file.filePointer = 0;
        }

        openDone = true;
        callbackError = fromPlatformError(_deviceFileHandle);
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(openCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!openDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::close(FileSystemTypes::File &file) {
    ErrorType callbackError = ErrorType::Failure;
    bool closeDone = false;

    auto closeCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsClose(_deviceFileHandle, NULL, NULL, 0);
        if (SL_FS_OK == retval) {
            _deviceFileHandle = -1;
            file.isOpen = false;
            file.openMode = FileSystemTypes::OpenMode::Unknown;
        }

        closeDone = true;
        callbackError = fromPlatformError(retval);
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(closeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!closeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::remove(FileSystemTypes::File &file) {
    ErrorType callbackError = ErrorType::Failure;
    bool removeDone = false;

    auto removeCallback = [&]() -> ErrorType {
        if (file.isOpen) {
            if (ErrorType::Success == (callbackError = close(file))) {
                _i32 retval = sl_FsDel(reinterpret_cast<const unsigned char *>(file.path.c_str()), 0);
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

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(removeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!removeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::readBlocking(FileSystemTypes::File &file, std::string &buffer) {
    ErrorType callbackError = ErrorType::Failure;
    bool readDone = false;

    auto readCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsRead(_deviceFileHandle, file.filePointer, reinterpret_cast<_u8 *>(buffer.data()), buffer.size());
        if (SL_FS_OK == retval) {
            file.filePointer += static_cast<FileOffset>(buffer.size());
        }

        readDone = true;
        callbackError = fromPlatformError(retval);
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(readCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!readDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::readNonBlocking(FileSystemTypes::File &file, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {

    auto readCallback = [&, callback](std::shared_ptr<std::string> buffer) -> ErrorType {
        ErrorType error = readBlocking(file, *buffer);
        callback(error, buffer);
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(readCallback, buffer));
    return _storage.addEvent(event);
}

ErrorType FileSystem::writeBlocking(FileSystemTypes::File &file, const std::string &data) {
    ErrorType error = ErrorType::Failure;
    bool writeDone = false;

    //SimpleLink write function does not accept a constant string.
    std::string &dataToWrite = const_cast<std::string &>(data);

    auto writeCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsWrite(_deviceFileHandle, file.filePointer, reinterpret_cast<_u8 *>(dataToWrite.data()), dataToWrite.size());
        if (SL_FS_OK != retval) {
            writeDone = true;
            error = fromPlatformError(retval);
            return error;
        }

        writeDone = true;
        error = ErrorType::Success;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(writeCallback));
    error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!writeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType FileSystem::writeNonBlocking(FileSystemTypes::File &file, const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto writeCallback = [&, callback](std::shared_ptr<std::string> data) -> ErrorType {
        ErrorType error = writeBlocking(file, *data);
        callback(error, data->size());
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(writeCallback, data));
    return _storage.addEvent(event);
}

ErrorType FileSystem::synchronize(const FileSystemTypes::File &file) {
    return ErrorType::NotAvailable;
}

ErrorType FileSystem::size(FileSystemTypes::File &file) {
    return ErrorType::NotAvailable;
}