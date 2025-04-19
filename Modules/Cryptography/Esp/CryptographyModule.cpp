//Modules
#include "CryptographyModule.hpp"
//C++
#include <cstring>
#include <cstdarg>
//Sodium
#include "sodium.h"

crypto_generichash_state genericHashState;

Cryptography::Cryptography(std::string_view privateStaticKey, Bytes keySize) : CryptographyAbstraction(privateStaticKey, keySize) {
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

ErrorType Cryptography::generateKeys(CryptographyTypes::Algorithm algorithm) {

    switch (algorithm) {
        case CryptographyTypes::Algorithm::Curve25519:
            return generateKeysX25519();
        case CryptographyTypes::Algorithm::EllipticCurveDiffieHellman:
            return generateKeysEllipticCurveDiffieHellman();
        default:
            return ErrorType::InvalidParameter;
    }
}

ErrorType Cryptography::generatePrivateKey(CryptographyTypes::Algorithm algorithm, std::string_view myPrivateKey, std::string_view theirPublicKey, std::string &newPrivateKey) {

    switch(algorithm) {
        case CryptographyTypes::Algorithm::Curve25519:
            return ErrorType::NotImplemented;
        case CryptographyTypes::Algorithm::EllipticCurveDiffieHellman:
            return generatePrivateKeyEllipticCurveDiffieHellman(myPrivateKey, theirPublicKey, newPrivateKey);
        default:
            return ErrorType::InvalidParameter;
    }
}

ErrorType Cryptography::encrypt(std::string_view dataToEncrypt, std::string &encryptedData, const CryptographyTypes::AlgorithmParameters &parameters) {
    ErrorType error = ErrorType::Failure;

    if (parameters.algorithmType() == CryptographyTypes::Algorithm::AeadChaCha20Poly1305Ietf) {
        const auto &aeadParameters = static_cast<const CryptographyTypes::AeadChaCha20Poly1305IetfParameters &>(parameters);
        error = encryptAeadChaCha20Poly1305Ietf(dataToEncrypt, encryptedData, aeadParameters.associatedData(), aeadParameters.nonce(), aeadParameters.key());
    }
    else {
        error = ErrorType::InvalidParameter;
    }

    return error;
}

ErrorType Cryptography::decrypt(std::string_view encrpytedData, std::string &decryptedData, const CryptographyTypes::AlgorithmParameters &parameters) {
    ErrorType error = ErrorType::Failure;

    if (parameters.algorithmType() == CryptographyTypes::Algorithm::AeadChaCha20Poly1305Ietf) {
        const auto &aeadParameters = static_cast<const CryptographyTypes::AeadChaCha20Poly1305IetfParameters &>(parameters);
        error = decrpytAeadChaCha20Poly1305Ietf(encrpytedData, decryptedData, aeadParameters.associatedData(), aeadParameters.nonce(), aeadParameters.key());
    }
    else {
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


ErrorType Cryptography::generatePrivateKeyEllipticCurveDiffieHellman(std::string_view myPrivateKey, std::string_view theirPublicKey, std::string &newPrivateKey) {
    if (0 != crypto_scalarmult(reinterpret_cast<unsigned char *>(newPrivateKey.data()),
                                reinterpret_cast<const unsigned char *>(myPrivateKey.data()),
                                reinterpret_cast<const unsigned char *>(theirPublicKey.data()))) {
        return ErrorType::Failure;
    }

    return ErrorType::Success;
}

ErrorType Cryptography::encryptAeadChaCha20Poly1305Ietf(std::string_view dataToEncrypt, std::string &encryptedData, std::string_view ad, uint64_t n, std::string_view k) {
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};
    memcpy(&nonce[4], &n, sizeof(n));
    long long unsigned int encryptedDataLength = 0;

    crypto_aead_chacha20poly1305_ietf_encrypt(reinterpret_cast<unsigned char *>(encryptedData.data()),
                                                &encryptedDataLength,
                                                dataToEncrypt.empty() ? NULL : reinterpret_cast<const unsigned char *>(dataToEncrypt.data()),
                                                static_cast<unsigned long long>(dataToEncrypt.size()),
                                                reinterpret_cast<const unsigned char *>(ad.data()),
                                                static_cast<unsigned long long>(ad.size()),
                                                NULL,
                                                nonce,
                                                reinterpret_cast<const unsigned char *>(k.data()));
    encryptedData.resize(encryptedDataLength);
    return ErrorType::Success;
}
ErrorType Cryptography::decrpytAeadChaCha20Poly1305Ietf(std::string_view encryptedData, std::string &decryptedData, std::string_view ad, uint64_t n, std::string_view k) {
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};
    memcpy(&nonce[4], &n, sizeof(n));
    long long unsigned int decryptedDataLength = 0;

    if (0 == crypto_aead_chacha20poly1305_ietf_decrypt(decryptedData.empty() ? NULL : reinterpret_cast<unsigned char *>(decryptedData.data()),
                                                        &decryptedDataLength,
                                                        NULL,
                                                        reinterpret_cast<const unsigned char *>(encryptedData.data()), //Cipher
                                                        static_cast<unsigned long long>(encryptedData.size()),
                                                        reinterpret_cast<const unsigned char *>(ad.data()),
                                                        static_cast<unsigned long long>(ad.size()),
                                                        nonce,
                                                        reinterpret_cast<const unsigned char *>(k.data()))) {

        decryptedData.resize(decryptedDataLength);
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

//https://doc.libsodium.org/hashing/generic_hashing
ErrorType Cryptography::hash(CryptographyTypes::HashFunction hashFunction, std::string_view key, std::string_view data, std::string &hashedData, const CryptographyTypes::HashPart hashPart) {
    switch (hashFunction) {
        case CryptographyTypes::HashFunction::Blake2B:
            return hashBlake2b(data, key, hashedData, hashPart);
        case CryptographyTypes::HashFunction::Argon2:
            return ErrorType::NotImplemented;
        case CryptographyTypes::HashFunction::SipHash_2_4:
            return ErrorType::NotImplemented;
        default:
            return ErrorType::InvalidParameter;
    }
}

ErrorType Cryptography::hashBlake2b(std::string_view data, std::string_view key, std::string &hashedData, const CryptographyTypes::HashPart hashPart) {
    switch (hashPart) {
        case CryptographyTypes::HashPart::Single:
            return fromPlatformError(crypto_generichash(reinterpret_cast<unsigned char *>(hashedData.data()),
                                                      hashedData.size(),
                                                      data.empty() ? NULL : reinterpret_cast<const unsigned char *>(data.data()),
                                                      data.size(),
                                                      key.empty() ? NULL : reinterpret_cast<const unsigned char *>(key.data()),
                                                      key.size()));
        case CryptographyTypes::HashPart::Init:
            return fromPlatformError(crypto_generichash_init(&genericHashState,
                                                           key.empty() ? NULL : reinterpret_cast<const unsigned char *>(key.data()),
                                                           key.size(),
                                                           hashedData.size()));
        case CryptographyTypes::HashPart::Update:
            return fromPlatformError(crypto_generichash_update(&genericHashState,
                                                             reinterpret_cast<const unsigned char *>(data.data()),
                                                             data.size()));
        case CryptographyTypes::HashPart::Final:
            return fromPlatformError(crypto_generichash_final(&genericHashState,
                                                            reinterpret_cast<unsigned char *>(hashedData.data()),
                                                            hashedData.size()));
        default:
            return ErrorType::InvalidParameter;
    }
}