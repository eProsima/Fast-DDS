/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef _UTILITIES_HPP_
#define _UTILITIES_HPP_

#include <climits>
#include <string>
#include <vector>

/*!
 * @brief Converts a string to an unsigned 32-bit integer.
 *
 * @param n Reference to the variable where the result will be stored.
 * @param str The string to convert.
 * @param base The numerical base to use for the conversion (default is 10).
 * @return true if the conversion was successful, false otherwise.
 */
static bool read_uint32(
        uint32_t& n,
        std::string const& str,
        int base = 10)
{
    char* endp;
    unsigned long value = strtoul(str.c_str(), &endp, base);
    if ((endp == str.c_str()) || (value == ULONG_MAX && errno == ERANGE))
    {
        return false;
    }

    n = value;
    return true;
}

#endif // _UTILITIES_HPP_
