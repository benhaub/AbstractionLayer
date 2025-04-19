//Modules
#include "CryptographyModule.hpp"
//C++
#include <cstring>

Cryptography::Cryptography(std::string_view privateStaticKey, Bytes keySize) : CryptographyAbstraction(privateStaticKey, keySize) {
}

ErrorType Cryptography::generateKeys(CryptographyTypes::Algorithm algorithm) {

    switch (algorithm) {
        case CryptographyTypes::Algorithm::Curve25519:
        case CryptographyTypes::Algorithm::EllipticCurveDiffieHellman:
        case CryptographyTypes::Algorithm::AeadChaCha20Poly1305Ietf:
        default:
            return ErrorType::NotImplemented;
    }
}

ErrorType Cryptography::generatePrivateKey(CryptographyTypes::Algorithm algorithm, std::string_view myPrivateKey, std::string_view theirPublicKey, std::string &newPrivateKey) {
    return ErrorType::NotImplemented;
}

ErrorType Cryptography::encrypt(std::string_view dataToEncrypt, std::string &encryptedData, const CryptographyTypes::AlgorithmParameters &parameters) {
    return ErrorType::NotImplemented;
}

ErrorType Cryptography::decrypt(std::string_view encrpytedData, std::string &decryptedData, const CryptographyTypes::AlgorithmParameters &parameters) {
    return ErrorType::NotImplemented;
}

ErrorType Cryptography::hash(CryptographyTypes::HashFunction hashFunction, std::string_view key, std::string_view data, std::string &hashedData, const CryptographyTypes::HashPart hashPart) {
    return ErrorType::NotImplemented;
}