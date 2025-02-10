#ifndef __FILE_MODULE_HPP__
#define __FILE_MODULE_HPP__

//AbstractionLayer
#include "FileSystemAbstraction.hpp"
//TI driverlib
#include "ti/drivers/net/wifi/fs.h"

class FileSystem : public FileSystemAbstraction {

    public:
    FileSystem(std::string name, FileSystemTypes::Implementation implementation, StorageAbstraction &storage) : FileSystemAbstraction(name, implementation, storage) {}
    ~FileSystem() = default;

    ErrorType mount() override;
    ErrorType unmount() override; 
    ErrorType maxPartitionSize(Bytes &size) override;
    ErrorType availablePartition(Bytes &size) override;
    ErrorType erasePartition() override;
    ErrorType open(const std::string &path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) override;
    ErrorType close(FileSystemTypes::File &file) override;
    ErrorType remove(FileSystemTypes::File &file) override;
    ErrorType readBlocking(FileSystemTypes::File &file, std::string &buffer) override; 
    ErrorType readNonBlocking(FileSystemTypes::File &file, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType writeBlocking(FileSystemTypes::File &file, const std::string &data) override; 
    ErrorType writeNonBlocking(FileSystemTypes::File &file, const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType synchronize(const FileSystemTypes::File &file) override;
    ErrorType size(FileSystemTypes::File &file) override;

    private:
    _i32 _deviceFileHandle = -1;

    _u32 toCc32xxAccessMode(const FileSystemTypes::OpenMode mode, ErrorType &error) {
        error = ErrorType::Success;
        switch (mode) {
            case FileSystemTypes::OpenMode::ReadOnly:
                return SL_FS_READ;
            case FileSystemTypes::OpenMode::ReadWriteAppend:
                return SL_FS_READ | SL_FS_WRITE;
            case FileSystemTypes::OpenMode::ReadWriteNew:
                return SL_FS_READ | SL_FS_WRITE | SL_FS_CREATE;
            case FileSystemTypes::OpenMode::ReadWriteTruncate:
                return SL_FS_READ | SL_FS_WRITE | SL_FS_OVERWRITE;
            case FileSystemTypes::OpenMode::WriteOnlyAppend:
                return SL_FS_WRITE;
            case FileSystemTypes::OpenMode::WriteOnlyNew:
                return SL_FS_WRITE | SL_FS_CREATE;
            case FileSystemTypes::OpenMode::WriteOnlyTruncate:
                return SL_FS_WRITE | SL_FS_OVERWRITE;
            default:
                error = ErrorType::NotAvailable;
                return SL_FS_READ;
        }
    }
};

#endif //__FILE_MODULE_HPP__