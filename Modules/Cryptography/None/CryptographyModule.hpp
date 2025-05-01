#ifndef __CRYPTOGRAPHY_MODULE_HPP__
#define __CRYPTOGRAPHY_MODULE_HPP__

//AbstractionLayer
#include "CryptographyAbstraction.hpp"

class Cryptography final : public CryptographyAbstraction {
    
    public:
    Cryptography(const std::string &privateStaticKey, Bytes keySize);
    virtual ~Cryptography() = default;

    ErrorType generateKeys(CryptographyTypes::Algorithm algorithm) override;
    ErrorType generatePrivateKey(CryptographyTypes::Algorithm algorithm, const std::string &myPrivateKey, const std::string &theirPublicKey, std::string &newPrivateKey) override;

    ErrorType encrypt(const std::string &dataToEncrypt, std::string &encryptedData, const CryptographyTypes::AlgorithmParameters &parameters) override;
    ErrorType decrypt(const std::string &encrpytedData, std::string &decryptedData, const CryptographyTypes::AlgorithmParameters &parameters) override;
    ErrorType hash(CryptographyTypes::HashFunction hashFunction, const std::string &key, const std::string &data, std::string &hashedData, const CryptographyTypes::HashPart hashPart) override;
};

#endif // __CRYPTOGRAPHY_MODULE_HPP__
