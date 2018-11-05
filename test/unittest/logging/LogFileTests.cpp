// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/log/FileConsumer.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>

using namespace eprosima::fastrtps;
using namespace std;

TEST(LogFileTests, file_consumer)
{
    // First remove previous executions file
    std::remove("file_consumer.log");

    std::unique_ptr<FileConsumer> fileConsumer(new FileConsumer("file_consumer.log"));
    Log::ClearConsumers();
    Log::RegisterConsumer(std::move(fileConsumer));
    Log::SetVerbosity(Log::Info);

    vector<unique_ptr<thread>> threads;
    for (int i = 0; i != 5; i++)
    {
        threads.emplace_back(new thread([i]{
            logWarning(Multithread, "I'm thread " << i);
        }));
    }

    for (auto& thread: threads) {
        thread->join();
    }

    Log::ClearConsumers(); // Force close file

    std::ifstream ifs("file_consumer.log");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    for (int i = 0; i != 5; ++i)
    {
        std::string str("I'm thread " + i);
        std::size_t found = content.find(str);
        ASSERT_TRUE(found != std::string::npos);
    }
}

TEST(LogFileTests, file_consumer_append)
{
    // First remove previous executions file
    std::remove("append.log");

    std::unique_ptr<FileConsumer> fileConsumer(new FileConsumer("append.log", true));
    Log::ClearConsumers();
    Log::RegisterConsumer(std::move(fileConsumer));
    Log::SetVerbosity(Log::Info);

    vector<unique_ptr<thread>> threads;
    for (int i = 0; i != 5; i++)
    {
        threads.emplace_back(new thread([i]{
            logWarning(Multithread, "I'm thread " << i);
        }));
    }

    for (auto& thread: threads) {
        thread->join();
    }

    Log::ClearConsumers(); // Force close file

    std::unique_ptr<FileConsumer> fileConsumer2(new FileConsumer("append.log", true));
    Log::RegisterConsumer(std::move(fileConsumer2));

    vector<unique_ptr<thread>> threads2;
    for (int i = 0; i != 5; i++)
    {
        threads2.emplace_back(new thread([i]{
            logWarning(Multithread, "I'm thread " << i+5);
        }));
    }

    for (auto& thread: threads2) {
        thread->join();
    }

    Log::ClearConsumers(); // Force close file

    std::ifstream ifs("append.log");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    for (int i = 0; i != 10; ++i)
    {
        std::string str("I'm thread " + i);
        std::size_t found = content.find(str);
        ASSERT_TRUE(found != std::string::npos);
    }
}

TEST(LogFileTests, file_xml_consumer_append)
{
    // First remove previous executions file
    std::remove("test1.log");

    xmlparser::XMLProfileManager::loadXMLFile("log_node_file_append.xml");

    Log::SetVerbosity(Log::Info);

    vector<unique_ptr<thread>> threads;
    for (int i = 0; i != 5; i++)
    {
        threads.emplace_back(new thread([i]{
            logWarning(Multithread, "I'm thread " << i);
        }));
    }

    for (auto& thread: threads) {
        thread->join();
    }

    Log::ClearConsumers(); // Force close file

    xmlparser::XMLProfileManager::loadXMLFile("log_node_file_append.xml");

    vector<unique_ptr<thread>> threads2;
    for (int i = 0; i != 5; i++)
    {
        threads2.emplace_back(new thread([i]{
            logWarning(Multithread, "I'm thread " << i+5);
        }));
    }

    for (auto& thread: threads2) {
        thread->join();
    }

    Log::ClearConsumers(); // Force close file

    std::ifstream ifs("test1.log");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    for (int i = 0; i != 10; ++i)
    {
        std::string str("I'm thread " + i);
        std::size_t found = content.find(str);
        ASSERT_TRUE(found != std::string::npos);
    }
}

TEST(LogFileTests, log_inactive)
{
    xmlparser::XMLProfileManager::loadXMLFile("log_inactive.xml");

    Log::SetVerbosity(Log::Info);

    vector<unique_ptr<thread>> threads;
    for (int i = 0; i != 5; i++)
    {
        threads.emplace_back(new thread([i]{
            logError(Multithread, "You should not view this message: " << i);
        }));
    }

    for (auto& thread: threads) {
        thread->join();
    }
}

TEST(LogFileTests, file_and_default)
{
    // First remove previous executions file
    std::remove("output.log");

    xmlparser::XMLProfileManager::loadXMLFile("log_def_file.xml");

    Log::SetVerbosity(Log::Info);

    vector<unique_ptr<thread>> threads;
    for (int i = 0; i != 5; i++)
    {
        threads.emplace_back(new thread([i]{
            logWarning(Multithread, "I'm thread " << i);
        }));
    }

    for (auto& thread: threads) {
        thread->join();
    }

    //Log::ClearConsumers(); // Force close file
    Log::KillThread();

    std::ifstream ifs("output.log");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    for (int i = 0; i != 5; ++i)
    {
        std::string str("I'm thread " + i);
        std::size_t found = content.find(str);
        ASSERT_TRUE(found != std::string::npos);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
