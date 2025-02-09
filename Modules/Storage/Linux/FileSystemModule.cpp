//AbstractionLayer
#include "FileSystemModule.hpp"
#include "OperatingSystemModule.hpp"
//Posix
#include <sys/statvfs.h>
#include <sys/stat.h>

ErrorType FileSystem::mount() {
    const bool fileSystemHasNotBeenMounted = !_status.mounted;
    if (fileSystemHasNotBeenMounted) {
        _mountPrefix.assign(_storage.rootPrefixConst());
        _mountPrefix.append("/").append(_name);
        mkdir(_mountPrefix.c_str(), S_IRWXU); 
        _status.mounted = true;
    }

    return ErrorType::Success;
}

ErrorType FileSystem::unmount() {
    _status.mounted = false;
    return ErrorType::Success;
}

ErrorType FileSystem::maxPartitionSize(Bytes &size) {
    bool maxStorageQueryDone = false;
    ErrorType error = ErrorType::Failure;

    auto maxStorageQueryCallback = [&]() -> ErrorType {
        struct statvfs fiData;

        if (0 == statvfs(mountPrefixConst().c_str(), &fiData)) {
            size = fiData.f_blocks * fiData.f_frsize;
        }
        else {
            error = fromPlatformError(errno);
            size = 0;
        }

        maxStorageQueryDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(maxStorageQueryCallback));
    error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!maxStorageQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType FileSystem::availablePartition(Bytes &size) {
    bool availableStorageQueryDone = false;
    ErrorType error = ErrorType::Failure;

    auto availableStorageQueryCallback = [&]() -> ErrorType {
        struct statvfs fiData;
        ErrorType error = ErrorType::Success;

        if (0 == statvfs(mountPrefixConst().c_str(), &fiData)) {
            size = fiData.f_bavail * fiData.f_frsize;
        }
        else {
            error = fromPlatformError(errno);
            size = 0;
        }

        availableStorageQueryDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(availableStorageQueryCallback));
    error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }   

    while (!availableStorageQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType FileSystem::erasePartition() {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::open(const std::string &path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    bool openDone = false;
    ErrorType error = ErrorType::Failure;
    assert(path.size() > 0);

    auto openCallback = [&]() -> ErrorType {
        if (nullptr == _handle.get()) {
            _handle = std::make_unique<std::fstream>();
            assert(nullptr != _handle.get());
        }
        if (isOpen() || !_storage.statusConst().isInitialized) {
            //Failure because the file mode can't be set if it's already open.
            openDone = true;
            error = ErrorType::PrerequisitesNotMet;
            return error;
        }

        std::ios_base::openmode openMode = toStdOpenMode(mode, error);
        if (ErrorType::Success != error) {
            return error;
        }
        //I gotta have a motherfuckin map of these bitches so that you can open more than one file at once.
        const std::string absolutePath = mountPrefixConst() + path;
        _handle->open(absolutePath, openMode);

        if (!isOpen()) {
            openDone = true;
            error = ErrorType::Failure;
            return error;
        }

        file.path.assign(path);
        file.isOpen = true;
        file.openMode = mode;
        if (ErrorType::Success != (error = size(file))) {
            close(file);
        }

        file.filePointer = static_cast<FileOffset>(file.size);
        _handle->imbue(std::locale::classic());

        openDone = true;
        error = ErrorType::Success;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(openCallback));
    error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!openDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType FileSystem::close(FileSystemTypes::File &file) {
    bool closeDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto closeCallback = [&]() -> ErrorType {
        if (!isOpen()) {
            callbackError = ErrorType::Success;
            closeDone = true;
            return callbackError;
        }

        if (ErrorType::Success != synchronize(file)) {
            callbackError = ErrorType::Failure;
            closeDone = true;
            return callbackError;
        }

        _handle->close();
        file.openMode = FileSystemTypes::OpenMode::Unknown;
        file.isOpen = false;

        closeDone = true;
        callbackError = ErrorType::Success;
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
    bool removeDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto removeCallback = [&]() -> ErrorType {
        if (ErrorType::Success == (callbackError = close(file))) {
            if (0 == std::remove(file.path.c_str())) {
                //TODO: Get a map goin
                _handle.reset();
            }

        }

        removeDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::EventQueue::Event<>>(std::bind(removeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!removeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType FileSystem::readBlocking(FileSystemTypes::File &file, std::string &buffer) {
    bool readDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto readCallback = [&]() -> ErrorType {
        //If the buffer doesn't have a size, you won't be able to read anything.
        assert(buffer.size() > 0);

        if (canReadFromFile(file.openMode)) {
            if (_handle->seekg(file.filePointer, std::ios_base::beg).good()) {
                std::istream &is = _handle->read(buffer.data(), buffer.size());

                if (is.rdstate() & std::ios_base::eofbit) {
                    callbackError = ErrorType::EndOfFile;
                }
                else {
                    callbackError = ErrorType::Success;
                }

                buffer.resize(is.gcount());
                file.filePointer += static_cast<FileOffset>(buffer.size());

            }
        }
        else {
            callbackError = ErrorType::PrerequisitesNotMet;
        }

        //Very important to clear otherwise future calls to fstream functions may fail because the bits are set.
        _handle->clear();
        readDone = true;
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
    bool writeDone = false;
    ErrorType callbackError = ErrorType::PrerequisitesNotMet;

    auto writeCallback = [&](const std::string &data) -> ErrorType {
        if (isOpen()) {
            if (canWriteToFile(file.openMode)) {
                if (_handle->seekp(file.filePointer, std::ios_base::beg).good()) {
                    if (_handle->write(data.c_str(), static_cast<std::streamsize>(data.size())).good()) {
                        callbackError = synchronize(file);
                        file.size += data.size();
                    }
                }
                else {
                    callbackError = ErrorType::Failure;
                }
            }
        }

        _handle->clear();
        writeDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(writeCallback, data));
    ErrorType error = _storage.addEvent(event);
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
    bool synchronizeDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto synchronizeCallback = [&]() -> ErrorType {
        if (nullptr != _handle.get()) {
            if (_handle->is_open()) {
                if (_handle->flush().good()) {
                    callbackError = ErrorType::Success;
                }
            }
            else {
                callbackError = ErrorType::PrerequisitesNotMet;
            }
        }

        _handle->clear();
        synchronizeDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(synchronizeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!synchronizeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType FileSystem::size(FileSystemTypes::File &file) {
    bool sizeQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto sizeQueryCallback = [&]() -> ErrorType {
        if (_handle->is_open()) {
            if(_handle->seekg(0, std::ios_base::end).good()) {
                file.size = _handle->tellg();
                if (_handle->seekg(0, std::ios_base::beg).good()) {
                    callbackError = ErrorType::Success;
                }
            }
        }
        else {
            callbackError = ErrorType::PrerequisitesNotMet;
        }

        _handle->clear();
        sizeQueryDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<StorageAbstraction::Event<>>(std::bind(sizeQueryCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!sizeQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}