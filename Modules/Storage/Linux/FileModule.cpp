//AbstractionLayer
#include "EventQueue.hpp"
#include "FileModule.hpp"
#include "StorageModule.hpp"
#include "OperatingSystemModule.hpp"

ErrorType File::open(const std::string &filename, const OpenMode mode) {
    bool openDone = false;
    ErrorType error = ErrorType::Failure;
    assert(filename.size() > 0);

    auto openCallback = [this, &openDone, &error](const std::string &filename, const OpenMode mode) -> ErrorType {
        if (nullptr == _handle.get()) {
            _handle = std::make_unique<std::fstream>();
            assert(nullptr != _handle.get());
        }
        if (isOpen() || !storage().statusConst().isInitialized) {
            //Failure because the file mode can't be set if it's already open.
            openDone = true;
            error = ErrorType::PrerequisitesNotMet;
            return error;
        }

        std::ios_base::openmode openMode = toStdOpenMode(mode);
        _handle->open(storage().rootPrefix() + filename, openMode);

        if (!isOpen()) {
            openDone = true;
            error = ErrorType::Failure;
            return error;
        }

        _filename = std::string(filename);
        _mode = mode;
        _handle->imbue(std::locale::classic());

        openDone = true;
        error = ErrorType::Success;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(openCallback, filename, mode));
    error = storage().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!openDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType File::close() {
    bool closeDone = false;
    ErrorType error = ErrorType::Failure;

    auto closeCallback = [this, &closeDone, &error]() -> ErrorType {
        if (!isOpen()) {
            error = ErrorType::PrerequisitesNotMet;
            closeDone = true;
            return error;
        }

        if (ErrorType::Success != synchronize()) {
            error = ErrorType::Failure;
            closeDone = true;
            return error;
        }

        _handle->close();
        _mode = OpenMode::Unknown;

        closeDone = true;
        error = ErrorType::Success;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(closeCallback));
    error = storage().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!closeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType File::seek(const FileOffset &offset) {
    return ErrorType::NotAvailable;
}

ErrorType File::remove() {
    bool removeDone = false;
    ErrorType error = ErrorType::Failure;

    auto removeCallback = [this, &removeDone, &error]() -> ErrorType {
        error = close();   
        if (ErrorType::Failure == error) {
            removeDone = true;
            return error;
        }

        int returnValue = std::remove(path().c_str());

        if (0 == returnValue) {
            _filename.clear();
            _handle.reset();
            removeDone = true;
            return error;
        }
        else {
            error = ErrorType::Failure;
            removeDone = true;
            return error;
        }
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(removeCallback));
    error = storage().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!removeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType File::readBlocking(const FileOffset offset, std::string &buffer) {
    bool readDone = false;
    ErrorType error = ErrorType::Success;

    auto readCallback = [&]() {
        //If the buffer doesn't have a size, you won't be able to read anything.
        assert(buffer.size() > 0);

        if (!canReadFromFile()) {
            _handle->clear();
            readDone = true;
            error = ErrorType::PrerequisitesNotMet;
            return error;
        }

        if (_handle->tellg() != offset) {
            if (!_handle->seekg(offset, std::ios_base::beg).good()) {
                _handle->clear();
                readDone = true;
                error = ErrorType::Failure;
                return error;
            }

            assert(_handle->tellg() == offset);
        }

        std::istream &is = _handle->read(buffer.data(), buffer.size());

        if (_handle->gcount() < static_cast<std::streamsize>(buffer.size())) {
            error = ErrorType::EndOfFile;
        }

        buffer.resize(is.gcount());

        //Very important to clear otherwise future calls to fstream functions may fail because the bits are set.
        _handle->clear();
        readDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(readCallback));
    error = storage().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!readDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType File::readNonBlocking(const FileOffset offset, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {

    auto read = [this, callback](FileOffset offset, std::shared_ptr<std::string> buffer) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        error = readBlocking(offset, *(buffer.get()));

        if (nullptr != callback) {
            callback(error, buffer);
        }

        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(read, offset, buffer));
    return storage().addEvent(event);    
}

ErrorType File::writeBlocking(const FileOffset offset, const std::string &data) {
    bool writeDone = false;
    ErrorType error = ErrorType::Failure;

    auto writeCallback = [this, &writeDone, &error](const FileOffset offset, const std::string &data) -> ErrorType {
        if (!isOpen()) {
            error = ErrorType::PrerequisitesNotMet;
            writeDone = true;
            return error;
        }
        if (!canWriteToFile()) {
            error = ErrorType::PrerequisitesNotMet;
            writeDone = true;
            return error;
        }

        if (_handle->tellp() != offset) {
            if (!_handle->seekp(offset, std::ios_base::beg).good()) {
                _handle->clear();
                writeDone = true;
                error = ErrorType::Failure;
                return error;
            }

            assert(_handle->tellp() == offset);
        }

        if (_handle->write(data.c_str(), static_cast<std::streamsize>(data.size())).good()) {
            error = synchronize();
            writeDone = true;
        }

        _handle->clear();
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(writeCallback, offset, data));
    error = storage().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!writeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType File::writeNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {

    auto write = [this, callback](std::shared_ptr<std::string> data) -> ErrorType {
        ErrorType error = ErrorType::Failure;
        FileOffset offset = 0;

        error = writeBlocking(offset, *(data.get()));

        if (nullptr != callback) {
            callback(error, data->size());
        }

        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(write, data));
    return storage().addEvent(event);
}

ErrorType File::synchronize() {
    bool synchronizeDone = false;
    ErrorType error = ErrorType::Failure;

    auto synchronizeCallback = [this, &synchronizeDone, &error]() -> ErrorType {
        if (nullptr == _handle.get()) {
            error = ErrorType::PrerequisitesNotMet;
            synchronizeDone = true;
            return error;
        }
        else if (!_handle->is_open()) {
            error = ErrorType::PrerequisitesNotMet;
            synchronizeDone = true;
            return error;
        }

        if (_handle->flush().good()) {
            error = ErrorType::Success;
            synchronizeDone = true;
            return error;
        }
        else {
            _handle->clear();
            error = ErrorType::Failure;
            synchronizeDone = true;
            return error;
        }
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(synchronizeCallback));
    error = storage().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!synchronizeDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

std::string File::path() const {
    return _storage->rootPrefix() + _filename;
}

ErrorType File::size(Bytes &size) const {
    bool sizeQueryDone = false;
    ErrorType error = ErrorType::Failure;

    auto sizeQueryCallback = [&]() -> ErrorType {
        if (!_handle->is_open()) {
            error = ErrorType::PrerequisitesNotMet;
            sizeQueryDone = true;
            return error;
        }

        if(!_handle->seekg(0, std::ios_base::end).good()) {
            _handle->clear();
            sizeQueryDone = true;
            error = ErrorType::Failure;
            return error;
        }

        size = _handle->tellg();

        if (!_handle->seekg(0, std::ios_base::beg).good()) {
            _handle->clear();
            sizeQueryDone = true;
            error = ErrorType::Failure;
            return error;
        }

        sizeQueryDone = true;
        error = ErrorType::Success;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(sizeQueryCallback));
    error = storage().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!sizeQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}