/***************************************************************************//**
* @author   Ben Haubrich
* @file     HttpServerAbstraction.hpp
* @details  \b Synopsis: \n Interface for creating a client on any network
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __HTTP_SERVER_SBSTRACTION_HPP__
#define __HTTP_SERVER_SBSTRACTION_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"
//C++
#include <cstring>

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
        TextCss,                   ///< text/css
        TextJavascript,            ///< text/javascript
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
     * @details Since not all headers may be used, it's desireable to have a string as opposed to std::array with fixed size.
     *          Also these headeers are usually short enough for a Small String Optimization.
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
class HttpServerAbstraction {

    public:

    /// @brief Constructor
    HttpServerAbstraction(IpServerAbstraction &ipServer) : _ipServer(&ipServer) {}
    /// @brief Destructor
    virtual ~HttpServerAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "HttpServer";

    void printStatus() {
        _ipServer->printStatus();
    }

    /**
     * @brief Listen for connections on a port
     * @param[in] protocol The protocol to use for the connection
     * @sa IpServerTypes::Protocol
     * @param[in] version The version to use for the connection
     * @param[in] port The port to listen to
     * @sa IpServerTypes::Version
    */
    virtual ErrorType listenTo(const IpServerTypes::Protocol protocol, const IpServerTypes::Version version, const Port port) = 0;
    /**
     * @brief Accept a connection from a client connecting to the socket given
     * @param[out] socket The socket that the connection was accepted on
     * @param[in] timeout The time to wait to accept a connection.
     * @returns ErrorType::Success on success
     * @returns ErrorType::LimitReached if the maximum number of connections has been accepted
     * @returns ErrorType::Timeout if no connections were accepted within the given timeout.
     * @returns ErrorType::Failure otherwise
    */
    virtual ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) = 0;
    /**
     * @brief Close the connection
     * @param[in] socket The socket to close
     * @returns ErrorType::Success on success
    */
    virtual ErrorType closeConnection(const Socket socket) = 0;
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
     * @param[out] socket If negative, will contain the socket in which the data was received from. Otherwise it will be the socket in which data is explicitely received from.
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
     * @param[out] socket If negative, will contain the socket in which the data was received from. Otherwise it will be the socket in which data is explicitely received from.
     * @param[in] callback The callback to call when the receive is complete
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpServerTypes::Request> buffer)> callback) = 0;

    /// @brief The Ip server to perform lower level network communications on.
    IpServerAbstraction *_ipServer;

    /**
     * @brief Search a raw http request for the specified header.
     * @param[in] request The raw http request.
     * @param[in] headerName The header to serach for
     * @param[in] value The value of the header to match against.
     * @returns ErrorType::Success if the http request contains the header.
     * @returns ErrorType::Failure if the http request does not contain the header.
     */
    ErrorType findHeaderValue(const std::string &request, const char headerName[], const char value[]) {
        const size_t theIndexThatTheHeaderStartsAt = request.find(headerName);
        const size_t theIndexThatTheHeaderEndsAt = request.find("\r\n", theIndexThatTheHeaderStartsAt);

        assert(theIndexThatTheHeaderStartsAt <= theIndexThatTheHeaderEndsAt);

        if (std::string::npos != theIndexThatTheHeaderStartsAt && std::string::npos != theIndexThatTheHeaderEndsAt) {
            std::string contentTypeHeader = request.substr(theIndexThatTheHeaderStartsAt, theIndexThatTheHeaderEndsAt);
            if (std::string::npos != contentTypeHeader.find(value)) {
                return ErrorType::Success;
            }
        }

        return ErrorType::Failure;
    }

    /**
     * @brief Convert a raw Http request to an AbstractionLayer htpp request.
     * @sa HttpServerTypes::Request
     * @param[in] buffer The raw data of the http request.
     * @param[out] request The converted htpp request.
     * @returns The http request.
     */
    ErrorType toHttpRequest(const std::string &buffer, HttpServerTypes::Request &request) {
        size_t uriStartIndex, uriEndIndex = 0;

        if (buffer.size() <= 0) {
            return ErrorType::NoData;
        }

        if (std::string::npos != (uriStartIndex = buffer.find("GET"))) {
            request.requestLine.method = HttpServerTypes::Method::Get;
            uriStartIndex += sizeof("GET");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("POST"))) {
            request.requestLine.method = HttpServerTypes::Method::Post;
            uriStartIndex += sizeof("POST");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("PUT"))) {
            request.requestLine.method = HttpServerTypes::Method::Put;
            uriStartIndex += sizeof("PUT");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("CONNECT"))) {
            request.requestLine.method = HttpServerTypes::Method::Connect;
            uriStartIndex += sizeof("CONNECT");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("DELETE"))) {
            request.requestLine.method = HttpServerTypes::Method::Delete;
            uriStartIndex += sizeof("DELETE");
        }
        else {
            request.requestLine.method = HttpServerTypes::Method::Unknown;
        }

        if (std::string::npos != (uriEndIndex = buffer.find("HTTP/1.0"))) {
            request.requestLine.version = HttpServerTypes::Version::Http1_0;
            uriEndIndex -= 1;
        }
        else if (std::string::npos != (uriEndIndex = buffer.find("HTTP/1.1"))) {
            request.requestLine.version = HttpServerTypes::Version::Http1_1;
            uriEndIndex -= 1;
        }
        else if (std::string::npos != (uriEndIndex = buffer.find("HTTP/2.0"))) {
            request.requestLine.version = HttpServerTypes::Version::Http2_0;
            uriEndIndex -= 1;
        }
        else if (std::string::npos != (uriEndIndex = buffer.find("HTTP/3.0"))) {
            request.requestLine.version = HttpServerTypes::Version::Http3_0;
            uriEndIndex -= 1;
        }
        else {
            request.requestLine.version = HttpServerTypes::Version::Unknown;
        }

        if (std::string::npos != uriEndIndex && std::string::npos != uriStartIndex) {
            uriStartIndex >= uriEndIndex ? request.requestLine.uri = "" : request.requestLine.uri = buffer.substr(uriStartIndex, uriEndIndex - uriStartIndex);
        }
        else {
            request.requestLine.uri = "";
        }

        if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "text/html")) {
            request.headers.contentType = HttpServerTypes::Type::TextHtml;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "application/json")) {
            request.headers.contentType = HttpServerTypes::Type::ApplicationJson;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "application/xml")) {
            request.headers.contentType = HttpServerTypes::Type::ApplicationXml;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "text/css")) {
            request.headers.contentType = HttpServerTypes::Type::TextCss;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "text/javascript")) {
            request.headers.contentType = HttpServerTypes::Type::TextJavascript;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "image/png")) {
            request.headers.contentType = HttpServerTypes::Type::ImagePng;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "image/jpeg")) {
            request.headers.contentType = HttpServerTypes::Type::ImageJpeg;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "image/svg+xml")) {
            request.headers.contentType = HttpServerTypes::Type::ImageSvgXml;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "image/tiff")) {
            request.headers.contentType = HttpServerTypes::Type::ImageTiff;
        }
        else {
            request.headers.contentType = HttpServerTypes::Type::Unknown;
        }

        if (ErrorType::Success == findHeaderValue(buffer, "Connection:", "keep-alive")) {
            request.headers.connection = HttpServerTypes::Connection::KeepAlive;
        }
        else if (ErrorType::Success == findHeaderValue(buffer, "Connection:", "close")) {
            request.headers.connection = HttpServerTypes::Connection::Close;
        }
        else {
            request.headers.connection = HttpServerTypes::Connection::Unknown;
        }

        if (ErrorType::Success == findHeaderValue(buffer, "Accept:", "text/html")) {
            request.headers.accept.push_back(HttpServerTypes::Type::TextHtml);
        }
        if (ErrorType::Success == findHeaderValue(buffer, "Accept:", "application/json")) {
            request.headers.accept.push_back(HttpServerTypes::Type::TextHtml);
        }
        if (ErrorType::Success == findHeaderValue(buffer, "Accept:", "application/xml")) {
            request.headers.accept.push_back(HttpServerTypes::Type::TextHtml);
        }

        if (ErrorType::Success == findHeaderValue(buffer, "Content-Encoding:", "gzip")) {
            request.headers.encoding.push_back(HttpServerTypes::Encoding::Gzip);
        }
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Encoding:", "deflate")) {
            request.headers.encoding.push_back(HttpServerTypes::Encoding::Deflate);
        }
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Encoding:", "br")) {
            request.headers.encoding.push_back(HttpServerTypes::Encoding::Br);
        }
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Encoding:", "identity")) {
            request.headers.encoding.push_back(HttpServerTypes::Encoding::Identity);
        }

        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "af")) {request.headers.language.push_back(HttpServerTypes::Language::Afrikaans);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "am")) {request.headers.language.push_back(HttpServerTypes::Language::Amharic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ar-sa")) {request.headers.language.push_back(HttpServerTypes::Language::Arabic_SaudiArabia);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "as")) {request.headers.language.push_back(HttpServerTypes::Language::Assamese);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "az-latn")) {request.headers.language.push_back(HttpServerTypes::Language::Azerbaijani_Latin);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "be")) {request.headers.language.push_back(HttpServerTypes::Language::Belarusian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "bg")) {request.headers.language.push_back(HttpServerTypes::Language::Bulgarian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "bn-BD")) {request.headers.language.push_back(HttpServerTypes::Language::Bangla_Bangladesh);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "bn-IN")) {request.headers.language.push_back(HttpServerTypes::Language::Bangla_India);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "bs")) {request.headers.language.push_back(HttpServerTypes::Language::Bosnian_Latin);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ca")) {request.headers.language.push_back(HttpServerTypes::Language::CatalanSpanish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ca-ES-valencia")) {request.headers.language.push_back(HttpServerTypes::Language::Valencian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "cs")) {request.headers.language.push_back(HttpServerTypes::Language::Czech);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "cy")) {request.headers.language.push_back(HttpServerTypes::Language::Welsh);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "da")) {request.headers.language.push_back(HttpServerTypes::Language::Danish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "de") || std::string::npos != buffer.find("de-de")) {request.headers.language.push_back(HttpServerTypes::Language::German_Germany);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "el")) {request.headers.language.push_back(HttpServerTypes::Language::Greek);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "en-GB")) {request.headers.language.push_back(HttpServerTypes::Language::English_UnitedKingdom);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "en-US")) {request.headers.language.push_back(HttpServerTypes::Language::English_UnitedStates);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "es") || std::string::npos != buffer.find("es-ES")) {request.headers.language.push_back(HttpServerTypes::Language::Spanish_Spain);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "es-US")) {request.headers.language.push_back(HttpServerTypes::Language::Spanish_UnitedStates);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "es-MX")) {request.headers.language.push_back(HttpServerTypes::Language::Spanish_Mexico);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "et")) {request.headers.language.push_back(HttpServerTypes::Language::Estonian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "eu")) {request.headers.language.push_back(HttpServerTypes::Language::Basque);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "fa")) {request.headers.language.push_back(HttpServerTypes::Language::Persian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "fi")) {request.headers.language.push_back(HttpServerTypes::Language::Finnish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "fil-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Filipino);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "fr")) {request.headers.language.push_back(HttpServerTypes::Language::French_France);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "fr-CA")) {request.headers.language.push_back(HttpServerTypes::Language::French_Canada);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ga")) {request.headers.language.push_back(HttpServerTypes::Language::Irish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "gd-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::ScottishGaelic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "gl")) {request.headers.language.push_back(HttpServerTypes::Language::Galician);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "gu")) {request.headers.language.push_back(HttpServerTypes::Language::Gujarati);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ha-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Hausa_Latin);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "he")) {request.headers.language.push_back(HttpServerTypes::Language::Hebrew);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "hi")) {request.headers.language.push_back(HttpServerTypes::Language::Hindi);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "hr")) {request.headers.language.push_back(HttpServerTypes::Language::Croation);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "hu")) {request.headers.language.push_back(HttpServerTypes::Language::Hungarian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "hy")) {request.headers.language.push_back(HttpServerTypes::Language::Armenian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "id")) {request.headers.language.push_back(HttpServerTypes::Language::Indonesian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ig-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Igbo);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "is")) {request.headers.language.push_back(HttpServerTypes::Language::Icelandic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "it") || std::string::npos != buffer.find("it-it")) {request.headers.language.push_back(HttpServerTypes::Language::Italian_Italy);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ja")) {request.headers.language.push_back(HttpServerTypes::Language::Japanese);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ka")) {request.headers.language.push_back(HttpServerTypes::Language::Georgian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "kk")) {request.headers.language.push_back(HttpServerTypes::Language::Kazakh);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "km")) {request.headers.language.push_back(HttpServerTypes::Language::Khmer);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "kn")) {request.headers.language.push_back(HttpServerTypes::Language::Kannada);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ko")) {request.headers.language.push_back(HttpServerTypes::Language::Korean);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "kok")) {request.headers.language.push_back(HttpServerTypes::Language::Konkani);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ku-Arab")) {request.headers.language.push_back(HttpServerTypes::Language::CentralKurdish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ky-Cyrl")) {request.headers.language.push_back(HttpServerTypes::Language::Kyrgyz);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "lb")) {request.headers.language.push_back(HttpServerTypes::Language::LuxemBourgish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "lt")) {request.headers.language.push_back(HttpServerTypes::Language::Lithuanian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "lv")) {request.headers.language.push_back(HttpServerTypes::Language::Latvian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "mi-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Maori);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "mk")) {request.headers.language.push_back(HttpServerTypes::Language::Macedonian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ml")) {request.headers.language.push_back(HttpServerTypes::Language::Malayalam);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "mn-Cyrl")) {request.headers.language.push_back(HttpServerTypes::Language::Mongolian_Cyrillic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "mr")) {request.headers.language.push_back(HttpServerTypes::Language::Maylay_Maylaysia);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "mt")) {request.headers.language.push_back(HttpServerTypes::Language::Maltese);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "nb")) {request.headers.language.push_back(HttpServerTypes::Language::Norwegian_Bokmal);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ne")) {request.headers.language.push_back(HttpServerTypes::Language::Nepali_Nepal);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "nl") || std::string::npos != buffer.find("nl-BE")) {request.headers.language.push_back(HttpServerTypes::Language::Dutch_Netherlands);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "nn")) {request.headers.language.push_back(HttpServerTypes::Language::Norwegian_Nynorsk);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "nso")) {request.headers.language.push_back(HttpServerTypes::Language::SesothoSaLeboa);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "or")) {request.headers.language.push_back(HttpServerTypes::Language::Odia);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "pa")) {request.headers.language.push_back(HttpServerTypes::Language::Punjabi_Gurmukhi);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "pa-Arab")) {request.headers.language.push_back(HttpServerTypes::Language::Punjabi_Arabic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "pl")) {request.headers.language.push_back(HttpServerTypes::Language::Polish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "prs-Arab")) {request.headers.language.push_back(HttpServerTypes::Language::Dari);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "pt-BR")) {request.headers.language.push_back(HttpServerTypes::Language::Portuguese_Brazil);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "pt-PT")) {request.headers.language.push_back(HttpServerTypes::Language::Portuguese_Portugal);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "qut-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Kiche);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "quz")) {request.headers.language.push_back(HttpServerTypes::Language::Quechua_Peru);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ro")) {request.headers.language.push_back(HttpServerTypes::Language::Romanian_Romania);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ru")) {request.headers.language.push_back(HttpServerTypes::Language::Russian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "rw")) {request.headers.language.push_back(HttpServerTypes::Language::Kinyarwanda);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sd-Arab")) {request.headers.language.push_back(HttpServerTypes::Language::Sindhi_Arabic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "si")) {request.headers.language.push_back(HttpServerTypes::Language::Sinhala);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sk")) {request.headers.language.push_back(HttpServerTypes::Language::Slovak);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sl")) {request.headers.language.push_back(HttpServerTypes::Language::Slovenian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sq")) {request.headers.language.push_back(HttpServerTypes::Language::Albanian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sr-Cyrl-BA")) {request.headers.language.push_back(HttpServerTypes::Language::Serbian_Cyrillic_BosniaHerzegovina);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sr-Cyrl-RS")) {request.headers.language.push_back(HttpServerTypes::Language::Serbian_Cyrillic_Serbia);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sr-Latn-RS")) {request.headers.language.push_back(HttpServerTypes::Language::Serbian_Latin_Serbia);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sv")) {request.headers.language.push_back(HttpServerTypes::Language::Swedish_Swedin);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "sw")) {request.headers.language.push_back(HttpServerTypes::Language::Kiswahili);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ta")) {request.headers.language.push_back(HttpServerTypes::Language::Tamil);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "te")) {request.headers.language.push_back(HttpServerTypes::Language::Telugu);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "tg-Cyrl")) {request.headers.language.push_back(HttpServerTypes::Language::Tajik_Cyrillic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "th")) {request.headers.language.push_back(HttpServerTypes::Language::Thai);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ti")) {request.headers.language.push_back(HttpServerTypes::Language::Tigrinya);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "tk-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Turkmen_Latin);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "tn")) {request.headers.language.push_back(HttpServerTypes::Language::Setswana);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "tr")) {request.headers.language.push_back(HttpServerTypes::Language::Turkish);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "tt-Cyrl")) {request.headers.language.push_back(HttpServerTypes::Language::Tatar_Cyrillic);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ug-Arab")) {request.headers.language.push_back(HttpServerTypes::Language::Uyghu);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "uk")) {request.headers.language.push_back(HttpServerTypes::Language::Ukranian);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "ur")) {request.headers.language.push_back(HttpServerTypes::Language::Urda);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "uz-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Uzbek_Latin);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "vi")) {request.headers.language.push_back(HttpServerTypes::Language::Vietnamese);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "wo")) {request.headers.language.push_back(HttpServerTypes::Language::Wolof);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "xh")) {request.headers.language.push_back(HttpServerTypes::Language::isiXhosa);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "yo-Latn")) {request.headers.language.push_back(HttpServerTypes::Language::Yoruba);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "zh-Hans")) {request.headers.language.push_back(HttpServerTypes::Language::Chinese_Simplified);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "zh-Hant")) {request.headers.language.push_back(HttpServerTypes::Language::Chinese_Traditional);}
        if (ErrorType::Success == findHeaderValue(buffer, "Content-Language:", "zu")) {request.headers.language.push_back(HttpServerTypes::Language::isiZulu);}

        size_t contentLengthBegin = buffer.find("Content-Length:");
        size_t contentLengthEnd = buffer.find("\r\n", contentLengthBegin);
        if (std::string::npos != contentLengthBegin) {
            contentLengthBegin += sizeof("Content-Length:");
            request.headers.contentLength = std::stoi(buffer.substr(contentLengthBegin, contentLengthEnd - contentLengthBegin));
        }
        else {
            request.headers.contentLength = 0;
        }

        const Bytes currentCapacity = request.messageBody.capacity();
        const bool messageBodySizeNeedsToIncrease = currentCapacity < request.headers.contentLength;
        request.messageBody.reserve(request.headers.contentLength);
        const bool messageBodySizeWasNotIncreased = currentCapacity < request.messageBody.capacity();
        if (messageBodySizeNeedsToIncrease && messageBodySizeWasNotIncreased) {
            request.messageBody.resize(0);
            return ErrorType::NoMemory;
        }
        else {
            request.messageBody = buffer.substr(buffer.size() - request.headers.contentLength, request.headers.contentLength);
            return ErrorType::Success;
        }
    }

    /**
     * @brief Conver the version to a string
     * @sa HttpServerTypes::Version
     * @param[in] version The version to convert
     * @returns The version as a string.
     */
    const std::string toStringVersion(HttpServerTypes::Version version) {
        if (HttpServerTypes::Version::Http1_0 == version) {
            return std::string("HTTP/1.0");
        }
        else if (HttpServerTypes::Version::Http1_1 == version) {
            return std::string("HTTP/1.1");
        }
        else if (HttpServerTypes::Version::Http2_0 == version) {
            return std::string("HTTP/2.0");
        }
        else if (HttpServerTypes::Version::Http3_0 == version) {
            return std::string("HTTP/3.0");
        }
        else {
            return std::string();
        }
    }

    /**
     * @brief Convert a status code to a string
     * @sa HttpServerTypes::StatusCode
     * @param[in] statusCode The status code to convert
     * @returns The status code as a string.
     */
    const std::string toStringStatusCode(HttpServerTypes::StatusCode statusCode) {
        switch (statusCode) {
            case HttpServerTypes::StatusCode::Unknown:
                return std::string();
            case HttpServerTypes::StatusCode::Continue:
                return std::string("100 Continue");
            case HttpServerTypes::StatusCode::SwitchingProtocols:
                return std::string("101 Switching Protocols");
            case HttpServerTypes::StatusCode::Processing:
                return std::string("102 Processing");
            case HttpServerTypes::StatusCode::EarlyHints:
                return std::string("103 Early Hints");
            case HttpServerTypes::StatusCode::Ok:
                return std::string("200 OK");
            case HttpServerTypes::StatusCode::Created:
                return std::string("201 Created");
            case HttpServerTypes::StatusCode::Accepted:
                return std::string("202 Accepted");
            case HttpServerTypes::StatusCode::NonAuthoritativeInformation:
                return std::string("203 Non-Authoritative Information");
            case HttpServerTypes::StatusCode::NoContent:
                return std::string("204 No Content");
            case HttpServerTypes::StatusCode::ResetContent:
                return std::string("205 Reset Content");
            case HttpServerTypes::StatusCode::PartialContent:
                return std::string("206 Partial Content");
            case HttpServerTypes::StatusCode::MultiStatus:
                return std::string("207 Multi-Status");
            case HttpServerTypes::StatusCode::AlreadyReported:
                return std::string("208 Already Reported");
            case HttpServerTypes::StatusCode::ImUsed:
                return std::string("226 IM Used");
            case HttpServerTypes::StatusCode::MultipleChoices:
                return std::string("300 Multiple Choices");
            case HttpServerTypes::StatusCode::MovedPermanently:
                return std::string("301 Moved Permanently");
            case HttpServerTypes::StatusCode::Found:
                return std::string("302 Found");
            case HttpServerTypes::StatusCode::SeeOther:
                return std::string("303 See Other");
            case HttpServerTypes::StatusCode::NotModified:
                return std::string("304 Not Modified");
            case HttpServerTypes::StatusCode::UseProxy:
                return std::string("305 Use Proxy");
            case HttpServerTypes::StatusCode::TemporaryRedirect:
                return std::string("307 Temporary Redirect");
            case HttpServerTypes::StatusCode::PermanentRedirect:
                return std::string("308 Permanent Redirect");
            case HttpServerTypes::StatusCode::BadRequest:
                return std::string("400 Bad Request");
            case HttpServerTypes::StatusCode::Unauthorized:
                return std::string("401 Unauthorized");
            case HttpServerTypes::StatusCode::PaymentRequired:
                return std::string("402 Payment Required");
            case HttpServerTypes::StatusCode::Forbidden:
                return std::string("403 Forbidden");
            case HttpServerTypes::StatusCode::NotFound:
                return std::string("404 Not Found");
            case HttpServerTypes::StatusCode::MethodNotAllowed:
                return std::string("405 Method Not Allowed");
            case HttpServerTypes::StatusCode::NotAcceptable:
                return std::string("406 Not Acceptable");
            case HttpServerTypes::StatusCode::ProxyAuthenticationRequired:
                return std::string("407 Proxy Authentication Required");
            case HttpServerTypes::StatusCode::RequestTimeout:
                return std::string("408 Request Timeout");
            case HttpServerTypes::StatusCode::Conflict:
                return std::string("409 Conflict");
            case HttpServerTypes::StatusCode::Gone:
                return std::string("410 Gone");
            case HttpServerTypes::StatusCode::LengthRequired:
                return std::string("411 Length Required");
            case HttpServerTypes::StatusCode::PreconditionFailed:
                return std::string("412 Precondition Failed");
            case HttpServerTypes::StatusCode::RequestEntityTooLarge:
                return std::string("413 Request Entity Too Large");
            case HttpServerTypes::StatusCode::RequestUriTooLong:
                return std::string("414 Request-URI Too Long");
            case HttpServerTypes::StatusCode::UnsupportedMediaType:
                return std::string("415 Unsupported Media Type");
            case HttpServerTypes::StatusCode::RequestedRangeNotSatisfiable:
                return std::string("416 Requested Range Not Satisfiable");
            case HttpServerTypes::StatusCode::ExpectationFailed:
                return std::string("417 Expectation Failed");
            case HttpServerTypes::StatusCode::InternalServerError:
                return std::string("500 Internal Server Error");
            case HttpServerTypes::StatusCode::NotImplemented:
                return std::string("501 Not Implemented");
            case HttpServerTypes::StatusCode::BadGateway:
                return std::string("502 Bad Gateway");
            case HttpServerTypes::StatusCode::ServiceUnavailable:
                return std::string("503 Service Unavailable");
            case HttpServerTypes::StatusCode::GatewayTimeout:
                return std::string("504 Gateway Timeout");
            case HttpServerTypes::StatusCode::HttpVersionNotSupported:
                return std::string("505 HTTP Version Not Supported");
            default:
                return std::string();
        }
    }

    /**
     * @brief Conver a content type to a string
     * @sa HttpServerTypes::Type
     * @param[in] contentType
     * @returns The content type as a string.
     */
    const std::string toStringContentType(HttpServerTypes::Type contentType) {
        if (HttpServerTypes::Type::TextHtml == contentType) {
            return std::string("Content-Type: text/html");
        }
        else if (HttpServerTypes::Type::ApplicationJson == contentType) {
            return std::string("Content-Type: application/json");
        }
        else if (HttpServerTypes::Type::ApplicationXml == contentType) {
            return std::string("Content-Type: application/xml");
        }
        else if (HttpServerTypes::Type::ImagePng == contentType) {
            return std::string("Content-Type: image/png");
        }
        else if (HttpServerTypes::Type::ImageJpeg == contentType) {
            return std::string("Content-Type: image/jpeg");
        }
        else if (HttpServerTypes::Type::ImageGif == contentType) {
            return std::string("Content-Type: image/gif");
        }
        else if (HttpServerTypes::Type::ImageSvgXml == contentType) {
            return std::string("Content-Type: image/svg+xml");
        }
        else if (HttpServerTypes::Type::ImageTiff == contentType) {
            return std::string("Content-Type: image/tiff");
        }
        else if (HttpServerTypes::Type::TextCss == contentType) {
            return std::string("Content-Type: text/css");
        }
        else if (HttpServerTypes::Type::TextJavascript == contentType) {
            return std::string("Content-Type: text/javascript");
        }
        else {
            return std::string();
        }
    }

    /**
     * @brief Convert an encoding to a string
     * @sa HttpServerTypes::Encoding
     * @param[in] encoding The encoding
     * @returns The encoding as a string.
     */
    const std::string toStringEncoding(const std::vector<HttpServerTypes::Encoding> encoding) {
        std::string encodings("Content-Encoding: ");

        if (encoding.size() == 0) {
            return std::string();
        }

        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpServerTypes::Encoding::Gzip)) {
            encodings.append("gzip, ");
        }
        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpServerTypes::Encoding::Deflate)) {
            encodings.append("deflate, ");
        }
        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpServerTypes::Encoding::Br)) {
            encodings.append("br, ");
        }
        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpServerTypes::Encoding::Identity)) {
            encodings.append("identity, ");
        }

        if (encodings.size() > 0) {
            //Get rid of the trailing commad and space
            encodings.resize(encodings.size() - 2);
        }


        return encodings;
    }

    /**
     * @brief Convert a content language to a string.
     * @sa HttpServerTypes::Language
     * @param[in] contentLanguage The content language.
     * @returns The content language as a string.
     */
    const std::string toStringContentLanguage(const std::vector<HttpServerTypes::Language> contentLanguage) {
        std::string contentLanguages("Content-Language: ");

        if (contentLanguage.size() == 0) {
            return std::string();
        }

        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Afrikaans)) {contentLanguages.append("af, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Amharic)) {contentLanguages.append("am, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Arabic_SaudiArabia)) {contentLanguages.append("ar-sa, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Assamese)) {contentLanguages.append("as, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Azerbaijani_Latin)) {contentLanguages.append("az-latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Belarusian)) {contentLanguages.append("be, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Bulgarian)) {contentLanguages.append("bg, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Bangla_Bangladesh)) {contentLanguages.append("bn-BD, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Bangla_India)) {contentLanguages.append("bn-IN, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Bosnian_Latin)) {contentLanguages.append("bs, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::CatalanSpanish)) {contentLanguages.append("ca, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Valencian)) {contentLanguages.append("ca-ES-valencia, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Czech)) {contentLanguages.append("cs, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Welsh)) {contentLanguages.append("cy, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Danish)) {contentLanguages.append("da, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::German_Germany)) {contentLanguages.append("de, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Greek)) {contentLanguages.append("el, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::English_UnitedKingdom)) {contentLanguages.append("en-GB, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::English_UnitedStates)) {contentLanguages.append("en-US, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Spanish_Spain)) {contentLanguages.append("es-ES, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Spanish_UnitedStates)) {contentLanguages.append("es-US, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Spanish_Mexico)) {contentLanguages.append("es-MX, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Estonian)) {contentLanguages.append("et, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Basque)) {contentLanguages.append("eu, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Persian)) {contentLanguages.append("fa, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Finnish)) {contentLanguages.append("fi, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Filipino)) {contentLanguages.append("fil-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::French_France)) {contentLanguages.append("fr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::French_Canada)) {contentLanguages.append("fr-CA, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Irish)) {contentLanguages.append("ga, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::ScottishGaelic)) {contentLanguages.append("gd-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Galician)) {contentLanguages.append("gl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Gujarati)) {contentLanguages.append("gu, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Hausa_Latin)) {contentLanguages.append("ha-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Hebrew)) {contentLanguages.append("he, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Hindi)) {contentLanguages.append("hi, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Croation)) {contentLanguages.append("hr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Hungarian)) {contentLanguages.append("hu, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Armenian)) {contentLanguages.append("hy, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Indonesian)) {contentLanguages.append("id, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Igbo)) {contentLanguages.append("ig-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Icelandic)) {contentLanguages.append("is, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Italian_Italy)) {contentLanguages.append("it, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Japanese)) {contentLanguages.append("ja, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Georgian)) {contentLanguages.append("ka, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Kazakh)) {contentLanguages.append("kk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Khmer)) {contentLanguages.append("km, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Kannada)) {contentLanguages.append("kn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Korean)) {contentLanguages.append("ko, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Konkani)) {contentLanguages.append("kok, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::CentralKurdish)) {contentLanguages.append("ku-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Kyrgyz)) {contentLanguages.append("ky-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::LuxemBourgish)) {contentLanguages.append("lb, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Lithuanian)) {contentLanguages.append("lt, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Latvian)) {contentLanguages.append("lv, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Maori)) {contentLanguages.append("mi-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Macedonian)) {contentLanguages.append("mk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Malayalam)) {contentLanguages.append("ml, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Mongolian_Cyrillic)) {contentLanguages.append("mn-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Maylay_Maylaysia)) {contentLanguages.append("mr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Maltese)) {contentLanguages.append("mt, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Norwegian_Bokmal)) {contentLanguages.append("nb, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Nepali_Nepal)) {contentLanguages.append("ne, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Dutch_Netherlands)) {contentLanguages.append("nl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Norwegian_Nynorsk)) {contentLanguages.append("nn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::SesothoSaLeboa)) {contentLanguages.append("nso, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Odia)) {contentLanguages.append("or, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Punjabi_Gurmukhi)) {contentLanguages.append("pa, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Punjabi_Arabic)) {contentLanguages.append("pa-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Polish)) {contentLanguages.append("pl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Dari)) {contentLanguages.append("prs-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Portuguese_Brazil)) {contentLanguages.append("pt-BR, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Portuguese_Portugal)) {contentLanguages.append("pt-PT, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Kiche)) {contentLanguages.append("qut-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Quechua_Peru)) {contentLanguages.append("quz, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Romanian_Romania)) {contentLanguages.append("ro, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Russian)) {contentLanguages.append("ru, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Kinyarwanda)) {contentLanguages.append("rw, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Sindhi_Arabic)) {contentLanguages.append("sd-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Sinhala)) {contentLanguages.append("si, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Slovak)) {contentLanguages.append("sk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Slovenian)) {contentLanguages.append("sl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Albanian)) {contentLanguages.append("sq, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Serbian_Cyrillic_BosniaHerzegovina)) {contentLanguages.append("sr-Cyrl-BA, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Serbian_Latin_Serbia)) {contentLanguages.append("sr-Latn-RS, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Swedish_Swedin)) {contentLanguages.append("sv, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Kiswahili)) {contentLanguages.append("sw, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Tamil)) {contentLanguages.append("ta, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Telugu)) {contentLanguages.append("te, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Tajik_Cyrillic)) {contentLanguages.append("tg-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Thai)) {contentLanguages.append("th, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Tigrinya)) {contentLanguages.append("ti, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Turkmen_Latin)) {contentLanguages.append("tk-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Setswana)) {contentLanguages.append("tn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Turkish)) {contentLanguages.append("tr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Tatar_Cyrillic)) {contentLanguages.append("tt-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Uyghu)) {contentLanguages.append("ug-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Ukranian)) {contentLanguages.append("uk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Urda)) {contentLanguages.append("ur, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Uzbek_Latin)) {contentLanguages.append("uz-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Vietnamese)) {contentLanguages.append("vi, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Wolof)) {contentLanguages.append("wo, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::isiXhosa)) {contentLanguages.append("xh, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Yoruba)) {contentLanguages.append("yo-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Chinese_Simplified)) {contentLanguages.append("zh-Hans, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::Chinese_Traditional)) {contentLanguages.append("zh-Hant, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpServerTypes::Language::isiZulu)) {contentLanguages.append("zu, ");}

        //Get rid of the trailing comma and space.
        contentLanguages.resize(contentLanguages.size() - 2);

        return contentLanguages;
    }

    /**
     * @brief Convert and http server type into a string
     * @sa HttpServerTypes::Type
     * @param[in] type The http server type
     * @returns The http server type as a string
     */
    const std::string toStringHttpServerType(const HttpServerTypes::Type type) {
        switch (type) {
            case HttpServerTypes::Type::TextHtml:
                return "Content-Type: text/html";
            case HttpServerTypes::Type::ApplicationJson:
                return "Content-Type: application/json";
            case HttpServerTypes::Type::ApplicationXml:
                return "Content-Type: application/xml";
            case HttpServerTypes::Type::ApplicationXhtmlXml:
                return "Content-Type: application/xhtml+xml";
            case HttpServerTypes::Type::ApplicationOctetStream:
                return "Content-Type: application/octet-stream";
            case HttpServerTypes::Type::ApplicationFormUrlencoded:
                return "Content-Type: application/x-www-form-urlencoded";
            case HttpServerTypes::Type::ImageGif:
                return "Content-Type: image/gif";
            case HttpServerTypes::Type::ImageJpeg:
                return "Content-Type: image/jpeg";
            case HttpServerTypes::Type::ImagePng:
                return "Content-Type: image/png";
            case HttpServerTypes::Type::ImageTiff:
                return "Content-Type: image/tiff";
            case HttpServerTypes::Type::ImageSvgXml:
                return "Content-Type: image/svg+xml";
            default:
                return "";
        }
    }

    /**
     * @brief Converts a numeric content length into a string.
     * @param[in] length The content length
     * @returns The content length as a string.
     */
    const std::string toStringContentLength(const Bytes length) {
        return std::string("Content-Length: ").append(std::to_string(length));
    }

    /**
     * @brief Converts an HttpServerTypes::Response to ascii suitable for sending on the network.
     * @param response The response to convert.
     * @param buffer The buffer to hold the ascii conversion in that will be sent on the network to the client.
     * @returns ErrorType Success if the response header and body were appended
     * @returns ErrorType::NoData if no response header was added.
     * @returns ErrorType::Failure otherwise.
     * @post If the response has been cleared then no header will be appended. Suitable for messages that send
     *       a large body and need multiple segments to send.
     * @sa clearResponseHeader
     */
    ErrorType toHttpResponse(const HttpServerTypes::Response &response, std::string &buffer) {
        buffer.resize(0);

        //Keep checking the buffer size to make sure that it changes with each append. If it doesn't,
        //it means the toString* function returned an empty string because this response does not contain
        //that header.
        Bytes currentBufferSize = buffer.size();
        buffer.append(toStringVersion(response.statusLine.version));
        if (currentBufferSize != buffer.size()) {
            buffer.push_back(' ');
        }
        else {
            //If the response doesn't even contain the status line the don't include it all since it won't be valid anyway.
            buffer.append(response.messageBody);
            return ErrorType::NoData;
        }

        currentBufferSize = buffer.size();
        buffer.append(toStringStatusCode(response.statusLine.statusCode));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(toStringEncoding(response.representationHeaders.contentEncoding));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(toStringContentType(response.representationHeaders.contentType));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(toStringContentLength(response.representationHeaders.contentLength));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(toStringContentLanguage(response.representationHeaders.contentLanguage));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }

        buffer.append("Connection: close\r\n");
        buffer.append("Server: AutomaticPetFeeder\r\n");
        buffer.append("\r\n");
        buffer.append(response.messageBody);

        return ErrorType::Success;
    }

    /**
     * @brief Clears the header of a response
     * @param response The response to clear.
     * @return ErrorType::Success always
     * @post If the response is clear, then the next time you send an http response it will only contain the body.
     */
    static ErrorType clearResponseHeader(HttpServerTypes::Response &response) {
        response.representationHeaders.contentEncoding = std::vector<HttpServerTypes::Encoding>();
        response.representationHeaders.contentLanguage = std::vector<HttpServerTypes::Language>();
        response.representationHeaders.contentLength = 0;
        response.representationHeaders.contentType = HttpServerTypes::Type::Unknown;

        memset(&response.responseHeaders.date, 0, sizeof(response.responseHeaders.date));
        memset(&response.responseHeaders.server, 0, sizeof(response.responseHeaders.server));

        response.statusLine.statusCode = HttpServerTypes::StatusCode::Unknown;
        response.statusLine.version = HttpServerTypes::Version::Unknown;

        return ErrorType::Success;
    }

    /**
     * @brief Given a file name, automatically determine it's content-type by examining the extension.
     * @param filename The file name to examine for the content type.
     * @param error The error that occurred while trying to determine the content type.
     * @return The content type of the file. Will be HttpServerTypes::Type::Unknown if an error occurred.
     * @post The error may be any of ErrorType::Success, ErrorType::InvalidParameter or ErrorType::NotSupported;
     */
    static HttpServerTypes::Type determineContentType(const std::string &filename, ErrorType &error) {
        error = ErrorType::Success;

        if (filename.empty()) {
            error = ErrorType::InvalidParameter;
            return HttpServerTypes::Type::Unknown;
        }

        for (int i = filename.length()-1; i >= 0; i--) {
            if (filename.at(i) == '.') {
                if (0 == strncmp(&filename.at(i), ".html", sizeof(".html")-1)) {
                    return HttpServerTypes::Type::TextHtml;
                }
                else if (0 == strncmp(&filename.at(i), ".json", sizeof(".json")-1)) {
                    return HttpServerTypes::Type::ApplicationJson;
                }
                else if (0 == strncmp(&filename.at(i), ".css", sizeof(".css")-1)) {
                    return HttpServerTypes::Type::TextCss;
                }
                else if (0 == strncmp(&filename.at(i), ".js", sizeof(".js")-1)) {
                    return HttpServerTypes::Type::TextJavascript;
                }
                else if (0 == strncmp(&filename.at(i), ".png", sizeof(".png")-1)) {
                    return HttpServerTypes::Type::ImagePng;
                }
                else if (0 == strncmp(&filename.at(i), ".jpg", sizeof(".jpg")-1)) {
                    return HttpServerTypes::Type::ImageJpeg;
                }
                else if (0 == strncmp(&filename.at(i), ".gif", sizeof(".gif")-1)) {
                    return HttpServerTypes::Type::ImageGif;
                }
                else if (0 == strncmp(&filename.at(i), ".svg", sizeof(".svg")-1)) {
                    return HttpServerTypes::Type::ImageSvgXml;
                }
            }
        }

        error = ErrorType::NotSupported;
        return HttpServerTypes::Type::Unknown;
    }
};
#endif // __HTTP_SERVER_SBSTRACTION_HPP__