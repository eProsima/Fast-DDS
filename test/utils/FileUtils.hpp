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
//
#ifndef TEST_UTILS__FILEUTILS_HPP
#define TEST_UTILS__FILEUTILS_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace eprosima {
namespace fastdds {
namespace testing {

std::string load_file(
        const std::string& file_path)
{
    std::ifstream file(file_path);

    // Check if the file was opened successfully
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file " + file_path);
    }

    // Use a stringstream to read the file contents into a string
    std::ostringstream buffer;
    buffer << file.rdbuf();

    // Close the file after reading
    file.close();

    // Return the string containing the file contents
    return buffer.str();
}

} // namespace testing
} // namespace fastdds
} // namespace eprosima

#endif // TEST_UTILS__FILEUTILS_HPP
