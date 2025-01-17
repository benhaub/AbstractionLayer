#ifndef __HTTP_SERVER_SBSTRACTION_HPP__
#define __HTTP_SERVER_SBSTRACTION_HPP__

#include "IpServerAbstraction.hpp"

namespace HttpServerTypes {

    enum class Type : uint8_t {
        Unknown = 0,
        TextHtml,
        ApplicationJson,
        ApplicationXml,
        ApplicationXhtmlXml,
        ApplicationOctetStream,
        ApplicationFormUrlencoded,
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

    enum class Method : uint8_t {
        Unknown = 0,
        Get,
        Post,
        Put,
        Delete,
        Head,
        Connect,
        Options,
        Trace
    };

    enum class Version : uint8_t {
        Unknown = 0, ///< Unknown
        Http1_0,     ///< Hypertext Transfer Protocol 1.0
        Http1_1,     ///< Hypertext Transfer Protocol 1.1
        Http2_0,     ///< Hypertext Transfer Protocol 2.0
        Http3_0      ///< Hypertext Transfer Protocol 3.0
    };

    enum class Connection : uint8_t {
        Unknown = 0,
        KeepAlive,
        Close
    };

    enum class Language : uint8_t {
        Unknown = 0,Afrikaans,Amharic,Arabic_SaudiArabia,Assamese,Azerbaijani_Latin,Belarusian,Bulgarian,Bangla_Bangladesh,Bangla_India,
        Bosnian_Latin,CatalanSpanish,Valencian,Czech,Welsh,Danish,German_Germany,Greek, English_UnitedKingdom,English_UnitedStates,
        Spanish_Spain,Spanish_UnitedStates,Spanish_Mexico,Estonian,Basque,Persian,Finnish,Filipino,French_France,French_Canada,Irish,
        ScottishGaelic,Galician,Gujarati,Hausa_Latin,Hebrew,Hindi,Croation,Hungarian,Armenian,Indonesian,Igbo,Icelandic,Italian_Italy,
        Japanese,Georgian,Kazakh,Khmer,Kannada,Korean,Konkani,CentralKurdish,Kyrgyz,LuxemBourgish,Lithuanian,Latvian,Maori,Macedonian,
        Malayalam,Mongolian_Cyrillic,Marathi,Maylay_Maylaysia,Maltese,Norwegian_Bokmal,Nepali_Nepal,Dutch_Netherlands,Norwegian_Nynorsk,
        SesothoSaLeboa,Odia,Punjabi_Gurmukhi,Punjabi_Arabic,Polish,Dari,Portuguese_Brazil,Portuguese_Portugal,Kiche,Quechua_Peru,
        Romanian_Romania,Russian,Kinyarwanda,Sindhi_Arabic,Sinhala,Slovak,Slovenian,Albanian,Serbian_Cyrillic_BosniaHerzegovina,
        Serbian_Cyrillic_Serbia,Serbian_Latin_Serbia,Swedish_Swedin,Kiswahili,Tamil,Telugu,Tajik_Cyrillic,Thai,Tigrinya,Turkmen_Latin,
        Setswana,Turkish,Tatar_Cyrillic, Uyghu,Ukranian,Urda,Uzbek_Latin,Vietnamese,Wolof,isiXhosa,Yoruba,Chinese_Simplified,Chinese_Traditional,
        isiZulu
    };

    enum class Encoding : uint8_t {
        Unknown = 0,
        Gzip,
        Deflate,
        Br,
        Identity
    };

    struct Headers {
        std::string userAgent;
        std::string host;
        std::string referer;
        std::vector<Language> language;
        std::vector<Type> accept;
        std::vector<Encoding> encoding;
        Type contentType;
        Connection connection;
        Bytes contentLength;
    };

    struct CustomHeaders {
        std::array<std::string, 10> headers;
    };

    struct RequestLine {
        Method method;
        std::string uri;
        Version version;        
    };

    struct Request {
        RequestLine requestLine;
        Headers headers;
        CustomHeaders customHeaders;
        std::string messageBody;
    };

    enum class StatusCode : uint16_t {
        Unknown = 0,
        Continue = 100,
        SwitchingProtocols = 101,
        Processing = 102,
        EarlyHints = 103,
        Ok = 200,
        Created = 201,
        Accepted = 202,
        NonAuthoritativeInformation = 203,
        NoContent = 204,
        ResetContent = 205,
        PartialContent = 206,
        MultiStatus = 207,
        AlreadyReported = 208,
        ImUsed = 226,
        MultipleChoices = 300,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        UseProxy = 305,
        TemporaryRedirect = 307,
        PermanentRedirect = 308,
        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        NotAcceptable = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout = 408,
        Conflict = 409,
        Gone = 410,
        LengthRequired = 411,
        PreconditionFailed = 412,
        RequestEntityTooLarge = 413,
        RequestUriTooLong = 414,
        UnsupportedMediaType = 415,
    };

    struct StatusLine {
        Version version;
        StatusCode statusCode;
        char statusMessage[64];
    };

    struct ResponseHeaders {
        char server[64];
        char date[32];
    };

    struct RepresentationHeaders {
        Type contentType;
        Encoding contentEncoding;
        Bytes contentLength;
        Language contentLanguage;
    };

    struct Response {
        StatusLine statusLine;
        ResponseHeaders responseHeaders;
        RepresentationHeaders representationHeaders;
        std::string messageBody;
    };
};

class HttpServerAbstraction : public IpServerAbstraction {

    public:
    HttpServerAbstraction() : IpServerAbstraction() {}
    virtual ~HttpServerAbstraction() = default;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

    //Http servers take and and return responses and requests. If using an IpServerPointer, try casting to an HttpServer
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) override { return ErrorType::NotSupported; }
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override { return ErrorType::NotSupported; }
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) override { return ErrorType::NotSupported; }
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) override { return ErrorType::NotSupported; }

    virtual ErrorType sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout, const Socket socket) = 0;
    virtual ErrorType receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout, Socket &socket) = 0;
    virtual ErrorType sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    virtual ErrorType receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) = 0;

#pragma GCC diagnostic pop

};
#endif // __HTTP_SERVER_SBSTRACTION_HPP__