// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file fixed_size_string.hpp
 *
 */

#ifndef FASTRTPS_UTILS_FIXED_SIZE_STRING_HPP_
#define FASTRTPS_UTILS_FIXED_SIZE_STRING_HPP_

#include <string>
#include <cstring>

#ifdef _WIN32
#define MEMCCPY _memccpy
#else
#define MEMCCPY memccpy
#endif // ifdef _WIN32

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
namespace eprosima {
namespace fastrtps {

/**
 * Template class for non-alloc strings. Will be truncated when assigned from a longer string.
 * @tparam MAX_CHARS Maximum number of characters is specified as the template parameter.
 *                   Space for an additional null terminator will be reserved.
 * @ingroup UTILITIES_MODULE
 */
template <size_t MAX_CHARS>
struct fixed_string
{
public:

    static constexpr size_t max_size = MAX_CHARS;

    /// Default constructor.
    fixed_string() noexcept
    {
        memset(string_data, 0, sizeof(string_data));
        string_len = 0;
    }

    // We don't need to define copy/move constructors/assignment operators as the default ones would be enough

    // Construct / assign from a char array
    fixed_string(
            const char* c_array,
            size_t n_chars) noexcept
    {
        assign(c_array, n_chars);
    }

    fixed_string& assign(
            const char* c_array,
            size_t n_chars) noexcept
    {
        string_len = (nullptr == c_array) ? 0 :
                (MAX_CHARS < n_chars) ? MAX_CHARS : n_chars;
        if (0 < string_len)
        {
            memcpy(string_data, c_array, string_len);
        }
        return *this;
    }

    // Construct / assign from a C string
    fixed_string (
            const char* c_string) noexcept
        : fixed_string()
    {
        set(c_string != nullptr ? c_string : "");
    }

    fixed_string& operator = (
            const char* c_string) noexcept
    {
        set(c_string != nullptr ? c_string : "");
        return *this;
    }

    // Construct / assign from a std::string
    fixed_string (
            const std::string& str) noexcept
        : fixed_string()
    {
        set(str.c_str());
    }

    fixed_string& operator = (
            const std::string& str) noexcept
    {
        set(str.c_str()); return *this;
    }

    // Assign from fixed_string of any size
    template<size_t N> fixed_string& operator = (
            const fixed_string<N>& rhs) noexcept
    {
        set(rhs.c_str()); return *this;
    }

    // Converters to standard types
    const char* c_str() const noexcept
    {
        return string_data;
    }

    std::string to_string() const
    {
        return std::string(string_data);
    }

    // Equality comparisons
    bool operator == (
            const char* rhs) const noexcept
    {
        return strncmp(string_data, rhs, MAX_CHARS) == 0;
    }

    bool operator == (
            const std::string& rhs) const noexcept
    {
        return strncmp(string_data, rhs.c_str(), MAX_CHARS) == 0;
    }

    template<size_t N>  bool operator == (
            const fixed_string<N>& rhs) const noexcept
    {
        return strncmp(string_data, rhs.c_str(), MAX_CHARS) == 0;
    }

    // Inequality comparisons
    bool operator != (
            const char* rhs) const noexcept
    {
        return strncmp(string_data, rhs, MAX_CHARS) != 0;
    }

    bool operator != (
            const std::string& rhs) const noexcept
    {
        return strncmp(string_data, rhs.c_str(), MAX_CHARS) != 0;
    }

    template<size_t N>  bool operator != (
            const fixed_string<N>& rhs) const noexcept
    {
        return strncmp(string_data, rhs.c_str(), MAX_CHARS) != 0;
    }

    template<size_t N>  bool operator < (
            const fixed_string<N>& rhs) const noexcept
    {
        return 0 > compare(rhs);
    }

    template<size_t N>  bool operator > (
            const fixed_string<N>& rhs) const noexcept
    {
        return 0 < compare(rhs);
    }

    bool operator < (
            const std::string& rhs) const noexcept
    {
        return 0 > compare(rhs);
    }

    bool operator > (
            const std::string& rhs) const noexcept
    {
        return 0 < compare(rhs);
    }

    operator const char* () const noexcept {
        return c_str();
    }

    size_t size() const noexcept
    {
        return string_len;
    }

    /*!
     * Compare with a C string.
     *
     * @param str C string to be compared with.
     *
     * @return Integer value with the result of the comparison as described in `std::string::compare()`.
     */
    int compare(
            const char* str) const noexcept
    {
        return strncmp(string_data, str, MAX_CHARS);
    }

    /*!
     * Compare with a std::string.
     *
     * @param str std::string to be compared with.
     *
     * @return Integer value with the result of the comparison as described in `std::string::compare()`.
     */
    int compare(
            const std::string& str) const noexcept
    {
        return strncmp(string_data, str.c_str(), MAX_CHARS);
    }

    /*!
     * Compare with a fixed_string
     *
     * @param str fixed_string to be compared with.
     *
     * @return Integer value with the result of the comparison as described in `std::string::compare()`.
     */
    template<size_t N>  int compare(
            const fixed_string<N>& str) const noexcept
    {
        return strncmp(string_data, str.c_str(), MAX_CHARS);
    }

private:

    void set(
            const char* c_string) noexcept
    {
        char* result = (char*) MEMCCPY(string_data, c_string, '\0', MAX_CHARS);
        string_len = (result == nullptr) ? MAX_CHARS : (size_t)(result - string_data) - 1u;
    }

    char string_data[MAX_CHARS + 1];      ///< Holds string data, including ending null character.
    size_t string_len;                    ///< Holds current string length.
};

using string_255 = fixed_string<255>;

} /* namespace fastrtps */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif /* FASTRTPS_UTILS_FIXED_SIZE_STRING_HPP_ */
