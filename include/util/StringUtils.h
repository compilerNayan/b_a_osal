/**
 * @file StringUtils.h
 * @brief String helpers: ToString (number to StdString), Format (single placeholder).
 */

#ifndef CPP_UTILS_STRING_UTILS_H
#define CPP_UTILS_STRING_UTILS_H

#include <StandardDefines.h>
#include <string>
#include <type_traits>

namespace cpp_utils {

/** Convert numeric value to StdString. One overload per actual type (avoids redefinition when Size == UInt on 32-bit). */
template<typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, StdString>::type ToString(T value) {
    return std::to_string(value);
}

/** Replace first "{}" in format with string form of value. Returns formatted string. */
template<typename T>
StdString Format(const StdString& fmt, const T& value) {
    StdString s = std::to_string(value);
    StdString::size_type pos = fmt.find("{}");
    if (pos == StdString::npos) return fmt;
    return fmt.substr(0, pos) + s + fmt.substr(pos + 2);
}

/** Overload for StdString value (no conversion). */
inline StdString Format(const StdString& fmt, const StdString& value) {
    StdString::size_type pos = fmt.find("{}");
    if (pos == StdString::npos) return fmt;
    return fmt.substr(0, pos) + value + fmt.substr(pos + 2);
}

/** Overload for const char* (e.g. literal). */
inline StdString Format(const StdString& fmt, const char* value) {
    StdString::size_type pos = fmt.find("{}");
    if (pos == StdString::npos) return fmt;
    return fmt.substr(0, pos) + (value ? value : "") + fmt.substr(pos + 2);
}

}  // namespace cpp_utils

#endif  // CPP_UTILS_STRING_UTILS_H
