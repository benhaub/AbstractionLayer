//AbstractionLayer
#include "CyclicRedundancyCheckModule.hpp"
//ESP
#include "esp32s3/rom/crc.h"

ErrorType Crc::crc8LittleEndian(uint8_t initialCrc, const std::string data, uint8_t &result) {
    result = crc8_le(initialCrc, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return ErrorType::Success;
}

ErrorType Crc::crc8BigEndian(uint8_t initialCrc, const std::string data, uint8_t &result) {
    result = crc8_be(initialCrc, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return ErrorType::Success;
}

ErrorType Crc::crc16LittleEndian(uint16_t initialCrc, const std::string data, uint16_t &result) {
    result = crc16_le(initialCrc, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return ErrorType::Success;
}

ErrorType Crc::crc16BigEndian(uint16_t initialCrc, const std::string data, uint16_t &result) {
    result = crc16_be(initialCrc, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return ErrorType::Success;
}

ErrorType Crc::crc32LittleEndian(uint32_t initialCrc, const std::string data, uint32_t &result) {
    result = crc32_le(initialCrc, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return ErrorType::Success;
}

ErrorType Crc::crc32BigEndian(uint32_t initialCrc, const std::string data, uint32_t &result) {
    result = crc32_be(initialCrc, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return ErrorType::Success;
}