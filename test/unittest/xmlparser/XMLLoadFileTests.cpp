// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima
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


#include <thread>
#include <mutex>
#include <fstream>

#include <gtest/gtest.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include "../logging/mock/MockConsumer.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif // ifdef _WIN32

using namespace eprosima::fastrtps;
using namespace ::xmlparser;
using namespace ::testing;

class XMLLoadFileTests : public ::testing::Test
{
public:

    void helper_block_for_at_least_entries(
            uint32_t amount)
    {
        std::unique_lock<std::mutex> lck(*xml_mutex_);
        mock_consumer_->cv().wait(lck, [this, amount]
                {
                    return mock_consumer_->ConsumedEntriesSize_nts() >= amount;
                });
    }

protected:

    void SetUp() override
    {
        xml_mutex_ = new std::mutex();
    }

    void TearDown() override
    {
        delete xml_mutex_;
        xml_mutex_ = nullptr;
    }

    MockConsumer* mock_consumer_;

private:

    mutable std::mutex* xml_mutex_;
};

/*
 * This test checks that the default XML file is loaded only once when there is a DEFAULT_FASTRTPS_PROFILES.xml file
 * in the current directory and the environment variable FASTRTPS_DEFAULT_PROFILES_FILE is set pointing to the same
 * file.
 * 1. Initialize Mock Consumer to consume the LogInfo entry that the library generates when the file has been already
 * parsed. Set filters to consume only the desired entry.
 * 2. Get current path to set the environment variable to the DEFAULT_FASTRTPS_PROFILES.xml file.
 * 3. Write the DEFAULT_FASTRTPS_PROFILES.xml file in the current directory.
 * 4. Load the default XML file.
 * 5. Wait for the log entry to be consumed.
 */
TEST_F(XMLLoadFileTests, load_twice_default_xml)
{
    // Initialize Mock Consumer
    mock_consumer_ = new MockConsumer();

    Log::RegisterConsumer(std::unique_ptr<eprosima::fastrtps::LogConsumer>(mock_consumer_));
    Log::SetVerbosity(Log::Info);
    Log::SetCategoryFilter(std::regex("(XMLPARSER)"));
    Log::SetErrorStringFilter(std::regex("(already parsed)"));

    // Current directory
#ifdef _WIN32
    char current_directory[MAX_PATH];
    uint32_t ret = GetCurrentDirectory(MAX_PATH, current_directory);
    ASSERT_NE(ret, 0u);
    strcat_s(current_directory, MAX_PATH, "\\");
    strcat_s(current_directory, MAX_PATH, DEFAULT_FASTRTPS_PROFILES);
    // Set environment variable
    _putenv_s("FASTRTPS_DEFAULT_PROFILES_FILE", current_directory);
#else
    char* current_directory = nullptr;
    current_directory = getcwd(current_directory, PATH_MAX);
    ASSERT_NE(current_directory, nullptr);
    strcat(current_directory, "/");
    strcat(current_directory, DEFAULT_FASTRTPS_PROFILES);
    // Set environment variable
    setenv("FASTRTPS_DEFAULT_PROFILES_FILE", current_directory, 1);
#endif // _WIN32

    // Write DEFAULT_FASTRTPS_PROFILES.xml
    std::ofstream xmlFile;
    xmlFile.open("DEFAULT_FASTRTPS_PROFILES.xml");
    xmlFile << "<dds xmlns=\"http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles\">";
    xmlFile << "<profiles><participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">";
    xmlFile << "<rtps><useBuiltinTransports>true</useBuiltinTransports><name>test_name</name></rtps></participant>";
    xmlFile << "</profiles></dds>";
    xmlFile.close();

    // Load default XML file
    xmlparser::XMLProfileManager::loadDefaultXMLFile();

    // Log consumer
    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 1u);

    Log::Reset();
    Log::KillThread();

    std::remove("DEFAULT_FASTRTPS_PROFILES.xml");
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
