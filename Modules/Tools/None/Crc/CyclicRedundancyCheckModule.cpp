//AbstractionLayer
#include "CyclicRedundancyCheckModule.hpp"

namespace Crc {

    ErrorType crc8LittleEndian(uint8_t initialCrc, const std::string data, uint8_t &result) {
        return ErrorType::NotImplemented;
    }
    ErrorType crc8BigEndian(uint8_t initialCrc, const std::string data, uint8_t &result) {
        return ErrorType::NotImplemented;
    }
    ErrorType crc16LittleEndian(uint16_t initialCrc, const std::string data, uint16_t &result) {
        return ErrorType::NotImplemented;
    }
    ErrorType crc16BigEndian(uint16_t initialCrc, const std::string data, uint16_t &result) {
        return ErrorType::NotImplemented;
    }
    ErrorType crc32LittleEndian(uint32_t initialCrc, const std::string data, uint32_t &result) {
        return ErrorType::NotImplemented;
    }
    ErrorType crc32BigEndian(uint32_t initialCrc, const std::string data, uint32_t &result) {
        return ErrorType::NotImplemented;
    }
}
