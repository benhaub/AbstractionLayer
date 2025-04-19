/***************************************************************************//**
* @author   Ben Haubrich
* @file     CryptographyAbstraction.hpp
* @details  Abstraction layer for Cryptography
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __CRYPTOGRAPHY_ABSTRACTION_HPP__
#define __CRYPTOGRAPHY_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
//C++
#include <string>
#include <variant>
/**
 * @namespace CryptographyTypes
 * @brief Types for the Cryptography Abstraction
 */
namespace CryptographyTypes {

    /**
     * @enum Algorithm
     * @brief Cryptography algorithms for generating keys
    */
    enum class Algorithm {
        EllipticCurveDiffieHellman = 0, ///< Elliptic Curve Diffie-Hellman
        Curve25519,                     ///< Curve25519
        AeadChaCha20Poly1305Ietf        ///< AEAD ChaCha20Poly1305 IETF

    };

    /**
     * @struct AlgorithmParameters
     * @brief Parameters for a cryptographic algorithm
     * @details Base class for custom parameters defined in the module.
    */
    struct AlgorithmParameters {
        public:
        virtual Algorithm algorithmType() const = 0;
    };
    /**
     * @struct EllipticCurveDiffieHellmanParameters
     * @brief Parameters for the Elliptic Curve Diffie-Hellman algorithm
     * @details Base class for custom parameters defined in the module.
    */
    struct EllipticCurveDiffieHellmanParameters final : public AlgorithmParameters {
        public:
        Algorithm algorithmType() const override { return Algorithm::EllipticCurveDiffieHellman; }
    };
    /**
     * @struct Curve25519Parameters
     * @brief Parameters for the Curve25519 algorithm
     * @details Base class for custom parameters defined in the module.
    */
    struct Curve25519Parameters final : public AlgorithmParameters {
        public:
        Algorithm algorithmType() const override { return Algorithm::Curve25519; }
    };
    /**
     * @struct AeadChaCha20Poly1305IetfParameters
     * @brief Parameters for the AEAD ChaCha20Poly1305 IETF algorithm
     * @details Base class for custom parameters defined in the module.
    */
    struct AeadChaCha20Poly1305IetfParameters final : public AlgorithmParameters {
        public:
        AeadChaCha20Poly1305IetfParameters(std::string_view associatedData, uint64_t nonce, std::string_view key) :
        _associatedData(associatedData), _nonce(nonce), _key(key)
        {

        }
        ~AeadChaCha20Poly1305IetfParameters() = default;

        Algorithm algorithmType() const override { return Algorithm::AeadChaCha20Poly1305Ietf; }

        const std::string_view &associatedData() const { return _associatedData; }
        const uint64_t &nonce() const { return _nonce; }
        const std::string_view &key() const { return _key; }

        private:
        std::string_view _associatedData;
        uint64_t _nonce;
        std::string_view _key;
    };

    /**
     * @enum HashFunction
     * @brief Hash functions
     */
    enum class HashFunction{
        Unknown = 0, ///< Unknown hash function
        Blake2B,     ///< Blake2B
        Argon2,      ///< Argon2
        SipHash_2_4  ///< SipHash
    };

    /**
     * @enum HashPart
     * @brief The part of the hash to perform
     */
    enum class HashPart {
        Unknown = 0, ///< Unknown hash part.
        Single = 1,  ///< Single part hash.
        Init = 2,    ///< First step in a multi part hash
        Update = 3,  ///< Update a multi-part hash
        Final = 4    ///< Finish a multi-part hash
    };
}

/**
 * @class CryptographyAbstraction
 * @brief Abstraction layer for Cryptography
*/
class CryptographyAbstraction {

    public:
    /**
     * @brief Constructor
     * @param[in] keySize The size of the keys to generate
     * @param[in] privateStaticKey Initialize the private and public static keys with an existing private key.
     * @post If privateStaticKey is not empty, the private and public static keys will be initialized with the private key.
     *       If privateStaticKey is empty, the private and public static keys will be generated.
    */
    CryptographyAbstraction(std::string_view privateStaticKey, Bytes keySize) : _privateKey(privateStaticKey) {
        _privateKey.reserve(keySize);
        _publicKey.reserve(keySize);
    }
    virtual ~CryptographyAbstraction() = default;
    /**
     * @brief Generate a static private and public key for encryption/decryption
     * @returns ErrorType::Success if the key was generated successfully
    */
    virtual ErrorType generateKeys(CryptographyTypes::Algorithm algorithm) = 0;
    /**
     * @brief Generate a new secret key
     * @pre All input keys have been sized appropriately (i.e std::string::resize())
     * @param[in] algorithm The algorithm to generate the key
     * @param[in] myPrivateKey The local private key to use.
     * @param[in] theirPublicKey The remote public key to use.
     * @param[out] newPrivateKey The generated secret key
     * @returns ErrorType::Success if the key was generated successfully
     * @post The size of the new private key can be obtained from privateKey.size()
    */
    virtual ErrorType generatePrivateKey(CryptographyTypes::Algorithm algorithm, std::string_view myPrivateKey, std::string_view theirPublicKey, std::string &newPrivateKey) = 0;
    /**
     * @brief Encrypt data
     * @param[in] dataToEncrypt The data to encrypt
     * @param[out] encryptedData The encrypted data
     * @param[in] parameters The parameters for the encryption
     * @return ErrorType::Success if the data was encrypted successfully
     * @code
     * CryptographyTypes::AeadChaCha20Poly1305IetfParameters parameters {
     *     .associatedData = "my associated data",
     *     .nonce = 1234567890,
     *     .key = "my key"
     * };
     * // The encyption algroithm used is determined by the parameters type.
     * ErrorType error = crypto->encrypt(dataToEncrypt, encryptedData, parameters);
     * @endcode
     */
    virtual ErrorType encrypt(std::string_view dataToEncrypt, std::string &encryptedData, const CryptographyTypes::AlgorithmParameters &parameters) = 0;
    /**
     * @brief Decrypt data
     * @param[in] encryptedData The data to decrypt
     * @param[out] decryptedData The decrypted data
     * @param[in] parameters The parameters for the decryption
     * @returns ErrorType::Success if the data was decrypted successfully
     * @returns ErrorType::Failure if the data was not decrypted successfully.
     * @sa CryptographyAbstraction::encrypt() for more information on the parameters.
     */
    virtual ErrorType decrypt(std::string_view encryptedData, std::string &decryptedData, const CryptographyTypes::AlgorithmParameters &parameters) = 0;
    /**
     * @brief Produce a hash
     * @param[in] algorithm The hash algorithm to use.
     * @param[in] data The data to hash
     * @param[out] hashedData The hashed data
     * @param[in] hashPart The part of the has to perform. Defaults to single hash.
     * @returns ErrorType::Success if the data was hashed.
     * @returns ErrorType::Failure if the data could not be hashed.
     */
    virtual ErrorType hash(CryptographyTypes::HashFunction hashFunction, std::string_view key, std::string_view data, std::string &hashedData, const CryptographyTypes::HashPart hashPart = CryptographyTypes::HashPart::Single) = 0;

    /// @brief Get the private key as a constant reference
    const std::string &privateKeyConst() const { return _privateKey; }
    /// @brief Get the public key as a constant reference
    const std::string &publicKeyConst() const { return _publicKey; }
    /// @brief Get the private key as a mutable reference
    std::string &privateKey() { return _privateKey; }
    /// @brief Get the public key as a mutable reference
    std::string &publicKey() { return _publicKey; }

    protected:
    /// @brief The private key
    std::string _privateKey;
    /// @brief The public key
    std::string _publicKey;
};

#endif // __CRYPTOGRAPHY_ABSTRACTION_HPP__
