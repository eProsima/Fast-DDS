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

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif // _WIN32

#include <gtest/gtest.h>
#include <fastrtps/types/TypesBase.h>
#include <utils/SystemInfo.hpp>

#define SIZE 512

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

/*
 * This test checks the get_username static method of the SystemInfo class
 * The test only checks that the method returns RETCODE_OK and that the username is not an empty string
 */
TEST(SystemInfoTests, GetUsernameTest)
{
    std::string username;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::get_username(username));
    EXPECT_FALSE(username.empty());
}

/**
 * This test checks the file_exists static method of the SystemInfo class
 */
TEST(SystemInfoTests, FileExistsTest)
{
    // 1. Check that a valid file in the current directory exists
    std::string filename = "environment_test_file.json";
    EXPECT_TRUE(eprosima::SystemInfo::file_exists(filename));

    // 2. Check that a valid path and filename returns correctly
    char buffer[SIZE];
    char* current_dir = getcwd(buffer, SIZE);
    if (current_dir)
    {
        filename = current_dir;
        filename += "/environment_test_file.json";
        EXPECT_TRUE(eprosima::SystemInfo::file_exists(filename));
    }

    // 3. Check that a non valid filename fails
    filename = "non_existent.txt";
    EXPECT_FALSE(eprosima::SystemInfo::file_exists(filename));

    // 4. Check that a path to a non-existent file fails
    if (current_dir)
    {
        std::cout << current_dir << std::endl;
        filename = current_dir;
        EXPECT_FALSE(eprosima::SystemInfo::file_exists(filename + "/non_existent.txt"));
    }

    // 5. Check that an incomplete path (not including an existing file) fails
    if (current_dir)
    {
        EXPECT_FALSE(eprosima::SystemInfo::file_exists(current_dir));
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
