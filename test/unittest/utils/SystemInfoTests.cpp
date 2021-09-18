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
#include <string>
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
 * This test checks both get_env static methods of the SystemInfo class
 */
TEST_F(SystemInfoTests, GetEnvTest)
{
    const std::string env_var_name("TEST_ENVIRONMENT_VARIABLE");
    const std::string empty_var_name("EMPTY_ENV_VAR");
    const std::string value("TESTING");
    std::string filename("environment_test_file.json");
    std::string env_value;

    // 1. Set the testing environment variable
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s(env_var_name.c_str(), value.c_str()));
    ASSERT_EQ(0, _putenv_s(eprosima::FASTDDS_ENVIRONMENT_FILE_ENV_VAR, filename.c_str()));
    ASSERT_EQ(0, _putenv_s(eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, value.c_str()));
    ASSERT_EQ(0, _putenv_s("EMPTY_ENV_VAR", value.c_str()));
#else
    ASSERT_EQ(0, setenv(env_var_name.c_str(), value.c_str(), 1));
    ASSERT_EQ(0, setenv(eprosima::FASTDDS_ENVIRONMENT_FILE_ENV_VAR, filename.c_str(), 1));
    ASSERT_EQ(0, setenv(eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, value.c_str(), 1));
    ASSERT_EQ(0, setenv(empty_var_name.c_str(), value.c_str(), 1));
#endif // _WIN32
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::set_environment_file());

    // 2. Read environment variable not contained in the file but set in the environment
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::get_env(env_var_name, env_value));
    EXPECT_EQ(env_value, value);

    // 3. Read environment variable contained in the file and in the environment: file has priority
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::get_env(eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI,
            env_value));
    EXPECT_EQ("localhost:11811", env_value);

    // 4. Read variable set empty in the file but with a valid value in the environment: file has priority
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::get_env(empty_var_name, env_value));
    EXPECT_TRUE(env_value.empty());

    // 5. Invalid environment name: neither in the file nor in the environment
    EXPECT_EQ(eprosima::SystemInfo::get_env("INVALID_NAME", env_value), ReturnCode_t::RETCODE_NO_DATA);

    // 6. Bad parameters: empty environment name
    std::string non_init_string;
    EXPECT_EQ(eprosima::SystemInfo::get_env("", env_value), ReturnCode_t::RETCODE_BAD_PARAMETER);
    EXPECT_EQ(eprosima::SystemInfo::get_env(non_init_string, env_value), ReturnCode_t::RETCODE_BAD_PARAMETER);

    // 7. Check that reading the environment variable directly from file returns correctly
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::get_env(filename,
            eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, env_value));
    EXPECT_EQ("localhost:11811", env_value);

    // 8. Check that an empty environment variable returns correctly
    std::string empty_environment_variable = "EMPTY_ENV_VAR";
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::get_env(filename,
            empty_environment_variable, env_value));
    EXPECT_TRUE(env_value.empty());

    // 9. Check that a non-existent tag returns RETCODE_NO_DATA
    std::string non_existent_env_variable = "NON_EXISTENT_ENV_VARIBLE";
    env_value.clear();
    EXPECT_EQ(ReturnCode_t::RETCODE_NO_DATA, eprosima::SystemInfo::get_env(filename,
            non_existent_env_variable, env_value));
    EXPECT_TRUE(env_value.empty());

    // 10. Check that a non-existent file returns RETCODE_BAD_PARAMETER
    filename = "non_existent.json";
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, eprosima::SystemInfo::get_env(filename,
            eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, env_value));
    EXPECT_TRUE(env_value.empty());

    // 11. Check that a wrong formatted file returns RETCODE_ERROR
    filename = "empty_environment_test_file.json";
    EXPECT_EQ(ReturnCode_t::RETCODE_ERROR, eprosima::SystemInfo::get_env(filename,
            eprosima::fastdds::rtps::DEFAULT_ROS2_MASTER_URI, env_value));
    EXPECT_TRUE(env_value.empty());
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
 * Test that checks set_environment_file and get_environment_file
 */
TEST_F(SystemInfoTests, EnvironmentFileTest)
{
    // 1. Environment variable not set: call to set_environment_variable returns RETCODE_NO_DATA and
    // get_environment_file returns empty
    EXPECT_EQ(ReturnCode_t::RETCODE_NO_DATA, eprosima::SystemInfo::set_environment_file());
    EXPECT_TRUE(eprosima::SystemInfo::get_environment_file().empty());

    // 2. Set environment file variable and see that it returns correctly
    const std::string value("TESTING");
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s(eprosima::FASTDDS_ENVIRONMENT_FILE_ENV_VAR, value.c_str()));
#else
    ASSERT_EQ(0, setenv(eprosima::FASTDDS_ENVIRONMENT_FILE_ENV_VAR, value.c_str(), 1));
#endif // _WIN32
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::SystemInfo::set_environment_file());
    EXPECT_EQ(0, eprosima::SystemInfo::get_environment_file().compare(value));
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
            eprosima::SystemInfo::watch_file(filename, [&]()
                    {
                        {
                            std::unique_lock<std::mutex> lck(*mutex_);
                            times_called_.store(++times_called_, std::memory_order_release);
                        }
                        cv_.notify_all();
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

#if defined(_WIN32) || defined(__unix__)
    // Check modifications.
    block_for_at_least_N_callbacks(1);
    uint32_t times_called = times_called_;
    EXPECT_EQ(1u, times_called);
#else
    // Unsupported platforms will not call the callback
    uint32_t times_called = times_called_;
    EXPECT_EQ(0u, times_called);
#endif // defined(_WIN32) || defined(__unix__)

    // Required because the file modification time has resolution of seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));

    file_content["EMPTY_ENV_VAR"] = "another_value";

    {
        std::ofstream file(filename);
        file << file_content;
    }

#if defined(_WIN32) || defined(__unix__)
    // Check modifications.
    block_for_at_least_N_callbacks(2);
    times_called = times_called_;
    EXPECT_EQ(2u, times_called);
#else
    // Unsupported platforms will not call the callback
    times_called = times_called_;
    EXPECT_EQ(0u, times_called);
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
