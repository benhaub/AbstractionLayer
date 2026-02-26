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
#include <cstddef>
#include <memory>

/**
 * @class MemoryPool
 * @brief Statically allocate a block of memory and then allocate chunks of it from it at runtime as if it were dynamically allocated.
 * @details If we get carried away with features and trying to manage fragementation, we'll just end up implementing a heap allocator.
 *          The mempool is meant to be simple and naive so that it saves time in comparision to a heap allocator. If you are using a mempool,
 *          You are making regular and frequent allocations that you know ahead of time what the max size of the allocations will be and the
 *          max number of items that will could be allocated at once. If you do not know this then a mempool is not a good idea.
 * @pre _poolSize must be a multiple of sizeof(T) otherwise you're just wasting memory!
 * @tparam _blockSize The size of the block to allocate from the pool.
 * @tparam _poolSize The size of the pool.
 * @details We want to be able to initialize this at compile time so that's what the template is for.
 * @attention This class is not threadsafe. It is not reccomended to use the same pool accross multiple threads.
 */
template<typename T, Bytes _numberOfBlocks>
class MemoryPool {

    public:
    /// @brief Return the size of the memory pool
    static constexpr Bytes poolSize() { return _numberOfBlocks * sizeof(T); }
    /// @brief Return the size of a block in the memory pool
    static constexpr Bytes blockSize() { return sizeof(T); }

    /**
     * @brief Allocate memory from the pool.
     * @param[out] item The pointer to the item allocated from the pool.
     * @return ErrorType::Success if the memory was allocated
     * @returns ErrorType::NoMemory if the memory was not allocated.
     * @sa setData for a safe way to set the newly allocated block
     */ 
    constexpr ErrorType allocate(T *&item) {
        for (size_t i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (blockIsAvailable(i)) {
                item = reinterpret_cast<T*>(&_pool[i*sizeof(T)]);
                new (item) T();
                _blockAllocationMap[i] = 1;
                return ErrorType::Success;
            }
        }

        return ErrorType::NoMemory;
    }

    /**
     * @brief Deallocate memory from the pool.
     * @param[in] item The pointer to the block of memory to deallocate.
     * @return ErrorType::Success if the memory was deallocated
     * @returns ErrorType::InvalidParameter if the memory was not deallocated.
     */
    constexpr ErrorType deallocate(const T *const item) {
        for (size_t i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (item == reinterpret_cast<T*>(&_pool[i*sizeof(T)])) {
                std::destroy_at(const_cast<T*>(item));
                _blockAllocationMap[i] = 0;
                return ErrorType::Success;
            }
        }

        return ErrorType::InvalidParameter;
    }

    /**
     * @brief Get the available memory in the pool.
     * @param[out] size The size of the available memory in the pool.
     * @return ErrorType::Success always
     */
    constexpr ErrorType available(Bytes &size) const {
        size = 0;
        for (size_t i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (blockIsAvailable(i)) {
                size += sizeof(T);
            }
        }

        return ErrorType::Success;
    }

    private:
    /// @brief The pool of memory
    std::array<uint8_t, poolSize()> _pool;
    /// @brief A map of which blocks are available.
    std::array<uint8_t, _numberOfBlocks> _blockAllocationMap = {0};

    /**
     * @brief Check if a block is available.
     * @param[in] i The index of the block to check.
     * @return true if the block is available.
     * @return false if the block is not available.
     */
    constexpr bool blockIsAvailable(const size_t i) const {
        constexpr int blockMarkedAvailable = 0;
        return _blockAllocationMap[i] == blockMarkedAvailable;
    }
};

#endif // __MEMORY_POOL_HPP__