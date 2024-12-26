//AbstractionLayer
#include "IpServerModule.hpp"

ErrorType IpServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::acceptConnection(Socket &socket) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::closeConnection() {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::sendBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    SlNetAppRequest_t netAppRequest;
    OperatingSystem::Instance().receiveFromQueue(SimpleLinkEventQueue, &netAppRequest, timeout, false);

    if (netAppRequest.AppId != SL_NETAPP_HTTP_SERVER_ID) {
        return ErrorType::Failure;
    }

    return ErrorType::NotImplemented;
}

ErrorType IpServer::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

#ifdef __cplusplus
extern "C" {

void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t *buffer) {
    return;
}

//Called whenever a connected client makes an http request.
//Places the request received into a queue to be processed later and prepares the response.
void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t *pNetAppRequest, SlNetAppResponse_t *pNetAppResponse) {
    pNetAppResponse->ResponseData.pMetadata = NULL;
    pNetAppResponse->ResponseData.MetadataLen = 0;
    pNetAppResponse->ResponseData.pPayload = NULL;
    pNetAppResponse->ResponseData.PayloadLen = 0;
    pNetAppResponse->ResponseData.Flags = 0;

    if((pNetAppRequest->Type == SL_NETAPP_REQUEST_HTTP_GET) ||
       (pNetAppRequest->Type == SL_NETAPP_REQUEST_HTTP_DELETE) ||
       (pNetAppRequest->Type == SL_NETAPP_REQUEST_HTTP_POST) ||
       (pNetAppRequest->Type == SL_NETAPP_REQUEST_HTTP_PUT)) {
        /* Prepare pending response */
        pNetAppResponse->Status = SL_NETAPP_RESPONSE_PENDING;
    }
    else {
        pNetAppResponse->Status = SL_NETAPP_RESPONSE_NONE;
        return;
    }

    OperatingSystem::Instance().sendToQueue(SimpleLinkEventQueue, pNetAppRequest, 0, false, false);
}

void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t *pHttpEvent, SlNetAppHttpServerResponse_t * pHttpResponse) {
    return;
}
#endif

#ifdef __cplusplus
}
#endif