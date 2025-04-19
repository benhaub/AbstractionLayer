#ifndef __CRYPTOGRAPHY_MODULE_HPP__
#define __CRYPTOGRAPHY_MODULE_HPP__

//AbstractionLayer
#include "CryptographyAbstraction.hpp"

class Cryptography : public CryptographyAbstraction {
    
    public:
    Cryptography(std::string_view privateStaticKey, Bytes keySize);
    virtual ~Cryptography() = default;

    ErrorType generateKeys(CryptographyTypes::Algorithm algorithm) override;
    ErrorType generatePrivateKey(CryptographyTypes::Algorithm algorithm, std::string_view myPrivateKey, std::string_view theirPublicKey, std::string &newPrivateKey) override;

    ErrorType encrypt(std::string_view dataToEncrypt, std::string &encryptedData, const CryptographyTypes::AlgorithmParameters &parameters) override;
    ErrorType decrypt(std::string_view encrpytedData, std::string &decryptedData, const CryptographyTypes::AlgorithmParameters &parameters) override;
    ErrorType hash(CryptographyTypes::HashFunction hashFunction, std::string_view key, std::string_view data, std::string &hashedData, const CryptographyTypes::HashPart hashPart) override;
};

#endif // __CRYPTOGRAPHY_MODULE_HPP__
