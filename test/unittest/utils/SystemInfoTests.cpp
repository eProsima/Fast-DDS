// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <stdlib.h>

#include <gtest/gtest.h>
#include <fastrtps/types/TypesBase.h>
#include <utils/SystemInfo.hpp>

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

/*
 * This test checks the get_env static method of the SystemInfo class
 * 1. Set an environment variable
 * 2. Use get_env to read the environment variable
 * 3. Call get_env with nullptr parameters
 * 4. Call get_env with empty environment name
 * 5. Call get_env with an invalid environment variable
 */
TEST(SystemInfoTests, GetEnvTest)
{
    const char* env_var_name = "TEST_ENVIRONMENT_VARIABLE";
    const char* value = "TESTING";
    const char* env_value;

    // 1. Set the testing environment variable
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s(env_var_name, value));
#else
    ASSERT_EQ(0, setenv(env_var_name, value, 1));
#endif // _WIN32

    // 2. Read environment variable
    EXPECT_EQ(eprosima::SystemInfo::get_env(env_var_name, &env_value), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(strcmp(env_value, value), 0);

    // 3. Bad parameters
    EXPECT_EQ(eprosima::SystemInfo::get_env(nullptr, &env_value), ReturnCode_t::RETCODE_BAD_PARAMETER);
    EXPECT_EQ(eprosima::SystemInfo::get_env(env_var_name, nullptr), ReturnCode_t::RETCODE_BAD_PARAMETER);

    // 4. Empty environment name
    EXPECT_EQ(eprosima::SystemInfo::get_env("", &env_value), ReturnCode_t::RETCODE_BAD_PARAMETER);

    // 5. Invalid environment name
    EXPECT_EQ(eprosima::SystemInfo::get_env("INVALID_NAME", &env_value), ReturnCode_t::RETCODE_NO_DATA);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
