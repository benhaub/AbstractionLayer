#ifndef __HTTP_SERVER_MODULE_HPP__
#define __HTTP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "HttpServerAbstraction.hpp"
#include "IpServer.hpp"
#include "OperatingSystemModule.hpp"
//TI Drivers
#include "ti/drivers/net/wifi/simplelink.h"

//Global because the SimpleLink C handlers are outside the scope of this class and need to be able to access the queue.
static constexpr std::array<char, OperatingSystemTypes::MaxQueueNameLength> SimpleLinkEventQueue = {"SLEventQueue"};

//I am aware of the Network Services API that includes various servers and clients pre-made by TI and that one of them is an HTTP server.
//For some reason, the example code in the SDK does not use this API so unfortunately I only became aware of this API after I started working
//on this implementation using the lower level Host Driver API.
class HttpServer final : public HttpServerAbstraction {
    public:
    HttpServer() : HttpServerAbstraction() {
        OperatingSystem::Instance().createQueue(SimpleLinkEventQueue, sizeof(SlNetAppRequest_t), 10);
    }

    ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;

    ErrorType sendBlocking(const HttpTypes::Response &response, const Milliseconds timeout, const Socket socket) override;
    ErrorType receiveBlocking(HttpTypes::Request &request, const Milliseconds timeout, Socket &socket) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpTypes::Request> buffer)> callback) override;

    void setNetwork(NetworkAbstraction &network) override {
        HttpServerAbstraction::setNetwork(network);
        _ipServer.setNetwork(*_network);
    }

    private:
    IpServer _ipServer;

    ErrorType toHttpRequest(const SlNetAppRequest_t &netAppRequest, HttpTypes::Request &request);
    HttpTypes::Version fromStringVersion(const std::string version) {
        if (std::string::npos != version.find("1.0")) {
            return HttpTypes::Version::Http1_0;
        }
        else if (std::string::npos != version.find("1.1")) {
            return HttpTypes::Version::Http1_1;
        }
        else if (std::string::npos != version.find("2.0")) {
            return HttpTypes::Version::Http2_0;
        }
        else if (std::string::npos != version.find("3.0")) {
            return HttpTypes::Version::Http3_0;
        }
        else {
            return HttpTypes::Version::Unknown;
        }
    }
    HttpTypes::Type fromStringContentType(const std::string contentType) {
        if (std::string::npos != contentType.find("text/html")) {
            return HttpTypes::Type::TextHtml;
        }
        else if (std::string::npos != contentType.find("application/json")) {
            return HttpTypes::Type::ApplicationJson;
        }
        else if (std::string::npos != contentType.find("application/xml")) {
            return HttpTypes::Type::ApplicationXml;
        }
        else {
            return HttpTypes::Type::Unknown;
        }
    }
    HttpTypes::Connection fromStringConnection(const std::string connection) {
        if (std::string::npos != connection.find("keep-alive")) {
            return HttpTypes::Connection::KeepAlive;
        }
        else if (std::string::npos != connection.find("close")) {
            return HttpTypes::Connection::Close;
        }
        else {
            return HttpTypes::Connection::Unknown;
        }
    }
    std::vector<HttpTypes::Type> fromStringAccept(const std::string accept) {
        std::vector<HttpTypes::Type> accepts;

        if (std::string::npos != accept.find("text/html")) {
            accepts.push_back(HttpTypes::Type::TextHtml);
        }
        if (std::string::npos != accept.find("application/json")) {
            accepts.push_back(HttpTypes::Type::ApplicationJson);
        }
        if (std::string::npos != accept.find("application/xml")) {
            accepts.push_back(HttpTypes::Type::ApplicationXml);
        }

        return accepts;
    }
    std::vector<HttpTypes::Encoding> fromStringEncoding(const std::string encoding) {
        std::vector<HttpTypes::Encoding> encodings;

        if (std::string::npos != encoding.find("gzip")) {
            encodings.push_back(HttpTypes::Encoding::Gzip);
        }
        if (std::string::npos != encoding.find("deflate")) {
            encodings.push_back(HttpTypes::Encoding::Deflate);
        }
        if (std::string::npos != encoding.find("br")) {
            encodings.push_back(HttpTypes::Encoding::Br);
        }
        if (std::string::npos != encoding.find("identity")) {
            encodings.push_back(HttpTypes::Encoding::Identity);
        }

        return encodings;
    }
    std::vector<HttpTypes::Language> fromStringAcceptLanguage(const std::string acceptLanguage) {
        std::vector<HttpTypes::Language> acceptLanguages;

        if (std::string::npos != acceptLanguage.find("af")) {acceptLanguages.push_back(HttpTypes::Language::Afrikaans);}
        if (std::string::npos != acceptLanguage.find("am")) {acceptLanguages.push_back(HttpTypes::Language::Amharic);}
        if (std::string::npos != acceptLanguage.find("ar-sa")) {acceptLanguages.push_back(HttpTypes::Language::Arabic_SaudiArabia);}
        if (std::string::npos != acceptLanguage.find("as")) {acceptLanguages.push_back(HttpTypes::Language::Assamese);}
        if (std::string::npos != acceptLanguage.find("az-latn")) {acceptLanguages.push_back(HttpTypes::Language::Azerbaijani_Latin);}
        if (std::string::npos != acceptLanguage.find("be")) {acceptLanguages.push_back(HttpTypes::Language::Belarusian);}
        if (std::string::npos != acceptLanguage.find("bg")) {acceptLanguages.push_back(HttpTypes::Language::Bulgarian);}
        if (std::string::npos != acceptLanguage.find("bn-BD")) {acceptLanguages.push_back(HttpTypes::Language::Bangla_Bangladesh);}
        if (std::string::npos != acceptLanguage.find("bn-IN")) {acceptLanguages.push_back(HttpTypes::Language::Bangla_India);}
        if (std::string::npos != acceptLanguage.find("bs")) {acceptLanguages.push_back(HttpTypes::Language::Bosnian_Latin);}
        if (std::string::npos != acceptLanguage.find("ca")) {acceptLanguages.push_back(HttpTypes::Language::CatalanSpanish);}
        if (std::string::npos != acceptLanguage.find("ca-ES-valencia")) {acceptLanguages.push_back(HttpTypes::Language::Valencian);}
        if (std::string::npos != acceptLanguage.find("cs")) {acceptLanguages.push_back(HttpTypes::Language::Czech);}
        if (std::string::npos != acceptLanguage.find("cy")) {acceptLanguages.push_back(HttpTypes::Language::Welsh);}
        if (std::string::npos != acceptLanguage.find("da")) {acceptLanguages.push_back(HttpTypes::Language::Danish);}
        if (std::string::npos != acceptLanguage.find("de") || std::string::npos != acceptLanguage.find("de-de")) {acceptLanguages.push_back(HttpTypes::Language::German_Germany);}
        if (std::string::npos != acceptLanguage.find("el")) {acceptLanguages.push_back(HttpTypes::Language::Greek);}
        if (std::string::npos != acceptLanguage.find("en-GB")) {acceptLanguages.push_back(HttpTypes::Language::English_UnitedKingdom);}
        if (std::string::npos != acceptLanguage.find("en-US")) {acceptLanguages.push_back(HttpTypes::Language::English_UnitedStates);}
        if (std::string::npos != acceptLanguage.find("es") || std::string::npos != acceptLanguage.find("es-ES")) {acceptLanguages.push_back(HttpTypes::Language::Spanish_Spain);}
        if (std::string::npos != acceptLanguage.find("es-US")) {acceptLanguages.push_back(HttpTypes::Language::Spanish_UnitedStates);}
        if (std::string::npos != acceptLanguage.find("es-MX")) {acceptLanguages.push_back(HttpTypes::Language::Spanish_Mexico);}
        if (std::string::npos != acceptLanguage.find("et")) {acceptLanguages.push_back(HttpTypes::Language::Estonian);}
        if (std::string::npos != acceptLanguage.find("eu")) {acceptLanguages.push_back(HttpTypes::Language::Basque);}
        if (std::string::npos != acceptLanguage.find("fa")) {acceptLanguages.push_back(HttpTypes::Language::Persian);}
        if (std::string::npos != acceptLanguage.find("fi")) {acceptLanguages.push_back(HttpTypes::Language::Finnish);}
        if (std::string::npos != acceptLanguage.find("fil-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Filipino);}
        if (std::string::npos != acceptLanguage.find("fr")) {acceptLanguages.push_back(HttpTypes::Language::French_France);}
        if (std::string::npos != acceptLanguage.find("fr-CA")) {acceptLanguages.push_back(HttpTypes::Language::French_Canada);}
        if (std::string::npos != acceptLanguage.find("ga")) {acceptLanguages.push_back(HttpTypes::Language::Irish);}
        if (std::string::npos != acceptLanguage.find("gd-Latn")) {acceptLanguages.push_back(HttpTypes::Language::ScottishGaelic);}
        if (std::string::npos != acceptLanguage.find("gl")) {acceptLanguages.push_back(HttpTypes::Language::Galician);}
        if (std::string::npos != acceptLanguage.find("gu")) {acceptLanguages.push_back(HttpTypes::Language::Gujarati);}
        if (std::string::npos != acceptLanguage.find("ha-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Hausa_Latin);}
        if (std::string::npos != acceptLanguage.find("he")) {acceptLanguages.push_back(HttpTypes::Language::Hebrew);}
        if (std::string::npos != acceptLanguage.find("hi")) {acceptLanguages.push_back(HttpTypes::Language::Hindi);}
        if (std::string::npos != acceptLanguage.find("hr")) {acceptLanguages.push_back(HttpTypes::Language::Croation);}
        if (std::string::npos != acceptLanguage.find("hu")) {acceptLanguages.push_back(HttpTypes::Language::Hungarian);}
        if (std::string::npos != acceptLanguage.find("hy")) {acceptLanguages.push_back(HttpTypes::Language::Armenian);}
        if (std::string::npos != acceptLanguage.find("id")) {acceptLanguages.push_back(HttpTypes::Language::Indonesian);}
        if (std::string::npos != acceptLanguage.find("ig-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Igbo);}
        if (std::string::npos != acceptLanguage.find("is")) {acceptLanguages.push_back(HttpTypes::Language::Icelandic);}
        if (std::string::npos != acceptLanguage.find("it") || std::string::npos != acceptLanguage.find("it-it")) {acceptLanguages.push_back(HttpTypes::Language::Italian_Italy);}
        if (std::string::npos != acceptLanguage.find("ja")) {acceptLanguages.push_back(HttpTypes::Language::Japanese);}
        if (std::string::npos != acceptLanguage.find("ka")) {acceptLanguages.push_back(HttpTypes::Language::Georgian);}
        if (std::string::npos != acceptLanguage.find("kk")) {acceptLanguages.push_back(HttpTypes::Language::Kazakh);}
        if (std::string::npos != acceptLanguage.find("km")) {acceptLanguages.push_back(HttpTypes::Language::Khmer);}
        if (std::string::npos != acceptLanguage.find("kn")) {acceptLanguages.push_back(HttpTypes::Language::Kannada);}
        if (std::string::npos != acceptLanguage.find("ko")) {acceptLanguages.push_back(HttpTypes::Language::Korean);}
        if (std::string::npos != acceptLanguage.find("kok")) {acceptLanguages.push_back(HttpTypes::Language::Konkani);}
        if (std::string::npos != acceptLanguage.find("ku-Arab")) {acceptLanguages.push_back(HttpTypes::Language::CentralKurdish);}
        if (std::string::npos != acceptLanguage.find("ky-Cyrl")) {acceptLanguages.push_back(HttpTypes::Language::Kyrgyz);}
        if (std::string::npos != acceptLanguage.find("lb")) {acceptLanguages.push_back(HttpTypes::Language::LuxemBourgish);}
        if (std::string::npos != acceptLanguage.find("lt")) {acceptLanguages.push_back(HttpTypes::Language::Lithuanian);}
        if (std::string::npos != acceptLanguage.find("lv")) {acceptLanguages.push_back(HttpTypes::Language::Latvian);}
        if (std::string::npos != acceptLanguage.find("mi-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Maori);}
        if (std::string::npos != acceptLanguage.find("mk")) {acceptLanguages.push_back(HttpTypes::Language::Macedonian);}
        if (std::string::npos != acceptLanguage.find("ml")) {acceptLanguages.push_back(HttpTypes::Language::Malayalam);}
        if (std::string::npos != acceptLanguage.find("mn-Cyrl")) {acceptLanguages.push_back(HttpTypes::Language::Mongolian_Cyrillic);}
        if (std::string::npos != acceptLanguage.find("mr")) {acceptLanguages.push_back(HttpTypes::Language::Maylay_Maylaysia);}
        if (std::string::npos != acceptLanguage.find("mt")) {acceptLanguages.push_back(HttpTypes::Language::Maltese);}
        if (std::string::npos != acceptLanguage.find("nb")) {acceptLanguages.push_back(HttpTypes::Language::Norwegian_Bokmal);}
        if (std::string::npos != acceptLanguage.find("ne")) {acceptLanguages.push_back(HttpTypes::Language::Nepali_Nepal);}
        if (std::string::npos != acceptLanguage.find("nl") || std::string::npos != acceptLanguage.find("nl-BE")) {acceptLanguages.push_back(HttpTypes::Language::Dutch_Netherlands);}
        if (std::string::npos != acceptLanguage.find("nn")) {acceptLanguages.push_back(HttpTypes::Language::Norwegian_Nynorsk);}
        if (std::string::npos != acceptLanguage.find("nso")) {acceptLanguages.push_back(HttpTypes::Language::SesothoSaLeboa);}
        if (std::string::npos != acceptLanguage.find("or")) {acceptLanguages.push_back(HttpTypes::Language::Odia);}
        if (std::string::npos != acceptLanguage.find("pa")) {acceptLanguages.push_back(HttpTypes::Language::Punjabi_Gurmukhi);}
        if (std::string::npos != acceptLanguage.find("pa-Arab")) {acceptLanguages.push_back(HttpTypes::Language::Punjabi_Arabic);}
        if (std::string::npos != acceptLanguage.find("pl")) {acceptLanguages.push_back(HttpTypes::Language::Polish);}
        if (std::string::npos != acceptLanguage.find("prs-Arab")) {acceptLanguages.push_back(HttpTypes::Language::Dari);}
        if (std::string::npos != acceptLanguage.find("pt-BR")) {acceptLanguages.push_back(HttpTypes::Language::Portuguese_Brazil);}
        if (std::string::npos != acceptLanguage.find("pt-PT")) {acceptLanguages.push_back(HttpTypes::Language::Portuguese_Portugal);}
        if (std::string::npos != acceptLanguage.find("qut-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Kiche);}
        if (std::string::npos != acceptLanguage.find("quz")) {acceptLanguages.push_back(HttpTypes::Language::Quechua_Peru);}
        if (std::string::npos != acceptLanguage.find("ro")) {acceptLanguages.push_back(HttpTypes::Language::Romanian_Romania);}
        if (std::string::npos != acceptLanguage.find("ru")) {acceptLanguages.push_back(HttpTypes::Language::Russian);}
        if (std::string::npos != acceptLanguage.find("rw")) {acceptLanguages.push_back(HttpTypes::Language::Kinyarwanda);}
        if (std::string::npos != acceptLanguage.find("sd-Arab")) {acceptLanguages.push_back(HttpTypes::Language::Sindhi_Arabic);}
        if (std::string::npos != acceptLanguage.find("si")) {acceptLanguages.push_back(HttpTypes::Language::Sinhala);}
        if (std::string::npos != acceptLanguage.find("sk")) {acceptLanguages.push_back(HttpTypes::Language::Slovak);}
        if (std::string::npos != acceptLanguage.find("sl")) {acceptLanguages.push_back(HttpTypes::Language::Slovenian);}
        if (std::string::npos != acceptLanguage.find("sq")) {acceptLanguages.push_back(HttpTypes::Language::Albanian);}
        if (std::string::npos != acceptLanguage.find("sr-Cyrl-BA")) {acceptLanguages.push_back(HttpTypes::Language::Serbian_Cyrillic_BosniaHerzegovina);}
        if (std::string::npos != acceptLanguage.find("sr-Cyrl-RS")) {acceptLanguages.push_back(HttpTypes::Language::Serbian_Cyrillic_Serbia);}
        if (std::string::npos != acceptLanguage.find("sr-Latn-RS")) {acceptLanguages.push_back(HttpTypes::Language::Serbian_Latin_Serbia);}
        if (std::string::npos != acceptLanguage.find("sv")) {acceptLanguages.push_back(HttpTypes::Language::Swedish_Swedin);}
        if (std::string::npos != acceptLanguage.find("sw")) {acceptLanguages.push_back(HttpTypes::Language::Kiswahili);}
        if (std::string::npos != acceptLanguage.find("ta")) {acceptLanguages.push_back(HttpTypes::Language::Tamil);}
        if (std::string::npos != acceptLanguage.find("te")) {acceptLanguages.push_back(HttpTypes::Language::Telugu);}
        if (std::string::npos != acceptLanguage.find("tg-Cyrl")) {acceptLanguages.push_back(HttpTypes::Language::Tajik_Cyrillic);}
        if (std::string::npos != acceptLanguage.find("th")) {acceptLanguages.push_back(HttpTypes::Language::Thai);}
        if (std::string::npos != acceptLanguage.find("ti")) {acceptLanguages.push_back(HttpTypes::Language::Tigrinya);}
        if (std::string::npos != acceptLanguage.find("tk-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Turkmen_Latin);}
        if (std::string::npos != acceptLanguage.find("tn")) {acceptLanguages.push_back(HttpTypes::Language::Setswana);}
        if (std::string::npos != acceptLanguage.find("tr")) {acceptLanguages.push_back(HttpTypes::Language::Turkish);}
        if (std::string::npos != acceptLanguage.find("tt-Cyrl")) {acceptLanguages.push_back(HttpTypes::Language::Tatar_Cyrillic);}
        if (std::string::npos != acceptLanguage.find("ug-Arab")) {acceptLanguages.push_back(HttpTypes::Language::Uyghu);}
        if (std::string::npos != acceptLanguage.find("uk")) {acceptLanguages.push_back(HttpTypes::Language::Ukranian);}
        if (std::string::npos != acceptLanguage.find("ur")) {acceptLanguages.push_back(HttpTypes::Language::Urda);}
        if (std::string::npos != acceptLanguage.find("uz-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Uzbek_Latin);}
        if (std::string::npos != acceptLanguage.find("vi")) {acceptLanguages.push_back(HttpTypes::Language::Vietnamese);}
        if (std::string::npos != acceptLanguage.find("wo")) {acceptLanguages.push_back(HttpTypes::Language::Wolof);}
        if (std::string::npos != acceptLanguage.find("xh")) {acceptLanguages.push_back(HttpTypes::Language::isiXhosa);}
        if (std::string::npos != acceptLanguage.find("yo-Latn")) {acceptLanguages.push_back(HttpTypes::Language::Yoruba);}
        if (std::string::npos != acceptLanguage.find("zh-Hans")) {acceptLanguages.push_back(HttpTypes::Language::Chinese_Simplified);}
        if (std::string::npos != acceptLanguage.find("zh-Hant")) {acceptLanguages.push_back(HttpTypes::Language::Chinese_Traditional);}
        if (std::string::npos != acceptLanguage.find("zu")) {acceptLanguages.push_back(HttpTypes::Language::isiZulu);}

        return acceptLanguages;
    }

    ErrorType toSlNetAppResponse(const HttpTypes::Response &response, std::string &slNetAppResponse);
    std::string fromHttpServerType(const HttpTypes::Type type) {
        switch (type) {
            case HttpTypes::Type::TextHtml:
                return "text/html";
            case HttpTypes::Type::ApplicationJson:
                return "application/json";
            case HttpTypes::Type::ApplicationXml:
                return "application/xml";
            case HttpTypes::Type::ApplicationXhtmlXml:
                return "application/xhtml+xml";
            case HttpTypes::Type::ApplicationOctetStream:
                return "application/octet-stream";
            case HttpTypes::Type::ApplicationFormUrlencoded:
                return "application/x-www-form-urlencoded";
            case HttpTypes::Type::ImageGif:
                return "image/gif";
            case HttpTypes::Type::ImageJpeg:
                return "image/jpeg";
            case HttpTypes::Type::ImagePng:
                return "image/png";
            case HttpTypes::Type::ImageTiff:
                return "image/tiff";
            case HttpTypes::Type::ImageSvgXml:
                return "image/svg+xml";
            default:
                return "";
        }
    }
};

#endif // __HTTP_SERVER_MODULE_HPP__