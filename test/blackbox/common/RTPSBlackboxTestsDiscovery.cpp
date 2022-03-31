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

#include "BlackboxTests.hpp"

#include <gtest/gtest.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
};

class RTPSDiscovery : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

/*!
 * \test RTPS-CFT-EDC-01 Tests the callbacks `ReaderListener::on_writer_discovery()` is called successfully in
 * several iterations.
 */
TEST_P(RTPSDiscovery, ReaderListenerOnWriterDiscovery)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_WRITER,
        CHANGED_QOS_WRITER,
        REMOVED_WRITER,
        ERROR
    }
    iteration = NONE;
    eprosima::fastrtps::rtps::GUID_t writer_guid;
    std::vector<octet> user_data;

    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.set_on_writer_discovery(
        [&mutex, &cv, &iteration, &writer_guid, &user_data](
            WriterDiscoveryInfo::DISCOVERY_STATUS reason,
            const GUID_t& w_guid,
            const WriterProxyData* w_data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            writer_guid = w_guid;
            if (nullptr != w_data)
            {
                user_data = w_data->m_qos.m_userData;
            }
            if (Iterations::NONE == iteration && WriterDiscoveryInfo::DISCOVERY_STATUS::DISCOVERED_WRITER == reason)
            {
                iteration = Iterations::DISCOVERED_WRITER;
            }
            else if (Iterations::DISCOVERED_WRITER == iteration &&
            WriterDiscoveryInfo::DISCOVERY_STATUS::CHANGED_QOS_WRITER == reason)
            {
                iteration = Iterations::CHANGED_QOS_WRITER;
            }
            else if (Iterations::CHANGED_QOS_WRITER == iteration &&
            WriterDiscoveryInfo::DISCOVERY_STATUS::REMOVED_WRITER == reason)
            {
                iteration = Iterations::REMOVED_WRITER;
            }
            else
            {
                iteration = Iterations::ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(reader.isInitialized());

    // Test first iteration: expect WriterDiscoveryInfo::DISCOVERED_WRITER.
    writer.user_data({0, 1, 2, 3}).init();
    ASSERT_TRUE(writer.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_WRITER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
        ASSERT_EQ(std::vector<octet>({0, 1, 2, 3}), user_data);
    }

    // Test second iteration: expect WriterDiscoveryInfo::CHANGED_QOS_WRITER.
    writer.user_data({2, 3, 4, 5, 6}).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::CHANGED_QOS_WRITER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::CHANGED_QOS_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
        ASSERT_EQ(std::vector<octet>({2, 3, 4, 5, 6}), user_data);
    }

    // Test second iteration: expect WriterDiscoveryInfo::REMOVED_WRITER.
    GUID_t w_guid = writer.guid();
    writer.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_WRITER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_WRITER, iteration);
        ASSERT_EQ(writer_guid, w_guid);
    }
}

/*!
 * \test RTPS-CFT-EDC-02 Tests the callbacks `WriterListener::on_reader_discovery()` is called successfully in
 * several iterations.
 */
TEST_P(RTPSDiscovery, WriterListenerOnReaderDiscovery)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_READER,
        CHANGED_QOS_READER,
        REMOVED_READER,
        ERROR
    }
    iteration = NONE;
    eprosima::fastrtps::rtps::GUID_t reader_guid;
    std::vector<octet> user_data;

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration, &reader_guid, &user_data](
            ReaderDiscoveryInfo::DISCOVERY_STATUS reason,
            const GUID_t& r_guid,
            const ReaderProxyData* r_data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            reader_guid = r_guid;
            if (nullptr != r_data)
            {
                user_data = r_data->m_qos.m_userData;
            }
            if (Iterations::NONE == iteration && ReaderDiscoveryInfo::DISCOVERY_STATUS::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else if (Iterations::DISCOVERED_READER == iteration &&
            ReaderDiscoveryInfo::DISCOVERY_STATUS::CHANGED_QOS_READER == reason)
            {
                iteration = Iterations::CHANGED_QOS_READER;
            }
            else if (Iterations::CHANGED_QOS_READER == iteration &&
            ReaderDiscoveryInfo::DISCOVERY_STATUS::REMOVED_READER == reason)
            {
                iteration = Iterations::REMOVED_READER;
            }
            else
            {
                iteration = Iterations::ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    // Test first iteration: expect ReaderDiscoveryInfo::DISCOVERED_READER.
    reader.user_data({0, 1, 2, 3}).init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
        ASSERT_EQ(std::vector<octet>({0, 1, 2, 3}), user_data);
    }

    // Test second iteration: expect ReaderDiscoveryInfo::CHANGED_QOS_READER.
    reader.user_data({2, 3, 4, 5, 6}).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::CHANGED_QOS_READER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::CHANGED_QOS_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
        ASSERT_EQ(std::vector<octet>({2, 3, 4, 5, 6}), user_data);
    }

    // Test second iteration: expect ReaderDiscoveryInfo::REMOVED_READER.
    GUID_t r_guid = reader.guid();
    reader.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_READER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_READER, iteration);
        ASSERT_EQ(reader_guid, r_guid);
    }
}

/*!
 * \test RTPS-CFT-EDC-03 Tests the callbacks `ReaderListener::on_writer_discovery()` is called successfully in
 * several iterations.
 */
TEST_P(RTPSDiscovery, ReaderListenerOnWriterDiscoveryIncompatibleQoS)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_WRITER,
        REMOVED_WRITER,
        ERROR
    }
    iteration = NONE;
    eprosima::fastrtps::rtps::GUID_t writer_guid;

    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.set_on_writer_discovery(
        [&mutex, &cv, &iteration, &writer_guid](
            WriterDiscoveryInfo::DISCOVERY_STATUS reason,
            const GUID_t& w_guid,
            const WriterProxyData*)
        {
            std::unique_lock<std::mutex> lock(mutex);
            writer_guid = w_guid;
            if (Iterations::NONE == iteration && WriterDiscoveryInfo::DISCOVERY_STATUS::DISCOVERED_WRITER == reason)
            {
                iteration = Iterations::DISCOVERED_WRITER;
            }
            else if (Iterations::DISCOVERED_WRITER == iteration &&
            WriterDiscoveryInfo::DISCOVERY_STATUS::REMOVED_WRITER == reason)
            {
                iteration = Iterations::REMOVED_WRITER;
            }
            else
            {
                iteration = Iterations::ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(reader.isInitialized());

    // Test first iteration: expect WriterDiscoveryInfo::DISCOVERED_WRITER.
    writer.init();
    ASSERT_TRUE(writer.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_WRITER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
    }

    // Test second iteration: expect WriterDiscoveryInfo::CHANGED_QOS_WRITER.
    std::vector<std::string> partitions({"A"});
    writer.partitions(partitions).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_WRITER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
    }

    // Test second iteration: expect WriterDiscoveryInfo::REMOVED_WRITER.
    writer.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::seconds(1), [&iteration]()
                {
                    return Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_WRITER, iteration);
    }
}

/*!
 * \test RTPS-CFT-EDC-04 Tests the callbacks `WriterListener::on_reader_discovery()` is called successfully in
 * several iterations.
 */
TEST_P(RTPSDiscovery, WriterListenerOnReaderDiscoveryIncompatibleQoS)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_READER,
        REMOVED_READER,
        ERROR
    }
    iteration = NONE;
    eprosima::fastrtps::rtps::GUID_t reader_guid;

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration, &reader_guid](
            ReaderDiscoveryInfo::DISCOVERY_STATUS reason,
            const GUID_t& w_guid,
            const ReaderProxyData*)
        {
            std::unique_lock<std::mutex> lock(mutex);
            reader_guid = w_guid;
            if (Iterations::NONE == iteration && ReaderDiscoveryInfo::DISCOVERY_STATUS::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else if (Iterations::DISCOVERED_READER == iteration &&
            ReaderDiscoveryInfo::DISCOVERY_STATUS::REMOVED_READER == reason)
            {
                iteration = Iterations::REMOVED_READER;
            }
            else
            {
                iteration = Iterations::ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    // Test first iteration: expect ReaderDiscoveryInfo::DISCOVERED_READER.
    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
    }

    // Test second iteration: expect ReaderDiscoveryInfo::CHANGED_QOS_READER.
    std::vector<std::string> partitions({"A"});
    reader.partitions(partitions).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_READER == iteration || Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
    }

    // Test second iteration: expect ReaderDiscoveryInfo::REMOVED_READER.
    reader.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::seconds(1), [&iteration]()
                {
                    return Iterations::ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_READER, iteration);
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(RTPS,
        RTPSDiscovery,
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<RTPSDiscovery::ParamType>& info)
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
