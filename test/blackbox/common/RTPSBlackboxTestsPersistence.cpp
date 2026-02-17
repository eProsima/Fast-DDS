// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#if HAVE_SQLITE3

#include <cstring>
#include <thread>

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
};

class Persistence : public ::testing::TestWithParam<communication_type>
{
public:

    const std::string& db_file_name() const
    {
        return db_file_name_;
    }

    const GuidPrefix_t& guid_prefix() const
    {
        return guid_prefix_;
    }

    std::list<HelloWorld> not_received_data;

    void run_one_send_recv_test(
            RTPSWithRegistrationReader<HelloWorldPubSubType>& reader,
            RTPSWithRegistrationWriter<HelloWorldPubSubType>& writer,
            uint32_t seq_check = 0,
            bool reliable = false)
    {
        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        std::cout << "Discovery finished." << std::endl;

        auto data = default_helloworld_data_generator();
        size_t n_samples = data.size();
        not_received_data.insert(not_received_data.end(), data.begin(), data.end());

        reader.expected_data(not_received_data);
        reader.startReception();

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());

        // Block reader until reception finished or timeout.
        if (seq_check > 0)
        {
            std::cout << "Reader waiting for sequence " << seq_check << "." << std::endl;
            reader.block_until_seq_number_greater_or_equal({ 0, seq_check });
        }
        else
        {
            if (reliable)
            {
                std::cout << "Reader waiting for " << n_samples << " samples." << std::endl;
                reader.block_for_all();
            }
            else
            {
                std::cout << "Reader waiting for 2 samples." << std::endl;
                reader.block_for_at_least(2);
            }
        }

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

    std::string db_file_name_;
    GuidPrefix_t guid_prefix_;

    virtual void SetUp()
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(att);
                break;
            case TRANSPORT:
            default:
                break;
        }

        // Get info about current test
        auto info = ::testing::UnitTest::GetInstance()->current_test_info();

        // Create DB file name from test name and PID
        std::ostringstream ss;
        std::string test_case_name(info->test_case_name());
        std::string test_name(info->name());
        ss <<
            test_case_name.replace(test_case_name.find_first_of('/'), 1, "_") << "_" <<
            test_name.replace(test_name.find_first_of('/'), 1, "_")  << "_" << GET_PID() << ".db";
        db_file_name_ = ss.str();

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
        std::remove(db_file_name_.c_str());
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(att);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

TEST_P(Persistence, RTPSAsNonReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_transient(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_transient(db_file_name(), guid_prefix()).reliability(ReliabilityKind_t::BEST_EFFORT).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, false);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 13, false);

    std::cout << "Second round finished." << std::endl;
}

TEST_P(Persistence, AsyncRTPSAsNonReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_transient(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_transient(db_file_name(), guid_prefix()).reliability(ReliabilityKind_t::BEST_EFFORT).
            asynchronously(RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, false);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 13, false);

    std::cout << "Second round finished." << std::endl;
}

TEST_P(Persistence, RTPSAsReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_transient(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).
            reliability(ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_transient(db_file_name(), guid_prefix()).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, true);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 20, true);

    std::cout << "Second round finished." << std::endl;
}

TEST_P(Persistence, AsyncRTPSAsReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_transient(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).
            reliability(ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_transient(db_file_name(), guid_prefix()).history_depth(10).
            asynchronously(RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, true);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 20, true);

    std::cout << "Second round finished." << std::endl;
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(Persistence,
        Persistence,
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<Persistence::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });
#endif // if HAVE_SQLITE3
