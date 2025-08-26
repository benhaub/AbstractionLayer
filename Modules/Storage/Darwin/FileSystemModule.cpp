//AbstractionLayer
#include "FileSystemModule.hpp"
#include "OperatingSystemModule.hpp"
//Posix
#include <sys/stat.h> //For mkdir
//C++
#include <filesystem>

ErrorType FileSystem::mount() {
    const bool fileSystemHasNotBeenMounted = !_status.mounted;

    if (fileSystemHasNotBeenMounted) {
        _mountPrefix.assign(_storage.rootPrefix()->c_str());

        if (_mountPrefix.back() != '/') {
            _mountPrefix.append("/");
        }

        _mountPrefix.append(std::string(_status.partitionName.data(), strlen(_status.partitionName.data())));
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
    ErrorType callbackError = ErrorType::Failure;

    auto maxStorageQueryCallback = [&]() -> ErrorType {
        std::filesystem::space_info spaceInfo = std::filesystem::space(mountPrefix());
        size = spaceInfo.capacity;
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
        std::filesystem::space_info spaceInfo = std::filesystem::space(mountPrefix());
        size = spaceInfo.available;
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
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::open(std::string_view path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    bool openDone = false;
    ErrorType callbackError = ErrorType::PrerequisitesNotMet;
    assert(path.size() > 0);

    auto openCallback = [&]() -> ErrorType {
        if (_storage.status().isInitialized) {
            if (!isOpen(file)) {
                std::ios_base::openmode openMode = toStdOpenMode(mode, callbackError);
                if (ErrorType::Success == callbackError) {
                    std::string absolutePath(mountPrefix());
                    absolutePath.append(path);
                    openFiles[path] = std::fstream();
                    openFiles[path].open(absolutePath, openMode);
                    if (openFiles[path].good()) {
                        file.path->assign(path);
                        
                        if (ErrorType::Success == (callbackError = size(file))) {
                            file.isOpen = true;
                            file.openMode = mode;
                            file.filePointer = static_cast<FileOffset>(file.size);
                            openFiles[file.path->data()].imbue(std::locale::classic());
                            callbackError = ErrorType::Success;
                        }
                    }
                    else {
                        openFiles.erase(path);
                        callbackError = ErrorType::Failure;
                    }
                }
            }
            else {
                callbackError = ErrorType::Success;
                _status.openedFiles = openFiles.size();
            }
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
    ErrorType callbackError = ErrorType::Success;

    auto closeCallback = [&]() -> ErrorType {
        if (isOpen(file)) {
            if (ErrorType::Success == (callbackError = synchronize(file))) {
                openFiles[file.path->data()].close();
                file.openMode = FileSystemTypes::OpenMode::Unknown;
                file.isOpen = false;
                openFiles.erase(file.path->data());
                _status.openedFiles = openFiles.size();
            }
        }

        assert(false == isOpen(file));

        closeDone = true;
        callbackError = ErrorType::Success;
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
        if (ErrorType::Success == (callbackError = close(file))) {
            std::string absolutePath(mountPrefix().data());
            absolutePath.append(file.path->c_str());
            if (0 == std::remove(absolutePath.c_str())) {
                callbackError = ErrorType::Success;
            }
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
    ErrorType callbackError = ErrorType::PrerequisitesNotMet;

    auto readCallback = [&]() -> ErrorType {
        //If the buffer doesn't have a size, you won't be able to read anything.
        assert(buffer.size() > 0);

        if (canReadFromFile(file.openMode) && isOpen(file)) {
            if (openFiles[file.path->c_str()].seekg(file.filePointer, std::ios_base::beg).good()) {
                std::istream &is = openFiles[file.path->c_str()].read(buffer.data(), buffer.size());

                if (is.rdstate() & std::ios_base::eofbit) {
                    callbackError = ErrorType::EndOfFile;
                }
                else {
                    callbackError = ErrorType::Success;
                }

                buffer.resize(is.gcount());
                file.filePointer += static_cast<FileOffset>(buffer.size());
            }
            else {
                //Very important to clear otherwise future calls to fstream functions may fail because the bits are set.
                openFiles[file.path->c_str()].clear();
            }
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
    ErrorType callbackError = ErrorType::PrerequisitesNotMet;

    auto writeCallback = [&](const std::string &data) -> ErrorType {
        if (isOpen(file)) {
            if (canWriteToFile(file.openMode)) {
                if (openFiles[file.path->data()].seekp(file.filePointer, std::ios_base::beg).good()) {
                    if (openFiles[file.path->data()].write(data.c_str(), static_cast<std::streamsize>(data.size())).good()) {
                        callbackError = synchronize(file);
                        file.size += data.size();
                    }
                }
                else {
                    callbackError = ErrorType::Failure;
                    openFiles[file.path->data()].clear();
                }
            }
        }

        writeDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(writeCallback, data));
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
        ErrorType error = writeBlocking(file, *data);
        callback(error, data->size());
        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(writeCallback, data));
    return _storage.addEvent(event);
}

ErrorType FileSystem::synchronize(const FileSystemTypes::File &file) {
    bool synchronizeDone = false;
    ErrorType callbackError = ErrorType::PrerequisitesNotMet;

    auto synchronizeCallback = [&]() -> ErrorType {
        if (isOpen(file)) {
            if (openFiles[file.path->c_str()].flush().good()) {
                callbackError = ErrorType::Success;
            }
            else if (!canWriteToFile(file.openMode)) {
                //If the file wasn't opened for writing then there is nothing to sync anyway.
                openFiles[file.path->c_str()].clear();
                callbackError = ErrorType::Success;
            }
            else {
                openFiles[file.path->c_str()].clear();
                callbackError = ErrorType::Failure;
            }
        }

        synchronizeDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(synchronizeCallback));
    ErrorType error = _storage.addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!synchronizeDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType FileSystem::size(FileSystemTypes::File &file) {
    bool sizeQueryDone = false;
    ErrorType callbackError = ErrorType::PrerequisitesNotMet;

    auto sizeQueryCallback = [&]() -> ErrorType {
        if (isOpen(file)) {
            if(openFiles[file.path->data()].seekg(0, std::ios_base::end).good()) {
                file.size = openFiles[file.path->data()].tellg();
                if (openFiles[file.path->data()].seekg(0, std::ios_base::beg).good()) {
                    callbackError = ErrorType::Success;
                }
            }

            openFiles[file.path->data()].clear();
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
