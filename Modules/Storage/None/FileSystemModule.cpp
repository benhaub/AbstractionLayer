//AbstractionLayer
#include "FileSystemModule.hpp"
#include "OperatingSystemModule.hpp"

ErrorType FileSystem::mount() {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::unmount() {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::maxPartitionSize(Bytes &size) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::availablePartition(Bytes &size) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::erasePartition() {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::open(std::string_view path, const FileSystemTypes::OpenMode mode, FileSystemTypes::File &file) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::close(FileSystemTypes::File &file) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::remove(FileSystemTypes::File &file) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::readBlocking(FileSystemTypes::File &file, std::string &buffer) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::readNonBlocking(FileSystemTypes::File &file, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::writeBlocking(FileSystemTypes::File &file, const std::string &data) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::writeNonBlocking(FileSystemTypes::File &file, const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::synchronize(const FileSystemTypes::File &file) {
    return ErrorType::NotImplemented;
}

ErrorType FileSystem::size(FileSystemTypes::File &file) {
    return ErrorType::NotImplemented;
}