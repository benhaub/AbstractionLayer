#ifndef __SPIFFS_HPP__
#define __SPIFFS_HPP__

//AbstractionLayer
#include "FileSystemModule.hpp"
//C++
#include <cstring>
//ESP
#include "sys/stat.h"

/**
 * @namespace Spiffs
 * @brief SPIFFS storage.
 */
namespace Spiffs {
    static std::string toPosixOpenMode(const FileSystemTypes::OpenMode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case FileSystemTypes::OpenMode::ReadOnly:
                return "r";
            case FileSystemTypes::OpenMode::ReadWriteNew:
                return "a+";
            case FileSystemTypes::OpenMode::ReadWriteAppend:
                return "r+";
            case FileSystemTypes::OpenMode::ReadWriteTruncate:
                return "rw";
            case FileSystemTypes::OpenMode::WriteOnlyNew:
                return "w";
            case FileSystemTypes::OpenMode::WriteOnlyAppend:
                return "a";
            case FileSystemTypes::OpenMode::WriteOnlyTruncate:
                return "w+";
            default:
                error = ErrorType::InvalidParameter;
                return "";
        }
    }
    static ErrorType maxPartitionSize(FileSystem &fs, Bytes &size) {
        size_t maxSize;
        ErrorType error = fromPlatformError(esp_spiffs_info(fs.nameConst().c_str(), &maxSize, NULL));
        size = maxSize;
        return error;
    }

    static ErrorType availablePartition(FileSystem &fs, Bytes &size) {
        size_t availableSize;
        ErrorType error = fromPlatformError(esp_spiffs_info(fs.nameConst().c_str(), NULL, &availableSize));
        size = availableSize;
        return error;
    }

    static ErrorType mount(FileSystem &fs) {
        ErrorType error;
        const esp_vfs_spiffs_conf_t conf = {
            //Base path has to be not null and not "/" or the undocumented ESP_ERR_INVALID_ARG is returned.
            .base_path = "/www",
            .partition_label = fs.nameConst().c_str(),
            .max_files = FileSystem::_MaxOpenFiles,
            .format_if_mount_failed = false
        };

        esp_err_t err = esp_vfs_spiffs_register(&conf);
        if (ErrorType::Success != (error = fromPlatformError(err))) {
            return error;
        }

        err = esp_spiffs_check(conf.partition_label);
        error = fromPlatformError(err);
        if (ErrorType::Success != error) {
            return error;
        }

        Bytes partitionSize, availableSize;
        error = maxPartitionSize(fs, partitionSize);
        if (ErrorType::Success != error) {
            return error;
        }
        error = availablePartition(fs, availableSize);
        if (ErrorType::Success != error) {
            return error;
        }

        if (availableSize > partitionSize) {
            return ErrorType::Failure;
        }

        return ErrorType::Success;
    }

    static ErrorType unmount(FileSystem &fs) {
        return fromPlatformError(esp_vfs_spiffs_unregister(fs.nameConst().c_str()));
    }

    static ErrorType erasePartition(FileSystem &fs) {
        return fromPlatformError(esp_spiffs_format(fs.nameConst().c_str()));
    }

    static ErrorType size(FileSystemTypes::File &file) {
        struct stat fileStat;
        if (0 != stat(file.path.c_str(), &fileStat)) {
            return fromPlatformError(errno);
        }

        file.size = fileStat.st_size;
        return ErrorType::Success;
    }

    static ErrorType close(FileSystemTypes::File &file, FILE *spiffsFile) {
        assert(nullptr != spiffsFile);

        if (0 == fclose(spiffsFile)) {
            file.isOpen = false;
            return ErrorType::Success;
        }
        else {
            return fromPlatformError(errno);
        }
    }


    static ErrorType open(const std::string &path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file, FILE *&spiffsFile) {
        ErrorType error;

        const std::string posixMode = toPosixOpenMode(mode, error);
        if (ErrorType::Success != error) {
            return error;
        }

        spiffsFile = fopen(path.c_str(), posixMode.c_str());
        if (nullptr != spiffsFile) {
            file.path.assign(path);
            file.isOpen = true;
            file.openMode = mode;
            if (ErrorType::Success != (error = size(file))) {
                close(file, spiffsFile);
            }

            file.filePointer = static_cast<FileOffset>(file.size);
            return error;
        }
        else {
            return fromPlatformError(errno);
        }
    }

    static ErrorType remove(FileSystemTypes::File &file, FILE *spiffsFile) {
        assert(nullptr != spiffsFile);

        if (file.isOpen) {
            ErrorType error = close(file, spiffsFile);
            if (ErrorType::Success != error) {
                return error;
            }
        }

        if (0 != unlink(file.path.c_str())) {
            return fromPlatformError(errno);
        }

        return ErrorType::Success;
    }

    static ErrorType readBlocking(FILE *spiffsFile, FileSystemTypes::File &file, std::string &buffer) {
        ErrorType error = ErrorType::Success;
        Bytes read = 0;

        if (0 == fseek(spiffsFile, file.filePointer, SEEK_SET)) {
            read = fread(buffer.data(), 1, buffer.size(), spiffsFile);

            if (feof(spiffsFile)) {
                error = ErrorType::EndOfFile;
            }
            else if (ferror(spiffsFile)) {
                error = fromPlatformError(errno);
            }
        }
        else {
            error = fromPlatformError(errno);
        }

        file.filePointer += static_cast<FileOffset>(read);
        buffer.resize(read);
        return error;
    }

    static ErrorType writeBlocking(FILE *spiffsFile, FileSystemTypes::File &file, const std::string &data) {
        if (0 != fseek(spiffsFile, file.filePointer, SEEK_SET)) {
            return fromPlatformError(errno);
        }

        if (0 != fwrite(data.c_str(), 1, data.size(), spiffsFile)) {
            return fromPlatformError(errno);
        }

        if (ferror(spiffsFile)) {
            return fromPlatformError(errno);
        }
        else {
            file.size += static_cast<Bytes>(data.size());
            file.filePointer += static_cast<FileOffset>(data.size());
            return ErrorType::Success;
        }
    }

    static ErrorType synchronize(FILE *spiffsFile) {
        if (0 != fflush(spiffsFile)) {
            return fromPlatformError(errno);
        }
        else {
            return ErrorType::Success;
        }
    }
}

#endif /* __SPIFFS_HPP__ */