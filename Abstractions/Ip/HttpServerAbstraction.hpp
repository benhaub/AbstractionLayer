/***************************************************************************//**
* @author   Ben Haubrich
* @file     HttpServerAbstraction.hpp
* @details  \b Synopsis: \n Interface for creating a client on any network
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __HTTP_SERVER_SBSTRACTION_HPP__
#define __HTTP_SERVER_SBSTRACTION_HPP__

#include "IpServerAbstraction.hpp"

/**
 * @namespace HttpServerTypes
 * @brief Http server types
 */
namespace HttpServerTypes {

    /**
     * @enum Type
     * @brief Http server MIME types
     */
    enum class Type : uint8_t {
        Unknown = 0,               ///< Unknown
        TextHtml,                  ///< text/html
        ApplicationJson,           ///< application/json
        ApplicationXml,            ///< application/xml
        ApplicationXhtmlXml,       ///< application/xhtml+xml
        ApplicationOctetStream,    ///< application/octet-stream
        ApplicationFormUrlencoded, ///< Application/x-www-form-urlencoded
        ImageGif,                  ///< image/gif
        ImageJpeg,                 ///< image/jpeg
        ImagePng,                  ///< image/png
        ImageTiff,                 ///< image/tiff
        ImageSvgXml,               ///< image/svg+xml
        AudioBasic,                ///< audio/basic
        AudioMpeg,                 ///< audio/mpeg
        AudioVorbis,               ///< audio/vorbis
        AudioVndWave,              ///< audio/vnd.wave
        AudioWebm,                 ///< audio/webm
        VideoMpeg,                 ///< video/mpeg
        VideoMp4,                  ///< video/mp4
        VideoWebm                  ///< video/webm
    };

    /**
     * @enum Method
     * @brief Http server methods
     */
    enum class Method : uint8_t {
        Unknown = 0, ///< Unknown
        Get,         ///< GET
        Post,        ///< POST
        Put,         ///< PUT
        Delete,      ///< DELETE
        Head,        ///< HEAD
        Connect,     ///< CONNECT
        Options,     ///< OPTIONS
        Trace,       ///< TRACE
    };

    /**
     * @enum Version
     * @brief Http version
     */
    enum class Version : uint8_t {
        Unknown = 0, ///< Unknown
        Http1_0,     ///< Hypertext Transfer Protocol 1.0
        Http1_1,     ///< Hypertext Transfer Protocol 1.1
        Http2_0,     ///< Hypertext Transfer Protocol 2.0
        Http3_0      ///< Hypertext Transfer Protocol 3.0
    };

    /**
     * @enum Connection
     * @brief Connection types for the HTTP session
     */
    enum class Connection : uint8_t {
        Unknown = 0, ///< Unknown
        KeepAlive,   ///< Keep the connection alive
        Close        ///< Close the connection after the request
    };

    /**
     * @enum Language
     * @brief The language of the request
     */
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

    /**
     * @enum Encoding
     * @brief The ecnoding
     */
    enum class Encoding : uint8_t {
        Unknown = 0, ///< Unknown
        Gzip,        ///< GNU zip
        Deflate,     ///< Deflate
        Br,          ///< Brotli
        Identity     ///< Identity
    };

    /**
     * @struct Headers
     * @brief Http Headers
     */
    struct Headers {
        std::string userAgent;          ///< User-Agent:
        std::string host;               ///< Host:
        std::string referer;            ///< Referer:
        std::vector<Language> language; ///< Language:
        std::vector<Type> accept;       ///< Accept:
        std::vector<Encoding> encoding; ///< Encoding:
        Type contentType;               ///< Content-Type:
        Connection connection;          ///< Connection:
        Bytes contentLength;            ///< Content-Length
    };

    /**
     * @struct CustomHeaders
     * @brief Any custom headers that you include with your request or response
     */
    struct CustomHeaders {
        std::array<std::string, 10> headers; ///< Array for the custom headers
    };

    /**
     * @struct RequestLine
     * @brief The request line of an Http request
     */
    struct RequestLine {
        Method method;   ///< Method
        std::string uri; ///< Universal Resource Identifier
        Version version; ///< Http protocol version
    };

    /**
     * @struct Request
     * @brief The request
     */
    struct Request {
        RequestLine requestLine;     ///< Request Line
        Headers headers;             ///< Headers
        CustomHeaders customHeaders; ///< Custom Headers
        std::string messageBody;     ///< Message body
    };

    /**
     * @enum StatusCode
     * @brief The status code
     */
    enum class StatusCode : uint16_t {
        Unknown = 0,                       ///< Unknown
        Continue = 100,                    ///< Continue
        SwitchingProtocols = 101,          ///< Switching Protocols
        Processing = 102,                  ///< Processing
        EarlyHints = 103,                  ///< Early Hints
        Ok = 200,                          ///< OK
        Created = 201,                     ///< Created
        Accepted = 202,                    ///< Accepted
        NonAuthoritativeInformation = 203, ///< Non-Authoritative Information
        NoContent = 204,                   ///< No Content
        ResetContent = 205,                ///< Reset Content
        PartialContent = 206,              ///< Partial Content
        MultiStatus = 207,                 ///< Multi-Status
        AlreadyReported = 208,             ///< Already Reported
        ImUsed = 226,                      ///< IM Used
        MultipleChoices = 300,             ///< Multiple Choices
        MovedPermanently = 301,            ///< Moved Permanently
        Found = 302,                       ///< Found
        SeeOther = 303,                    ///< See Other
        NotModified = 304,                 ///< Not Modified
        UseProxy = 305,                    ///< Use Proxy
        TemporaryRedirect = 307,           ///< Temporary Redirect
        PermanentRedirect = 308,           ///< Permanent Redirect
        BadRequest = 400,                  ///< Bad Request
        Unauthorized = 401,                ///< Unauthorized
        PaymentRequired = 402,             ///< Payment Required
        Forbidden = 403,                   ///< Forbidden
        NotFound = 404,                    ///< Not Found
        MethodNotAllowed = 405,            ///< Method Not Allowed
        NotAcceptable = 406,               ///< Not Acceptable
        ProxyAuthenticationRequired = 407, ///< Proxy Authentication Required
        RequestTimeout = 408,              ///< Request Timeout
        Conflict = 409,                    ///< Conflict
        Gone = 410,                        ///< Gone
        LengthRequired = 411,              ///< Length Required
        PreconditionFailed = 412,          ///< Precondition Failed
        RequestEntityTooLarge = 413,       ///< Request Entity Too Large
        RequestUriTooLong = 414,           ///< Request-URI Too Long
        UnsupportedMediaType = 415,        ///< Unsupported Media Type
        RequestedRangeNotSatisfiable = 416,///< Requested Range Not Satisfiable
        ExpectationFailed = 417,           ///< Expectation Failed
        InternalServerError = 500,         ///< Internal Server Error
        NotImplemented = 501,              ///< Not Implemented
        BadGateway = 502,                  ///< Bad Gateway
        ServiceUnavailable = 503,          ///< Service Unavailable
        GatewayTimeout = 504,              ///< Gateway Timeout
        HttpVersionNotSupported = 505,     ///< HTTP Version Not Supported
    };

    /**
     * @struct StatusLine
     * @brief The status line
     */
    struct StatusLine {
        Version version;       ///< Http protocol version
        StatusCode statusCode; ///< Status code
    };

    /**
     * @struct ResponseHeaders
     * @brief The response headers
     */
    struct ResponseHeaders {
        char server[64]; ///< Server name
        char date[32];   ///< date.
    };

    /**
     * @struct RepresentationHeaders
     * @brief The representation headers
     */
    struct RepresentationHeaders {
        Type contentType;                      ///< Content-Type:
        std::vector<Encoding> contentEncoding; ///< Content-Encoding:
        Bytes contentLength;                   ///< Content-Length:
        std::vector<Language> contentLanguage; ///< Content-Language:
    };

    /**
     * @struct Response
     * @brief An Http response
     */
    struct Response {
        StatusLine statusLine;                       ///< Status line
        ResponseHeaders responseHeaders;             ///< Response headers
        RepresentationHeaders representationHeaders; ///< Representation headers
        std::string messageBody;                     ///< Message body
    };
};

/**
 * @class HttpServerAbstraction
 * @brief An Http server
 * @note This is almost a decorator pattern execpt that the parameters of an HttpServerAbstraction overload (instead of override) the base class
 *       and the base class virtual functions are now deprecated. This is probably in violation of Liskov Substitution but from a user perspective
 *       it seems to perform well. You just have to remember to cast this class if you're storing it in an IpServerAbstraction or CommunicationProtocol
 *       and if you don't and call the functions anyway they will return an appropriate error code which you could actually argue does not break the program.
 * @sa For other examples of this, see I2cAbstraction which adds device address and register address to the parameter list and IpServerAbstraction which adds sockets.
 * https://en.wikipedia.org/wiki/Decorator_pattern
 */
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
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) override { return ErrorType::NotSupported; }

    /**
     * @brief Send a response
     * @param[in] response The response to send
     * @param[in] timeout The timeout
     * @param[in] socket The socket to use
     * @returns ErrorType::Success if the response could be sent
     * @returns ErrorType::Timeout if the send could not be completed within the given timeout
     */
    virtual ErrorType sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout, const Socket socket) = 0;
    /**
     * @brief Receive a request
     * @param[in] request The request to receive
     * @param[in] timeout The timeout
     * @param[in] socket The socket that the message was received on
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     */
    virtual ErrorType receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout, Socket &socket) = 0;
    /**
     * @brief Send a response
     * @param[in] data The data to send
     * @param[in] timeout The timeout
     * @param[in] socket The socket to use
     * @param[in] callback The callback to call when the send is complete
     * @returns ErrorType::Success if the response could be sent
     * @returns ErrorType::Timeout if the send could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief Receive a request
     * @param[in] buffer The buffer to receive the request into
     * @param[in] timeout The timeout
     * @param[out] socket The socket that the message was received on
     * @param[in] callback The callback to call when the receive is complete
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpServerTypes::Request> buffer)> callback) = 0;

#pragma GCC diagnostic pop

};
#endif // __HTTP_SERVER_SBSTRACTION_HPP__