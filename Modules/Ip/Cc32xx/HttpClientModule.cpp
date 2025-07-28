//AbstractionLayer
#include "HttpClientModule.hpp"

ErrorType HttpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}

ErrorType HttpClient::disconnect() {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}

ErrorType HttpClient::sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}

ErrorType HttpClient::receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}

ErrorType HttpClient::sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}

ErrorType HttpClient::receiveNonBlocking(std::shared_ptr<HttpTypes::Response> request, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}