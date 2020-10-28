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

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class PersistenceLargeData : public testing::TestWithParam<bool>
{
public:

    const std::string& db_file_name() const
    {
        return db_file_name_;
    }

protected:

    std::string db_file_name_;

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
            xmlparser::XMLProfileManager::library_settings(library_settings);
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

    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
            xmlparser::XMLProfileManager::library_settings(library_settings);
        }
        std::remove(db_file_name_.c_str());
    }

    void fragment_data(
            bool large_data)
    {
        PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);
        PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->sendBufferSize = 32768;
        testTransport->maxMessageSize = 32768;
        testTransport->receiveBufferSize = 32768;

        writer
                .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .make_persistent(db_file_name(), "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64")
                .disable_builtin_transport()
                .add_user_transport_to_pparams(testTransport)
                .init();

        ASSERT_TRUE(writer.isInitialized());

        auto data = default_data16kb_data_generator();
        if (large_data)
        {
            data = default_data300kb_data_generator();
        }
        auto unreceived_data = data;

        // Send data
        writer.send(data);
        // All data should be sent
        ASSERT_TRUE(data.empty());
        // Destroy the DataWriter
        writer.destroy();
        // Load the persistent DataWriter with the changes saved in the database
        writer.init();

        ASSERT_TRUE(writer.isInitialized());

        reader
                .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
                .history_depth(10)
                .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
                .socket_buffer_size(1048576)
                .init();

        ASSERT_TRUE(reader.isInitialized());

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        reader.startReception(unreceived_data);

        // Block reader until reception finished or timeout.
        reader.block_for_all();
    }

};

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentWithFrag)
{
    fragment_data(true);
}

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentNoFrag)
{
    fragment_data(false);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w, )
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PersistenceLargeData,
        PersistenceLargeData,
        testing::Values(false, true),
        [](const testing::TestParamInfo<PersistenceLargeData::ParamType>& info)
        {
            if (info.param)
            {
                return "Intraprocess";
            }
            return "NonIntraprocess";
        });
