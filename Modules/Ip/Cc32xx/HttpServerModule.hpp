#ifndef __HTTP_SERVER_MODULE_HPP__
#define __HTTP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "HttpServerAbstraction.hpp"
#include "OperatingSystemModule.hpp"
//TI Drivers
#include "ti/drivers/net/wifi/simplelink.h"

//Global because the SimpleLink C handlers are outside the scope of this class and need to be able to access the queue.
static constexpr char SimpleLinkEventQueue[] = "SimpleLinkEventQueue";

class HttpServer : public HttpServerAbstraction {
    public:
    HttpServer() : HttpServerAbstraction() {
        OperatingSystem::Instance().createQueue(SimpleLinkEventQueue, sizeof(SlNetAppRequest_t), 10);
    }
    virtual ~HttpServer() = default;

    ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;
    //These functions could perhaps be used to implement websockets
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override { return ErrorType::NotSupported; }
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override { return ErrorType::NotSupported; }

    ErrorType sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout) override;
    ErrorType receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;

    private:
    ErrorType toHttpRequest(const SlNetAppRequest_t &netAppRequest, HttpServerTypes::Request &request);
    HttpServerTypes::Version fromStringVersion(const std::string version) {
        if (std::string::npos != version.find("1.0")) {
            return HttpServerTypes::Version::Http1_0;
        }
        else if (std::string::npos != version.find("1.1")) {
            return HttpServerTypes::Version::Http1_1;
        }
        else if (std::string::npos != version.find("2.0")) {
            return HttpServerTypes::Version::Http2_0;
        }
        else if (std::string::npos != version.find("3.0")) {
            return HttpServerTypes::Version::Http3_0;
        }
        else {
            return HttpServerTypes::Version::Unknown;
        }
    }
    HttpServerTypes::Type fromStringContentType(const std::string contentType) {
        if (std::string::npos != contentType.find("text/html")) {
            return HttpServerTypes::Type::TextHtml;
        }
        else if (std::string::npos != contentType.find("application/json")) {
            return HttpServerTypes::Type::ApplicationJson;
        }
        else if (std::string::npos != contentType.find("application/xml")) {
            return HttpServerTypes::Type::ApplicationXml;
        }
        else {
            return HttpServerTypes::Type::Unknown;
        }
    }
    HttpServerTypes::Connection fromStringConnection(const std::string connection) {
        if (std::string::npos != connection.find("keep-alive")) {
            return HttpServerTypes::Connection::KeepAlive;
        }
        else if (std::string::npos != connection.find("close")) {
            return HttpServerTypes::Connection::Close;
        }
        else {
            return HttpServerTypes::Connection::Unknown;
        }
    }
    std::vector<HttpServerTypes::Type> fromStringAccept(const std::string accept) {
        std::vector<HttpServerTypes::Type> accepts;

        if (std::string::npos != accept.find("text/html")) {
            accepts.push_back(HttpServerTypes::Type::TextHtml);
        }
        if (std::string::npos != accept.find("application/json")) {
            accepts.push_back(HttpServerTypes::Type::ApplicationJson);
        }
        if (std::string::npos != accept.find("application/xml")) {
            accepts.push_back(HttpServerTypes::Type::ApplicationXml);
        }

        return accepts;
    }
    std::vector<HttpServerTypes::Encoding> fromStringEncoding(const std::string encoding) {
        std::vector<HttpServerTypes::Encoding> encodings;

        if (std::string::npos != encoding.find("gzip")) {
            encodings.push_back(HttpServerTypes::Encoding::Gzip);
        }
        if (std::string::npos != encoding.find("deflate")) {
            encodings.push_back(HttpServerTypes::Encoding::Deflate);
        }
        if (std::string::npos != encoding.find("br")) {
            encodings.push_back(HttpServerTypes::Encoding::Br);
        }
        if (std::string::npos != encoding.find("identity")) {
            encodings.push_back(HttpServerTypes::Encoding::Identity);
        }

        return encodings;
    }
    std::vector<HttpServerTypes::Language> fromStringAcceptLanguage(const std::string acceptLanguage) {
        std::vector<HttpServerTypes::Language> acceptLanguages;

        if (std::string::npos != acceptLanguage.find("af")) {acceptLanguages.push_back(HttpServerTypes::Language::Afrikaans);}
        if (std::string::npos != acceptLanguage.find("am")) {acceptLanguages.push_back(HttpServerTypes::Language::Amharic);}
        if (std::string::npos != acceptLanguage.find("ar-sa")) {acceptLanguages.push_back(HttpServerTypes::Language::Arabic_SaudiArabia);}
        if (std::string::npos != acceptLanguage.find("as")) {acceptLanguages.push_back(HttpServerTypes::Language::Assamese);}
        if (std::string::npos != acceptLanguage.find("az-latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Azerbaijani_Latin);}
        if (std::string::npos != acceptLanguage.find("be")) {acceptLanguages.push_back(HttpServerTypes::Language::Belarusian);}
        if (std::string::npos != acceptLanguage.find("bg")) {acceptLanguages.push_back(HttpServerTypes::Language::Bulgarian);}
        if (std::string::npos != acceptLanguage.find("bn-BD")) {acceptLanguages.push_back(HttpServerTypes::Language::Bangla_Bangladesh);}
        if (std::string::npos != acceptLanguage.find("bn-IN")) {acceptLanguages.push_back(HttpServerTypes::Language::Bangla_India);}
        if (std::string::npos != acceptLanguage.find("bs")) {acceptLanguages.push_back(HttpServerTypes::Language::Bosnian_Latin);}
        if (std::string::npos != acceptLanguage.find("ca")) {acceptLanguages.push_back(HttpServerTypes::Language::CatalanSpanish);}
        if (std::string::npos != acceptLanguage.find("ca-ES-valencia")) {acceptLanguages.push_back(HttpServerTypes::Language::Valencian);}
        if (std::string::npos != acceptLanguage.find("cs")) {acceptLanguages.push_back(HttpServerTypes::Language::Czech);}
        if (std::string::npos != acceptLanguage.find("cy")) {acceptLanguages.push_back(HttpServerTypes::Language::Welsh);}
        if (std::string::npos != acceptLanguage.find("da")) {acceptLanguages.push_back(HttpServerTypes::Language::Danish);}
        if (std::string::npos != acceptLanguage.find("de") || std::string::npos != acceptLanguage.find("de-de")) {acceptLanguages.push_back(HttpServerTypes::Language::German_Germany);}
        if (std::string::npos != acceptLanguage.find("el")) {acceptLanguages.push_back(HttpServerTypes::Language::Greek);}
        if (std::string::npos != acceptLanguage.find("en-GB")) {acceptLanguages.push_back(HttpServerTypes::Language::English_UnitedKingdom);}
        if (std::string::npos != acceptLanguage.find("en-US")) {acceptLanguages.push_back(HttpServerTypes::Language::English_UnitedStates);}
        if (std::string::npos != acceptLanguage.find("es") || std::string::npos != acceptLanguage.find("es-ES")) {acceptLanguages.push_back(HttpServerTypes::Language::Spanish_Spain);}
        if (std::string::npos != acceptLanguage.find("es-US")) {acceptLanguages.push_back(HttpServerTypes::Language::Spanish_UnitedStates);}
        if (std::string::npos != acceptLanguage.find("es-MX")) {acceptLanguages.push_back(HttpServerTypes::Language::Spanish_Mexico);}
        if (std::string::npos != acceptLanguage.find("et")) {acceptLanguages.push_back(HttpServerTypes::Language::Estonian);}
        if (std::string::npos != acceptLanguage.find("eu")) {acceptLanguages.push_back(HttpServerTypes::Language::Basque);}
        if (std::string::npos != acceptLanguage.find("fa")) {acceptLanguages.push_back(HttpServerTypes::Language::Persian);}
        if (std::string::npos != acceptLanguage.find("fi")) {acceptLanguages.push_back(HttpServerTypes::Language::Finnish);}
        if (std::string::npos != acceptLanguage.find("fil-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Filipino);}
        if (std::string::npos != acceptLanguage.find("fr")) {acceptLanguages.push_back(HttpServerTypes::Language::French_France);}
        if (std::string::npos != acceptLanguage.find("fr-CA")) {acceptLanguages.push_back(HttpServerTypes::Language::French_Canada);}
        if (std::string::npos != acceptLanguage.find("ga")) {acceptLanguages.push_back(HttpServerTypes::Language::Irish);}
        if (std::string::npos != acceptLanguage.find("gd-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::ScottishGaelic);}
        if (std::string::npos != acceptLanguage.find("gl")) {acceptLanguages.push_back(HttpServerTypes::Language::Galician);}
        if (std::string::npos != acceptLanguage.find("gu")) {acceptLanguages.push_back(HttpServerTypes::Language::Gujarati);}
        if (std::string::npos != acceptLanguage.find("ha-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Hausa_Latin);}
        if (std::string::npos != acceptLanguage.find("he")) {acceptLanguages.push_back(HttpServerTypes::Language::Hebrew);}
        if (std::string::npos != acceptLanguage.find("hi")) {acceptLanguages.push_back(HttpServerTypes::Language::Hindi);}
        if (std::string::npos != acceptLanguage.find("hr")) {acceptLanguages.push_back(HttpServerTypes::Language::Croation);}
        if (std::string::npos != acceptLanguage.find("hu")) {acceptLanguages.push_back(HttpServerTypes::Language::Hungarian);}
        if (std::string::npos != acceptLanguage.find("hy")) {acceptLanguages.push_back(HttpServerTypes::Language::Armenian);}
        if (std::string::npos != acceptLanguage.find("id")) {acceptLanguages.push_back(HttpServerTypes::Language::Indonesian);}
        if (std::string::npos != acceptLanguage.find("ig-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Igbo);}
        if (std::string::npos != acceptLanguage.find("is")) {acceptLanguages.push_back(HttpServerTypes::Language::Icelandic);}
        if (std::string::npos != acceptLanguage.find("it") || std::string::npos != acceptLanguage.find("it-it")) {acceptLanguages.push_back(HttpServerTypes::Language::Italian_Italy);}
        if (std::string::npos != acceptLanguage.find("ja")) {acceptLanguages.push_back(HttpServerTypes::Language::Japanese);}
        if (std::string::npos != acceptLanguage.find("ka")) {acceptLanguages.push_back(HttpServerTypes::Language::Georgian);}
        if (std::string::npos != acceptLanguage.find("kk")) {acceptLanguages.push_back(HttpServerTypes::Language::Kazakh);}
        if (std::string::npos != acceptLanguage.find("km")) {acceptLanguages.push_back(HttpServerTypes::Language::Khmer);}
        if (std::string::npos != acceptLanguage.find("kn")) {acceptLanguages.push_back(HttpServerTypes::Language::Kannada);}
        if (std::string::npos != acceptLanguage.find("ko")) {acceptLanguages.push_back(HttpServerTypes::Language::Korean);}
        if (std::string::npos != acceptLanguage.find("kok")) {acceptLanguages.push_back(HttpServerTypes::Language::Konkani);}
        if (std::string::npos != acceptLanguage.find("ku-Arab")) {acceptLanguages.push_back(HttpServerTypes::Language::CentralKurdish);}
        if (std::string::npos != acceptLanguage.find("ky-Cyrl")) {acceptLanguages.push_back(HttpServerTypes::Language::Kyrgyz);}
        if (std::string::npos != acceptLanguage.find("lb")) {acceptLanguages.push_back(HttpServerTypes::Language::LuxemBourgish);}
        if (std::string::npos != acceptLanguage.find("lt")) {acceptLanguages.push_back(HttpServerTypes::Language::Lithuanian);}
        if (std::string::npos != acceptLanguage.find("lv")) {acceptLanguages.push_back(HttpServerTypes::Language::Latvian);}
        if (std::string::npos != acceptLanguage.find("mi-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Maori);}
        if (std::string::npos != acceptLanguage.find("mk")) {acceptLanguages.push_back(HttpServerTypes::Language::Macedonian);}
        if (std::string::npos != acceptLanguage.find("ml")) {acceptLanguages.push_back(HttpServerTypes::Language::Malayalam);}
        if (std::string::npos != acceptLanguage.find("mn-Cyrl")) {acceptLanguages.push_back(HttpServerTypes::Language::Mongolian_Cyrillic);}
        if (std::string::npos != acceptLanguage.find("mr")) {acceptLanguages.push_back(HttpServerTypes::Language::Maylay_Maylaysia);}
        if (std::string::npos != acceptLanguage.find("mt")) {acceptLanguages.push_back(HttpServerTypes::Language::Maltese);}
        if (std::string::npos != acceptLanguage.find("nb")) {acceptLanguages.push_back(HttpServerTypes::Language::Norwegian_Bokmal);}
        if (std::string::npos != acceptLanguage.find("ne")) {acceptLanguages.push_back(HttpServerTypes::Language::Nepali_Nepal);}
        if (std::string::npos != acceptLanguage.find("nl") || std::string::npos != acceptLanguage.find("nl-BE")) {acceptLanguages.push_back(HttpServerTypes::Language::Dutch_Netherlands);}
        if (std::string::npos != acceptLanguage.find("nn")) {acceptLanguages.push_back(HttpServerTypes::Language::Norwegian_Nynorsk);}
        if (std::string::npos != acceptLanguage.find("nso")) {acceptLanguages.push_back(HttpServerTypes::Language::SesothoSaLeboa);}
        if (std::string::npos != acceptLanguage.find("or")) {acceptLanguages.push_back(HttpServerTypes::Language::Odia);}
        if (std::string::npos != acceptLanguage.find("pa")) {acceptLanguages.push_back(HttpServerTypes::Language::Punjabi_Gurmukhi);}
        if (std::string::npos != acceptLanguage.find("pa-Arab")) {acceptLanguages.push_back(HttpServerTypes::Language::Punjabi_Arabic);}
        if (std::string::npos != acceptLanguage.find("pl")) {acceptLanguages.push_back(HttpServerTypes::Language::Polish);}
        if (std::string::npos != acceptLanguage.find("prs-Arab")) {acceptLanguages.push_back(HttpServerTypes::Language::Dari);}
        if (std::string::npos != acceptLanguage.find("pt-BR")) {acceptLanguages.push_back(HttpServerTypes::Language::Portuguese_Brazil);}
        if (std::string::npos != acceptLanguage.find("pt-PT")) {acceptLanguages.push_back(HttpServerTypes::Language::Portuguese_Portugal);}
        if (std::string::npos != acceptLanguage.find("qut-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Kiche);}
        if (std::string::npos != acceptLanguage.find("quz")) {acceptLanguages.push_back(HttpServerTypes::Language::Quechua_Peru);}
        if (std::string::npos != acceptLanguage.find("ro")) {acceptLanguages.push_back(HttpServerTypes::Language::Romanian_Romania);}
        if (std::string::npos != acceptLanguage.find("ru")) {acceptLanguages.push_back(HttpServerTypes::Language::Russian);}
        if (std::string::npos != acceptLanguage.find("rw")) {acceptLanguages.push_back(HttpServerTypes::Language::Kinyarwanda);}
        if (std::string::npos != acceptLanguage.find("sd-Arab")) {acceptLanguages.push_back(HttpServerTypes::Language::Sindhi_Arabic);}
        if (std::string::npos != acceptLanguage.find("si")) {acceptLanguages.push_back(HttpServerTypes::Language::Sinhala);}
        if (std::string::npos != acceptLanguage.find("sk")) {acceptLanguages.push_back(HttpServerTypes::Language::Slovak);}
        if (std::string::npos != acceptLanguage.find("sl")) {acceptLanguages.push_back(HttpServerTypes::Language::Slovenian);}
        if (std::string::npos != acceptLanguage.find("sq")) {acceptLanguages.push_back(HttpServerTypes::Language::Albanian);}
        if (std::string::npos != acceptLanguage.find("sr-Cyrl-BA")) {acceptLanguages.push_back(HttpServerTypes::Language::Serbian_Cyrillic_BosniaHerzegovina);}
        if (std::string::npos != acceptLanguage.find("sr-Cyrl-RS")) {acceptLanguages.push_back(HttpServerTypes::Language::Serbian_Cyrillic_Serbia);}
        if (std::string::npos != acceptLanguage.find("sr-Latn-RS")) {acceptLanguages.push_back(HttpServerTypes::Language::Serbian_Latin_Serbia);}
        if (std::string::npos != acceptLanguage.find("sv")) {acceptLanguages.push_back(HttpServerTypes::Language::Swedish_Swedin);}
        if (std::string::npos != acceptLanguage.find("sw")) {acceptLanguages.push_back(HttpServerTypes::Language::Kiswahili);}
        if (std::string::npos != acceptLanguage.find("ta")) {acceptLanguages.push_back(HttpServerTypes::Language::Tamil);}
        if (std::string::npos != acceptLanguage.find("te")) {acceptLanguages.push_back(HttpServerTypes::Language::Telugu);}
        if (std::string::npos != acceptLanguage.find("tg-Cyrl")) {acceptLanguages.push_back(HttpServerTypes::Language::Tajik_Cyrillic);}
        if (std::string::npos != acceptLanguage.find("th")) {acceptLanguages.push_back(HttpServerTypes::Language::Thai);}
        if (std::string::npos != acceptLanguage.find("ti")) {acceptLanguages.push_back(HttpServerTypes::Language::Tigrinya);}
        if (std::string::npos != acceptLanguage.find("tk-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Turkmen_Latin);}
        if (std::string::npos != acceptLanguage.find("tn")) {acceptLanguages.push_back(HttpServerTypes::Language::Setswana);}
        if (std::string::npos != acceptLanguage.find("tr")) {acceptLanguages.push_back(HttpServerTypes::Language::Turkish);}
        if (std::string::npos != acceptLanguage.find("tt-Cyrl")) {acceptLanguages.push_back(HttpServerTypes::Language::Tatar_Cyrillic);}
        if (std::string::npos != acceptLanguage.find("ug-Arab")) {acceptLanguages.push_back(HttpServerTypes::Language::Uyghu);}
        if (std::string::npos != acceptLanguage.find("uk")) {acceptLanguages.push_back(HttpServerTypes::Language::Ukranian);}
        if (std::string::npos != acceptLanguage.find("ur")) {acceptLanguages.push_back(HttpServerTypes::Language::Urda);}
        if (std::string::npos != acceptLanguage.find("uz-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Uzbek_Latin);}
        if (std::string::npos != acceptLanguage.find("vi")) {acceptLanguages.push_back(HttpServerTypes::Language::Vietnamese);}
        if (std::string::npos != acceptLanguage.find("wo")) {acceptLanguages.push_back(HttpServerTypes::Language::Wolof);}
        if (std::string::npos != acceptLanguage.find("xh")) {acceptLanguages.push_back(HttpServerTypes::Language::isiXhosa);}
        if (std::string::npos != acceptLanguage.find("yo-Latn")) {acceptLanguages.push_back(HttpServerTypes::Language::Yoruba);}
        if (std::string::npos != acceptLanguage.find("zh-Hans")) {acceptLanguages.push_back(HttpServerTypes::Language::Chinese_Simplified);}
        if (std::string::npos != acceptLanguage.find("zh-Hant")) {acceptLanguages.push_back(HttpServerTypes::Language::Chinese_Traditional);}
        if (std::string::npos != acceptLanguage.find("zu")) {acceptLanguages.push_back(HttpServerTypes::Language::isiZulu);}

        return acceptLanguages;
    }
};

#endif // __HTTP_SERVER_MODULE_HPP__