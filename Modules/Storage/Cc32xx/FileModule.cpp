#include "FileModule.hpp"

ErrorType File::open(const std::string &filename, OpenMode mode) {
    return ErrorType::NotImplemented;
}
ErrorType File::close() {
    return ErrorType::NotImplemented;
}
ErrorType File::seek(const FileOffset &offset) {
    return ErrorType::NotImplemented;
}
ErrorType File::remove() {
    return ErrorType::NotImplemented;
}
ErrorType File::readBlocking(const FileOffset offset, std::string &buffer) {
    return ErrorType::NotImplemented;
}
ErrorType File::readNonBlocking(const FileOffset offset, std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType File::writeBlocking(const FileOffset offset, const std::string &data) {
    return ErrorType::NotImplemented;
}
ErrorType File::writeNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType File::synchronize() {
    return ErrorType::NotImplemented;
}
std::string File::path() const {
    return "";
}