#include "FileModule.hpp"
#include "StorageAbstraction.hpp"
#include "OperatingSystemModule.hpp"

ErrorType File::open(const std::string &filename, OpenMode mode) {
    ErrorType error = ErrorType::Failure;
    bool openDone = false;

    auto openCallback = [&]() -> ErrorType {
        _u32 token = 0;
        //Comes from the Host driver documentation. Not sure how to query for this or if there is a constant somewhere.
        constexpr Bytes maxFileSize = 62.5f * 1024;

        Bytes maxSize = SL_FS_CREATE_MAX_SIZE(filename.size());
        assert(maxSize <= maxFileSize);

        _deviceFileHandle = sl_FsOpen(reinterpret_cast<const _u8 *>(filename.c_str()), toCc32xxAccessMode(mode, error) | maxSize, &token);
        if (_deviceFileHandle != SL_FS_OK) {
            openDone = true;
            error = fromPlatformError(_deviceFileHandle);
            return error;
        }

        _filename.assign(filename);
        _mode = mode;

        error = ErrorType::Success;
        openDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(openCallback));
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
    ErrorType error = ErrorType::Failure;
    bool closeDone = false;

    auto closeCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsClose(_deviceFileHandle, NULL, NULL, 0);
        if (SL_FS_OK != retval) {
            closeDone = true;
            error = fromPlatformError(retval);
            return error;
        }

        _deviceFileHandle = -1;
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
    ErrorType error = ErrorType::Failure;
    bool removeDone = false;

    auto removeCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsDel(reinterpret_cast<const unsigned char *>(_filename.c_str()), 0);
        if (SL_FS_OK != retval) {
            removeDone = true;
            error = fromPlatformError(retval);
            return error;
        }

        _filename.clear();
        _mode = OpenMode::Unknown;

        removeDone = true;
        error =  ErrorType::Success;
        return error;
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
    ErrorType error = ErrorType::Failure;
    bool readDone = false;

    auto readCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsRead(_deviceFileHandle, offset, reinterpret_cast<_u8 *>(buffer.data()), buffer.size());
        if (SL_FS_OK != retval) {
            readDone = true;
            error = fromPlatformError(retval);
            return error;
        }

        readDone = true;
        error = ErrorType::Success;
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
    auto readCallback = [&]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        _i32 retval = sl_FsRead(_deviceFileHandle, offset, reinterpret_cast<_u8 *>(buffer->data()), buffer->size());
        if (SL_FS_OK != retval) {
            error = fromPlatformError(retval);
        }

        callback(error, buffer);
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(readCallback));
    return storage().addEvent(event);
}

ErrorType File::writeBlocking(const FileOffset offset, const std::string &data) {
    ErrorType error = ErrorType::Failure;
    bool writeDone = false;

    //SimpleLink write function does not accept a constant string.
    std::string dataToWrite = const_cast<std::string &>(data);

    auto writeCallback = [&]() -> ErrorType {
        _i32 retval = sl_FsWrite(_deviceFileHandle, offset, reinterpret_cast<_u8 *>(dataToWrite.data()), dataToWrite.size());
        if (SL_FS_OK != retval) {
            writeDone = true;
            error = fromPlatformError(retval);
            return error;
        }

        writeDone = true;
        error = ErrorType::Success;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(writeCallback));
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
    auto writeCallback = [&]() -> ErrorType {
        ErrorType error = ErrorType::Failure;
        _i32 retval = sl_FsWrite(_deviceFileHandle, 0, reinterpret_cast<_u8 *>(data->data()), data->size());
        if (SL_FS_OK != retval) {
            error = fromPlatformError(retval);
        }

        callback(error, data->size());
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(writeCallback));
    return storage().addEvent(event);
}

ErrorType File::synchronize() {
    return ErrorType::NotAvailable;
}

std::string File::path() const {
    return _filename;
}

ErrorType File::size(Bytes &size) const {
    return ErrorType::NotAvailable;
}