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
    FileSystem(StorageAbstraction &storage, FileSystemTypes::Implementation implementation, FileSystemTypes::PartitionName partitionName) : FileSystemAbstraction(storage, implementation, partitionName) {}

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
    ErrorType writeBlocking(FileSystemTypes::File &file, std::string_view data) override; 
    ErrorType writeNonBlocking(FileSystemTypes::File &file, const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType synchronize(const FileSystemTypes::File &file) override;
    ErrorType size(FileSystemTypes::File &file) override;

    private:
    std::map<std::string_view, std::fstream> openFiles;

    std::ios_base::openmode toStdOpenMode(FileSystemTypes::OpenMode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch(mode) {
            //https://stackoverflow.com/questions/8255935/c-open-a-file-as-read-only
            case FileSystemTypes::OpenMode::ReadOnly:
                return std::ios_base::in | std::ios_base::out | std::ios_base::app;
            case FileSystemTypes::OpenMode::WriteOnlyNew:
            case FileSystemTypes::OpenMode::WriteOnlyAppend:
                return std::ios_base::out | std::ios_base::app;
            case FileSystemTypes::OpenMode::WriteOnlyTruncate:
                return std::ios_base::out | std::ios_base::trunc | std::ios_base::ate;
            case FileSystemTypes::OpenMode::ReadWriteNew:
            case FileSystemTypes::OpenMode::ReadWriteAppend:
                return std::ios_base::in | std::ios_base::out | std::ios_base::app;
            case FileSystemTypes::OpenMode::ReadWriteTruncate:
                return std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
            default:
                error = ErrorType::InvalidParameter;
        }

        return std::ios_base::in;
    }

    inline bool isOpen(const FileSystemTypes::File &file) {
        if (openFiles.contains(file.path->c_str()) && openFiles[file.path->c_str()].is_open()) {
                return true;
        }

        return false;
    }
};

#endif //__FILE_SYSTEM_MODULE_HPP__