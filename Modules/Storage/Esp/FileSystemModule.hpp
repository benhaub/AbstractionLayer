#ifndef __FILE_SYSTEM_MODULE_HPP__
#define __FILE_SYSTEM_MODULE_HPP__

//AbstractionLayer
#include "FileSystemAbstraction.hpp"
#include "StorageAbstraction.hpp"
//ESP
#include "nvs_handle.hpp"
#include "esp_spiffs.h"
//C++
#include <map>

class FileSystem final : public FileSystemAbstraction {

    public:
    FileSystem(const std::array<char, 16> &name, FileSystemTypes::Implementation implementation, StorageAbstraction &storage) : FileSystemAbstraction(name, implementation, storage) {}

    static constexpr Count _MaxOpenFiles = 10;

    ErrorType mount() override;
    ErrorType unmount() override; 
    ErrorType maxPartitionSize(Bytes &size) override;
    ErrorType availablePartition(Bytes &size) override;
    ErrorType erasePartition() override;
    ErrorType open(std::string_view path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) override;
    ErrorType close(FileSystemTypes::File &file) override;
    ErrorType remove(FileSystemTypes::File &file) override;
    ErrorType readBlocking(FileSystemTypes::File &file, std::string &buffer) override; 
    ErrorType readNonBlocking(FileSystemTypes::File &file, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType writeBlocking(FileSystemTypes::File &file, const std::string &data) override; 
    ErrorType writeNonBlocking(FileSystemTypes::File &file, const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType synchronize(const FileSystemTypes::File &file) override;
    ErrorType size(FileSystemTypes::File &file) override;

    nvs_open_mode_t toEspOpenMode(FileSystemTypes::OpenMode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case FileSystemTypes::OpenMode::ReadOnly:
                return NVS_READONLY;
            case FileSystemTypes::OpenMode::ReadWriteNew:
            case FileSystemTypes::OpenMode::ReadWriteAppend:
            case FileSystemTypes::OpenMode::ReadWriteTruncate:
            case FileSystemTypes::OpenMode::WriteOnlyNew:
            case FileSystemTypes::OpenMode::WriteOnlyAppend:
            case FileSystemTypes::OpenMode::WriteOnlyTruncate:
                return NVS_READWRITE;
            default:
                error = ErrorType::InvalidParameter;
                return NVS_READONLY;
        }
    }

    private:
    static constexpr std::array<char, NVS_NS_NAME_MAX_SIZE> _nameSpace = {"nvsFlash"};
    std::unique_ptr<nvs::NVSHandle> _handle;
    std::map<std::string_view, FILE *> spiffsFiles;

    

};

#endif //__FILE_SYSTEM_MODULE_HPP__