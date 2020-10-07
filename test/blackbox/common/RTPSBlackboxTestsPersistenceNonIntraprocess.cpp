// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "BlackboxTests.hpp"

#if HAVE_SQLITE3

#include <cstring>
#include <thread>

#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"
#include "../../unittest/logging/mock/MockConsumer.h"

#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

class PersistenceNonIntraprocess : public ::testing::Test
{
public:

    const std::string& db_file_name_reader() const
    {
        return db_file_name_reader_;
    }

    const std::string& db_file_name_writer() const
    {
        return db_file_name_writer_;
    }

    const GuidPrefix_t& guid_prefix() const
    {
        return guid_prefix_;
    }

    std::vector<Log::Entry> helper_wait_for_entries(uint32_t amount)
    {
        size_t entries = 0;
        for (uint32_t i = 0; i != async_tries_; i++)
        {
            entries = mock_consumer_->ConsumedEntries().size();
            if (entries == amount)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(async_waits_ms_));
        }
        return mock_consumer_->ConsumedEntries();
    }

    void run_partial_send_recv_test(
            RTPSWithRegistrationReader<HelloWorldType>& reader,
            RTPSWithRegistrationWriter<HelloWorldType>& writer)
    {
        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        std::cout << "Discovery finished." << std::endl;

        auto data = default_helloworld_data_generator();
        auto data_aux = default_helloworld_data_generator();
        size_t n_samples = data.size();
        not_received_data.insert(not_received_data.end(), data.begin(), data.end());

        reader.expected_data(not_received_data);
        reader.startReception();

        // Send data
        writer.send(data, 5);
        // In this test not all data should be sent.
        EXPECT_FALSE(data.empty());
        std::cout << "First batch of data sent." << std::endl;

        writer.destroy();

        // Remove writer's database file
        EXPECT_EQ(std::remove(db_file_name_writer().c_str()), 0);

        // Wait for undiscovery
        reader.wait_undiscovery();

        writer.make_persistent(db_file_name_writer(), guid_prefix()).
                reliability(ReliabilityKind_t::RELIABLE).init();

        // Wait for discovery
        writer.wait_discovery();
        reader.wait_discovery();

        std::cout << "Rediscovery finished." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        writer.send(data_aux);
        EXPECT_TRUE(data_aux.empty());

        std::cout << "Reader waiting for " << n_samples << " samples." << std::endl;
        reader.block_for_all();

        std::cout << "Last received sequence was " << reader.get_last_received_sequence_number() << std::endl;

        std::cout << "Destroying reader..." << std::endl;
        reader.destroy();
        std::cout << "Destroying writer..." << std::endl;
        writer.destroy();

        data = reader.not_received_data();
        print_non_received_messages(data, default_helloworld_print);
        not_received_data = data;
    }

protected:

    std::list<HelloWorld> not_received_data;
    MockConsumer* mock_consumer_;
    const uint32_t async_tries_ = 5;
    const uint32_t async_waits_ms_ = 25;

    std::string db_file_name_writer_;
    std::string db_file_name_reader_;
    GuidPrefix_t guid_prefix_;

    virtual void SetUp()
    {
        // Get info about current test
        auto info = ::testing::UnitTest::GetInstance()->current_test_info();

        // Create DB file name from test name and PID
        std::ostringstream ss_writer;
        std::ostringstream ss_reader;
        std::string test_case_name(info->test_case_name());
        std::string test_name(info->name());

        ss_writer << test_case_name << "_" << test_name << "_" << GET_PID() << "_writer.db";
        ss_reader << test_case_name << "_" << test_name << "_" << GET_PID() << "_reader.db";
        db_file_name_writer_ = ss_writer.str();
        db_file_name_reader_ = ss_reader.str();

        // Fill guid prefix
        const int32_t info_line = info->line();
        memcpy(guid_prefix_.value, &info_line, sizeof(info_line));
        const int32_t pid = GET_PID();
        memcpy(guid_prefix_.value + 4, &pid, sizeof(pid));
        guid_prefix_.value[8] = HAVE_SECURITY;
        guid_prefix_.value[9] = 3; // PREALLOCATED_MEMORY_MODE
        LocatorList_t loc;
        IPFinder::getIP4Address(&loc);
        if (loc.size() > 0)
        {
            guid_prefix_.value[10] = loc.begin()->address[14];
            guid_prefix_.value[11] = loc.begin()->address[15];
        }
        else
        {
            guid_prefix_.value[10] = 127;
            guid_prefix_.value[11] = 1;
        }
    }

    virtual void TearDown()
    {
        std::remove(db_file_name_reader_.c_str());
        std::remove(db_file_name_writer_.c_str());
    }
};

TEST_F(PersistenceNonIntraprocess, InconsistentAcknackReceived)
{
    mock_consumer_ = new MockConsumer();

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mock_consumer_));
    Log::SetVerbosity(Log::Warning);

    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_persistent(db_file_name_reader(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).
            reliability(ReliabilityKind_t::RELIABLE).init();
    EXPECT_TRUE(reader.isInitialized());

    writer.make_persistent(db_file_name_writer(), guid_prefix()).
            reliability(ReliabilityKind_t::RELIABLE).init();

    EXPECT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_partial_send_recv_test(reader, writer);

    auto consumed_entries = helper_wait_for_entries(2);
    // Expect at least 2 log messages: LogError (PersistentStatefulWriter) and LogWarning (StatefulWriter)
    EXPECT_GE(2u, consumed_entries.size());

    std::cout << "Round finished." << std::endl;

    Log::Reset();
    Log::KillThread();
}

#endif // HAVE_SQLITE3
