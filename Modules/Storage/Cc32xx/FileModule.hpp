#ifndef __FILE_MODULE_HPP__
#define __FILE_MODULE_HPP__

//AbstractionLayer
#include "FileAbstraction.hpp"
//TI driverlib
#include "ti/drivers/net/wifi/fs.h"

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

    private:
    _i32 _deviceFileHandle = -1;

    _u32 toCc32xxAccessMode(const OpenMode mode, ErrorType &error) {
        error = ErrorType::Success;
        switch (mode) {
            case OpenMode::ReadOnly:
                return SL_FS_READ;
            case OpenMode::ReadWriteAppend:
                return SL_FS_READ | SL_FS_WRITE;
            case OpenMode::ReadWriteAppendNew:
                return SL_FS_READ | SL_FS_WRITE | SL_FS_CREATE;
            case OpenMode::ReadWriteTruncate:
                return SL_FS_READ | SL_FS_WRITE | SL_FS_OVERWRITE;
            case OpenMode::ReadWriteTruncateNew:
                return SL_FS_READ | SL_FS_WRITE | SL_FS_CREATE | SL_FS_OVERWRITE;
            case OpenMode::WriteOnlyAppend:
                return SL_FS_WRITE;
            case OpenMode::WriteOnlyAppendNew:
                return SL_FS_WRITE | SL_FS_CREATE;
            case OpenMode::WriteOnlyTruncate:
                return SL_FS_WRITE | SL_FS_OVERWRITE;
            default:
                error = ErrorType::NotAvailable;
                return SL_FS_READ;
        }
    }
};

#endif //__FILE_MODULE_HPP__