//AbstractionLayer
#include "FileSystemModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Spiffs.hpp"
#include "KeyValue.hpp"

ErrorType FileSystem::mount() {
    bool mountDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto mountCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::mount(*this, _nameSpace, _handle);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::mount(*this);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

        mountDone = true;
        if (ErrorType::Success == callbackError) {
            _status.mounted = true;
        }

        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(mountCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!mountDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::unmount() {
    bool unmountDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto unmountCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::unmount(*this, _handle);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::unmount(*this);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

        unmountDone = true;
        if (ErrorType::Success == callbackError) {
            _status.mounted = false;
        }

        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(unmountCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!unmountDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::maxPartitionSize(Bytes &size) {
    auto maxStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto maxStorageQueryCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::maxPartitionSize(*this, size);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::maxPartitionSize(*this, size);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

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
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::availablePartition(*this, size);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::availablePartition(*this, size);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

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

ErrorType FileSystem::erasePartition() {
    bool erasePartitionDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto erasePartitionCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::erasePartition(*this);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::erasePartition(*this);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

        erasePartitionDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(erasePartitionCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!erasePartitionDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::open(std::string_view path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    bool openDone = false;
    ErrorType callbackError = ErrorType::Failure;
    assert(path.size() > 0);

    auto openCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::open(*this, path, mode, file);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            if (spiffsFiles.size() >= _MaxOpenFiles) {
                callbackError = ErrorType::LimitReached;
            }
            else {
                FILE *spiffsFile = nullptr;
                callbackError = Spiffs::open(path, mode, file, spiffsFile);
                if (ErrorType::Success == callbackError) {
                    spiffsFiles[file.path->c_str()] = spiffsFile;
                    _status.openedFiles = spiffsFiles.size();
                }
            }
        }
        else {
            callbackError = ErrorType::NotSupported;
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
    bool closeDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto closeCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::close(_handle, file);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::close(file, spiffsFiles[file.path->c_str()]);
            if (callbackError == ErrorType::Success) {
                spiffsFiles.erase(file.path->c_str());
                _status.openedFiles = spiffsFiles.size();
            }
        }
        else {
            callbackError = ErrorType::NotSupported;
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
    bool removeDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto removeCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::remove(_handle, file);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::remove(file, spiffsFiles[file.path->c_str()]);
            spiffsFiles.erase(file.path->c_str());
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

        removeDone = true;
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
    bool readDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto readCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::readBlocking(_handle, file, buffer);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::readBlocking(spiffsFiles[file.path->c_str()], file, buffer);
        }
        else {
            callbackError = ErrorType::NotSupported;
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
    bool writeDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto writeCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::writeBlocking(_handle, file, data);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::writeBlocking(spiffsFiles[file.path->c_str()], file, data);
        }
        else {
            callbackError = ErrorType::NotSupported;
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

    return callbackError;
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
    bool syncDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto syncCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::synchronize(_handle);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::synchronize(spiffsFiles[file.path->c_str()]);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

        syncDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(syncCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!syncDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::size(FileSystemTypes::File &file) {
    bool sizeQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto sizeQueryCallback = [&]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            callbackError = KeyValue::size(_handle, file);
        }
        else if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            callbackError = Spiffs::size(file);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

        sizeQueryDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(sizeQueryCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!sizeQueryDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}