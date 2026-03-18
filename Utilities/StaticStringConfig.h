//Boost
/// @def BOOST_STATIC_STRING_STANDALONE
/// @brief We don't use all of boost
#define BOOST_STATIC_STRING_STANDALONE
/// @def BOOST_STATIC_STRING_THROW(ex)
/// @brief AbstractionLayer does not use exceptions
#define BOOST_STATIC_STRING_THROW(ex) assert(false)
#include "boost/static_string/config.hpp"