/***************************************************************************//**
* @author   Ben Haubrich
* @file     FileAbstraction.hpp
* @details  General file operations that can implement any file system.
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __FILE_ABSTRACTION_HPP__
#define __FILE_ABSTRACTION_HPP__

//Foundation
#include "Error.hpp"
#include "Types.hpp"
//C++
#include <string> //For std::string
#include <functional> //For std::function
#include <memory> //For std::shared_ptr

class StorageAbstraction;

/**
 * @enum OpenMode
 * @brief File open modes.
*/
enum class OpenMode : uint8_t {
    Unknown = 0,                           ///< Unknown open mode.
    ReadOnly = 1,                          ///< File can be read, but not written.
    WriteOnlyAppend = 2,                   ///< Written data is added on to the end of the file.
    WriteOnlyAppendExisting = 3,           ///< Written data is added on to the end of the file only if it already exists.
    WriteOnlyAppendNew = 4,                ///< Written data is added on to the end of the file only if it doesn't already exist.
    WriteOnlyTruncate = 5,                 ///< Erases all the existing data upon opening.
    WriteOnlyTruncateExisting = 6,         ///< Erases all the existing data upon opening if it already exists.
    WriteOnlyTruncateNew = 7,              ///< Erases all the existing data upon opening if it doesn't already exist.
    ReadWriteAppend = 8,                   ///< Read the file and write by appending.
    ReadWriteAppendExisting = 9,           ///< Read the file and write by appending if it already exists.
    ReadWriteAppendNew = 10,               ///< Read the file and write by appending if it doesn't already exist.
    ReadWriteTruncate = 11,                ///< Read the file and write by truncating first.
    ReadWriteTruncateExisting = 12,        ///< Read the file and write by truncating first if it already exists.
    ReadWriteTruncateNew = 13              ///< Read the file and write by truncating first if it doesn't already exist.
};

/**
 * @class FileAbstraction
 * @brief General file operations that can implement any file system.
*/
class FileAbstraction {

    public:
    /**
     * @brief Constructor.
     * @param[in] storage underlying storage implemenation
    */
    FileAbstraction(StorageAbstraction &storage) : _storage(&storage) {}
    /**
     * @brief Destructor.
     * @post The implementation may close the file upon destruction.
    */
    ~FileAbstraction() = default;

    /**
     * @brief Opens a file.
     * @param[in] path The path to the file.
     * @param[in] mode The OpenMode.
     * @returns ErrorType::Success if the file was opened.
     * @returns ErrorType::PrerequisitesNotMet if the file was already open since the OpenMode can not be changed.
     * @returns ErrorType::PrerequisitesNotMet if the storage has not been initialized
     * @returns ErrorType::Failure if could not be opened for any other reason
    */
    virtual ErrorType open(const std::string &path, OpenMode mode) = 0;
    /**
     * @brief Closes a file.
     * @returns ErrorType::Success if the file was closed.
     * @returns ErrorType::Failure if the file could not be synchronized before closing.
     * @returns ErrorType::Failure if could not be closed for any other reason
    */
    virtual ErrorType close() = 0;
    /**
     * @brief Seeks to an offset in a file.
     * @param[in] offset The offset to seek to.
     * @returns ErrorType::Success if the offset was set.
     * @returns ErrorType::FileNotFound if the file does not exist
     * @returns ErrorType::PrerequisitesNotMet if the file is not open.
     * @returns ErrorType::Failure otherwise
    */
    virtual ErrorType seek(const FileOffset &offset) = 0;
    /**
     * @brief Removes a file.
     * @returns ErrorType::Success if the file was removed.
     * @returns The error returned from FileAbstraction::close if the file could not be closed.
     * @returns ErrorType::Failure otherwise.
    */
    virtual ErrorType remove() = 0;
    /**
     * @brief Reads data from a file.
     * 
     * This is a blocking operation. Control will not return until the file contents are completely handed off to the underlying driver.
     * @param[in] offset The offset to start reading from.
     * @param[out] buffer The buffer to read into.
     * @pre The number of bytes to read is equal to the size of the buffer (use std::string::resize()).
     * @pre The file must be open with a mode that supports reads.
     * @returns ErrorType::Success if the data was read.
     * @returns ErrorType::PrequsistesNotMet if the mode does not allow a write.
     * @returns ErrorType::PrequsistesNotMet if the file has not be opened or has been closed.
    */
    virtual ErrorType readBlocking(FileOffset offset, std::string &buffer) = 0; 
    /**
     * @brief Reads data from a file.
     * This is a non-blocking operating. Control will return as soon as the required data is passed to the calling thread
     * In other words, the file will have not actually been read yet.
     * This is a blocking operation. Control will not return until the file contents are completely handed off to the underlying driver.
     * @param[in] offset The offset to start reading from.
     * @param[out] buffer The buffer to read into.
     * @param[in] callback The callback to invoke when the write is complete. nullptr by default
     * @pre The number of bytes to read is equal to the size of the buffer (use std::string::resize()).
     * @pre The file must be open with a mode that supports reads.
     * @returns ErrorType::Success if the data was read.
     * @returns ErrorType::PrequsistesNotMet if the mode does not allow a write.
     * @returns ErrorType::PrequsistesNotMet if the file has not be opened or has been closed.
     * @post The number of bytes read is represented by the size of data (use std::string::size())
    */
    virtual ErrorType readNonBlocking(FileOffset offset, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) = 0;
    /**
     * @brief Writes data to a file.
     * This is a blocking operation. Control will not return until the file contents are completely handed off to the underlying driver.
     * @param[in] offset The offset to start writing to.
     * @param[in] data The data to write.
     * @pre The file must be open with a mode that supports writes
     * @pre The number of bytes to write is equal to the size of the data (use std::string::resize()).
     * @returns ErrorType::Success if the data was written and synchronized.
     * @returns ErrorType::PrequsistesNotMet if the mode does not allow a write.
     * @returns ErrorType::PrequsistesNotMet if the file has not be opened or has been closed.
     * @returns ErrorType::Failure if the data could not be written for any other reason.
     * @post The number of bytes read is represented by the size of data (use std::string::size())
    */
    virtual ErrorType writeBlocking(FileOffset offset, const std::string &data) = 0; 
    /**
     * @brief Writes data to a file.
     * This is a non-blocking operating. Control will return as soon as the required data is passed to the calling thread
     * @param[in] data The data to write.
     * @param[in] callback [in] The callback to invoke when the write is complete. nullptr by default
     * @returns ErrorType::Success if the data was written and synchronized.
     * @returns ErrorType::PrequsistesNotMet if the mode does not allow a write.
     * @returns ErrorType::PrequsistesNotMet if the file has not be opened or has been closed.
     * @returns ErrorType::Failure if the data could not be written for any other reason.
     * @post The number of bytes read is represented by the size of data (use std::string::size())
    */
    virtual ErrorType writeNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) = 0;
    /**
     * @brief Writes data from the internal file buffer to the media
     * @returns ErrorType::Success if the data was written.
     * @returns ErrorType::Failure if the data was not written.
    */
    virtual ErrorType synchronize() = 0;
    /**
    * @brief Get the path.
    * @returns The path of the file. Pure virtual because StorageAbstraction is forward declared.
    */
    virtual std::string path() const = 0;
    /**
     * @brief Get the size of the file
     * @pre The file is open
     * @return ErrorType::Success if the file size was returned
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType size(Bytes &size) const = 0;

    /**
    * @brief Get the api to the underlying storage
    * @returns The underlying storage that the file will write the bytes too.
    */
    StorageAbstraction &storage() const { return *_storage; }
    /**
    * @brief Get the mode
    * @returns The OpenMode of the file.
    */
    OpenMode mode() const { return _mode; }

    protected:
    /// @brief The name of the file.
    std::string _filename;
    /// @brief The mode to open the file in.
    OpenMode _mode;
    /// @brief Tag for logging
    static constexpr char TAG[] = "File";
    /// @brief The underlying storage medium implementation.
    StorageAbstraction *_storage;
};

#endif //__FILE_ABSTRACTION_HPP__