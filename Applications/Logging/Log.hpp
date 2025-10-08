/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Log.hpp
* @details Pretty printed logs. Much of this is borrowed from ESPs logging implementation
* @ingroup Applications
*******************************************************************************/
#ifndef __LOG_HPP__
#define __LOG_HPP__

//Modules
#include "LoggerModule.hpp"

//Use the menuconfig to enable/disable colors
#ifndef __XTENSA__
/// @def CONFIG_LOG_COLORS
/// @brief Enable colored log output
#define CONFIG_LOG_COLORS
#endif

#ifdef CONFIG_LOG_COLORS
/// @def APP_LOG_COLOR_BLACK
/// @brief The color black
#define APP_LOG_COLOR_BLACK   "30"
/// @def APP_LOG_COLOR_RED
/// @brief The color red
#define APP_LOG_COLOR_RED     "31"
/// @def APP_LOG_COLOR_GREEN
/// @brief The color green
#define APP_LOG_COLOR_GREEN   "32"
/// @def APP_LOG_COLOR_BROWN
/// @brief The color brown
#define APP_LOG_COLOR_BROWN   "33"
/// @def APP_LOG_COLOR_BLUE
/// @brief The color blue
#define APP_LOG_COLOR_BLUE    "34"
/// @def APP_LOG_COLOR_PURPLE
/// @brief The color purple
#define APP_LOG_COLOR_PURPLE  "35"
/// @def APP_LOG_COLOR_CYAN
/// @brief The color cyan
#define APP_LOG_COLOR_CYAN    "36"
/// @def APP_LOG_COLOR(COLOR)
/// @brief Used to make text colored.
#define APP_LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
/// @def APP_LOG_BOLD(COLOR)
/// @brief Used to make text bold and colored.
#define APP_LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
/// @def APP_LOG_RESET_COLOR
/// @brief Used to stop writing text that is coloured.
#define APP_LOG_RESET_COLOR   "\033[0m"
/// @def APP_LOG_COLOR_E
/// @brief The color for error messages
#define APP_LOG_COLOR_E       APP_LOG_COLOR(APP_LOG_COLOR_RED)
/// @def APP_LOG_COLOR_W
/// @brief The color for warnings
#define APP_LOG_COLOR_W       APP_LOG_COLOR(APP_LOG_COLOR_BROWN)
/// @def APP_LOG_COLOR_I
/// @brief The color for information messages
#define APP_LOG_COLOR_I       APP_LOG_COLOR(APP_LOG_COLOR_GREEN)
/// @def APP_LOG_COLOR_D
/// @brief The color for debugging messages
#define APP_LOG_COLOR_D       APP_LOG_COLOR(APP_LOG_COLOR_PURPLE)
/// @def APP_LOG_COLOR_V
/// @brief The color for verbose messages
#define APP_LOG_COLOR_V       APP_LOG_COLOR(APP_LOG_COLOR_CYAN)
#else //CONFIG_LOG_COLORS
/// @def APP_LOG_COLOR_E
/// @brief The color for error messages
#define APP_LOG_COLOR_E
/// @def APP_LOG_COLOR_W
/// @brief The color for warnings
#define APP_LOG_COLOR_W
/// @def APP_LOG_COLOR_I
/// @brief The color for information messages
#define APP_LOG_COLOR_I
/// @def APP_LOG_COLOR_D
/// @brief The color for debugging messages
#define APP_LOG_COLOR_D
/// @def APP_LOG_COLOR_V
/// @brief The color for verbose messages
#define APP_LOG_COLOR_V
/// @def APP_LOG_RESET_COLOR
/// @brief Used to stop writing text that is coloured.
#define APP_LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

/**
 * @def APP_LOG_FORMAT
 * @brief The format of a log message
 * @param letter The letter that represents the type of the log (warning, info, debug, etc.)
 * @param format The format of the log. Format is a printf-style format
 */
#define APP_LOG_FORMAT(letter, format)  APP_LOG_COLOR_## letter #letter " %s: " format APP_LOG_RESET_COLOR "\n"

/**
 * @def PLT_LOG_LEVEL
 * @brief Print a formatted log message to the terminal.
 * @details PLT is short for Platform
 * @param[in] type The type of the log
 * @sa LogType
 * @param[in] tag The tag of the log. Can be any string you like.
 * @param[in] format The format of the log. Format is a printf-style format string
 * @param[in] ... The arguments to the format string
 */
#define PLT_LOG_LEVEL(type, tag, format, ...) do { \
    if      (type == LogType::Error)   { Logger::Instance().log(type, tag, APP_LOG_FORMAT(E, format), tag, ##__VA_ARGS__); } \
    else if (type == LogType::Warning) { Logger::Instance().log(type, tag, APP_LOG_FORMAT(W, format), tag, ##__VA_ARGS__); } \
    else if (type == LogType::Info)    { Logger::Instance().log(type, tag, APP_LOG_FORMAT(I, format), tag, ##__VA_ARGS__); } \
    else if (type == LogType::Debug)   { Logger::Instance().log(type, tag, APP_LOG_FORMAT(D, format), tag, ##__VA_ARGS__); } \
} while(0)

/**
 * @def PLT_LOGE(tag, format, ...)
 * @brief Log an error
 * @param[in] tag The tag of the log
 * @param[in] format The format of the log. Format is a printf-style format string
 * @post Logs a message to stdout. If the terminal supports colored output, the message is printed in red.
 */
#define PLT_LOGE(tag, format, ... ) PLT_LOG_LEVEL(LogType::Error,   tag, format, ##__VA_ARGS__)

/**
 * @def PLT_LOGW(tag, format, ...)
 * @brief Log an error
 * @param[in] tag The tag of the log
 * @param[in] format The format of the log. Format is a printf-style format string
 * @note format is a printf-style format string
 * @post Logs a message to stdout. If the terminal supports colored output, the message is printed in yellow.
 */
#define PLT_LOGW(tag, format, ... ) PLT_LOG_LEVEL(LogType::Warning, tag, format, ##__VA_ARGS__)

/**
 * @def PLT_LOGI(tag, format, ...)
 * @brief Log an error
 * @param[in] tag The tag of the log
 * @param[in] format The format of the log. Format is a printf-style format string
 * @post Logs a message to stdout. If the terminal supports colored output, the message is printed in green
 */
#define PLT_LOGI(tag, format, ... ) PLT_LOG_LEVEL(LogType::Info,    tag, format, ##__VA_ARGS__)

/**
 * @def PLT_LOGD(tag, format, ...)
 * @brief Log an error
 * @param[in] tag The tag of the log
 * @param[in] format The format of the log. Format is a printf-style format string
 * @post Logs a message to stdout.
 */
#define PLT_LOGD(tag, format, ... ) PLT_LOG_LEVEL(LogType::Debug,   tag, format, ##__VA_ARGS__)

/**
 * @def PLT_LOG_BUFFER_HEXDUMP(TAG, buffer, buff_len, LogType)
 * @brief Dump the memory contents of a buffer.
 * @param[in] tag The log tag. Can be whatever you want to help you identify where the log came from.
 * @param[in] buffer The buffer to dump
 * @param[in] buff_len The length of the buffer
 * @param[in] type The type of the log
*/
#define PLT_LOG_BUFFER_HEXDUMP(tag, buffer, buff_len, type) do { \
    Logger::Instance().logBuffer(type, tag, buffer, buff_len); \
} while(0);

#endif // __LOG_HPP__
