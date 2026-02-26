/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   StaticString.hpp
* @details Wrapper for boost::static_string that is not templated so it does not need a fixed size type.
* @ingroup Utilities
*******************************************************************************/
#ifndef __STATIC_STRING_HPP__
#define __STATIC_STRING_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
#include "StaticStringConfig.h"
//Boost
#include "boost/static_string.hpp"
//C++
#include <any>
#include <type_traits>
#include <atomic>

/**
 * @namespace StaticString
 * @details Allows a templated string to be stored in a unified type, regardless of it's size.
 */
namespace StaticString {

    /**
     * @class StandardStringInterface
     * @brief Interface for implementing std::string-like strings.
     * @note substr is not inlcuded because it creates a static_string of the same length as the original (doubling the memory usage)
     */
    class StandardStringInterface {

        public:
        virtual ~StandardStringInterface() = default;
        /// @brief Implementation defined.
        virtual const char *c_str() const = 0;
        /// @brief Implementation defined.
        virtual size_t size() const = 0;
        /// @brief Implementation defined.
        virtual size_t length() const { return size(); }
        /// @brief Implementation defined.
        virtual char *data() = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface &append(const char *s, const size_t length) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface &append(std::string_view s) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface &assign(const char *s, const size_t length) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface &assign(std::string_view) = 0;
        /// @brief Implementation defined.
        virtual void clear() = 0;
        /// @brief Implementation defined.
        virtual bool empty() const = 0;
        /// @brief Implementation defined
        virtual constexpr size_t capacity() const = 0;
        /// @brief Implementation defined.
        virtual void resize(const size_t n) = 0;
        /// @brief Implementation defined.
        virtual char &back() = 0;
        /// @brief Implementation defined
        virtual void push_back(const char &c) = 0;
        /// @brief Implementation defined
        virtual size_t find(const char &c, const size_t pos = 0) const = 0;
        /// @brief Implementation defined
        virtual size_t find(const char *s, const size_t len) const = 0;
        /// @brief Implementation defined
        virtual size_t find(std::string_view) const = 0;
        /// @brief Implementation defined
        virtual size_t find(const char *s, const size_t pos, const size_t len) const = 0;
        /// @brief Implementation defined
        virtual StandardStringInterface &erase(const size_t pos, const size_t n) = 0;
        /// @brief Implementation defined
        virtual StandardStringInterface &erase(const char *iterator) = 0;
        /// @brief Implementation defined
        virtual char &at(const size_t pos) = 0;
        /// @brief Implementation defined
        virtual char *begin() = 0;
        /// @brief Implementation defined
        virtual const char *begin() const = 0;
        /// @brief Implementation defined
        virtual char *end() = 0;
        /// @brief Implementation defined
        virtual const char *end() const = 0;
    };

    /**
     * @class Data
     * @brief Implements the StandardStringInterface using static strings.
     * @tparam _n The size of the static string.
     * @note Try to use string sizes similar to sizes used elsewhere in the program to avoid excessive template instantiations.
     */
    template<size_t _n>
    class Data final : public StandardStringInterface {

        public:
        /**
         * @brief Constructor.
         * @param s The string to initialize the static string to.
         */
        constexpr Data(const char (&s)[_n]) : _str(s) {}
        constexpr Data(std::string_view s) : _str(s) {}
        /// @brief Constructor
        constexpr Data() : _str() {}

        const char *c_str() const override { return _str.c_str(); }
        size_t size() const override { return _str.size(); }
        char *data() override { return _str.data(); }
        StandardStringInterface &append(const char *s, const size_t length) override {
            _str.append(s, length); return *this;
        }
        StandardStringInterface &append(std::string_view s) override { _str.append(s); return *this;}
        StandardStringInterface &assign(const char *s, const size_t length) override {
            _str.assign(s, length); return *this;
        }
        StandardStringInterface &assign(std::string_view s) override { _str.assign(s); return *this; }
        void clear() override { _str.clear(); }
        bool empty() const override { return _str.empty(); }
        constexpr size_t capacity() const override { return _str.capacity(); }
        void resize(const size_t n) override { _str.resize(n); }
        char &back() override { return _str.back(); }
        void push_back(const char &c) override { return _str.push_back(c); }
        size_t find(const char &c, const size_t pos) const override { return _str.find(c, pos); }
        size_t find(const char *s, const size_t len) const override { return _str.find(s, 0, len); }
        size_t find(std::string_view s) const override { return _str.find(s); }
        size_t find(const char *s, const size_t pos, const size_t len) const override { return _str.find(s, pos, len); }
        StandardStringInterface &erase(const size_t pos, const size_t n) override { _str.erase(pos, n); return *this; }
        StandardStringInterface &erase(const char *iterator) override { _str.erase(static_cast<typename decltype(_str)::const_iterator>(iterator)); return *this; }
        char &at(const size_t pos) override { return _str.at(pos); }
        char *begin() override { return _str.begin(); }
        const char *begin() const override { return _str.begin(); }
        char *end() override { return _str.end(); }
        const char *end() const override { return _str.end(); }

        /// @brief Array access
        char &operator[](const size_t pos) {
            return _str[pos];
        }
        /// @brief Array access const
        const char &operator[](const size_t pos) const {
            return _str[pos];
        }

        private:
        /// @brief The underlying static string.
        boost::static_string<_n> _str;

    };

    /**
     * @class Container
     * @brief An owning, type-erased, and statically allocated container for any subclass of StandardStringInterface.
     */
    class Container {

        public:
        /// @brief Default Constructor
        constexpr Container() = default;
        /**
         * @brief Initializing a Container with Data.
         * @tparam _n The maximum number of bytes the string can store.
         * @returns A Container of string length _n
         */
        template <size_t _n>
        constexpr Container(const char (&s)[_n]) {
            set<_n>(std::string_view(s, _n));
        }
        template <size_t _n>
        constexpr Container(std::integral_constant<size_t, _n>) {
            set<_n>(std::string_view());
        }
        
        /// @brief Deleted because the container does not store the template type of the data and we can't use any_cast
        /// to store the new pointer.
        Container(const Container &other) = delete;
        
        /// @brief Move constructor
        Container(Container &&other) noexcept {
            *this = std::move(other);
        }

        ~Container() {
            if (_destroy != nullptr) {
                _destroy(_dataPtr);

                if (_dataBufferisFree != nullptr) {
                    _dataBufferisFree->store(false);
                }
            }
        }
        
        /// @brief The pure virtual interface that the container returns.
        using Interface = StandardStringInterface;
        /**
         * @brief Static storage for data.
         * @details When set<_n> is called and the buffer is not in use, the Container uses this storage.
         *          When the buffer is currently being used by another Container, allocation falls back
         *          to std::any (dynamic).
         */
        template <size_t _n>
        struct DataBuffer {
            alignas(Data<_n>) std::byte storage[sizeof(Data<_n>)];
            std::atomic<bool> in_use{false};
        };
        /// @brief The dynamically allocated data when it is too large to fit in the static buffer.
        std::any _data;
        /// @brief A pointer to access the interface from.
        Interface *_dataPtr = nullptr;
        void (*_destroy)(void *) = nullptr;
        /// @brief When non-null, points at the per-size static slot's in_use flag; clear it on destroy/reset/move-from.
        std::atomic<bool> *_dataBufferisFree = nullptr;

        template <size_t _n>
        static void destroyInBuffer(void *p) {
            static_cast<Data<_n> *>(p)->~Data();
        }

        /**
         * @brief Set the contained string to a specific type.
         * @tparam T The type of static string to store.
         * @param value The value to set the string to.
         */
        template <size_t _n>
        constexpr void set(std::string_view value) {
            static DataBuffer<_n> staticBuffer;

            if (!staticBuffer.in_use.exchange(true)) {
                new (staticBuffer.storage) Data<_n>(value);
                _dataPtr = reinterpret_cast<Interface *>(staticBuffer.storage);
                _destroy = &destroyInBuffer<_n>;
                _dataBufferisFree = &staticBuffer.in_use;
            }
            else {
                _destroy = nullptr;
                _dataBufferisFree = nullptr;
                _data = std::make_any<Data<_n>>(value);
                Data<_n> *dataPtr = std::any_cast<Data<_n>>(&_data);
                _dataPtr = static_cast<Interface *>(dataPtr);
            }
        }

        /// @brief Get a constant interface pointer
        const Interface *getConst() const {
            assert(nullptr != _dataPtr);
            return _dataPtr;
        }
        /// @brief Get a mutable interface pointer
        Interface *get() {
            assert(nullptr != _dataPtr);
            return _dataPtr;
        }
        /// @brief Iterator support for range-based for loops
        char* begin() {
            return get()->begin();
        }
        /// @brief Iterator support for range-based for loops (const)
        const char* begin() const {
            return getConst()->begin();
        }
        /// @brief Iterator support for range-based for loops
        char* end() {
            return get()->end();
        }
        /// @brief Iterator support for range-based for loops (const)
        const char* end() const {
            return getConst()->end();
        }

        /// @brief Reset the container to an empty state.
        void reset() {
            if (_destroy != nullptr) {
                _destroy(_dataPtr);

                if (_dataBufferisFree != nullptr) {
                    _dataBufferisFree->store(false);
                }

                _destroy = nullptr;
                _dataBufferisFree = nullptr;
            }
            else {
                _data.reset();
            }

            _dataPtr = nullptr;
        }

        /// @brief Shorthand operator for Container::get
        Interface *operator->() {
            return get();
        }
        /// @brief Shorthand operator for Container::get const
        const Interface *operator->() const {
            return getConst();
        }
        
        /// @copydoc Container(const Container &other)
        Container &operator=(const Container &other) = delete;
        
        /// @brief Move assignment operator
        Container &operator=(Container &&other) noexcept {
            if (this != &other) {
                reset();

                if (other._data.has_value()) {
                    _data = std::move(other._data);
                    _dataPtr = other._dataPtr;
                    other.reset();
                }
                else {
                    _dataPtr = other._dataPtr;
                    _destroy = other._destroy;
                    _dataBufferisFree = other._dataBufferisFree;
                    //Prevent the temporary from setting our buffer to unused. Only the current object
                    //through it's own destructor should set the buffer to unused.
                    other._dataBufferisFree = nullptr;
                    other.reset();
                }
            }

            return *this;
        }
        
        /// @brief Array access. No bounds checking
        char &operator[](const size_t pos) {
            return get()->data()[pos];
        }
        /// @brief Array access const. No bounds checking
        const char &operator[](const size_t pos) const {
            return getConst()->c_str()[pos];
        }
    };
}

#endif //__STATIC_STRING_HPP__