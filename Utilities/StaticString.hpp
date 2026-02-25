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


#define APP_STATIC_STRING_BUFFER_SIZE 128
#ifndef APP_STATIC_STRING_BUFFER_SIZE
#error "Please define that size before static strings dynamically allocate."
#else
#define BUFFER_OPTIMIZATION sizeof(Data<APP_STATIC_STRING_BUFFER_SIZE>)
#endif

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
            if (!_data.has_value()) {

                if (_destroy != nullptr) {
                    _destroy(_staticBuffer.data());
                }
            }
        }
        
        private:
        /// @brief The pure virtual interface that the container returns.
        using Interface = StandardStringInterface;
        /// @brief The dynamically allocated data when it is too large to fit in the static buffer.
        std::any _data;
        /// @brief The static buffer for the data when it is small enough to fit.
        std::array<std::byte, BUFFER_OPTIMIZATION> _staticBuffer = {};
        /**
         * @brief A pointer so that the we any_cast to a Interface pointer.
         * @details A cast directly from T to Interface* is not allowed, so we need this intermediate step.
         */
        Interface *_dataPtr = nullptr;
        void (*_destroy)(void *) = nullptr;
        void (*_move)(void *dest, void *src) = nullptr;

        template <size_t _n>
        static void destroyInBuffer(void *p) {
            static_cast<Data<_n> *>(p)->~Data();
        }

        template <size_t _n>
        static void moveBetweenBuffers(void *dest, void *src) {
            new (dest) Data<_n>(std::move(*static_cast<Data<_n> *>(src)));
        }

        public:
        /**
         * @brief Set the contained string to a specific type.
         * @tparam T The type of static string to store.
         * @param value The value to set the string to.
         * @post If the size of T is less than BUFFER_OPTIMIZATION then data will be stored in the static buffer
         *.      and _data.has_value() will be false for the lifetime of this object.
         */
        template <size_t _n>
        constexpr void set(std::string_view value) {
            if constexpr (sizeof(Data<_n>) > BUFFER_OPTIMIZATION) {
                _destroy = nullptr;
                _move = nullptr;
                _data = std::make_any<Data<_n>>(value);
                Data<_n> *dataPtr = std::any_cast<Data<_n>>(&_data);
                _dataPtr = static_cast<Interface *>(dataPtr);
            }
            else {
                new (_staticBuffer.data()) Data<_n>(value);
                _dataPtr = reinterpret_cast<Interface *>(_staticBuffer.data());
                _destroy = &destroyInBuffer<_n>;
                _move = &moveBetweenBuffers<_n>;
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
            _data.reset();
            _dataPtr = nullptr;
            _staticBuffer = {};
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

                if (other._data.has_value()) {
                    _data = std::move(other._data);
                    _dataPtr = other._dataPtr;
                    other._data.reset();
                    other._dataPtr = nullptr;
                }
                else {
                    other._move(_staticBuffer.data(), other._staticBuffer.data());
                    other._destroy(other._staticBuffer.data());
                    _dataPtr = reinterpret_cast<Interface *>(_staticBuffer.data());
                    _destroy = other._destroy;
                    _move = other._move;
                    other._dataPtr = nullptr;
                    other._destroy = nullptr;
                    other._move = nullptr;
                    other._staticBuffer = {};
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