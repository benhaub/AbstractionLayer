#ifndef __HTTP_TYPES_HPP__
#define __HTTP_TYPES_HPP__

//C++
#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <cassert>
#include <algorithm>
//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"

/**
 * @namespace HttpTypes
 * @brief Http types
 */
namespace HttpTypes {

    /**
     * @enum Type
     * @brief Http MIME types
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

        /// @brief Constructor
        Headers() {
            reset();
        }

        /**
         * @brief Reset the headers to their initial state
         */
        void reset() {
            userAgent.clear();
            host.clear();
            referer.clear();
            language.clear();
            accept.clear();
            encoding.clear();
            contentType = Type::Unknown;
            connection = Connection::Unknown;
            contentLength = 0;
        }
    };

    /**
     * @struct CustomHeaders
     * @brief Any custom headers that you include with your request or response
     */
    struct CustomHeaders {
        std::array<std::string, 10> headers; ///< Array for the custom headers

        /// @brief Constructor
        CustomHeaders() {
            reset();
        }

        /**
         * @brief Reset the custom headers to their initial state
         */
        void reset() {
            for (auto &header : headers) {
                header.clear();
            }
        }
    };

    /**
     * @struct RequestLine
     * @brief The request line of an Http request
     */
    struct RequestLine {
        Method method;   ///< Method
        std::string uri; ///< Universal Resource Identifier
        Version version; ///< Http protocol version

        /// @brief Constructor
        RequestLine() {
            reset();
        }

        /**
         * @brief Reset the request line to its initial state
         */
        void reset() {
            method = Method::Unknown;
            uri.clear();
            version = Version::Unknown;
        }
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

        /// @brief Constructor
        Request() {
            reset();
        }

        /**
         * @brief Reset the request to its initial state
         * @details sending and receiving use the state of the request to determine if they should attempt to parse the headers or not.
         *          Use this function to reset the request so that it will be parsed again.
         */
        void reset() {
            requestLine.reset();
            headers.reset();
            customHeaders.reset();
            messageBody.clear();
        }
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

        /// @brief Constructor
        StatusLine() {
            reset();
        }

        /**
         * @brief Reset the status line to its initial state
         */
        void reset() {
            version = Version::Unknown;
            statusCode = StatusCode::Unknown;
        }
    };

    /**
     * @struct ResponseHeaders
     * @brief The response headers
     */
    struct ResponseHeaders {
        std::array<char, 64> server; ///< Server name
        std::array<char, 32> date;   ///< Date.

        /// @brief Constructor
        ResponseHeaders() {
            reset();
        }

        /**
         * @brief Reset the response headers to their initial state
         */
        void reset() {
            server.fill(0);
            date.fill(0);
        }
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

        /// @brief Constructor
        RepresentationHeaders() {
            reset();
        }

        /**
         * @brief Reset the representation headers to their initial state
         */
        void reset() {
            contentType = Type::Unknown;
            contentEncoding.clear();
            contentLength = 0;
            contentLanguage.clear();
        }
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

        /// @brief Constructor
        Response() {
            reset();
        }

        /**
         * @brief Reset the response to its initial state
         * @details sending and receiving use the state of the response to determine if they should attempt to parse the headers or not.
         *          Use this function to reset the response so that it will be parsed again.
         */
        void reset() {
            statusLine.reset();
            responseHeaders.reset();
            representationHeaders.reset();
            messageBody.clear();
        }
    };

    /**
     * @struct Url
     * @brief A url broken down into its components
     */
    struct Url {
        std::string_view scheme;     ///< The scheme of the url
        std::string_view domainName; ///< The domain name of the url
        std::string_view path;       ///< The path of the url
    };

    /**
     * @brief Search a raw http request for the specified header.
     * @param[in] request The raw http request.
     * @param[in] headerName The header to serach for
     * @param[in] value The value of the header to match against.
     * @returns ErrorType::Success if the http request contains the header.
     * @returns ErrorType::Failure if the http request does not contain the header.
     */
    inline ErrorType FindHeaderValue(const std::string &request, const char headerName[], const char value[]) {
        const size_t theIndexThatTheHeaderStartsAt = request.find(headerName);
        const size_t theIndexThatTheHeaderEndsAt = request.find("\r\n", theIndexThatTheHeaderStartsAt);

        assert(theIndexThatTheHeaderStartsAt <= theIndexThatTheHeaderEndsAt);

        if (std::string::npos != theIndexThatTheHeaderStartsAt && std::string::npos != theIndexThatTheHeaderEndsAt) {
            std::string_view requestView = std::string_view(request).substr(theIndexThatTheHeaderStartsAt, theIndexThatTheHeaderEndsAt);
            if (std::string::npos != requestView.find(value)) {
                return ErrorType::Success;
            }
        }

        return ErrorType::Failure;
    }

    /**
     * @brief Convert a raw Http request to an AbstractionLayer http request.
     * @details This function can be called iteratively to read the request headers as you receive them.
     * @sa HttpTypes::Request
     * @param[in] buffer The raw data of the http request.
     * @param[out] request The converted htpp request.
     * @returns The http request.
     */
    inline ErrorType ToHttpRequest(const std::string &buffer, HttpTypes::Request &request) {
        size_t uriStartIndex, uriEndIndex = 0;

        if (buffer.size() <= 0) {
            return ErrorType::NoData;
        }

        if (std::string::npos != (uriStartIndex = buffer.find("GET"))) {
            request.requestLine.method = HttpTypes::Method::Get;
            uriStartIndex += sizeof("GET");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("POST"))) {
            request.requestLine.method = HttpTypes::Method::Post;
            uriStartIndex += sizeof("POST");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("PUT"))) {
            request.requestLine.method = HttpTypes::Method::Put;
            uriStartIndex += sizeof("PUT");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("CONNECT"))) {
            request.requestLine.method = HttpTypes::Method::Connect;
            uriStartIndex += sizeof("CONNECT");
        }
        else if (std::string::npos != (uriStartIndex = buffer.find("DELETE"))) {
            request.requestLine.method = HttpTypes::Method::Delete;
            uriStartIndex += sizeof("DELETE");
        }

        if (std::string::npos != (uriEndIndex = buffer.find("HTTP/1.0"))) {
            request.requestLine.version = HttpTypes::Version::Http1_0;
            uriEndIndex -= 1;
        }
        else if (std::string::npos != (uriEndIndex = buffer.find("HTTP/1.1"))) {
            request.requestLine.version = HttpTypes::Version::Http1_1;
            uriEndIndex -= 1;
        }
        else if (std::string::npos != (uriEndIndex = buffer.find("HTTP/2.0"))) {
            request.requestLine.version = HttpTypes::Version::Http2_0;
            uriEndIndex -= 1;
        }
        else if (std::string::npos != (uriEndIndex = buffer.find("HTTP/3.0"))) {
            request.requestLine.version = HttpTypes::Version::Http3_0;
            uriEndIndex -= 1;
        }

        if (std::string::npos != uriEndIndex && std::string::npos != uriStartIndex) {

            if (uriStartIndex >= uriEndIndex) {
                request.requestLine.uri.clear();
            }
            else {
                std::string_view bufferView = std::string_view(buffer).substr(uriStartIndex, uriEndIndex - uriStartIndex);
                request.requestLine.uri.assign(bufferView);
            }
        }

        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "text/html")) {
            request.headers.contentType = HttpTypes::Type::TextHtml;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "application/json")) {
            request.headers.contentType = HttpTypes::Type::ApplicationJson;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "application/xml")) {
            request.headers.contentType = HttpTypes::Type::ApplicationXml;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "text/css")) {
            request.headers.contentType = HttpTypes::Type::TextCss;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "text/javascript")) {
            request.headers.contentType = HttpTypes::Type::TextJavascript;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "image/png")) {
            request.headers.contentType = HttpTypes::Type::ImagePng;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "image/jpeg")) {
            request.headers.contentType = HttpTypes::Type::ImageJpeg;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "image/svg+xml")) {
            request.headers.contentType = HttpTypes::Type::ImageSvgXml;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Content-Type", "image/tiff")) {
            request.headers.contentType = HttpTypes::Type::ImageTiff;
        }

        if (ErrorType::Success == FindHeaderValue(buffer, "Connection:", "keep-alive")) {
            request.headers.connection = HttpTypes::Connection::KeepAlive;
        }
        else if (ErrorType::Success == FindHeaderValue(buffer, "Connection:", "close")) {
            request.headers.connection = HttpTypes::Connection::Close;
        }

        if (ErrorType::Success == FindHeaderValue(buffer, "Accept:", "text/html")) {
            request.headers.accept.push_back(HttpTypes::Type::TextHtml);
        }
        if (ErrorType::Success == FindHeaderValue(buffer, "Accept:", "application/json")) {
            request.headers.accept.push_back(HttpTypes::Type::TextHtml);
        }
        if (ErrorType::Success == FindHeaderValue(buffer, "Accept:", "application/xml")) {
            request.headers.accept.push_back(HttpTypes::Type::TextHtml);
        }

        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Encoding:", "gzip")) {
            request.headers.encoding.push_back(HttpTypes::Encoding::Gzip);
        }
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Encoding:", "deflate")) {
            request.headers.encoding.push_back(HttpTypes::Encoding::Deflate);
        }
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Encoding:", "br")) {
            request.headers.encoding.push_back(HttpTypes::Encoding::Br);
        }
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Encoding:", "identity")) {
            request.headers.encoding.push_back(HttpTypes::Encoding::Identity);
        }

        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "af")) {request.headers.language.push_back(HttpTypes::Language::Afrikaans);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "am")) {request.headers.language.push_back(HttpTypes::Language::Amharic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ar-sa")) {request.headers.language.push_back(HttpTypes::Language::Arabic_SaudiArabia);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "as")) {request.headers.language.push_back(HttpTypes::Language::Assamese);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "az-latn")) {request.headers.language.push_back(HttpTypes::Language::Azerbaijani_Latin);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "be")) {request.headers.language.push_back(HttpTypes::Language::Belarusian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "bg")) {request.headers.language.push_back(HttpTypes::Language::Bulgarian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "bn-BD")) {request.headers.language.push_back(HttpTypes::Language::Bangla_Bangladesh);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "bn-IN")) {request.headers.language.push_back(HttpTypes::Language::Bangla_India);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "bs")) {request.headers.language.push_back(HttpTypes::Language::Bosnian_Latin);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ca")) {request.headers.language.push_back(HttpTypes::Language::CatalanSpanish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ca-ES-valencia")) {request.headers.language.push_back(HttpTypes::Language::Valencian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "cs")) {request.headers.language.push_back(HttpTypes::Language::Czech);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "cy")) {request.headers.language.push_back(HttpTypes::Language::Welsh);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "da")) {request.headers.language.push_back(HttpTypes::Language::Danish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "de") || std::string::npos != buffer.find("de-de")) {request.headers.language.push_back(HttpTypes::Language::German_Germany);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "el")) {request.headers.language.push_back(HttpTypes::Language::Greek);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "en-GB")) {request.headers.language.push_back(HttpTypes::Language::English_UnitedKingdom);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "en-US")) {request.headers.language.push_back(HttpTypes::Language::English_UnitedStates);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "es") || std::string::npos != buffer.find("es-ES")) {request.headers.language.push_back(HttpTypes::Language::Spanish_Spain);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "es-US")) {request.headers.language.push_back(HttpTypes::Language::Spanish_UnitedStates);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "es-MX")) {request.headers.language.push_back(HttpTypes::Language::Spanish_Mexico);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "et")) {request.headers.language.push_back(HttpTypes::Language::Estonian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "eu")) {request.headers.language.push_back(HttpTypes::Language::Basque);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "fa")) {request.headers.language.push_back(HttpTypes::Language::Persian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "fi")) {request.headers.language.push_back(HttpTypes::Language::Finnish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "fil-Latn")) {request.headers.language.push_back(HttpTypes::Language::Filipino);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "fr")) {request.headers.language.push_back(HttpTypes::Language::French_France);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "fr-CA")) {request.headers.language.push_back(HttpTypes::Language::French_Canada);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ga")) {request.headers.language.push_back(HttpTypes::Language::Irish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "gd-Latn")) {request.headers.language.push_back(HttpTypes::Language::ScottishGaelic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "gl")) {request.headers.language.push_back(HttpTypes::Language::Galician);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "gu")) {request.headers.language.push_back(HttpTypes::Language::Gujarati);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ha-Latn")) {request.headers.language.push_back(HttpTypes::Language::Hausa_Latin);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "he")) {request.headers.language.push_back(HttpTypes::Language::Hebrew);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "hi")) {request.headers.language.push_back(HttpTypes::Language::Hindi);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "hr")) {request.headers.language.push_back(HttpTypes::Language::Croation);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "hu")) {request.headers.language.push_back(HttpTypes::Language::Hungarian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "hy")) {request.headers.language.push_back(HttpTypes::Language::Armenian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "id")) {request.headers.language.push_back(HttpTypes::Language::Indonesian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ig-Latn")) {request.headers.language.push_back(HttpTypes::Language::Igbo);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "is")) {request.headers.language.push_back(HttpTypes::Language::Icelandic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "it") || std::string::npos != buffer.find("it-it")) {request.headers.language.push_back(HttpTypes::Language::Italian_Italy);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ja")) {request.headers.language.push_back(HttpTypes::Language::Japanese);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ka")) {request.headers.language.push_back(HttpTypes::Language::Georgian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "kk")) {request.headers.language.push_back(HttpTypes::Language::Kazakh);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "km")) {request.headers.language.push_back(HttpTypes::Language::Khmer);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "kn")) {request.headers.language.push_back(HttpTypes::Language::Kannada);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ko")) {request.headers.language.push_back(HttpTypes::Language::Korean);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "kok")) {request.headers.language.push_back(HttpTypes::Language::Konkani);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ku-Arab")) {request.headers.language.push_back(HttpTypes::Language::CentralKurdish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ky-Cyrl")) {request.headers.language.push_back(HttpTypes::Language::Kyrgyz);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "lb")) {request.headers.language.push_back(HttpTypes::Language::LuxemBourgish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "lt")) {request.headers.language.push_back(HttpTypes::Language::Lithuanian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "lv")) {request.headers.language.push_back(HttpTypes::Language::Latvian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "mi-Latn")) {request.headers.language.push_back(HttpTypes::Language::Maori);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "mk")) {request.headers.language.push_back(HttpTypes::Language::Macedonian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ml")) {request.headers.language.push_back(HttpTypes::Language::Malayalam);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "mn-Cyrl")) {request.headers.language.push_back(HttpTypes::Language::Mongolian_Cyrillic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "mr")) {request.headers.language.push_back(HttpTypes::Language::Maylay_Maylaysia);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "mt")) {request.headers.language.push_back(HttpTypes::Language::Maltese);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "nb")) {request.headers.language.push_back(HttpTypes::Language::Norwegian_Bokmal);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ne")) {request.headers.language.push_back(HttpTypes::Language::Nepali_Nepal);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "nl") || std::string::npos != buffer.find("nl-BE")) {request.headers.language.push_back(HttpTypes::Language::Dutch_Netherlands);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "nn")) {request.headers.language.push_back(HttpTypes::Language::Norwegian_Nynorsk);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "nso")) {request.headers.language.push_back(HttpTypes::Language::SesothoSaLeboa);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "or")) {request.headers.language.push_back(HttpTypes::Language::Odia);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "pa")) {request.headers.language.push_back(HttpTypes::Language::Punjabi_Gurmukhi);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "pa-Arab")) {request.headers.language.push_back(HttpTypes::Language::Punjabi_Arabic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "pl")) {request.headers.language.push_back(HttpTypes::Language::Polish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "prs-Arab")) {request.headers.language.push_back(HttpTypes::Language::Dari);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "pt-BR")) {request.headers.language.push_back(HttpTypes::Language::Portuguese_Brazil);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "pt-PT")) {request.headers.language.push_back(HttpTypes::Language::Portuguese_Portugal);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "qut-Latn")) {request.headers.language.push_back(HttpTypes::Language::Kiche);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "quz")) {request.headers.language.push_back(HttpTypes::Language::Quechua_Peru);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ro")) {request.headers.language.push_back(HttpTypes::Language::Romanian_Romania);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ru")) {request.headers.language.push_back(HttpTypes::Language::Russian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "rw")) {request.headers.language.push_back(HttpTypes::Language::Kinyarwanda);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sd-Arab")) {request.headers.language.push_back(HttpTypes::Language::Sindhi_Arabic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "si")) {request.headers.language.push_back(HttpTypes::Language::Sinhala);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sk")) {request.headers.language.push_back(HttpTypes::Language::Slovak);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sl")) {request.headers.language.push_back(HttpTypes::Language::Slovenian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sq")) {request.headers.language.push_back(HttpTypes::Language::Albanian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sr-Cyrl-BA")) {request.headers.language.push_back(HttpTypes::Language::Serbian_Cyrillic_BosniaHerzegovina);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sr-Cyrl-RS")) {request.headers.language.push_back(HttpTypes::Language::Serbian_Cyrillic_Serbia);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sr-Latn-RS")) {request.headers.language.push_back(HttpTypes::Language::Serbian_Latin_Serbia);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sv")) {request.headers.language.push_back(HttpTypes::Language::Swedish_Swedin);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "sw")) {request.headers.language.push_back(HttpTypes::Language::Kiswahili);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ta")) {request.headers.language.push_back(HttpTypes::Language::Tamil);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "te")) {request.headers.language.push_back(HttpTypes::Language::Telugu);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "tg-Cyrl")) {request.headers.language.push_back(HttpTypes::Language::Tajik_Cyrillic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "th")) {request.headers.language.push_back(HttpTypes::Language::Thai);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ti")) {request.headers.language.push_back(HttpTypes::Language::Tigrinya);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "tk-Latn")) {request.headers.language.push_back(HttpTypes::Language::Turkmen_Latin);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "tn")) {request.headers.language.push_back(HttpTypes::Language::Setswana);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "tr")) {request.headers.language.push_back(HttpTypes::Language::Turkish);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "tt-Cyrl")) {request.headers.language.push_back(HttpTypes::Language::Tatar_Cyrillic);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ug-Arab")) {request.headers.language.push_back(HttpTypes::Language::Uyghu);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "uk")) {request.headers.language.push_back(HttpTypes::Language::Ukranian);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "ur")) {request.headers.language.push_back(HttpTypes::Language::Urda);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "uz-Latn")) {request.headers.language.push_back(HttpTypes::Language::Uzbek_Latin);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "vi")) {request.headers.language.push_back(HttpTypes::Language::Vietnamese);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "wo")) {request.headers.language.push_back(HttpTypes::Language::Wolof);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "xh")) {request.headers.language.push_back(HttpTypes::Language::isiXhosa);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "yo-Latn")) {request.headers.language.push_back(HttpTypes::Language::Yoruba);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "zh-Hans")) {request.headers.language.push_back(HttpTypes::Language::Chinese_Simplified);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "zh-Hant")) {request.headers.language.push_back(HttpTypes::Language::Chinese_Traditional);}
        if (ErrorType::Success == FindHeaderValue(buffer, "Content-Language:", "zu")) {request.headers.language.push_back(HttpTypes::Language::isiZulu);}

        size_t contentLengthBegin = buffer.find("Content-Length:");
        size_t contentLengthEnd = buffer.find("\r\n", contentLengthBegin);
        if (std::string::npos != contentLengthBegin) {
            contentLengthBegin += sizeof("Content-Length:");
            std::string_view bufferView = std::string_view(buffer).substr(contentLengthBegin, contentLengthEnd - contentLengthBegin);
            request.headers.contentLength = std::stoul(bufferView.data(), nullptr, 10);
        }

        return ErrorType::Success;
    }

    /**
     * @brief Converts HttpTypes::Method to a string.
     * @param[in] method The method of the request
     * @sa HttpTypes::Request
     * @returns The HttpTypes::Method as a string.
     * @returns An empty string if the method is unknown or not supported.
     * @post A space character is included with each method so it can be directly appended to the URI
     */
    inline const std::string ToStringMethod(const HttpTypes::Method method) {
        switch (method) {
            case HttpTypes::Method::Connect:
                return "CONNECT ";
            case HttpTypes::Method::Delete:
                return "DELETE ";
            case HttpTypes::Method::Get:
                return "GET ";
            case HttpTypes::Method::Head:
                return "HEAD ";
            case HttpTypes::Method::Options:
                return "OPTIONS ";
            case HttpTypes::Method::Post:
                return "POST ";
            case HttpTypes::Method::Put:
                return "PUT ";
            case HttpTypes::Method::Trace:
                return "TRACE ";
            default:
                return std::string();
        }
    }

    /**
     * @brief Convert the version to a string
     * @sa HttpTypes::Version
     * @param[in] version The version to convert
     * @returns The version as a string.
     */
    inline const std::string ToStringVersion(HttpTypes::Version version) {
        if (HttpTypes::Version::Http1_0 == version) {
            return std::string("HTTP/1.0");
        }
        else if (HttpTypes::Version::Http1_1 == version) {
            return std::string("HTTP/1.1");
        }
        else if (HttpTypes::Version::Http2_0 == version) {
            return std::string("HTTP/2.0");
        }
        else if (HttpTypes::Version::Http3_0 == version) {
            return std::string("HTTP/3.0");
        }
        else {
            return std::string();
        }
    }

    /**
     * @brief Convert a status code to a string
     * @sa HttpTypes::StatusCode
     * @param[in] statusCode The status code to convert
     * @returns The status code as a string.
     */
    inline const std::string ToStringStatusCode(HttpTypes::StatusCode statusCode) {
        switch (statusCode) {
            case HttpTypes::StatusCode::Continue:
                return std::string("100 Continue");
            case HttpTypes::StatusCode::SwitchingProtocols:
                return std::string("101 Switching Protocols");
            case HttpTypes::StatusCode::Processing:
                return std::string("102 Processing");
            case HttpTypes::StatusCode::EarlyHints:
                return std::string("103 Early Hints");
            case HttpTypes::StatusCode::Ok:
                return std::string("200 OK");
            case HttpTypes::StatusCode::Created:
                return std::string("201 Created");
            case HttpTypes::StatusCode::Accepted:
                return std::string("202 Accepted");
            case HttpTypes::StatusCode::NonAuthoritativeInformation:
                return std::string("203 Non-Authoritative Information");
            case HttpTypes::StatusCode::NoContent:
                return std::string("204 No Content");
            case HttpTypes::StatusCode::ResetContent:
                return std::string("205 Reset Content");
            case HttpTypes::StatusCode::PartialContent:
                return std::string("206 Partial Content");
            case HttpTypes::StatusCode::MultiStatus:
                return std::string("207 Multi-Status");
            case HttpTypes::StatusCode::AlreadyReported:
                return std::string("208 Already Reported");
            case HttpTypes::StatusCode::ImUsed:
                return std::string("226 IM Used");
            case HttpTypes::StatusCode::MultipleChoices:
                return std::string("300 Multiple Choices");
            case HttpTypes::StatusCode::MovedPermanently:
                return std::string("301 Moved Permanently");
            case HttpTypes::StatusCode::Found:
                return std::string("302 Found");
            case HttpTypes::StatusCode::SeeOther:
                return std::string("303 See Other");
            case HttpTypes::StatusCode::NotModified:
                return std::string("304 Not Modified");
            case HttpTypes::StatusCode::UseProxy:
                return std::string("305 Use Proxy");
            case HttpTypes::StatusCode::TemporaryRedirect:
                return std::string("307 Temporary Redirect");
            case HttpTypes::StatusCode::PermanentRedirect:
                return std::string("308 Permanent Redirect");
            case HttpTypes::StatusCode::BadRequest:
                return std::string("400 Bad Request");
            case HttpTypes::StatusCode::Unauthorized:
                return std::string("401 Unauthorized");
            case HttpTypes::StatusCode::PaymentRequired:
                return std::string("402 Payment Required");
            case HttpTypes::StatusCode::Forbidden:
                return std::string("403 Forbidden");
            case HttpTypes::StatusCode::NotFound:
                return std::string("404 Not Found");
            case HttpTypes::StatusCode::MethodNotAllowed:
                return std::string("405 Method Not Allowed");
            case HttpTypes::StatusCode::NotAcceptable:
                return std::string("406 Not Acceptable");
            case HttpTypes::StatusCode::ProxyAuthenticationRequired:
                return std::string("407 Proxy Authentication Required");
            case HttpTypes::StatusCode::RequestTimeout:
                return std::string("408 Request Timeout");
            case HttpTypes::StatusCode::Conflict:
                return std::string("409 Conflict");
            case HttpTypes::StatusCode::Gone:
                return std::string("410 Gone");
            case HttpTypes::StatusCode::LengthRequired:
                return std::string("411 Length Required");
            case HttpTypes::StatusCode::PreconditionFailed:
                return std::string("412 Precondition Failed");
            case HttpTypes::StatusCode::RequestEntityTooLarge:
                return std::string("413 Request Entity Too Large");
            case HttpTypes::StatusCode::RequestUriTooLong:
                return std::string("414 Request-URI Too Long");
            case HttpTypes::StatusCode::UnsupportedMediaType:
                return std::string("415 Unsupported Media Type");
            case HttpTypes::StatusCode::RequestedRangeNotSatisfiable:
                return std::string("416 Requested Range Not Satisfiable");
            case HttpTypes::StatusCode::ExpectationFailed:
                return std::string("417 Expectation Failed");
            case HttpTypes::StatusCode::InternalServerError:
                return std::string("500 Internal Server Error");
            case HttpTypes::StatusCode::NotImplemented:
                return std::string("501 Not Implemented");
            case HttpTypes::StatusCode::BadGateway:
                return std::string("502 Bad Gateway");
            case HttpTypes::StatusCode::ServiceUnavailable:
                return std::string("503 Service Unavailable");
            case HttpTypes::StatusCode::GatewayTimeout:
                return std::string("504 Gateway Timeout");
            case HttpTypes::StatusCode::HttpVersionNotSupported:
                return std::string("505 HTTP Version Not Supported");
            default:
                return std::string();
        }
    }

    /**
     * @brief Conver a content type to a string
     * @sa HttpTypes::Type
     * @param[in] contentType
     * @returns The content type as a string.
     */
    inline const std::string ToStringContentType(HttpTypes::Type contentType) {
        if (HttpTypes::Type::TextHtml == contentType) {
            return std::string("Content-Type: text/html");
        }
        else if (HttpTypes::Type::ApplicationJson == contentType) {
            return std::string("Content-Type: application/json");
        }
        else if (HttpTypes::Type::ApplicationXml == contentType) {
            return std::string("Content-Type: application/xml");
        }
        else if (HttpTypes::Type::ImagePng == contentType) {
            return std::string("Content-Type: image/png");
        }
        else if (HttpTypes::Type::ImageJpeg == contentType) {
            return std::string("Content-Type: image/jpeg");
        }
        else if (HttpTypes::Type::ImageGif == contentType) {
            return std::string("Content-Type: image/gif");
        }
        else if (HttpTypes::Type::ImageSvgXml == contentType) {
            return std::string("Content-Type: image/svg+xml");
        }
        else if (HttpTypes::Type::ImageTiff == contentType) {
            return std::string("Content-Type: image/tiff");
        }
        else if (HttpTypes::Type::TextCss == contentType) {
            return std::string("Content-Type: text/css");
        }
        else if (HttpTypes::Type::TextJavascript == contentType) {
            return std::string("Content-Type: text/javascript");
        }
        else {
            return std::string();
        }
    }

    /**
     * @brief Convert an http request to a string of bytes suitable for sending on the network.
     * @param[in] request The request to convert
     * @param[out] data The data to send on the network.
     */
    inline ErrorType FromHttpRequest(const HttpTypes::Request &request, std::string &data) {
        data.resize(0);

        //Keep checking the buffer size to make sure that it changes with each append. If it doesn't,
        //it means the toString* function returned an empty string because this request does not contain
        //that header.
        Bytes currentBufferSize = data.size();
        data.append(ToStringMethod(request.requestLine.method));
        if (currentBufferSize == data.size()) {
            return ErrorType::InvalidParameter;
        }
        currentBufferSize = data.size();
        data.append(request.requestLine.uri);
        if (currentBufferSize == data.size()) {
            return ErrorType::InvalidParameter;
        }
        else {
            data.push_back(' ');
        }
        currentBufferSize = data.size();
        data.append(ToStringVersion(request.requestLine.version));
        if (currentBufferSize == data.size()) {
            return ErrorType::InvalidParameter;
        }
        else {
            data.append("\r\n");
        }

        currentBufferSize = data.size();
        data.append("Host: ").append(request.headers.host);
        if (currentBufferSize != data.size()) {
            data.append("\r\n");
        }
        currentBufferSize = data.size();
        data.append("User-Agent: ").append(request.headers.userAgent);
        if (currentBufferSize != data.size()) {
            data.append("\r\n");
        }
        currentBufferSize = data.size();
        for (const auto &mimeType : request.headers.accept) {
            data.append("Accept: ").append(ToStringContentType(mimeType));
            if (currentBufferSize != data.size()) {
                data.push_back(',');
            }
        }
        if (currentBufferSize != data.size()) {
            data.append("\r\n");
        }

        for (const auto &header : request.customHeaders.headers) {
            if (header.size() > 0) {
                data.append(header);
                if (currentBufferSize != data.size()) {
                    data.append("\r\n");
                }
                currentBufferSize = data.size();
            }
        }

        data.append("Connection: close\r\n");
        data.append("\r\n");

        if (HttpTypes::Method::Post == request.requestLine.method) {
            data.append(request.messageBody);
        }

        return ErrorType::Success;
    }

    /**
     * @brief Convert an encoding to a string
     * @sa HttpTypes::Encoding
     * @param[in] encoding The encoding
     * @returns The encoding as a string.
     */
    inline const std::string ToStringEncoding(const std::vector<HttpTypes::Encoding> encoding) {
        std::string encodings("Content-Encoding: ");

        if (encoding.size() == 0) {
            return std::string();
        }

        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpTypes::Encoding::Gzip)) {
            encodings.append("gzip, ");
        }
        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpTypes::Encoding::Deflate)) {
            encodings.append("deflate, ");
        }
        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpTypes::Encoding::Br)) {
            encodings.append("br, ");
        }
        if (encoding.end() != std::find(encoding.begin(), encoding.end(), HttpTypes::Encoding::Identity)) {
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
     * @sa HttpTypes::Language
     * @param[in] contentLanguage The content language.
     * @returns The content language as a string.
     */
    inline const std::string ToStringContentLanguage(const std::vector<HttpTypes::Language> contentLanguage) {
        std::string contentLanguages("Content-Language: ");

        if (contentLanguage.size() == 0) {
            return std::string();
        }

        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Afrikaans)) {contentLanguages.append("af, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Amharic)) {contentLanguages.append("am, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Arabic_SaudiArabia)) {contentLanguages.append("ar-sa, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Assamese)) {contentLanguages.append("as, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Azerbaijani_Latin)) {contentLanguages.append("az-latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Belarusian)) {contentLanguages.append("be, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Bulgarian)) {contentLanguages.append("bg, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Bangla_Bangladesh)) {contentLanguages.append("bn-BD, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Bangla_India)) {contentLanguages.append("bn-IN, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Bosnian_Latin)) {contentLanguages.append("bs, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::CatalanSpanish)) {contentLanguages.append("ca, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Valencian)) {contentLanguages.append("ca-ES-valencia, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Czech)) {contentLanguages.append("cs, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Welsh)) {contentLanguages.append("cy, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Danish)) {contentLanguages.append("da, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::German_Germany)) {contentLanguages.append("de, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Greek)) {contentLanguages.append("el, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::English_UnitedKingdom)) {contentLanguages.append("en-GB, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::English_UnitedStates)) {contentLanguages.append("en-US, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Spanish_Spain)) {contentLanguages.append("es-ES, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Spanish_UnitedStates)) {contentLanguages.append("es-US, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Spanish_Mexico)) {contentLanguages.append("es-MX, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Estonian)) {contentLanguages.append("et, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Basque)) {contentLanguages.append("eu, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Persian)) {contentLanguages.append("fa, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Finnish)) {contentLanguages.append("fi, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Filipino)) {contentLanguages.append("fil-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::French_France)) {contentLanguages.append("fr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::French_Canada)) {contentLanguages.append("fr-CA, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Irish)) {contentLanguages.append("ga, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::ScottishGaelic)) {contentLanguages.append("gd-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Galician)) {contentLanguages.append("gl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Gujarati)) {contentLanguages.append("gu, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Hausa_Latin)) {contentLanguages.append("ha-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Hebrew)) {contentLanguages.append("he, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Hindi)) {contentLanguages.append("hi, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Croation)) {contentLanguages.append("hr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Hungarian)) {contentLanguages.append("hu, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Armenian)) {contentLanguages.append("hy, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Indonesian)) {contentLanguages.append("id, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Igbo)) {contentLanguages.append("ig-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Icelandic)) {contentLanguages.append("is, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Italian_Italy)) {contentLanguages.append("it, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Japanese)) {contentLanguages.append("ja, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Georgian)) {contentLanguages.append("ka, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Kazakh)) {contentLanguages.append("kk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Khmer)) {contentLanguages.append("km, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Kannada)) {contentLanguages.append("kn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Korean)) {contentLanguages.append("ko, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Konkani)) {contentLanguages.append("kok, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::CentralKurdish)) {contentLanguages.append("ku-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Kyrgyz)) {contentLanguages.append("ky-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::LuxemBourgish)) {contentLanguages.append("lb, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Lithuanian)) {contentLanguages.append("lt, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Latvian)) {contentLanguages.append("lv, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Maori)) {contentLanguages.append("mi-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Macedonian)) {contentLanguages.append("mk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Malayalam)) {contentLanguages.append("ml, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Mongolian_Cyrillic)) {contentLanguages.append("mn-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Maylay_Maylaysia)) {contentLanguages.append("mr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Maltese)) {contentLanguages.append("mt, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Norwegian_Bokmal)) {contentLanguages.append("nb, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Nepali_Nepal)) {contentLanguages.append("ne, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Dutch_Netherlands)) {contentLanguages.append("nl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Norwegian_Nynorsk)) {contentLanguages.append("nn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::SesothoSaLeboa)) {contentLanguages.append("nso, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Odia)) {contentLanguages.append("or, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Punjabi_Gurmukhi)) {contentLanguages.append("pa, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Punjabi_Arabic)) {contentLanguages.append("pa-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Polish)) {contentLanguages.append("pl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Dari)) {contentLanguages.append("prs-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Portuguese_Brazil)) {contentLanguages.append("pt-BR, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Portuguese_Portugal)) {contentLanguages.append("pt-PT, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Kiche)) {contentLanguages.append("qut-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Quechua_Peru)) {contentLanguages.append("quz, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Romanian_Romania)) {contentLanguages.append("ro, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Russian)) {contentLanguages.append("ru, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Kinyarwanda)) {contentLanguages.append("rw, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Sindhi_Arabic)) {contentLanguages.append("sd-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Sinhala)) {contentLanguages.append("si, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Slovak)) {contentLanguages.append("sk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Slovenian)) {contentLanguages.append("sl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Albanian)) {contentLanguages.append("sq, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Serbian_Cyrillic_BosniaHerzegovina)) {contentLanguages.append("sr-Cyrl-BA, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Serbian_Latin_Serbia)) {contentLanguages.append("sr-Latn-RS, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Swedish_Swedin)) {contentLanguages.append("sv, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Kiswahili)) {contentLanguages.append("sw, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Tamil)) {contentLanguages.append("ta, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Telugu)) {contentLanguages.append("te, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Tajik_Cyrillic)) {contentLanguages.append("tg-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Thai)) {contentLanguages.append("th, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Tigrinya)) {contentLanguages.append("ti, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Turkmen_Latin)) {contentLanguages.append("tk-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Setswana)) {contentLanguages.append("tn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Turkish)) {contentLanguages.append("tr, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Tatar_Cyrillic)) {contentLanguages.append("tt-Cyrl, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Uyghu)) {contentLanguages.append("ug-Arab, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Ukranian)) {contentLanguages.append("uk, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Urda)) {contentLanguages.append("ur, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Uzbek_Latin)) {contentLanguages.append("uz-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Vietnamese)) {contentLanguages.append("vi, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Wolof)) {contentLanguages.append("wo, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::isiXhosa)) {contentLanguages.append("xh, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Yoruba)) {contentLanguages.append("yo-Latn, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Chinese_Simplified)) {contentLanguages.append("zh-Hans, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::Chinese_Traditional)) {contentLanguages.append("zh-Hant, ");}
        if (contentLanguage.end() != std::find(contentLanguage.begin(), contentLanguage.end(), HttpTypes::Language::isiZulu)) {contentLanguages.append("zu, ");}

        //Get rid of the trailing comma and space.
        contentLanguages.resize(contentLanguages.size() - 2);

        return contentLanguages;
    }

    /**
     * @brief Convert and http server type into a string
     * @sa HttpTypes::Type
     * @param[in] type The http server type
     * @returns The http server type as a string
     */
    inline const std::string ToStringHttpServerType(const HttpTypes::Type type) {
        switch (type) {
            case HttpTypes::Type::TextHtml:
                return "Content-Type: text/html";
            case HttpTypes::Type::ApplicationJson:
                return "Content-Type: application/json";
            case HttpTypes::Type::ApplicationXml:
                return "Content-Type: application/xml";
            case HttpTypes::Type::ApplicationXhtmlXml:
                return "Content-Type: application/xhtml+xml";
            case HttpTypes::Type::ApplicationOctetStream:
                return "Content-Type: application/octet-stream";
            case HttpTypes::Type::ApplicationFormUrlencoded:
                return "Content-Type: application/x-www-form-urlencoded";
            case HttpTypes::Type::ImageGif:
                return "Content-Type: image/gif";
            case HttpTypes::Type::ImageJpeg:
                return "Content-Type: image/jpeg";
            case HttpTypes::Type::ImagePng:
                return "Content-Type: image/png";
            case HttpTypes::Type::ImageTiff:
                return "Content-Type: image/tiff";
            case HttpTypes::Type::ImageSvgXml:
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
    inline const std::string ToStringContentLength(const Bytes length) {
        return std::string("Content-Length: ").append(std::to_string(length));
    }

    /**
     * @brief Converts an HttpTypes::Response to ascii suitable for sending on the network.
     * @param response The response to convert.
     * @param buffer The buffer to hold the ascii conversion in that will be sent on the network to the client.
     * @returns ErrorType Success if the response header and body were appended
     * @returns ErrorType::NoData if no response header was added.
     * @returns ErrorType::Failure otherwise.
     * @post If the response has been cleared then no header will be appended. Suitable for messages that send
     *       a large body and need multiple segments to send.
     * @sa clearResponseHeader
     */
    inline ErrorType ToHttpResponse(const HttpTypes::Response &response, std::string &buffer) {
        buffer.resize(0);

        //Keep checking the buffer size to make sure that it changes with each append. If it doesn't,
        //it means the toString* function returned an empty string because this response does not contain
        //that header.
        Bytes currentBufferSize = buffer.size();
        buffer.append(ToStringVersion(response.statusLine.version));
        if (currentBufferSize != buffer.size()) {
            buffer.push_back(' ');
        }
        else {
            //If the response doesn't even contain the status line the don't include it all since it won't be valid anyway.
            buffer.append(response.messageBody);
            return ErrorType::NoData;
        }

        currentBufferSize = buffer.size();
        buffer.append(ToStringStatusCode(response.statusLine.statusCode));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(ToStringEncoding(response.representationHeaders.contentEncoding));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(ToStringContentType(response.representationHeaders.contentType));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(ToStringContentLength(response.representationHeaders.contentLength));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }
        currentBufferSize = buffer.size();
        buffer.append(ToStringContentLanguage(response.representationHeaders.contentLanguage));
        if (currentBufferSize != buffer.size()) {
            buffer.append("\r\n");
        }

        buffer.append("Connection: close\r\n");
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
    inline ErrorType ClearResponseHeader(HttpTypes::Response &response) {
        response.representationHeaders.contentEncoding = std::vector<HttpTypes::Encoding>();
        response.representationHeaders.contentLanguage = std::vector<HttpTypes::Language>();
        response.representationHeaders.contentLength = 0;
        response.representationHeaders.contentType = HttpTypes::Type::Unknown;

        response.responseHeaders.date.fill(0);
        response.responseHeaders.server.fill(0);

        response.statusLine.statusCode = HttpTypes::StatusCode::Unknown;
        response.statusLine.version = HttpTypes::Version::Unknown;

        return ErrorType::Success;
    }

    /**
     * @brief Given a file name, automatically determine it's content-type by examining the extension.
     * @param filename The file name to examine for the content type.
     * @param error The error that occurred while trying to determine the content type.
     * @return The content type of the file. Will be HttpTypes::Type::Unknown if an error occurred.
     * @post The error may be any of ErrorType::Success, ErrorType::InvalidParameter or ErrorType::NotSupported;
     */
    inline HttpTypes::Type DetermineContentType(const std::string &filename, ErrorType &error) {
        error = ErrorType::Success;

        if (filename.empty()) {
            error = ErrorType::InvalidParameter;
            return HttpTypes::Type::Unknown;
        }

        for (int i = filename.length()-1; i >= 0; i--) {
            if (filename.at(i) == '.') {
                if (0 == strncmp(&filename.at(i), ".html", sizeof(".html")-1)) {
                    return HttpTypes::Type::TextHtml;
                }
                else if (0 == strncmp(&filename.at(i), ".json", sizeof(".json")-1)) {
                    return HttpTypes::Type::ApplicationJson;
                }
                else if (0 == strncmp(&filename.at(i), ".css", sizeof(".css")-1)) {
                    return HttpTypes::Type::TextCss;
                }
                else if (0 == strncmp(&filename.at(i), ".js", sizeof(".js")-1)) {
                    return HttpTypes::Type::TextJavascript;
                }
                else if (0 == strncmp(&filename.at(i), ".png", sizeof(".png")-1)) {
                    return HttpTypes::Type::ImagePng;
                }
                else if (0 == strncmp(&filename.at(i), ".jpg", sizeof(".jpg")-1)) {
                    return HttpTypes::Type::ImageJpeg;
                }
                else if (0 == strncmp(&filename.at(i), ".gif", sizeof(".gif")-1)) {
                    return HttpTypes::Type::ImageGif;
                }
                else if (0 == strncmp(&filename.at(i), ".svg", sizeof(".svg")-1)) {
                    return HttpTypes::Type::ImageSvgXml;
                }
            }
        }

        error = ErrorType::NotSupported;
        return HttpTypes::Type::Unknown;
    }

    inline ErrorType DisectUrl(std::string_view url, HttpTypes::Url &disectedUrl) {
        size_t endOfScheme = url.find_first_of(':');
        disectedUrl.scheme = url.substr(0, endOfScheme);
        std::string_view startOfDomainName = url.substr(endOfScheme + sizeof("//"));
        size_t endOfDomainName = startOfDomainName.rfind('/');
        disectedUrl.domainName = startOfDomainName.substr(0, endOfDomainName);
        disectedUrl.path = startOfDomainName.substr(endOfDomainName);
        return ErrorType::Success;
    }
};

#endif //__HTTP_TYPES_HPP__