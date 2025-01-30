//AbstractionLayer
#include "HttpServerModule.hpp"

ErrorType HttpServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    _ipServer->setNetwork(this->network());

    return _ipServer->listenTo(protocol, version, port);
}

ErrorType HttpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    return _ipServer->acceptConnection(socket, timeout);
}

ErrorType HttpServer::closeConnection(const Socket socket) {
    return _ipServer->closeConnection(socket);
}

ErrorType HttpServer::sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout, const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout, Socket &socket) {
    constexpr Bytes maxBufferSize = 512;
    std::string buffer(maxBufferSize, 0);
    ErrorType error = ErrorType::Failure;

    error = _ipServer->receiveBlocking(buffer, timeout, socket);

    if (buffer.size() > 0) {
        toHttpRequest(buffer, request);
    }

    return error;
}

ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::toHttpRequest(const std::string &buffer, HttpServerTypes::Request &request) {
    size_t uriStartIndex, uriEndIndex = 0;

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
        request.headers.contentType = HttpServerTypes::Type::TextHtml;
    }
    else if (ErrorType::Success == findHeaderValue(buffer, "Content-Type", "application/xml")) {
        request.headers.contentType = HttpServerTypes::Type::TextHtml;
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

    //TODO: Message body?
    return ErrorType::Success;
}

ErrorType HttpServer::findHeaderValue(const std::string &request, const std::string &headerName, const std::string &value) {
    const size_t theIndexThatTheHeaderStartsAt = request.find(headerName);
    const size_t theIndexThatTheHeaderEndsAt = request.find("\r\n");

    assert(theIndexThatTheHeaderStartsAt >= theIndexThatTheHeaderEndsAt);

    if (std::string::npos != theIndexThatTheHeaderStartsAt && std::string::npos != theIndexThatTheHeaderEndsAt) {
        std::string contentTypeHeader = request.substr(theIndexThatTheHeaderStartsAt, theIndexThatTheHeaderEndsAt);
        if (std::string::npos != contentTypeHeader.find(value)) {
            return ErrorType::Success;
        }
    }

    return ErrorType::Failure;
}