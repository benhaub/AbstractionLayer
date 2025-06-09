#ifndef __CRYPTOGRAPHY_MODULE_HPP__
#define __CRYPTOGRAPHY_MODULE_HPP__

#include "CryptographyAbstraction.hpp"

class Cryptography final : public CryptographyAbstraction {

    public:
    Cryptography(const std::string &privateStaticKey, Bytes keySize);

    ErrorType generateKeys(CryptographyTypes::Algorithm algorithm) override;
    ErrorType generatePrivateKey(CryptographyTypes::Algorithm algorithm, const std::string &myPrivateKey, const std::string &theirPublicKey, std::string &newPrivateKey) override;

    ErrorType encrypt(const std::string &dataToEncrypt, std::string &encryptedData, const CryptographyTypes::AlgorithmParameters &parameters) override;
    ErrorType decrypt(const std::string &encrpytedData, std::string &decryptedData, const CryptographyTypes::AlgorithmParameters &parameters) override;
    ErrorType hash(CryptographyTypes::HashFunction hashFunction, const std::string &key, const std::string &data, std::string &hashedData, const CryptographyTypes::HashPart hashPart) override;

    private:
    ErrorType generateKeysX25519();
    ErrorType generateKeysEllipticCurveDiffieHellman();
    ErrorType generatePrivateKeyEllipticCurveDiffieHellman(const std::string &myPrivateKey, const std::string &theirPublicKey, std::string &newPrivateKey);
    ErrorType encryptAeadChaCha20Poly1305Ietf(const std::string &dataToEncrypt, std::string &encryptedData, const std::string &ad, uint64_t n, const std::string &k);
    ErrorType decrpytAeadChaCha20Poly1305Ietf(const std::string &encryptedData, std::string &decryptedData, const std::string &ad, uint64_t n, const std::string &k);

    ErrorType hashBlake2b(const std::string &data, const std::string &key, std::string &hashedData, const CryptographyTypes::HashPart hashPart);
};

#endif // __CRYPTOGRAPHY_MODULE_HPP__