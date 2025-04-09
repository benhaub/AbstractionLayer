//Modules
#include "CryptographyModule.hpp"
//C++
#include <cstring>
//Sodium
#include "sodium.h"

crypto_generichash_state genericHashState;

Cryptography::Cryptography(const std::string &privateStaticKey, Bytes keySize) : CryptographyAbstraction(privateStaticKey, keySize) {
    const bool noExistingKeyIsStored = privateStaticKey.empty();
    if (noExistingKeyIsStored) {
        _privateKey.resize(keySize);
        randombytes_buf(_privateKey.data(), keySize);
    }
    else {
        _privateKey.assign(privateStaticKey);
    }

    _publicKey.resize(keySize);
    crypto_scalarmult_base(reinterpret_cast<unsigned char *>(_publicKey.data()), reinterpret_cast<const unsigned char *>(_privateKey.data()));
}

ErrorType Cryptography::generateKeys(CryptographyAlgorithm alogorithm) {

    switch (alogorithm) {
        case CryptographyAlgorithm::Curve25519:
            return generateKeysX25519();
        case CryptographyAlgorithm::EllipticCurveDiffieHellman:
            return generateKeysEllipticCurveDiffieHellman();
        default:
            return ErrorType::InvalidParameter;
    }
}

ErrorType Cryptography::generatePrivateKey(CryptographyAlgorithm algorithm, const std::string &myPrivateKey, const std::string &theirPublicKey, std::string &newPrivateKey) {

    switch(algorithm) {
        case CryptographyAlgorithm::Curve25519:
            return ErrorType::NotImplemented;
        case CryptographyAlgorithm::EllipticCurveDiffieHellman:
            return generatePrivateKeyEllipticCurveDiffieHellman(myPrivateKey, theirPublicKey, newPrivateKey);
        default:
            return ErrorType::InvalidParameter;
    }
}

ErrorType Cryptography::encrypt(CryptographyAlgorithm algorithm, const std::string &dataToEncrypt, std::string &encryptedData, ...) {
    ErrorType error = ErrorType::Success;

    switch(algorithm) {
        case CryptographyAlgorithm::AeadChaCha20Poly1305Ietf:
            va_list args;
            va_start(args, encryptedData);
            error = encryptAeadChaCha20Poly1305Ietf(dataToEncrypt, encryptedData, va_arg(args, std::string), va_arg(args, uint64_t), va_arg(args, std::string));
            va_end(args);
            break;
        default:
            error = ErrorType::InvalidParameter;
    }

    return error;
}

ErrorType Cryptography::decrypt(CryptographyAlgorithm algorithm, const std::string &encrpytedData, std::string &decryptedData, ...) {
    ErrorType error = ErrorType::Success;

    switch(algorithm) {
        case CryptographyAlgorithm::AeadChaCha20Poly1305Ietf:
            va_list args;
            va_start(args, decryptedData);
            error = decrpytAeadChaCha20Poly1305Ietf(encrpytedData, decryptedData, va_arg(args, std::string), va_arg(args, uint64_t), va_arg(args, std::string));
            va_end(args);
            break;
        default:
            error = ErrorType::InvalidParameter;
    }

    return error;
}

ErrorType Cryptography::generateKeysX25519() {
    publicKey().clear();
    publicKey().resize(publicKey().capacity());
    privateKey().clear();
    privateKey().resize(privateKey().capacity());
    crypto_box_keypair(reinterpret_cast<unsigned char *>(publicKey().data()), reinterpret_cast<unsigned char *>(privateKey().data()));

    return ErrorType::Success;
}

ErrorType Cryptography::generateKeysEllipticCurveDiffieHellman() {
    return ErrorType::NotImplemented;
}


ErrorType Cryptography::generatePrivateKeyEllipticCurveDiffieHellman(const std::string &myPrivateKey, const std::string &theirPublicKey, std::string &newPrivateKey) {
    if (0 != crypto_scalarmult(reinterpret_cast<unsigned char *>(newPrivateKey.data()),
                                reinterpret_cast<const unsigned char *>(myPrivateKey.data()),
                                reinterpret_cast<const unsigned char *>(theirPublicKey.data()))) {
        return ErrorType::Failure;
    }

    return ErrorType::Success;
}

ErrorType Cryptography::encryptAeadChaCha20Poly1305Ietf(const std::string &dataToEncrypt, std::string &encryptedData, const std::string &ad, uint64_t n, const std::string &k) {
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};
    memcpy(&nonce[4], &n, sizeof(n));
    long long unsigned int encryptedDataLength = 0;

    crypto_aead_chacha20poly1305_ietf_encrypt(reinterpret_cast<unsigned char *>(encryptedData.data()),
                                                &encryptedDataLength,
                                                dataToEncrypt.empty() ? NULL : reinterpret_cast<const unsigned char *>(dataToEncrypt.c_str()),
                                                static_cast<unsigned long long>(dataToEncrypt.size()),
                                                reinterpret_cast<const unsigned char *>(ad.c_str()),
                                                static_cast<unsigned long long>(ad.size()),
                                                NULL,
                                                nonce,
                                                reinterpret_cast<const unsigned char *>(k.c_str()));
    encryptedData.resize(encryptedDataLength);
    return ErrorType::Success;
}
ErrorType Cryptography::decrpytAeadChaCha20Poly1305Ietf(const std::string &encryptedData, std::string &decryptedData, const std::string &ad, uint64_t n, const std::string &k) {
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};
    memcpy(&nonce[4], &n, sizeof(n));
    long long unsigned int decryptedDataLength = 0;

    if (0 == crypto_aead_chacha20poly1305_ietf_decrypt(decryptedData.empty() ? NULL : reinterpret_cast<unsigned char *>(decryptedData.data()),
                                                        &decryptedDataLength,
                                                        NULL,
                                                        reinterpret_cast<const unsigned char *>(encryptedData.c_str()), //Cipher
                                                        static_cast<unsigned long long>(encryptedData.size()),
                                                        reinterpret_cast<const unsigned char *>(ad.c_str()),
                                                        static_cast<unsigned long long>(ad.size()),
                                                        nonce,
                                                        reinterpret_cast<const unsigned char *>(k.c_str()))) {

        decryptedData.resize(decryptedDataLength);
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

//https://doc.libsodium.org/hashing/generic_hashing
ErrorType Cryptography::hash(HashFunction hashFunction, const std::string &key, const std::string &data, std::string hashedData, const HashPart hashPart) {
    switch (hashFunction) {
        case HashFunction::Blake2B:
            return hashBlake2b(data, key, hashedData, hashPart);
        case HashFunction::Argon2:
            return ErrorType::NotImplemented;
        case HashFunction::SipHash_2_4:
            return ErrorType::NotImplemented;
        default:
            return ErrorType::InvalidParameter;
    }
}

ErrorType Cryptography::hashBlake2b(const std::string &data, const std::string &key, std::string hashedData, const HashPart hashPart) {
    switch (hashPart) {
        case HashPart::Single:
            return toPlatformError(crypto_generichash(reinterpret_cast<unsigned char *>(hashedData.data()),
                                                      hashedData.size(),
                                                      data.empty() ? NULL : reinterpret_cast<const unsigned char *>(data.data()),
                                                      data.size(),
                                                      key.empty() ? NULL : reinterpret_cast<const unsigned char *>(key.data()),
                                                      key.size()));
        case HashPart::Init:
            return toPlatformError(crypto_generichash_init(&genericHashState,
                                                           key.empty() ? NULL : reinterpret_cast<const unsigned char *>(key.data()),
                                                           key.size(),
                                                           hashedData.size()));
        case HashPart::Update:
            return toPlatformError(crypto_generichash_update(&genericHashState,
                                                             reinterpret_cast<const unsigned char *>(data.data()),
                                                             data.size()));
        case HashPart::Final:
            return toPlatformError(crypto_generichash_final(&genericHashState,
                                                            reinterpret_cast<unsigned char *>(hashedData.data()),
                                                            hashedData.size()));
        default:
            return ErrorType::InvalidParameter;
    }
}