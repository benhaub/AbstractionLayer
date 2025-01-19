/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   SerializationAbstraction.hpp
* @details \b Synopsis: \n Provides an abstraction serialization.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __SERIALIZATION_ABSTRACTION_HPP__
#define __SERIALIZATION_ABSTRACTION_HPP__

//C++
#include <string>
//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"

/**
 * @enum SerializationType
 * @brief The type of serialization.
*/
enum class SerializationType {
    Unknown = 0x00,                              ///< Unknown serialization type.
    FoundationToCleonDataRequest         = 0xFC, ///< Foundation to cleon data request.
    CleonToFoundationDataReply           = 0xCF, ///< Cleon to foundation data reply.
    FoundationToCloudSyncRequest         = 0xF0, ///< Synchronization request to cloud
    CloudToFoundationSyncRequest         = 0x0F, ///< Synchronization data from cloud
    FoundationToCleonSyncRequest         = 0x0F, ///< Synchronization request from cloud.
    FoundationToCloudSyncAcknowledge     = 0xF1, ///< Synchronization acknowledge to cloud.
    TelemetryUpdate                      = 0xBA  ///< Telemetry update
};

/**
 * @class SerializationAbstraction
 * @brief Provides an abstraction for serialization. The data will be serialized as long the underlying serializer supports it.
*/
class SerializationAbstraction {
    
    public:
    SerializationAbstraction() = default;
    virtual ~SerializationAbstraction() = default;

    /**
     * @brief Serializes the given data.
     * @param[in] data The data to serialize.
     * @param[out] serializedData The serialized data.
     * @param[in] type The type of the serialized data.
     * @pre serializedData needs to be sized big enough or the function will return an error.
     * @returns ErrorType::Success if the data was serialized.
     * @returns ErrorType::InvalidParameter if the data to serialize has not been resized to the amount of data it contains.
     * @returns ErrorType::InvalidParameter if the data to serialize has not been properly initialized.
     * @returns ErrorType::NotImplemented if not implemented.
     * @returns ErrorType::NotSupported if the type is not supported by the underlying deserializer.
     * @returns ErrorType::Failure for any other failure.
     * @note You may cast serializedData to the type that corresponds to the type given
     * @post The size of the encoded data is given by serializedData.size()
     */ 
    virtual ErrorType serialize(const std::string &data, std::string &serializedData, SerializationType type) = 0;
    /**
     * @brief Deserializes the given data.
     * @param[in] serializedData The data to deserialize.
     * @param[out] data The deserialized data.
     * @param[in] type The type of the serialized data.
     * @pre data needs to be sized big enough or the function will return an error.
     * @returns ErrorType::Success if the data was deserialized.
     * @returns ErrorType::InvalidParameter if the serialized data is invalid.
     * @returns ErrorType::NotImplemented if not implemented.
     * @returns ErrorType::NotSupported if the type is not supported by the underlying deserializer.
     * @returns ErrorType::Failure for any other failure.
    */
    virtual ErrorType deserialize(const std::string &serializedData, std::string &data, SerializationType type) = 0;
    /**
     * @brief Get the maximum size of the serialized data.
     * @param[out] maxSize The maximum size of the serialized data
     * @param[in] type The type to get the max size for
     * @returns ErrorType::Success if the max size could be determined
     * @returns ErrorType::NotSupported if the type is not supported by the underlying deserializer.
    */
    virtual ErrorType maxSerializedSize(Bytes &maxSize, SerializationType type) = 0;
    /**
     * @brief Get the maximum size of the deserialized data.
     * @param[out] maxSize The maximum size of the deserialized data
     * @param[in] type The type to get the max size for
     * @returns ErrorType::Success if the max size could be determined
     * @returns ErrorType::NotSupported if the type is not supported by the underlying deserializer.
    */
    virtual ErrorType maxDeserializedSize(Bytes &maxSize, SerializationType type) = 0;
};

#endif // __SERIALIZATION_ABSTRACTION_HPP__