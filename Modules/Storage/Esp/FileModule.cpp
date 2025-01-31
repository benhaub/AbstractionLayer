//Modules
#include "FileModule.hpp"
#include "StorageModule.hpp"

ErrorType File::open(const std::string &filename, OpenMode mode) {
    assert(filename.size() > 0);
    esp_err_t err;
    _handle = nvs::open_nvs_handle(storage().name().c_str(), NVS_READWRITE, &err);
    if (ErrorType::Success != fromPlatformError(err)) {
        return fromPlatformError(err);
    }

    _filename.assign(filename);
    return ErrorType::Success;
}

ErrorType File::close() {
    return synchronize();
}

ErrorType File::seek(const FileOffset &offset) {
    return ErrorType::NotAvailable;
}

ErrorType File::remove() {
    return fromPlatformError(_handle->erase_item(path().c_str()));
}

ErrorType File::readBlocking(FileOffset offset, std::string &buffer) {
    if (nullptr == _handle.get()) {
        return ErrorType::InvalidParameter;
    }

    esp_err_t err = _handle->get_blob(path().c_str(), buffer.data(), buffer.size());
    if (err != ESP_OK) {
        buffer.resize(0);
    }

    return fromPlatformError(err);
}

ErrorType File::readNonBlocking(FileOffset offset, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    if (nullptr == _handle.get()) {
        return ErrorType::InvalidParameter;
    }

    return ErrorType::NotImplemented;
}

ErrorType File::writeBlocking(FileOffset offset, const std::string &data) {
    //If the buffer doesn't have a size, you won't be able to read anything.
    assert(data.size() > 0);

    if (nullptr == _handle.get()) {
        return ErrorType::InvalidParameter;
    }

    return fromPlatformError(_handle->set_blob(path().c_str(), data.c_str(), data.size()));
}

ErrorType File::writeNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    if (nullptr == _handle.get()) {
        return ErrorType::InvalidParameter;
    }

    return ErrorType::NotImplemented;
}

ErrorType File::synchronize() {
    return fromPlatformError(_handle->commit());
}

std::string File::path() const {
    return storage().rootPrefix() + _filename;
}

ErrorType File::size(Bytes &size) const {
    return ErrorType::NotImplemented;
}