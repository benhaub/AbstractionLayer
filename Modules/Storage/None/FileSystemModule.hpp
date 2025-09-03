#ifndef __FILE_SYSTEM_MODULE_HPP__
#define __FILE_SYSTEM_MODULE_HPP__

//AbstractionLayer
#include "FileSystemAbstraction.hpp"
#include "StorageAbstraction.hpp"
//C++
#include <map>
#include <fstream>

class FileSystem final : public FileSystemAbstraction {

    public:
    FileSystem(const FileSystemTypes::Params<FileSystemTypes::PartitionName, FileSystemTypes::Implementation> &params, StorageAbstraction &storage) : FileSystemAbstraction(params, storage) {}
    ~FileSystem() = default;

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
};

#endif //__FILE_SYSTEM_MODULE_HPP__