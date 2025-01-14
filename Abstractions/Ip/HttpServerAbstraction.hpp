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

    struct MessageBody {
        std::string data;
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
        MessageBody messageBody;
    };

    struct Response {
        //TODO: It might be the in the response we have content and in the request we have accept.
        //TODO: I don't know what should go in here. I could maybe even typedef this to a Request.
        std::string data;
    };
};

class HttpServerAbstraction : public IpServerAbstraction {

    public:
    HttpServerAbstraction() : IpServerAbstraction() {}
    virtual ~HttpServerAbstraction() = default;

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Woverloaded-virtual"

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