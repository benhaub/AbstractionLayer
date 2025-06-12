#ifndef __KEY_VALUE_HPP
#define __KEY_VALUE_HPP

//AbstractionLayer
#include "FileSystemModule.hpp"
//ESP
#include "nvs_flash.h"

/**
 * @namespace KeyValue
 * @brief Key Value NVS storage.
 */
namespace KeyValue {
    ErrorType mount(FileSystem &fs, const char nameSpace[], std::unique_ptr<nvs::NVSHandle> &_handle) {
        esp_err_t err = nvs_flash_init_partition(&fs.name());
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            return fromPlatformError(err);
        }

        //This is a little different than how some other file systems would work. We only need to open once.
        //When we open the handle we get access to all the files (key/values) in flash. We actually do not want
        //to close until we unmount.
        _handle = nvs::open_nvs_handle_from_partition(&fs.name(), nameSpace, NVS_READWRITE, &err);

        return fromPlatformError(err);
    }

    ErrorType unmount(FileSystem &fs, std::unique_ptr<nvs::NVSHandle> &_handle) {
        esp_err_t err = nvs_flash_deinit_partition(&fs.name());
        _handle.release();
        return fromPlatformError(err);
    }

    ErrorType maxPartitionSize(FileSystem &fs, Bytes &size) {
        nvs_stats_t stats;
        esp_err_t err;

        err = nvs_get_stats(&fs.name(), &stats);

        //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#structure-of-a-page
        //One entry is 32 bytes.
        size = (stats.total_entries * 32);

        return fromPlatformError(err);
    }

    ErrorType availablePartition(FileSystem &fs, Bytes &size) {
        nvs_stats_t stats;
        esp_err_t err;

        err = nvs_get_stats(&fs.name(), &stats);

        //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#structure-of-a-page
        //One entry is 32 bytes.
        size = (stats.free_entries * 32);

        return fromPlatformError(err);
    }

    ErrorType erasePartition(FileSystem &fs) {
        return fromPlatformError(nvs_flash_erase_partition(&fs.name()));
    }

    ErrorType open(FileSystem &fs, const std::string &path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file ) {
        assert(path.length() > 0);
        ErrorType error = ErrorType::Success;
        file.isOpen = false;

        //Use the size call to get the size and will also tell use if the file (key) exists.
        error = fs.size(file);

        if (fs.fileMustExist(mode) && ErrorType::FileNotFound == error) {
            return ErrorType::FileNotFound;
        }
        else if (fs.fileMustNotExist(mode) && ErrorType::Success == error) {
            return ErrorType::FileExists;
        }
        else if (fs.fileShouldBeTruncated(mode)) {
            file.filePointer = 0;
        }
        else {
            //The file is either new or it should be appended to.
            file.filePointer = file.size;
        }

        file.path.assign(path);
        file.isOpen = true;
        file.openMode = mode;


        return ErrorType::Success;
    }

    ErrorType synchronize(std::unique_ptr<nvs::NVSHandle> &_handle) {
        return fromPlatformError(_handle->commit());
    }

    ErrorType close(std::unique_ptr<nvs::NVSHandle> &_handle, FileSystemTypes::File &file) {
        ErrorType error = synchronize(_handle);
        if (ErrorType::Success == error) {
            file.isOpen = false;
            file.openMode = FileSystemTypes::OpenMode::Unknown;
        }

        return error;
    }

    ErrorType remove(std::unique_ptr<nvs::NVSHandle> &_handle, FileSystemTypes::File &file) {
        ErrorType error = ErrorType::Failure;

        if (file.isOpen) {
            if (ErrorType::Success != (error = close(_handle, file))) {
                return error;
            }
        }

        error = fromPlatformError(_handle->erase_item(file.path.c_str()));

        return error;
    }

    ErrorType readBlocking(std::unique_ptr<nvs::NVSHandle> &_handle, const FileSystemTypes::File &file, std::string &buffer) {
        //If the buffer doesn't have a size, you won't be able to read anything.
        assert(buffer.size() > 0);

        if (nullptr == _handle.get()) {
            return ErrorType::InvalidParameter;
        }

        esp_err_t err = _handle->get_blob(file.path.c_str(), buffer.data(), buffer.size());
        if (err != ESP_OK) {
            buffer.resize(0);
        }

        return fromPlatformError(err);
    }

    ErrorType writeBlocking(std::unique_ptr<nvs::NVSHandle> &_handle, const FileSystemTypes::File &file, const std::string &data) {
        ErrorType error = ErrorType::InvalidParameter;
        if (nullptr != _handle.get()) {
            if (file.path.length() < NVS_KEY_NAME_MAX_SIZE) {
                error = fromPlatformError(_handle->set_blob(file.path.c_str(), data.c_str(), data.size()));
            }
        }

        return error;
    }

    ErrorType size(std::unique_ptr<nvs::NVSHandle> &_handle, FileSystemTypes::File &file) {
        size_t size;
        ErrorType error = fromPlatformError(_handle->get_item_size(nvs::ItemType::ANY, file.path.c_str(), size));
        file.size = size;
        return error;
    }
};

#endif /* __KEY_VALUE_HPP */