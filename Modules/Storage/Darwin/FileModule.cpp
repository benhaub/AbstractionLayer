//Modules
#include "FileModule.hpp"
#include "StorageModule.hpp"
//Abstraction Layer
#include "EventQueue.hpp"

ErrorType File::open(const std::string &filename, const OpenMode mode) {
    assert(filename.size() > 0);

    if (nullptr == _handle.get()) {
        _handle = std::make_unique<std::fstream>();
    }
    if (isOpen() || !storage().status().isInitialized) {
        //Failure because the file mode can't be set if it's already open.
        return ErrorType::PrerequisitesNotMet;
    }

    std::ios_base::openmode openMode = toStdOpenMode(mode);
    _handle->open(storage().rootPrefix() + filename, openMode);

    if (!isOpen()) {
        return ErrorType::Failure;
    }

    _filename = std::string(filename);
    _mode = mode;
    _handle->imbue(std::locale::classic());

    return ErrorType::Success;
}

ErrorType File::close() {
    if (!isOpen()) {
        return ErrorType::PrerequisitesNotMet;
    }

    if (ErrorType::Success != synchronize()) {
        return ErrorType::Failure;
    }

    _handle->close();
    _mode = OpenMode::Unknown;

    return ErrorType::Success;
}

ErrorType File::seek(const FileOffset &offset) {
    return ErrorType::NotAvailable;
}

ErrorType File::remove() {
    ErrorType error;

    error = close();   
    if (ErrorType::Failure == error) {
        return error;
    }

    int returnValue = std::remove(path().c_str());

    if (0 == returnValue) {
        _filename.clear();
        _handle.reset();
        return error;
    }
    else {
        return ErrorType::Failure;
    }
}

//TODO: Not thread safe. Should work the same as we've done in the IP module.
ErrorType File::readBlocking(const FileOffset offset, std::string &buffer) {
    //If the buffer doesn't have a size, you won't be able to read anything.
    assert(buffer.size() > 0);

    if (!canReadFromFile()) {
        return ErrorType::PrerequisitesNotMet;
    }

    if (_handle->tellg() != offset) {
        if (!_handle->seekg(offset, std::ios_base::beg).good()) {
            _handle->clear();
            return ErrorType::Failure;
        }

        assert(_handle->tellg() == offset);
    }

    std::istream &is = _handle->read(buffer.data(), buffer.size());
    buffer.resize(is.gcount());

    if (_handle->gcount() < static_cast<std::streamsize>(buffer.size())) {
        return ErrorType::EndOfFile;
    }

    //Very important to clear otherwise future calls to fstream functions may fail because the bits are set.
    _handle->clear();
    return ErrorType::Success;
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

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<File>>(std::bind(read, offset, buffer));
    return static_cast<Storage *>(&storage())->addEvent(event);    
}

ErrorType File::writeBlocking(const FileOffset offset, const std::string &data) {
    if (!isOpen()) {
        return ErrorType::PrerequisitesNotMet;
    }
    if (!canWriteToFile()) {
        return ErrorType::PrerequisitesNotMet;
    }

    if (_handle->tellp() != offset) {
        if (!_handle->seekp(offset, std::ios_base::beg).good()) {
            _handle->clear();
            return ErrorType::Failure;
        }

        assert(_handle->tellp() == offset);
    }

    if (_handle->write(data.c_str(), static_cast<std::streamsize>(data.size())).good()) {
        //Now make this a blocking call by waiting for the data to sync
        return synchronize();
    }

    _handle->clear();
    return ErrorType::Failure;
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

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<File>>(std::bind(write, data));

    return static_cast<Storage *>(&storage())->addEvent(event);
}

ErrorType File::synchronize() {
    if (nullptr == _handle.get()) {
        return ErrorType::PrerequisitesNotMet;
    }
    else if (!_handle->is_open()) {
        return ErrorType::PrerequisitesNotMet;
    }

    if (_handle->flush().good()) {
        return ErrorType::Success;
    }
    else {
        _handle->clear();
        return ErrorType::Failure;
    }
}

std::string File::path() const {
    return _storage->rootPrefix() + _filename;
}

ErrorType File::size(Bytes &size) const {
    return ErrorType::NotImplemented;
}