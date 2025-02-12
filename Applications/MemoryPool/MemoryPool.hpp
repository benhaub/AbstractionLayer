/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   MemoryPool.hpp
* @details \b Synopsis: \n Statically allocate memory and then allocate from it at run time in configurable blocks.
* @see https://en.wikipedia.org/wiki/Memory_pool
* @ingroup Applications
*******************************************************************************/
#ifndef __MEMORY_POOL_HPP__
#define __MEMORY_POOL_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
//C++
#include <array> //Some of the compilers that were tested did not have <span>

/**
 * @class MemoryPool
 * @brief Statically allocate memory and then dynamically allocate memory from it.
 * @pre _poolSize must be a multiple of _blockSize otherwise you're just wasting memory!
 * @tparam _poolSize The size of the pool.
 * @tparam _blockSize The size of each block in the pool.
 * @details We want to be able to initialize this at compile time so that's what the template is for.
 * @attention This class is not threadsafe. It is not reccomended to use the same pool accross multiple threads.
 */
template<Bytes _poolSize, Bytes _blockSize>
class MemoryPool {
    /// @brief Default constructor
    constexpr MemoryPool() {
        static_assert(_poolSize % _blockSize == 0, "Pool size should be a multiple of the block size");
    }

    /// @brief Return the size of the memory pool
    constexpr Bytes poolSize() const { return _poolSize; }
    /// @brief Return the size of a block in the memory pool
    constexpr Bytes blockSize() const { return _blockSize; }

    /**
     * @brief Allocate memory from the pool.
     * @param numBlocks The number of blocks to allocate.
     * @param poolBlock The pointer to the block of memory allocated from the pool.
     * @return ErrorType::Success if the memory was allocated
     * @returns ErrorType::NoMemory if the memory was not allocated.
     * @sa setData for a safe way to set the newly allocated block
     */ 
    ErrorType allocate(Count numBlocks, uint8_t *&poolBlock) {
        for (int i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (blockIsAvailable(i)) {
                _blockAllocationMap[i] = 1;
                poolBlock = &_pool[i * _blockSize];
                return ErrorType::Success;
            }
        }

        return ErrorType::NoMemory;
    }

    ErrorType deallocate(uint8_t *&poolBlock) {
        for (int i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (&_pool[i * _blockSize] == poolBlock) {
                _blockAllocationMap[i] = 0;
                return ErrorType::Success;
            }
        }

        return ErrorType::InvalidParameter;
    }

    ErrorType setData(uint8_t *&poolBlock, const uint8_t *data, Bytes dataSize) {
        if (dataSize > _blockSize) {
            return ErrorType::InvalidParameter;
        }

        std::copy(data, data + dataSize, poolBlock);
        return ErrorType::Success;
    }

    ErrorType available(Bytes &size) const {
        size = 0;
        for (int i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (blockIsAvailable(i)) {
                size += _blockSize;
            }
        }
        return ErrorType::Success;
    }

private:
    std::array<uint8_t, _poolSize> _pool;
    uint8_t _blockAllocationMap[_poolSize / _blockSize] = {0};

    ErrorType blockIsAvailable(int i) const {
        return _blockAllocationMap[i] == 0;
    };
};

#endif // __MEMORY_POOL_HPP__