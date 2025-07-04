//AbstractionLayer
#include "HttpServerModule.hpp"
#include "NetworkAbstraction.hpp"
#include "Log.hpp"

ErrorType HttpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    //The ROM includes an HTTP server that is already listening
    return ErrorType::NotAvailable;
}

ErrorType HttpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    return ErrorType::NotAvailable;
}

ErrorType HttpServer::closeConnection(const Socket socket) {
    return ErrorType::NotAvailable;
}

ErrorType HttpServer::sendBlocking(const HttpTypes::Response &response, const Milliseconds timeout, const Socket socket) {
    return ErrorType::NotImplemented;
}

//TODO: Not thread safe.
ErrorType HttpServer::receiveBlocking(HttpTypes::Request &request, const Milliseconds timeout, Socket &socket) {
    ErrorType error = ErrorType::Failure;
    SlNetAppRequest_t netAppRequest;
    error = OperatingSystem::Instance().receiveFromQueue(SimpleLinkEventQueue, &netAppRequest, timeout, false);
    if (ErrorType::Success != error) {
        return error;
    }

    if (netAppRequest.AppId != SL_NETAPP_HTTP_SERVER_ID) {
        return ErrorType::Failure;
    }

    error = toHttpRequest(netAppRequest, request);

    delete netAppRequest.requestData.pMetadata;
    delete netAppRequest.requestData.pPayload;

    socket = netAppRequest.Handle;

    return error;
}

ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto tx = [&, callback](const std::shared_ptr<HttpTypes::Response> response, const Socket socket, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;
        assert(nullptr != callback);

        if (nullptr == response.get()) {
            error = ErrorType::NoData;
            callback(error, Bytes(0));
            return error;
        }

        std::string frame(256, '\0');
        error = toSlNetAppResponse(*response, frame);
        if (ErrorType::Success != error) {
            callback(error, Bytes(0));
            return error;
        }

        assert(frame.size() <= SL_NETAPP_REQUEST_MAX_METADATA_LEN);
        assert(response->messageBody.size() <= SL_NETAPP_REQUEST_MAX_DATA_LEN);

        error = fromPlatformError(sl_NetAppSend(socket, frame.size(), reinterpret_cast<uint8_t *>(frame.data()), SL_NETAPP_REQUEST_RESPONSE_FLAGS_CONTINUATION | SL_NETAPP_REQUEST_RESPONSE_FLAGS_METADATA));
        if (ErrorType::Success != error) {
            callback(error, Bytes(0));
            return error;
        }
        constexpr _u32 markAsLastFragment = 0;
        error = fromPlatformError(sl_NetAppSend(socket, response->messageBody.size(), reinterpret_cast<uint8_t *>(response->messageBody.data()), markAsLastFragment));
        if (ErrorType::Success != error) {
            callback(error, Bytes(0));
            return error;
        }

        callback(error, Bytes(0));
        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx, data, socket, timeout));
    return _ipServer.network().addEvent(event);
}

ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpTypes::Request> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpTypes::Request> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::toHttpRequest(const SlNetAppRequest_t &netAppRequest, HttpTypes::Request &request) {
    //The request has been converted to a TLV structure by the ROM HTTP server.
    //Pg. 177, Network Processor User Guide.
    uint8_t *requestMetaData = netAppRequest.requestData.pMetadata;
    const uint16_t metaDataLength = netAppRequest.requestData.MetadataLen - 1;
    const uint8_t *metaDataEnd = requestMetaData + metaDataLength;

    if (metaDataLength < 3) {
        return ErrorType::Failure;
    }

    //In the example code for TIs httpserver, this code block is inside the loop. So we get the first value and then skip ahead
    //by two bytes hoping to miraculously land on the next tlv block regardelss of the length of the value while also praying that none of
    //the intermediate bytes in the value that we land on while traversing two bytes at a time are equal to these netapp request enums.
    //Obviously that didn't work and I'm not sure how that code made it through testing.
    uint8_t type = *requestMetaData;
    requestMetaData++;
    uint16_t valueLength = *((uint16_t *)requestMetaData);
    requestMetaData += 2;

    while (requestMetaData < metaDataEnd) {
        switch(type) {
            case SL_NETAPP_REQUEST_METADATA_TYPE_STATUS:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_VERSION:
                request.requestLine.version = fromStringVersion(reinterpret_cast<const char *>(requestMetaData));
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_REQUEST_URI:
                //Contrary to the documentation on Pg. 177, the length only corresponds to the entire TLV structure for the first value
                //(which is the value we have stored in metaDataLength). Subsequent length bitfields are only for the length of the
                //value and DO NOT include the length and type.
                request.requestLine.uri = std::string(reinterpret_cast<const char *>(requestMetaData), valueLength);
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_QUERY_STRING:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONTENT_LEN:
                request.headers.contentLength = Bytes(*((uint32_t *)requestMetaData));
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONTENT_TYPE:
                request.headers.contentType = fromStringContentType(reinterpret_cast<const char *>(requestMetaData));
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_LOCATION:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_SERVER:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_USER_AGENT:
                request.headers.userAgent = std::string(reinterpret_cast<const char *>(requestMetaData), valueLength);
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_COOKIE:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_SET_COOKIE:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_UPGRADE:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_REFERER:
                request.headers.referer = std::string(reinterpret_cast<const char *>(requestMetaData), valueLength);
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_ACCEPT:
                request.headers.accept = fromStringAccept(reinterpret_cast<const char *>(requestMetaData));
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONTENT_ENCODING:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONTENT_DISPOSITION:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONNECTION:
                request.headers.connection = fromStringConnection(reinterpret_cast<const char *>(requestMetaData));
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_ETAG:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_DATE:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HEADER_HOST:
                request.headers.host = std::string(reinterpret_cast<const char *>(requestMetaData), valueLength);
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_ACCEPT_ENCODING:
                request.headers.encoding = fromStringEncoding(reinterpret_cast<const char *>(requestMetaData));
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_ACCEPT_LANGUAGE:
                request.headers.language = fromStringAcceptLanguage(reinterpret_cast<const char *>(requestMetaData));
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_CONTENT_LANGUAGE:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_ORIGIN:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_ORIGIN_CONTROL_ACCESS:
                break;
            case SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_NONE:
                break;
        default:
            return ErrorType::NotSupported;
       }

       requestMetaData += strlen(reinterpret_cast<const char *>(requestMetaData)) + 1;
       type = *(requestMetaData - (sizeof(type) + sizeof(valueLength)));
       valueLength = *((uint16_t *)(requestMetaData - sizeof(valueLength)));
    }

    return ErrorType::Success;
}

//We are using the same TLV structure as we used when receiving the request. We are following the suggested response outlined
//on Pg. 181 of the Network Processor User Guide.
ErrorType HttpServer::toSlNetAppResponse(const HttpTypes::Response &response, std::string &slNetAppResponse) {

    slNetAppResponse.resize(0);

    {
    slNetAppResponse.push_back(SL_NETAPP_REQUEST_METADATA_TYPE_STATUS);
    uint16_t statusCodeSize = sizeof(uint16_t);
    slNetAppResponse.append(reinterpret_cast<const char *>(&statusCodeSize), sizeof(statusCodeSize));
    slNetAppResponse.append(reinterpret_cast<const char *>(&response.statusLine.statusCode), sizeof(response.statusLine.statusCode));
    }

    {
    slNetAppResponse.push_back(SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONTENT_TYPE);
    std::string contentType = fromHttpServerType(response.representationHeaders.contentType);
    uint16_t contentTypeLength = contentType.size();
    slNetAppResponse.append(reinterpret_cast<const char *>(&contentTypeLength), sizeof(contentTypeLength));
    slNetAppResponse.append(contentType);
    }

    {
    slNetAppResponse.push_back(SL_NETAPP_REQUEST_METADATA_TYPE_HTTP_CONTENT_LEN);
    uint16_t contentLengthSize = sizeof(uint32_t);
    slNetAppResponse.append(reinterpret_cast<const char *>(&contentLengthSize), sizeof(contentLengthSize));
    slNetAppResponse.append(reinterpret_cast<const char *>(&response.representationHeaders.contentLength), sizeof(response.representationHeaders.contentLength));
    }

    return ErrorType::Success;
}

#ifdef __cplusplus
extern "C" {

//Called whenever a connected client makes an http request.
//Places the request received into a queue to be processed later and prepares the response.
//Pg. 175, Network Processor User Guide. If the resource does not exist on the file system,
//the ROM http server does not send a response and request is deferred here.
//Pg. 176, Network Processor User Guide. This is called in the context of the SimpleLink task. It should be quick and
//Not attempt to call any other simplelink functions.
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

    //Since the NetAppRequest constains pointers in the struct, we need to explicitely dereference and copy that data.
    SlNetAppRequest_t netAppRequest = {
        .AppId = pNetAppRequest->AppId,
        .Type = pNetAppRequest->Type,
        .Handle = pNetAppRequest->Handle,
        .requestData = {
            .MetadataLen = pNetAppRequest->requestData.MetadataLen,
            .pMetadata = new unsigned char[pNetAppRequest->requestData.MetadataLen],
            .PayloadLen = pNetAppRequest->requestData.PayloadLen,
            .pPayload = new unsigned char[pNetAppRequest->requestData.PayloadLen],
            .Flags = pNetAppRequest->requestData.Flags
        }
    };

    if (0 != netAppRequest.requestData.MetadataLen && nullptr == netAppRequest.requestData.pMetadata) {
        pNetAppResponse->Status = SL_NETAPP_RESPONSE_NONE;
        return;
    }
    if (0 != netAppRequest.requestData.PayloadLen && nullptr != netAppRequest.requestData.pPayload) {
        pNetAppResponse->Status = SL_NETAPP_RESPONSE_NONE;
        return;
    }

    sl_Memcpy(netAppRequest.requestData.pMetadata, pNetAppRequest->requestData.pMetadata, pNetAppRequest->requestData.MetadataLen);
    sl_Memcpy(netAppRequest.requestData.pPayload, pNetAppRequest->requestData.pPayload, pNetAppRequest->requestData.PayloadLen);

    ErrorType error = OperatingSystem::Instance().sendToQueue(SimpleLinkEventQueue, &netAppRequest, 0, false, false);
    if (ErrorType::Success != error) {
        pNetAppResponse->Status = SL_NETAPP_RESPONSE_NONE;
    }

    return;
}

void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t *pHttpEvent, SlNetAppHttpServerResponse_t * pHttpResponse) {
    return;
}
#endif

#ifdef __cplusplus
}
#endif