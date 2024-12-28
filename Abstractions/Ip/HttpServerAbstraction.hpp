#ifndef __HTTP_SERVER_SBSTRACTION_HPP__
#define __HTTP_SERVER_SBSTRACTION_HPP__

#include "IpServerAbstraction.hpp"

namespace HttpServerTypes {

    enum class Accept : uint8_t {
        Unknown = 0,
        TextHtml,
        ApplicationJson,
        ApplicationXml,
        ApplicationXhtmlXml,
        ApplicationOctetStream,
        ApplicationFormUrlencoded,
        ApplicationXwwwFormUrlencoded,
        ImageGif,
        ImageJpeg,
        ImagePng,
        ImageTiff,
        ImageSvgXml,
        AudioBasic,
        AudioMpeg,
        AudioVorbis,
        AudioVndWave,
        AudioWebm,
        VideoMpeg,
        VideoMp
    };

    enum class ContentType : uint8_t {
        Unknown = 0,
        TextHtml,
        ApplicationJson,
        ApplicationXml,
        ApplicationXhtmlXml,
        ApplicationOctetStream,
        ApplicationFormUrlencoded,
        ApplicationXwwwFormUrlencoded,
        ImageGif,
        ImageJpeg,
        ImagePng,
        ImageTiff,
        ImageSvgXml,
        AudioBasic,
        AudioMpeg,
        AudioVorbis,
        AudioVndWave,
        AudioWebm,
        VideoMpeg,
        VideoMp
    };

    struct MessageBody {
        std::string data;
    };

    struct Headers {
        std::string hostname;
        Ipv4Address hostAddress;
        std::string userAgent;
        ContentType contentType;
        Bytes contentLength;
        Accept accept;
    };

    struct CustomHeaders {
        std::array<std::string, 10> headers;
    };

    struct RequestLine {
        std::string method;
        std::string uri;
        std::string version;        
    };

    struct Request {
        RequestLine requestLine;
        Headers headers;
        CustomHeaders customHeaders;
        MessageBody messageBody;
    };

    struct Response {
        //TODO: I don't know what should go in here. I could maybe even typedef this to a Request.
        std::string data;
    };
};

class HttpServerAbstraction : public IpServerAbstraction {
    public:
    HttpServerAbstraction() : IpServerAbstraction() {}
    virtual ~HttpServerAbstraction() = default;

    virtual ErrorType sendBlocking(HttpServerTypes::Response response, Milliseconds timeout) = 0;
    virtual ErrorType receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout) = 0;
    virtual ErrorType sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    virtual ErrorType receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) = 0;
};
#endif // __HTTP_SERVER_SBSTRACTION_HPP__