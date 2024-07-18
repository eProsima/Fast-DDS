// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <gtest/gtest.h>

#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

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
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
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
        WITH_ERROR
    }
    iteration = NONE;
    eprosima::fastdds::rtps::GUID_t writer_guid;
    std::vector<octet> user_data;

    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.set_on_writer_discovery(
        [&mutex, &cv, &iteration, &writer_guid, &user_data](
            WriterDiscoveryStatus reason,
            const GUID_t& w_guid,
            const PublicationBuiltinTopicData* w_data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            writer_guid = w_guid;
            if (nullptr != w_data)
            {
                user_data = w_data->user_data;
            }
            if (Iterations::NONE == iteration && WriterDiscoveryStatus::DISCOVERED_WRITER == reason)
            {
                iteration = Iterations::DISCOVERED_WRITER;
            }
            else if (Iterations::DISCOVERED_WRITER == iteration &&
            WriterDiscoveryStatus::CHANGED_QOS_WRITER == reason)
            {
                iteration = Iterations::CHANGED_QOS_WRITER;
            }
            else if (Iterations::CHANGED_QOS_WRITER == iteration &&
            WriterDiscoveryStatus::REMOVED_WRITER == reason)
            {
                iteration = Iterations::REMOVED_WRITER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(reader.isInitialized());

    // Test first iteration: expect WriterDiscoveryStatus::DISCOVERED_WRITER.
    writer.user_data({0, 1, 2, 3}).init();
    ASSERT_TRUE(writer.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_WRITER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
        ASSERT_EQ(std::vector<octet>({0, 1, 2, 3}), user_data);
    }

    // Test second iteration: expect WriterDiscoveryStatus::CHANGED_QOS_WRITER.
    writer.user_data({2, 3, 4, 5, 6}).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::CHANGED_QOS_WRITER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::CHANGED_QOS_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
        ASSERT_EQ(std::vector<octet>({2, 3, 4, 5, 6}), user_data);
    }

    // Test second iteration: expect WriterDiscoveryStatus::REMOVED_WRITER.
    GUID_t w_guid = writer.guid();
    writer.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_WRITER == iteration || Iterations::WITH_ERROR == iteration;
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
        WITH_ERROR
    }
    iteration = NONE;
    eprosima::fastdds::rtps::GUID_t reader_guid;
    std::vector<octet> user_data;

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration, &reader_guid, &user_data](
            ReaderDiscoveryStatus reason,
            const GUID_t& r_guid,
            const SubscriptionBuiltinTopicData* r_data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            reader_guid = r_guid;
            if (nullptr != r_data)
            {
                user_data = r_data->user_data;
            }
            if (Iterations::NONE == iteration && ReaderDiscoveryStatus::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else if (Iterations::DISCOVERED_READER == iteration && ReaderDiscoveryStatus::CHANGED_QOS_READER == reason)
            {
                iteration = Iterations::CHANGED_QOS_READER;
            }
            else if (Iterations::CHANGED_QOS_READER == iteration && ReaderDiscoveryStatus::REMOVED_READER == reason)
            {
                iteration = Iterations::REMOVED_READER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    // Test first iteration: expect ReaderDiscoveryStatus::DISCOVERED_READER.
    reader.user_data({0, 1, 2, 3}).init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
        ASSERT_EQ(std::vector<octet>({0, 1, 2, 3}), user_data);
    }

    // Test second iteration: expect ReaderDiscoveryStatus::CHANGED_QOS_READER.
    reader.user_data({2, 3, 4, 5, 6}).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::CHANGED_QOS_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::CHANGED_QOS_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
        ASSERT_EQ(std::vector<octet>({2, 3, 4, 5, 6}), user_data);
    }

    // Test second iteration: expect ReaderDiscoveryStatus::REMOVED_READER.
    GUID_t r_guid = reader.guid();
    reader.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_READER == iteration || Iterations::WITH_ERROR == iteration;
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
        WITH_ERROR
    }
    iteration = NONE;
    eprosima::fastdds::rtps::GUID_t writer_guid;

    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.set_on_writer_discovery(
        [&mutex, &cv, &iteration, &writer_guid](
            WriterDiscoveryStatus reason,
            const GUID_t& w_guid,
            const PublicationBuiltinTopicData*)
        {
            std::unique_lock<std::mutex> lock(mutex);
            writer_guid = w_guid;
            if (Iterations::NONE == iteration && WriterDiscoveryStatus::DISCOVERED_WRITER == reason)
            {
                iteration = Iterations::DISCOVERED_WRITER;
            }
            else if (Iterations::DISCOVERED_WRITER == iteration &&
            WriterDiscoveryStatus::REMOVED_WRITER == reason)
            {
                iteration = Iterations::REMOVED_WRITER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(reader.isInitialized());

    // Test first iteration: expect WriterDiscoveryStatus::DISCOVERED_WRITER.
    writer.init();
    ASSERT_TRUE(writer.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_WRITER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
    }

    // Test second iteration: expect WriterDiscoveryStatus::CHANGED_QOS_WRITER.
    std::vector<std::string> partitions({"A"});
    writer.partitions(partitions).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_WRITER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_WRITER, iteration);
        ASSERT_EQ(writer.guid(), writer_guid);
    }

    // Test second iteration: expect WriterDiscoveryStatus::REMOVED_WRITER.
    writer.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
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
        WITH_ERROR
    }
    iteration = NONE;
    eprosima::fastdds::rtps::GUID_t reader_guid;

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration, &reader_guid](
            ReaderDiscoveryStatus reason,
            const GUID_t& w_guid,
            const SubscriptionBuiltinTopicData*)
        {
            std::unique_lock<std::mutex> lock(mutex);
            reader_guid = w_guid;
            if (Iterations::NONE == iteration && ReaderDiscoveryStatus::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else if (Iterations::DISCOVERED_READER == iteration && ReaderDiscoveryStatus::REMOVED_READER == reason)
            {
                iteration = Iterations::REMOVED_READER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    // Test first iteration: expect ReaderDiscoveryStatus::DISCOVERED_READER.
    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
    }

    // Test second iteration: expect ReaderDiscoveryStatus::CHANGED_QOS_READER.
    std::vector<std::string> partitions({"A"});
    reader.partitions(partitions).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::REMOVED_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_READER, iteration);
        ASSERT_EQ(reader.guid(), reader_guid);
    }

    // Test second iteration: expect ReaderDiscoveryStatus::REMOVED_READER.
    reader.destroy();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::REMOVED_READER, iteration);
    }
}

/*!
 * \test RTPS-CFT-RRR-01 Tests a good `ContentFilterProperty` passed to `register_reader()` and `update_reader()` is
 * propagated successfully through discovery.
 */
TEST_P(RTPSDiscovery, ContentFilterRegistration)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_READER,
        CHANGED_QOS_READER,
        WITH_ERROR
    }
    iteration = NONE;
    eprosima::fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    eprosima::fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration, &content_filter_property](
            ReaderDiscoveryStatus reason,
            const GUID_t&,
            const SubscriptionBuiltinTopicData* r_data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (nullptr != r_data)
            {
                content_filter_property = r_data->content_filter;
            }
            if (Iterations::NONE == iteration && ReaderDiscoveryStatus::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else if (Iterations::DISCOVERED_READER == iteration && ReaderDiscoveryStatus::CHANGED_QOS_READER == reason)
            {
                iteration = Iterations::CHANGED_QOS_READER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    eprosima::fastdds::rtps::ContentFilterProperty cfp(content_filter_allocation);

    // Test first iteration: expect ReaderDiscoveryStatus::DISCOVERED_READER.
    cfp.content_filtered_topic_name = "CFP_TEST";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "This is my custom expression";
    reader.content_filter_property(cfp).init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
        ASSERT_EQ(cfp.content_filtered_topic_name, content_filter_property.content_filtered_topic_name);
        ASSERT_EQ(cfp.related_topic_name, content_filter_property.related_topic_name);
        ASSERT_EQ(cfp.filter_class_name, content_filter_property.filter_class_name);
        ASSERT_EQ(cfp.filter_expression, content_filter_property.filter_expression);
        ASSERT_EQ(cfp.expression_parameters.size(), content_filter_property.expression_parameters.size());
    }

    // Test second iteration: expect ReaderDiscoveryStatus::CHANGED_QOS_READER.
    cfp.filter_expression = "New custom expression";
    cfp.expression_parameters.push_back("100");
    cfp.expression_parameters.push_back("200");
    reader.content_filter_property(cfp).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::CHANGED_QOS_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::CHANGED_QOS_READER, iteration);
        ASSERT_EQ(cfp.content_filtered_topic_name, content_filter_property.content_filtered_topic_name);
        ASSERT_EQ(cfp.related_topic_name, content_filter_property.related_topic_name);
        ASSERT_EQ(cfp.filter_class_name, content_filter_property.filter_class_name);
        ASSERT_EQ(cfp.filter_expression, content_filter_property.filter_expression);
        ASSERT_EQ(cfp.expression_parameters.size(), content_filter_property.expression_parameters.size());
        ASSERT_EQ(cfp.expression_parameters[0], content_filter_property.expression_parameters[0]);
        ASSERT_EQ(cfp.expression_parameters[1], content_filter_property.expression_parameters[1]);
    }
}

/*!
 * \test RTPS-CFT-RRR-02 Tests a wrong `ContentFilterProperty` passed to `register_reader()` makes the function fail.
 */
TEST_P(RTPSDiscovery, ContentFilterWrongRegistration)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        WITH_ERROR
    }
    iteration = NONE;

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration](
            ReaderDiscoveryStatus,
            const GUID_t&,
            const SubscriptionBuiltinTopicData*)
        {
            std::unique_lock<std::mutex> lock(mutex);
            iteration = Iterations::WITH_ERROR;
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    eprosima::fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    eprosima::fastdds::rtps::ContentFilterProperty cfp(content_filter_allocation);

    // wrong content_filtered_topic_name
    cfp.content_filtered_topic_name = "";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "This is my custom expression";
    reader.content_filter_property(cfp).init();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::NONE, iteration);
    }
    reader.destroy();

    // wrong related_topic_name
    cfp.content_filtered_topic_name = "CFP_TEST";
    cfp.related_topic_name = "";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "This is my custom expression";
    reader.content_filter_property(cfp).init();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::NONE, iteration);
    }
    reader.destroy();

    // wrong filter_class_name
    cfp.content_filtered_topic_name = "CFT_TEST";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "";
    cfp.filter_expression = "This is my custom expression";
    reader.content_filter_property(cfp).init();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::NONE, iteration);
    }
    reader.destroy();

    // wrong filter_expression
    cfp.content_filtered_topic_name = "CFT_TEST";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "";
    reader.content_filter_property(cfp).init();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::NONE, iteration);
    }
    reader.destroy();
}

/*!
 * \test RTPS-CFT-RRR-03 Tests a wrong `ContentFilterProperty` passed to `update_reader()` makes the function fail.
 */
TEST_P(RTPSDiscovery, ContentFilterWrongUpdate)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_READER,
        WITH_ERROR
    }
    iteration = NONE;

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration](
            ReaderDiscoveryStatus reason,
            const GUID_t&,
            const SubscriptionBuiltinTopicData*)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (Iterations::NONE == iteration && ReaderDiscoveryStatus::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
    }

    eprosima::fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    eprosima::fastdds::rtps::ContentFilterProperty cfp(content_filter_allocation);

    // wrong content_filtered_topic_name
    cfp.content_filtered_topic_name = "";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "This is my custom expression";
    reader.content_filter_property(cfp).update();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
    }

    // wrong related_topic_name
    cfp.content_filtered_topic_name = "CFP_TEST";
    cfp.related_topic_name = "";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "This is my custom expression";
    reader.content_filter_property(cfp).update();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
    }

    // wrong filter_class_name
    cfp.content_filtered_topic_name = "CFT_TEST";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "";
    cfp.filter_expression = "This is my custom expression";
    reader.content_filter_property(cfp).update();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
    }

    // wrong filter_expression
    cfp.content_filtered_topic_name = "CFT_TEST";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "";
    reader.content_filter_property(cfp).update();
    ASSERT_FALSE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&iteration]()
                {
                    return Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
    }
}

/*!
 * \test RTPS-CFT-RRR-04 Tests `register_reader()` and `update_reader()` works successfully when the pointer to
 * `ContentFilterProperty` is `nullptr`.
 */
TEST_P(RTPSDiscovery, ContentFilterRegistrationWithoutCFP)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_READER,
        CHANGED_QOS_READER,
        WITH_ERROR
    }
    iteration = NONE;
    eprosima::fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    eprosima::fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration, &content_filter_property](
            ReaderDiscoveryStatus reason,
            const GUID_t&,
            const SubscriptionBuiltinTopicData* r_data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (nullptr != r_data)
            {
                content_filter_property = r_data->content_filter;
            }
            if (Iterations::NONE == iteration && ReaderDiscoveryStatus::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else if (Iterations::DISCOVERED_READER == iteration && ReaderDiscoveryStatus::CHANGED_QOS_READER == reason)
            {
                iteration = Iterations::CHANGED_QOS_READER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
        ASSERT_EQ(0u, content_filter_property.content_filtered_topic_name.size());
        ASSERT_EQ(0u, content_filter_property.related_topic_name.size());
        ASSERT_EQ(0u, content_filter_property.filter_class_name.size());
        ASSERT_EQ(0u, content_filter_property.filter_expression.size());
        ASSERT_EQ(0u, content_filter_property.expression_parameters.size());
    }

    // Test second iteration: expect ReaderDiscoveryStatus::CHANGED_QOS_READER.
    reader.update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::CHANGED_QOS_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::CHANGED_QOS_READER, iteration);
        ASSERT_EQ(0u, content_filter_property.content_filtered_topic_name.size());
        ASSERT_EQ(0u, content_filter_property.related_topic_name.size());
        ASSERT_EQ(0u, content_filter_property.filter_class_name.size());
        ASSERT_EQ(0u, content_filter_property.filter_expression.size());
        ASSERT_EQ(0u, content_filter_property.expression_parameters.size());
    }
}

/*!
 * \test RTPS-CFT-RRR_05 Tests ``update_reader()` works successfully when a `ContentFilterProperty` is added for first
 * time, because `register_reader()` passed a `nullptr`.
 */
TEST_P(RTPSDiscovery, ContentFilterRegistrationWithoutCFPButUpdate)
{
    std::mutex mutex;
    std::condition_variable cv;
    enum Iterations
    {
        NONE,
        DISCOVERED_READER,
        CHANGED_QOS_READER,
        WITH_ERROR
    }
    iteration = NONE;
    eprosima::fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    eprosima::fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.set_on_reader_discovery(
        [&mutex, &cv, &iteration, &content_filter_property](
            ReaderDiscoveryStatus reason,
            const GUID_t&,
            const SubscriptionBuiltinTopicData* r_data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (nullptr != r_data)
            {
                content_filter_property = r_data->content_filter;
            }
            if (Iterations::NONE == iteration && ReaderDiscoveryStatus::DISCOVERED_READER == reason)
            {
                iteration = Iterations::DISCOVERED_READER;
            }
            else if (Iterations::DISCOVERED_READER == iteration && ReaderDiscoveryStatus::CHANGED_QOS_READER == reason)
            {
                iteration = Iterations::CHANGED_QOS_READER;
            }
            else
            {
                iteration = Iterations::WITH_ERROR;
            }
            cv.notify_one();
        }
        ).init();
    ASSERT_TRUE(writer.isInitialized());

    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::DISCOVERED_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::DISCOVERED_READER, iteration);
        ASSERT_EQ(0, content_filter_property.content_filtered_topic_name.size());
        ASSERT_EQ(0, content_filter_property.related_topic_name.size());
        ASSERT_EQ(0, content_filter_property.filter_class_name.size());
        ASSERT_EQ(0, content_filter_property.filter_expression.size());
        ASSERT_EQ(0, content_filter_property.expression_parameters.size());
    }

    // Test second iteration: expect ReaderDiscoveryStatus::CHANGED_QOS_READER.
    eprosima::fastdds::rtps::ContentFilterProperty cfp(content_filter_allocation);
    cfp.content_filtered_topic_name = "CFP_TEST";
    cfp.related_topic_name = "TEST";
    cfp.filter_class_name = "MyFilterClass";
    cfp.filter_expression = "This is my custom expression";
    cfp.expression_parameters.push_back("100");
    cfp.expression_parameters.push_back("200");
    reader.content_filter_property(cfp).update();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&iteration]()
                {
                    return Iterations::CHANGED_QOS_READER == iteration || Iterations::WITH_ERROR == iteration;
                });
        ASSERT_EQ(Iterations::CHANGED_QOS_READER, iteration);
        ASSERT_EQ(cfp.content_filtered_topic_name, content_filter_property.content_filtered_topic_name);
        ASSERT_EQ(cfp.related_topic_name, content_filter_property.related_topic_name);
        ASSERT_EQ(cfp.filter_class_name, content_filter_property.filter_class_name);
        ASSERT_EQ(cfp.filter_expression, content_filter_property.filter_expression);
        ASSERT_EQ(cfp.expression_parameters.size(), content_filter_property.expression_parameters.size());
        ASSERT_EQ(cfp.expression_parameters[0], content_filter_property.expression_parameters[0]);
        ASSERT_EQ(cfp.expression_parameters[1], content_filter_property.expression_parameters[1]);
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
