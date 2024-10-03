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
#include "mock/BlackboxMockConsumer.h"

#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

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

    void run_partial_send_recv_test(
            RTPSWithRegistrationReader<HelloWorldPubSubType>& reader,
            RTPSWithRegistrationWriter<HelloWorldPubSubType>& writer)
    {
        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        std::cout << "Discovery finished." << std::endl;

        auto partial_data = default_helloworld_data_generator(5);
        auto complete_data = default_helloworld_data_generator();
        size_t n_samples = complete_data.size();
        not_received_data.insert(not_received_data.end(), complete_data.begin(), complete_data.end());

        reader.expected_data(not_received_data);
        reader.startReception();

        // Send data
        writer.send(partial_data);
        EXPECT_TRUE(partial_data.empty());
        std::cout << "First batch of data sent." << std::endl;

        writer.destroy();

        // Remove writer's database file
        EXPECT_EQ(std::remove(db_file_name_writer().c_str()), 0);

        // Wait for undiscovery
        reader.wait_undiscovery();

        writer.make_transient(db_file_name_writer(), guid_prefix())
                .reliability(ReliabilityKind_t::RELIABLE).init();

        // Wait for discovery
        writer.wait_discovery();
        reader.wait_discovery();

        std::cout << "Rediscovery finished." << std::endl;

        auto consumed_entries = helper_block_for_at_least_entries(2);

        writer.send(complete_data);
        EXPECT_TRUE(complete_data.empty());

        std::cout << "Reader waiting for " << n_samples << " samples." << std::endl;
        reader.block_for_all();

        std::cout << "Last received sequence was " << reader.get_last_received_sequence_number() << std::endl;

        std::cout << "Destroying reader..." << std::endl;
        reader.destroy();
        std::cout << "Destroying writer..." << std::endl;
        writer.destroy();

        complete_data = reader.not_received_data();
        print_non_received_messages(complete_data, default_helloworld_print);
        not_received_data = complete_data;

        // Expect at least 2 log messages: LogError (PersistentStatefulWriter) and LogWarning (StatefulWriter)
        EXPECT_GE(consumed_entries.size(), 2u);
        // Expect only 1 log error.
        uint32_t num_errors = 0;
        for (const auto& entry : consumed_entries)
        {
            if (entry.kind == Log::Kind::Error)
            {
                num_errors++;
            }
        }
        EXPECT_EQ(num_errors, 1u);
    }

protected:

    std::list<HelloWorld> not_received_data;
    BlackboxMockConsumer* mock_consumer_;
    const uint32_t async_tries_ = 5;
    const uint32_t async_waits_ms_ = 25;

    std::string db_file_name_writer_;
    std::string db_file_name_reader_;
    GuidPrefix_t guid_prefix_;

    std::vector<Log::Entry> helper_block_for_at_least_entries(
            uint32_t amount)
    {
        mock_consumer_->wait(amount);
        return mock_consumer_->ConsumedEntries();
    }

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
        guid_prefix_.value[9] = 3;
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
    mock_consumer_ = new BlackboxMockConsumer();

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mock_consumer_));
    Log::SetVerbosity(Log::Warning);
    Log::SetCategoryFilter(std::regex("(RTPS_WRITER)"));
    Log::SetErrorStringFilter(std::regex("(Inconsistent acknack)"));

    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_transient(db_file_name_reader(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).
            reliability(ReliabilityKind_t::RELIABLE).init();
    EXPECT_TRUE(reader.isInitialized());

    writer.make_transient(db_file_name_writer(), guid_prefix()).
            reliability(ReliabilityKind_t::RELIABLE).init();

    EXPECT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_partial_send_recv_test(reader, writer);

    std::cout << "Round finished." << std::endl;

    Log::Reset();
    Log::KillThread();
}

#endif // HAVE_SQLITE3
