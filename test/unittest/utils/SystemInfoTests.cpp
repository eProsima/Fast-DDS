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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <mutex>
#include <stdlib.h>
#include <thread>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif // _WIN32

#include <gtest/gtest.h>
#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastrtps/types/TypesBase.h>
#include <json.hpp>
#include <utils/SystemInfo.hpp>

#define SIZE 512

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

class SystemInfoTests : public ::testing::Test
{
public:

    void block_for_at_least_N_callbacks(
            uint32_t amount)
    {
        std::unique_lock<std::mutex> lck(*mutex_);
        cv_.wait(lck, [this, amount]
                {
                    return times_called_ >= amount;
                });
    }

    mutable std::mutex* mutex_;
    std::condition_variable cv_;
    std::atomic_uint32_t times_called_;

protected:

    void SetUp() override
    {
        mutex_ = new std::mutex();
        times_called_ = 0;
    }

    void TearDown() override
    {
        delete mutex_;
        mutex_ = nullptr;
    }
};

/*
 * This test checks the get_env static method of the SystemInfo class
 * 1. Set an environment variable
 * 2. Use get_env to read the environment variable
 * 3. Call get_env with nullptr parameters
 * 4. Call get_env with empty environment name
 * 5. Call get_env with an invalid environment variable
 */
TEST_F(SystemInfoTests, GetEnvTest)
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
TEST_F(SystemInfoTests, GetUsernameTest)
{
    std::string username;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::get_username(username));
    EXPECT_FALSE(username.empty());
}

/**
 * This test checks the file_exists static method of the SystemInfo class
 */
TEST_F(SystemInfoTests, FileExistsTest)
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

/**
 * This test checks the load_environment_file method of the SystemInfo class
 */
TEST_F(SystemInfoTests, LoadEnvironmentFileTest)
{
    // 1. Check that reading the environment variable ROS_DISCOVERY_SERVER returns the correct information
    std::string filename = "environment_test_file.json";
    std::string environment_value;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::load_environment_file(filename,
            eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, environment_value));
    EXPECT_EQ("localhost:11811", environment_value);

    // 2. Check that a non-existent tag returns RETCODE_NO_DATA
    std::string non_existent_env_variable = "NON_EXISTENT_ENV_VARIBLE";
    environment_value.clear();
    EXPECT_EQ(ReturnCode_t::RETCODE_NO_DATA, eprosima::SystemInfo::load_environment_file(filename,
            non_existent_env_variable, environment_value));
    EXPECT_TRUE(environment_value.empty());

    // 3. Check that an empty environment variable returns RETCODE_NO_DATA
    std::string empty_environment_variable = "EMPTY_ENV_VAR";
    EXPECT_EQ(ReturnCode_t::RETCODE_NO_DATA, eprosima::SystemInfo::load_environment_file(filename,
            empty_environment_variable, environment_value));
    EXPECT_TRUE(environment_value.empty());

    // 4. Check that a non-existent file returns RETCODE_BAD_PARAMETER
    filename = "non_existent.json";
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, eprosima::SystemInfo::load_environment_file(filename,
            eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, environment_value));
    EXPECT_TRUE(environment_value.empty());

    // 5. Check that a wrong formatted file returns RETCODE_ERROR
    filename = "empty_environment_test_file.json";
    EXPECT_EQ(ReturnCode_t::RETCODE_ERROR, eprosima::SystemInfo::load_environment_file(filename,
            eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, environment_value));
    EXPECT_TRUE(environment_value.empty());
}

/**
 * This test checks the file watchers
 */
TEST_F(SystemInfoTests, FileWatchTest)
{
    std::string filename = "environment_test_file.json";
    nlohmann::json file_content;

    // Create filewatch
    eprosima::FileWatchHandle watch =
            eprosima::SystemInfo::watch_file(filename, [&](const std::string&)
                    {
                        ++times_called_;
                        cv_.notify_one();
                    });

    // Read contents
    {
        std::ifstream file(filename);
        file >> file_content;
    }

    file_content["EMPTY_ENV_VAR"] = "not_empty";

    // Write new content
    {
        std::ofstream file(filename);
        file << file_content;
    }

    // Check modifications.
    block_for_at_least_N_callbacks(1);
    int times_called = times_called_;
#if defined(_WIN32) || defined(__unix__)
    EXPECT_EQ(1, times_called);

#else
    // Unsupported platforms will not call the callback
    EXPECT_EQ(0, times_called);
#endif // defined(_WIN32) || defined(__unix__)

    // Remove the watcher
    eprosima::SystemInfo::stop_watching_file(watch);

    // Restore content of file
    file_content["EMPTY_ENV_VAR"] = "";
    {
        std::ofstream file(filename);
        file << file_content;
    }

    // Check no modifications were notified
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(times_called, times_called_);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
