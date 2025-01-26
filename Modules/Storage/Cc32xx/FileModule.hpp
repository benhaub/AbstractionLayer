#ifndef __FILE_MODULE_HPP__
#define __FILE_MODULE_HPP__

//AbstractionLayer
#include "FileAbstraction.hpp"

class File : public FileAbstraction {

    public:
    File(StorageAbstraction &storage) : FileAbstraction(storage) {}
    ~File() {
        close();
    }

    ErrorType open(const std::string &filename, OpenMode mode) override;
    ErrorType close() override;
    ErrorType seek(const FileOffset &offset) override;
    ErrorType remove() override;
    ErrorType readBlocking(const FileOffset offset, std::string &buffer) override;
    ErrorType readNonBlocking(const FileOffset offset, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;
    ErrorType writeBlocking(const FileOffset offset, const std::string &data) override;
    ErrorType writeNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType synchronize() override;
    std::string path() const override;
};

#endif //__FILE_MODULE_HPP__