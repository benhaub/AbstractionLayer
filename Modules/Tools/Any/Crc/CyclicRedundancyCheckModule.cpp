//AbstractionLayer
#include "CyclicRedundancyCheckModule.hpp"
//PyCrc
extern "C" {
    #include "crc.h"
}

ErrorType Crc::crc8LittleEndian(uint8_t initialCrc, const std::string data, uint8_t &result) {
    return ErrorType::NotImplemented;
}

ErrorType Crc::crc8BigEndian(uint8_t initialCrc, const std::string data, uint8_t &result) {
    return ErrorType::NotImplemented;
}

ErrorType Crc::crc16LittleEndian(uint16_t initialCrc, const std::string data, uint16_t &result) {
    crc16_api_t crcCalculator = crc_api_crc16_arc;
    result = crcCalculator.finalize(crcCalculator.update(initialCrc, data.data(), data.size()));
    return ErrorType::Success;
}

ErrorType Crc::crc16BigEndian(uint16_t initialCrc, const std::string data, uint16_t &result) {
    return ErrorType::NotImplemented;
}

ErrorType Crc::crc32LittleEndian(uint32_t initialCrc, const std::string data, uint32_t &result) {
    crc32_api_t crcCalculator = crc_api_crc32_standard;
    result = crcCalculator.finalize(crcCalculator.update(initialCrc, data.data(), data.size()));
    return ErrorType::Success;
}

ErrorType Crc::crc32BigEndian(uint32_t initialCrc, const std::string data, uint32_t &result) {
    return ErrorType::NotImplemented;
}
