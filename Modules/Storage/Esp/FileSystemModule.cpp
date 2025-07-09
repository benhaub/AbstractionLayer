//AbstractionLayer
#include "FileSystemModule.hpp"
#include "OperatingSystemModule.hpp"
#include "KeyValue.hpp"
#include "Spiffs.hpp"

ErrorType FileSystem::mount() {
    bool mountDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto mountCallback = [&]() -> ErrorType {
        switch(_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::mount(*this, nameSpace, _handle);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::mount(*this);
                break;
            default:
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
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::unmount(*this, _handle);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::unmount(*this);
                break;
            default:
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
        switch(_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::maxPartitionSize(*this, size);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::maxPartitionSize(*this, size);
                break;
            default:
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
        switch(_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::availablePartition(*this, size);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::availablePartition(*this, size);
                break;
            default:
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
        switch(_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::erasePartition(*this);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::erasePartition(*this);
                break;
            default:
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

ErrorType FileSystem::open(const std::string &path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    bool openDone = false;
    ErrorType callbackError = ErrorType::Failure;
    assert(path.size() > 0);

    auto openCallback = [&]() -> ErrorType {
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::open(*this, path, mode, file);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                if (spiffsFiles.size() >= _MaxOpenFiles) {
                    callbackError = ErrorType::LimitReached;
                }
                else {
                    FILE *spiffsFile = nullptr;
                    callbackError = Spiffs::open(path, mode, file, spiffsFile);
                    if (ErrorType::Success == callbackError) {
                        spiffsFiles[file.path] = spiffsFile;
                        _status.openedFiles = spiffsFiles.size();
                    }
                }
                break;
            default:
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
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::close(_handle, file);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::close(file, spiffsFiles[file.path]);
                if (callbackError == ErrorType::Success) {
                    spiffsFiles.erase(file.path);
                    _status.openedFiles = spiffsFiles.size();
                }
                break;
            default:
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
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::remove(_handle, file);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::remove(file, spiffsFiles[file.path]);
                spiffsFiles.erase(file.path);
                break;
            default:
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
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::readBlocking(_handle, file, buffer);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                if (spiffsFiles.contains(file.path)) {
                    callbackError = Spiffs::readBlocking(spiffsFiles[file.path], file, buffer);
                }
                else {
                    callbackError = ErrorType::PrerequisitesNotMet;
                }
                break;
            default:
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

ErrorType FileSystem::writeBlocking(FileSystemTypes::File &file, const std::string &data) {
    bool writeDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto writeCallback = [&]() -> ErrorType {
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::writeBlocking(_handle, file, data);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::writeBlocking(spiffsFiles[file.path], file, data);
                break;
            default:
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
        ErrorType error = writeBlocking(file, *data);
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
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::synchronize(_handle);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::synchronize(spiffsFiles[file.path]);
                break;
            default:
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
        switch (_implementation) {
            case FileSystemTypes::Implementation::KeyValue:
                callbackError = KeyValue::size(_handle, file);
                break;
            case FileSystemTypes::Implementation::Spiffs:
                callbackError = Spiffs::size(file);
                break;
            default:
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