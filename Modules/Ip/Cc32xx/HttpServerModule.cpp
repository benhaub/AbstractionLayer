//AbstractionLayer
#include "HttpServerModule.hpp"
#include "Log.hpp"

ErrorType HttpServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    return ErrorType::NotImplemented;
}
ErrorType HttpServer::acceptConnection(Socket &socket) {
    return ErrorType::NotImplemented;
}
ErrorType HttpServer::closeConnection() {
    return ErrorType::NotImplemented;
}
ErrorType HttpServer::sendBlocking(HttpServerTypes::Response response, Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType HttpServer::receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    SlNetAppRequest_t netAppRequest;
    error = OperatingSystem::Instance().receiveFromQueue(SimpleLinkEventQueue, &netAppRequest, timeout, false);
    if (ErrorType::Success != error) {
        return error;
    }

    if (netAppRequest.AppId != SL_NETAPP_HTTP_SERVER_ID) {
        return ErrorType::Failure;
    }

    uint8_t *requestMetaData = netAppRequest.requestData.pMetadata;
    const uint16_t metaDataLength = netAppRequest.requestData.MetadataLen;
    const uint8_t *metaDataEnd = requestMetaData + metaDataLength;

    if (metaDataLength < 3) {
        return ErrorType::Failure;
    }

    while (requestMetaData < metaDataEnd) {
        uint8_t type = *requestMetaData;
        requestMetaData++;
        uint16_t length = *((uint16_t *)requestMetaData);
        requestMetaData += 2;

        switch(type) {
        case SL_NETAPP_REQUEST_METADATA_TYPE_STATUS:
           /* there are browsers that seem to send many 0 type for no reason */
           /* in this case, do not print anything */
            break;

        case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONTENT_LEN:
            PLT_LOGI(TAG, "Content Length: %d", length);
            break;

        case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_REQUEST_URI:
            PLT_LOGI(TAG, "URI: %s", reinterpret_cast<const char *>(requestMetaData));
            break;

        case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_QUERY_STRING:
            break;
        default:
            break;
       }
    }

    return error;
}
ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

#ifdef __cplusplus
extern "C" {

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