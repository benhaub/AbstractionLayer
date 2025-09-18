//AbstractionLayer
#include "FileSystemModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Spiffs.hpp"
#include "KeyValue.hpp"

ErrorType FileSystem::mount() {
    bool mountDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto mountCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::mount(*this, _nameSpace, _handle);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::mount(*this);
            }
        }

        if (ErrorType::Success == callbackError) {
            _status.mounted = true;
        }

        mountDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(mountCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!mountDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::unmount() {
    bool unmountDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto unmountCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::unmount(*this, _handle);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::unmount(*this);
            }
        }

        if (ErrorType::Success == callbackError) {
            _status.mounted = false;
        }

        unmountDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(unmountCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!unmountDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::maxPartitionSize(Bytes &size) {
    auto maxStorageQueryDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto maxStorageQueryCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::maxPartitionSize(*this, size);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::maxPartitionSize(*this, size);
            }
        }

        maxStorageQueryDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(maxStorageQueryCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!maxStorageQueryDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::availablePartition(Bytes &size) {
    bool availableStorageQueryDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto availableStorageQueryCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::availablePartition(*this, size);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::availablePartition(*this, size);
            }
        }

        availableStorageQueryDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(availableStorageQueryCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }   

    if (!availableStorageQueryDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::erasePartition() {
    bool erasePartitionDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto erasePartitionCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::erasePartition(*this);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::erasePartition(*this);
            }
        }

        erasePartitionDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(erasePartitionCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!erasePartitionDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::open(std::string_view path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    assert(path.size() > 0);
    bool openDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto openCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::open(*this, path, mode, file);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
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
        }

        openDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(openCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!openDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::close(FileSystemTypes::File &file) {
    bool closeDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto closeCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::close(_handle, file);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::close(file, spiffsFiles[file.path->c_str()]);
                if (callbackError == ErrorType::Success) {
                    spiffsFiles.erase(file.path->c_str());
                    _status.openedFiles = spiffsFiles.size();
                }
            }
        }

        closeDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(closeCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!closeDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::remove(FileSystemTypes::File &file) {
    bool removeDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto removeCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::remove(_handle, file);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::remove(file, spiffsFiles[file.path->c_str()]);
                spiffsFiles.erase(file.path->c_str());
            }
        }

        removeDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(removeCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!removeDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::readBlocking(FileSystemTypes::File &file, std::string &buffer) {
    bool readDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto readCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::readBlocking(_handle, file, buffer);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::readBlocking(spiffsFiles[file.path->c_str()], file, buffer);
            }
        }

        readDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(readCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!readDone) {
        OperatingSystem::Instance().block();
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
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto writeCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::writeBlocking(_handle, file, data);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {

            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::writeBlocking(spiffsFiles[file.path->c_str()], file, data);
            }
        }

        writeDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(writeCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!writeDone) {
        OperatingSystem::Instance().block();
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
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto syncCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::synchronize(_handle);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::synchronize(spiffsFiles[file.path->c_str()]);
            }
        }

        syncDone = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(syncCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!syncDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType FileSystem::size(FileSystemTypes::File &file) {
    bool sizeQueryDone = false;
    ErrorType callbackError = ErrorType::NotSupported;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto sizeQueryCallback = [&, thread]() -> ErrorType {
        if constexpr (ESP_FILE_SYSTEM_ENABLE_NVS) {
            
            if (FileSystemTypes::Implementation::KeyValue == implementation()) {
                callbackError = KeyValue::size(_handle, file);
            }
        }
        if constexpr (ESP_FILE_SYSTEM_ENABLE_SPIFFS) {
            
            if (FileSystemTypes::Implementation::Spiffs == implementation()) {
                callbackError = Spiffs::size(file);
            }
        }

        OperatingSystem::Instance().unblock(thread);
        sizeQueryDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(sizeQueryCallback);
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    if (!sizeQueryDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}
