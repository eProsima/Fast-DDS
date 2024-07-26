// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_UNITTEST_COMMON__UTILS_HPP
#define FASTDDS_TEST_UNITTEST_COMMON__UTILS_HPP

#include <cstddef>
#include <iostream>
#include <random>
#include <string>

/**
 * @brief Generates a random number between min_num and max_num.
 *
 * @param min_num Minimum number.
 * @param max_num Maximum number.
 * @return Random number.
 */
size_t generate_random_number(
        size_t min_num,
        size_t max_num)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<size_t> distribution(min_num, max_num);
    return distribution(generator);
}

/**
 * @brief Generates a random string of length str_length.
 *
 * @param str_length Length of the string.
 * @return Random string.
 */
std::string generate_random_string(
        size_t str_length)
{
    static const std::string chars =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "!@#$%^&*()_+{}|:<>?., []-";

    std::string str = "";

    for (size_t i = 0; i < str_length; ++i)
    {
        str += chars[generate_random_number(0, chars.size() - 1)];
    }

    return str;
}

#endif // FASTDDS_TEST_UNITTEST_COMMON__UTILS_HPP
