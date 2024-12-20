#ifndef __FILE_MODULE_HPP__
#define __FILE_MODULE_HPP__

//Common
#include "Log.hpp"
//AbstractionLayer
#include "FileAbstraction.hpp"
#include "StorageAbstraction.hpp"
//C++
#include <fstream>

class File : public FileAbstraction {

    public:
    File(StorageAbstraction &storage) : FileAbstraction(storage) {}
    ~File() {
        if (nullptr != _handle.get() && _handle->is_open()) {
            close();
        }
    }

    ErrorType open(const std::string &filename, OpenMode mode) override;
    ErrorType close() override;
    /**
     * @returns ErrorType::NotAvailable
     *   Not available since std::ifstream and std::ofstream each use separate seek functions. Seek by setting and offset in the
     *   read and write functions
     * @attention Stdlib implementation not available
    */
    ErrorType seek(const FileOffset &offset) override;
    ErrorType remove() override;
    ErrorType readBlocking(const FileOffset offset, std::string &buffer) override;
    /**
     * @attention May call the callback several times before reading all the data.
     * @post The callback is called by thread that runs this function. If it is a low prioirty thread, then make sure the callback function
     *        does not a have many slow processes that might slow down a higher prioirty thread from getting the data.
    */
    ErrorType readNonBlocking(const FileOffset offset, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;
    /**
     * @attention Blocking condition 
     * @return Returns when the data has been written to the media device. 
    */
    ErrorType writeBlocking(const FileOffset offset, const std::string &data) override;
    /**
     * @attention Post condition
     * @post The data is written to the internal buffer, but will not be present on the media device until it is synchronized with the buffer.
    */
    ErrorType writeNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType synchronize() override;
    std::string path() const override;
    FileOffset offset() { return _handle->tellg(); }

    private:
    std::ios_base::openmode toStdOpenMode(OpenMode mode) {
        switch(mode) {
            case OpenMode::ReadOnly:
                //https://stackoverflow.com/questions/8255935/c-open-a-file-as-read-only
                return std::ios_base::in | std::ios_base::out | std::ios_base::app;
            case OpenMode::WriteOnlyAppend:
            case OpenMode::WriteOnlyAppendExisting:
            case OpenMode::WriteOnlyAppendNew:
                return std::ios_base::out | std::ios_base::app;
            case OpenMode::WriteOnlyTruncate:
            case OpenMode::WriteOnlyTruncateExisting:
            case OpenMode::WriteOnlyTruncateNew:
                return std::ios_base::out | std::ios_base::trunc | std::ios_base::ate;
            case OpenMode::ReadWriteAppend:
            case OpenMode::ReadWriteAppendExisting:
            case OpenMode::ReadWriteAppendNew:
                return std::ios_base::in | std::ios_base::out | std::ios_base::app;
            case OpenMode::ReadWriteTruncate:
                return std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
            default:
                PLT_LOGW(TAG, "Unknown open mode: %u", mode);
        }

        return std::ios_base::in;
    }

    inline constexpr bool canReadFromFile() {
        if (_mode == OpenMode::WriteOnlyAppend || _mode == OpenMode::WriteOnlyTruncate) {
            return false;
        }

        if (nullptr == _handle.get()) {
            return false;
        }

        if (!_handle->is_open()) {
            return false;
        }

        return true;
    }
    inline constexpr bool canWriteToFile() {
        if (_mode == OpenMode::ReadOnly) {
            return false;
        }

        return true;
    }
    inline bool isOpen() {
        if (nullptr != _handle.get() && _handle->is_open()) {
            return true;
        }

        return false;
    }

    std::unique_ptr<std::fstream> _handle;
    Bytes currentFileSize = 0;
};

#endif //__FILE_MODULE_HPP__
