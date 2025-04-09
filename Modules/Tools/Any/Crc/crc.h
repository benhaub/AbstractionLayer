#ifndef __CRC_H__
#define __CRC_H__

/*
 * Refer to this CRC catalogue for more information
 * https://reveng.sourceforge.io/crc-catalogue/
 */

#include <stdint.h>
#include <stdlib.h>

/**
 * 16-bit CRC values.
 *
 */
typedef uint16_t crc16_t;

/**
 * \brief API struct for CRC16 algorithms.
 * \param init - Initial CRC value.
 * \param update - Function to update a CRC with more data.
 * \param finalize - Function to finalize a CRC calculation according to the algorithm requirements.
 */
typedef struct {
  crc16_t init;
  crc16_t (*update)(crc16_t crc, const void *data, size_t data_len);
  crc16_t (*finalize)(crc16_t crc);
} crc16_api_t;

/**
 * 32-bit CRC values.
 */
typedef uint32_t crc32_t;

/**
 * \brief API struct for CRC32 algorithms.
 * \param init - Initial CRC value.
 * \param update - Function to update a CRC with more data.
 * \param finalize - Function to finalize a CRC calculation according to the algorithm requirements.
 */
typedef struct {
  crc32_t init;
  crc32_t (*update)(crc32_t crc, const void *data, size_t data_len);
  crc32_t (*finalize)(crc32_t crc);
} crc32_api_t;

/*
 * API for CRC-16/ARC.
 * poly=0x800e init=0x0000 reflect-in=True reflect-out=True xor-out=0x0000
 * check=0xbb3d
 */
extern const crc16_api_t crc_api_crc16_arc;

/*
 * API for CRC32/BZIP2.
 * poly=0x04c11db7 init=0xffffffff reflect-in=False reflect-out=False xorout=0xffffffff
 * check=0xfc891918
 */
extern const crc32_api_t crc_api_crc32_bzip2;

/*
 * API for standard CRC32.
 * poly=0x04c11db7 init=0xffffffff reflect-in=True reflect-out=True xorout=0xffffffff
 * check=0xcbf43926
 */
extern const crc32_api_t crc_api_crc32_standard;
/* Aliases for standard CRC32 implementation */
#define crc_api_crc32_iso_hdlc crc_api_crc32_standard
#define crc_api_crc32_ieee crc_api_crc32_standard
#define crc_api_crc32_pkzip crc_api_crc32_standard
#define crc_api_crc32_ITUV42 crc_api_crc32_standard
#define crc_api_crc32_ISO3309 crc_api_crc32_standard

#endif /* __CRC_H__ */