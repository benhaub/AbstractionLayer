//AbstractionLayer
#include "IpClient.hpp"

ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) {
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Failure;


    auto connectCb = [&]() -> ErrorType {
        // Ensure any existing connection is properly closed
        disconnect();

        callbackError = network().connectTo(hostname, port, protocol, version, _socket, timeout);
        doneConnecting = true;
        _status.connected = callbackError == ErrorType::Success;

        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(connectCb);
    if (ErrorType::Success != (error = network().addEvent(event))) {
        PLT_LOGW(TAG, "Could not add connection event to network");
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::disconnect() {
    ErrorType error = ErrorType::Success;

    if (_socket != -1) {
        error = network().disconnect(_socket);
        _socket = -1;
    }

    _status.connected = ErrorType::Success == error;

    return error;
}

/**
 * @brief Sends data.
 * @pre The amount of data to send is equal to the size of data. See std::string::resize()
 * @param[in] data The data to send.
 * @param[in] timeout The time to wait to send the data.
 * @param[in] callback The callback to call when the data is sent.
 * @code{.cpp}
 * //Lambda callback
 * auto callback = [](const ErrorType error, const Bytes bytesWritten) -> void {
 *     if (ErrorType::Success == error) {
 *         // Data was sent
 *     }
 * };
 * error = sendNonBlocking(data, timeout, callback);
 * 
 * //Member function callback
 * void Foo::bar(const ErrorType error, const Bytes bytesWritten) {
 *     if (ErrorType::Success == error) {
 *         // Data was sent
 *     }
 * }
 * error = sendNonBlocking(data, timeout, std::bind(&Foo::bar, this, std::placeholders::_1, std::placeholders::_2)); 
 * @endcode
 * @returns ErrorType::Success if the data was sent.
 * @returns ErrorType::Failure if the data was not sent.
 * @post The callback will be called when the data has been sent. The bytes written is valid if and only if error is equal to ErrorType::Success.
*/
ErrorType IpClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto tx = [&, callback, data, timeout]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);
        assert(nullptr != data.get());

        error = sendBlocking(*data, timeout);
        callback(error, data->size());

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx));
    return network().addEvent(event);
}
/**
 * @brief Receives data.
 * @param[out] buffer The buffer to receive the data into.
 * @param[in] timeout The time to wait to receive the data.
 * @param[in] callback The callback to call when the data has been received.
 * @code{.cpp}
 * //Lambda callback
 * auto callback = [](const ErrorType error, std::shared_ptr<std::string> buffer) -> void {
 *     if (ErrorType::Success == error) {
 *         // Data was sent
 *     }
 * };
 * error = sendNonBlocking(data, timeout, callback);
 * 
 * //Member function callback
 * void Foo::bar(const ErrorType error, std::shared_ptr<std::string> buffer) {
 *     if (ErrorType::Success == error) {
 *         // Data was sent
 *     }
 * }
 * error = sendNonBlocking(data, timeout, std::bind(&Foo::bar, this, std::placeholders::_1, std::placeholders::_2)); 
 * @endcode
 * @post The callback will be called when the data has been received. The amount of data received is equal to the size of the
 *       data if ErrorType::Success is returned. See std::string::size().
*/
ErrorType IpClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    auto rx = [&, callback, buffer, timeout]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);
        assert(nullptr != buffer.get());

        error = receiveBlocking(*buffer, timeout);
        callback(error, buffer);

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx));
    return network().addEvent(event);
}