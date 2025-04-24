//Modules
#include "CryptographyModule.hpp"
//C++
#include <cstring>

Cryptography::Cryptography(const std::string &privateStaticKey, Bytes keySize) : CryptographyAbstraction(privateStaticKey, keySize) {
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

ErrorType Cryptography::generatePrivateKey(CryptographyTypes::Algorithm algorithm, const std::string &myPrivateKey, const std::string &theirPublicKey, std::string &newPrivateKey) {
    return ErrorType::NotImplemented;
}

ErrorType Cryptography::encrypt(const std::string &dataToEncrypt, std::string &encryptedData, const CryptographyTypes::AlgorithmParameters &parameters) {
    return ErrorType::NotImplemented;
}

ErrorType Cryptography::decrypt(const std::string &encrpytedData, std::string &decryptedData, const CryptographyTypes::AlgorithmParameters &parameters) {
    return ErrorType::NotImplemented;
}

ErrorType Cryptography::hash(CryptographyTypes::HashFunction hashFunction, const std::string &key, const std::string &data, std::string &hashedData, const CryptographyTypes::HashPart hashPart) {
    return ErrorType::NotImplemented;
}