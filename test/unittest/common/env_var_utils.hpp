// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file env_var_utils.hpp
 */

#ifndef EPROSIMA_TESTING_ENV_VAR_UTILS_HPP
#define EPROSIMA_TESTING_ENV_VAR_UTILS_HPP

#include <cstdlib>

#include <gtest/gtest.h>

namespace eprosima {
namespace testing {

static void set_environment_variable(
        const char* const env_var,
        const char* const value)
{
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s(env_var, value));
#else
    ASSERT_EQ(0, setenv(env_var, value, 1));
#endif // _WIN32
}

static void clear_environment_variable(
        const char* const env_var)
{
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s(env_var, ""));
#else
    ASSERT_EQ(0, unsetenv(env_var));
#endif // _WIN32
}

}  // namespace testing
}  // namespace eprosima

#endif  // EPROSIMA_TESTING_ENV_VAR_UTILS_HPP
