/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   FileSystemAbstraction.hpp
* @details Interface for any file system
* @ingroup Abstractions
*******************************************************************************/
#ifndef __FILE_SYSTEM_ABSTRACTION_HPP__
#define __FILE_SYSTEM_ABSTRACTION_HPP__

//AbstractionLayer
#include "Log.hpp"
#include "StaticString.hpp"
//C++
#include <memory>

class StorageAbstraction;

/**
 * @namespace FileSystemTypes
 * @brief Types related to file system
 */
namespace FileSystemTypes {

    static constexpr Count PartitionNameLength = 16;
    using PartitionName = std::array<char, PartitionNameLength>;

    /**
     * @struct Status
     * @brief The status of the file system.
    */
    struct Status {
        bool mounted;                                        ///< True if the file system is mounted, false otherwise.
        Percent freeSpace;                                   ///< The free space on the file system.
        Count openedFiles;                                   ///< The number of files opened on the file system.

        constexpr Status() : mounted(false), freeSpace(0), openedFiles(0) {}
    };

    /**
     * @enum Implementation
     * @brief The implementation of the file system
     * @details I'm not sure if there is a name for this. Wikipedia just refers to all of these as FileSystems but you need a different name for
     *        the exact type (format?) if you collectively refer to all of them as a FileSystem. Wikipedia seemed to just land on the term
     *        implementation. https://en.wikipedia.org/wiki/File_system#Implementations
     * @note A platform does not have to strictly honour the implementation type given. As a user of this abstraction, all you need to do is read
     *       and write files. If your system has multiple partitions on one medium and each partition has it's own filesystem with a unique API then
     *       this can be used to direct calls to the appropriate code for data retrieval and storage. If it just has one partition and all of the data
     *       is available on it then you could ignore the implementation.
     */
    enum class Implementation : uint8_t {
        Unknown = 0, ///< Unknown   
        Fat,         ///< File Allocation Table
        Spiffs,      ///< SPI Flash File Storage
        KeyValue,    ///< Key Value Storage
        Ntfs,        ///< Windows NT File System
        Apfs,        ///< Apple File System
        Ext4,        ///< Ext4 File System (Linux)
        Eeprom,      ///< EEPROM
        Otp          ///< One Time Programmable
    };

    /**
     * @enum OpenMode
     * @brief File open modes.
    */
    enum class OpenMode : uint8_t {
        Unknown = 0,             ///< Unknown open mode.
        ReadOnly,                ///< Open the file for reading only.
        WriteOnlyNew,            ///< Written data is added on to the end of a file that doesn't already exists.
        WriteOnlyAppend,         ///< Write in addition to any existing data on the end of the file.
        WriteOnlyTruncate,       ///< Truncate first before only writing to the file.
        ReadWriteNew,            ///< Read and/or write a file that doesn't already exist.
        ReadWriteAppend,         ///< Write in addition to any existing data and read from an existing file.
        ReadWriteTruncate        ///< Truncate before reading or writing the file.
    };

    /**
     * @struct File
     * @brief A resource for recording data on the storage device.
     */
    struct File {
        StaticString::Container path; ///< The name of the file.
        Bytes size;             ///< The size of the file.
        FileOffset filePointer; ///< Current offset into the file.
        OpenMode openMode;      ///< The open mode of the file.
        bool isOpen;            ///< True if the file is open, false otherwise.
    };
}

/**
 * @class FileSystemAbstraction
 * @brief An abstraction for any file system.
 */
class FileSystemAbstraction {

    public:
    /**
     * @brief Constructor
     * @param storage The storage medium the file system is on.
     */
    FileSystemAbstraction(StorageAbstraction &storage, FileSystemTypes::Implementation implementation, FileSystemTypes::PartitionName partitionName) :
                          _storage(storage), _implementation(implementation), _partitionName(partitionName) {}
    /// @brief Destructor
    virtual ~FileSystemAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "FileSystem";

    /**
     * @brief Print the status of the file system
     */
    void printStatus() {
        PLT_LOGI(TAG, "<FileSystem%s> <Implementation:&u, Mounted:%u, Open Files:%u, Free (%%):%.1f> <Omit, Pie, Stairs, Line>",
                      partitionName().data(), implementation(), status().mounted, status().openedFiles, status().freeSpace);
    }

    /**
     * @brief Mount the file system
     * @returns ErrorType::Success if the file system was mounted
     * @returns ErrorType::Failure otherwise
     * @post FileSystemTypes::Status::mounted is set to true when this call returns with ErrorType::Success
     * @pre configure must be called first
     */
    virtual ErrorType mount() = 0;
    /**
     * @brief Unmount the file system
     * @returns ErrorType::Success if the file system was unmounted
     * @returns ErrorType::Failure otherwise
     * @post FileSystemTypes::Status::mounted is set to false when this call returns with anything other than ErrorType::Success
     */
    virtual ErrorType unmount() = 0;
    /**
     * @brief Get the size of the partition
     * @pre mount must be called first
     * @param[out] size The size of the partition
     * @returns ErrorType::Success if the size was retrieved
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType maxPartitionSize(Bytes &size) = 0;
    /**
     * @brief Get the size of the partition
     * @pre mount must be called first
     * @param[out] size The size of the partition
     * @returns ErrorType::Success if the size was retrieved
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType availablePartition(Bytes &size) = 0;
    /**
     * @brief erase the specified partition
     * @returns ErrorType::Success if the partition was erased
     * @returns ErrorType::Failure otherwise
    */
    virtual ErrorType erasePartition() = 0;
    /**
     * @brief Opens a file.
     * @param[in] path The path to the file.
     * @param[in] mode The OpenMode.
     * @param[out] file The opened file that was opened. Valid only if the return value is ErrorType::Success.
     * @returns ErrorType::Success if the file was opened.
     * @returns ErrorType::Failure if could not be opened for any other reason
    */
    virtual ErrorType open(std::string_view path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) = 0;
    /**
     * @brief Closes a file.
     * @param[in] file The file to close.
     * @returns ErrorType::Success if the file was closed.
     * @returns ErrorType::Failure if the file could not be synchronized before closing.
     * @returns ErrorType::Failure if could not be closed for any other reason
    */
    virtual ErrorType close(FileSystemTypes::File &file) = 0;
    /**
     * @brief Removes a file.
     * @param[in] file The file to remove.
     * @returns ErrorType::Success if the file was removed.
     * @returns The error returned from FileAbstraction::close if the file could not be closed.
     * @returns ErrorType::Failure otherwise.
    */
    virtual ErrorType remove(FileSystemTypes::File &file) = 0;
    /**
     * @brief Reads data from a file.
     * @param[in] file The file to read from.
     * @param[out] buffer The buffer to read into.
     * @pre The number of bytes to read is equal to the size of the buffer (use std::string::resize()).
     * @pre The file must be open with a mode that supports reads.
     * @returns ErrorType::Success if the data was read.
     * @returns ErrorType::PrerequisitesNotMet if the mode does not allow a read.
     * @returns ErrorType::PrerequisitesNotMet if the file has not be opened or has been closed.
    */
    virtual ErrorType readBlocking(FileSystemTypes::File &file, std::string &buffer) = 0; 
    /**
     * @brief Reads data from a file.
     * This is a non-blocking operating. Control will return as soon as the required data is passed to the calling thread
     * In other words, the file will have not actually been read yet.
     * This is a blocking operation. Control will not return until the file contents are completely handed off to the underlying driver.
     * @param[in] file The file to read from.
     * @param[out] buffer The buffer to read into.
     * @param[in] callback The callback to invoke when the write is complete. nullptr by default
     * @pre The number of bytes to read is equal to the size of the buffer (use std::string::resize()).
     * @pre The file must be open with a mode that supports reads.
     * @returns ErrorType::Success if the data was read.
     * @returns ErrorType::PrerequisitesNotMet if the mode does not allow a read.
     * @returns ErrorType::PrerequisitesNotMet if the file has not be opened or has been closed.
    */
    virtual ErrorType readNonBlocking(FileSystemTypes::File &file, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) = 0;
    /**
     * @brief Writes data to a file.
     * This is a blocking operation. Control will not return until the file contents are completely handed off to the underlying driver.
     * @param[in] file The file to write to.
     * @param[in] data The data to write.
     * @pre The file must be open with a mode that supports writes
     * @pre The number of bytes to write is equal to the size of the data (use std::string::resize()).
     * @returns ErrorType::Success if the data was written and synchronized.
     * @returns ErrorType::PrerequisitesNotMet if the mode does not allow a write.
     * @returns ErrorType::PrerequisitesNotMet if the file has not be opened or has been closed.
     * @returns ErrorType::Failure if the data could not be written for any other reason.
     * @post The number of bytes read is represented by the size of data (use std::string::size())
    */
    virtual ErrorType writeBlocking(FileSystemTypes::File &file, const std::string &data) = 0; 
    /**
     * @brief Writes data to a file.
     * This is a non-blocking operating. Control will return as soon as the required data is passed to the calling thread
     * @param[in] file The file to write to.
     * @param[in] data The data to write.
     * @param[in] callback The callback to invoke when the write is complete. nullptr by default
     * @returns ErrorType::Success if the data was written and synchronized.
     * @returns ErrorType::PrerequisitesNotMet if the mode does not allow a write.
     * @returns ErrorType::PrerequisitesNotMet if the file has not be opened or has been closed.
     * @returns ErrorType::Failure if the data could not be written for any other reason.
    */
    virtual ErrorType writeNonBlocking(FileSystemTypes::File &file, const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief Writes data from the internal file buffer to the media
     * @param[in] file The file to write to.
     * @returns ErrorType::Success if the data was written.
     * @returns ErrorType::Failure if the data was not written.
    */
    virtual ErrorType synchronize(const FileSystemTypes::File &file) = 0;
    /**
     * @brief Get the size of a file on storage
     * @param[in] file The file to get the size of.
     * @returns The size of the file.
    */
    virtual ErrorType size(FileSystemTypes::File &file) = 0;

    /// @brief Get the mount prefix as a constant reference
    std::string_view mountPrefix() const { return _mountPrefix->c_str(); }
    /// @brief Get the status of the file system as a constant reference
    const FileSystemTypes::Status &status() {
        //Bytes is a uint32_t so on systems with more than 4GiB of storage, this will overflow.
        //which is why you might see percentages greater than 100%. Bytes is used a lot in the abstraction layer
        //and since the primary goal of it is to develop applications for embedded systems it's not going to be that
        //useful to change the type of Bytes to uint64_t and then force the CPU to do 2 accesses to memory
        //to get values.
        Bytes available;
        availablePartition(available);
        Bytes maxSize;
        maxPartitionSize(maxSize);
        if (maxSize == 0) {
            _status.freeSpace = 0;
        }
        else {
            _status.freeSpace = ((float)available / maxSize) * 100.0f;
        }

        return _status;
    }

    /// @brief Get the name of the file system as a constant reference
    const FileSystemTypes::PartitionName partitionName() const { return _partitionName; }
    /// @brief Get the implementation of the file system as a constant reference
    FileSystemTypes::Implementation implementation() const { return _implementation; }

    /**
     * @brief Check if a file must exist
     * @param mode The open mode
     * @return True if the file must exist, false otherwise
     */
    constexpr inline bool fileMustExist(FileSystemTypes::OpenMode mode) {
        return (FileSystemTypes::OpenMode::ReadWriteAppend == mode ||
                FileSystemTypes::OpenMode::ReadWriteTruncate == mode ||
                FileSystemTypes::OpenMode::WriteOnlyAppend == mode ||
                FileSystemTypes::OpenMode::WriteOnlyTruncate == mode);
    }
    /**
     * @brief Check if a file must not exist
     * @param mode The open mode
     * @return True if the file must not exist, false otherwise
     */
    constexpr inline bool fileMustNotExist(FileSystemTypes::OpenMode mode) {
        return (FileSystemTypes::OpenMode::ReadWriteNew == mode ||
                FileSystemTypes::OpenMode::WriteOnlyNew == mode);
    }
    /**
     * @brief Check if a file should be truncated
     * @param mode The open mode
     * @return True if the file should be truncated, false otherwise
     */
    constexpr inline bool fileShouldBeTruncated(FileSystemTypes::OpenMode mode) {
        return (FileSystemTypes::OpenMode::ReadWriteTruncate == mode ||
                FileSystemTypes::OpenMode::WriteOnlyTruncate == mode);
    }
    /**
     * @brief Check if a file can be read from
     * @param mode the open mode of the file
     * @return True if the file can be read from, false otherwise
     */
    constexpr inline bool canReadFromFile(FileSystemTypes::OpenMode mode) {
        return mode == FileSystemTypes::OpenMode::ReadOnly ||
               mode == FileSystemTypes::OpenMode::ReadWriteNew ||
               mode == FileSystemTypes::OpenMode::ReadWriteAppend ||
               mode == FileSystemTypes::OpenMode::ReadWriteTruncate;
    }
    /**
     * @brief Check if a file can be written to
     * @param mode the open mode of the file
     * @return True if the file can be written to, false otherwise
     */
    constexpr inline bool canWriteToFile(FileSystemTypes::OpenMode mode) {
        return mode != FileSystemTypes::OpenMode::ReadOnly;
    }

    protected:
    /// @brief A path that is prepened to all file names for storage
    StaticString::Container _mountPrefix;
    /// @brief The status of the file system.
    FileSystemTypes::Status _status;
    /// @brief The storage this abstraction is bound to
    StorageAbstraction &_storage;
    /// @brief The implementation of the file system
    FileSystemTypes::Implementation _implementation;
    /// @brief The name of the file system
    FileSystemTypes::PartitionName _partitionName;
};

#endif //__FILE_SYSTEM_ABSTRACTION_HPP__