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

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <forward_list>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>
#include <type_traits>

#include <asio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fastdds/dds/common/InstanceHandle.hpp"
#include "fastdds/dds/core/policy/QosPolicies.hpp"
#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/LoanableArray.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/StackAllocatedSequence.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include "../../common/CustomPayloadPool.hpp"
#include "../../logging/mock/MockConsumer.h"
#include "FooBoundedType.hpp"
#include "FooBoundedTypeSupport.hpp"
#include "FooType.hpp"
#include "FooTypeSupport.hpp"

#if defined(__cplusplus_winrt)
#define GET_PID GetCurrentProcessId
#elif defined(_WIN32)
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

static constexpr LoanableCollection::size_type num_test_elements = 10;

FASTDDS_SEQUENCE(FooSeq, FooType);
FASTDDS_SEQUENCE(FooBoundedSeq, FooBoundedType);
using FooArray = LoanableArray<FooType, num_test_elements>;
using FooStack = StackAllocatedSequence<FooType, num_test_elements>;
using SampleInfoArray = LoanableArray<SampleInfo, num_test_elements>;

class DataReaderTests : public ::testing::Test
{
public:

    void SetUp() override
    {
        type_.reset(new FooTypeSupport());

        std::ostringstream topic_name_s;
        topic_name_s << "footopic" << "_" << asio::ip::host_name() << "_" << GET_PID();
        topic_name = topic_name_s.str();
    }

    void TearDown() override
    {
        if (!destroy_entities_)
        {
            return;
        }

        if (data_writer_)
        {
            ASSERT_EQ(publisher_->delete_datawriter(data_writer_), RETCODE_OK);
        }
        if (data_reader_)
        {
            ASSERT_EQ(subscriber_->delete_datareader(data_reader_), RETCODE_OK);
        }
        if (topic_)
        {
            ASSERT_EQ(participant_->delete_topic(topic_), RETCODE_OK);
        }
        if (publisher_)
        {
            ASSERT_EQ(participant_->delete_publisher(publisher_), RETCODE_OK);
        }
        if (subscriber_)
        {
            ASSERT_EQ(participant_->delete_subscriber(subscriber_), RETCODE_OK);
        }
        if (participant_)
        {
            auto factory = DomainParticipantFactory::get_instance();
            ASSERT_EQ(factory->delete_participant(participant_), RETCODE_OK);
        }
    }

protected:

    void create_entities(
            DataReaderListener* rlistener = nullptr,
            const DataReaderQos& rqos = DATAREADER_QOS_DEFAULT,
            const SubscriberQos& sqos = SUBSCRIBER_QOS_DEFAULT,
            const DataWriterQos& wqos = DATAWRITER_QOS_DEFAULT,
            const PublisherQos& pqos = PUBLISHER_QOS_DEFAULT,
            const TopicQos& tqos = TOPIC_QOS_DEFAULT,
            const DomainParticipantQos& part_qos = PARTICIPANT_QOS_DEFAULT)
    {
        participant_ =
                DomainParticipantFactory::get_instance()->create_participant(
            (uint32_t)GET_PID() % 230, part_qos);
        ASSERT_NE(participant_, nullptr);

        subscriber_ = participant_->create_subscriber(sqos);
        ASSERT_NE(subscriber_, nullptr);

        publisher_ = participant_->create_publisher(pqos);
        ASSERT_NE(publisher_, nullptr);

        type_.register_type(participant_);

        topic_ = participant_->create_topic(topic_name, type_.get_type_name(), tqos);
        ASSERT_NE(topic_, nullptr);

        data_reader_ = subscriber_->create_datareader(topic_, rqos, rlistener);
        ASSERT_NE(data_reader_, nullptr);

        data_writer_ = publisher_->create_datawriter(topic_, wqos);
        ASSERT_NE(data_writer_, nullptr);
    }

    void create_instance_handles()
    {
        FooType data;

        data.index(0);
        type_.compute_key(&data, handle_ok_);

        data.index(2);
        type_.compute_key(&data, handle_wrong_);
    }

    void reset_lengths_if_ok(
            const ReturnCode_t& code,
            LoanableCollection& data_values,
            SampleInfoSeq& infos)
    {
        if (RETCODE_OK == code)
        {
            data_values.length(0);
            infos.length(0);
        }
    }

    void send_data(
            LoanableCollection& data_values,
            SampleInfoSeq& infos)
    {
        LoanableCollection::size_type n = infos.length();
        auto buffer = data_values.buffer();
        for (LoanableCollection::size_type i = 0; i < n; ++i)
        {
            if (infos[i].valid_data)
            {
                EXPECT_EQ(RETCODE_OK, data_writer_->write(buffer[i], HANDLE_NIL));
            }
        }
    }

    /**
     * @brief Calls `return_loan` on a DataReader and checks the result.
     *
     * This method is designed to be used inside `check_instance_methods` and `basic_read_apis_check`, but may be
     * used on future tests if desired.
     *
     * @param code         Return code that was expected on the `read/take` call.
     *                     The return value to expect from `return_loan` will be calculated from this one as follows:
     *                     - NOT_ENABLED => NOT_ENABLED (calling `return_loan` on a not enabled reader).
     *                     - OK => OK (successfully returning a loan).
     *                     - Any other => RETCODE_PRECONDITION_NOT_MET (trying to return non-empty collections which
     *                       the reader did not loan).
     * @param data_reader  The reader on which to return the loan.
     * @param data_values  The data collection to return.
     * @param infos        The SampleInfo collection to return.
     * @param seq_max      The value to expect as `maximum` on the collections after return_loan returns OK.
     */
    void check_return_loan(
            const ReturnCode_t& code,
            DataReader* data_reader,
            LoanableCollection& data_values,
            SampleInfoSeq& infos,
            int32_t seq_max)
    {
        ReturnCode_t expected_return_loan_ret = RETCODE_PRECONDITION_NOT_MET;
        if (RETCODE_OK == code || RETCODE_NOT_ENABLED == code)
        {
            expected_return_loan_ret = code;
        }
        EXPECT_EQ(expected_return_loan_ret, data_reader->return_loan(data_values, infos));

        if (RETCODE_OK == expected_return_loan_ret)
        {
            EXPECT_TRUE(data_values.has_ownership());
            EXPECT_EQ(seq_max, data_values.maximum());
            EXPECT_TRUE(infos.has_ownership());
            EXPECT_EQ(seq_max, infos.maximum());
        }
    }

    /**
     * @brief Test calls to `read_instance` and `take_instance` with a valid instance handle
     *
     * @param handle            Handle of instance to read
     * @param instance_ok_code  Expected result of calls to `read/take_instance`
     * @param loan_return_code  Expected result of calls to `return_loan`
     * @param data_reader       DataReader on which to perform calls
     * @param data_values       The data collection to use
     * @param infos             The sample_info collection to use
     * @param max_samples       The value to pass as `max_samples` on calls to `read/take_instance`
     */
    void check_correct_instance_methods(
            const InstanceHandle_t& handle,
            const ReturnCode_t& instance_ok_code,
            const ReturnCode_t& loan_return_code,
            DataReader* data_reader,
            LoanableCollection& data_values,
            SampleInfoSeq& infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            int32_t seq_max = 0)
    {
        EXPECT_EQ(instance_ok_code, data_reader->read_instance(data_values, infos, max_samples, handle));
        check_return_loan(loan_return_code, data_reader, data_values, infos, seq_max);
        reset_lengths_if_ok(instance_ok_code, data_values, infos);

        EXPECT_EQ(instance_ok_code, data_reader->take_instance(data_values, infos, max_samples, handle));
        if (RETCODE_OK == instance_ok_code)
        {
            // Write received data so it can be taken again
            send_data(data_values, infos);
        }
        check_return_loan(loan_return_code, data_reader, data_values, infos, seq_max);
        reset_lengths_if_ok(instance_ok_code, data_values, infos);
    }

    /**
     * @brief Test calls to `read_instance` and `take_instance` with an invalid instance handle
     *
     * @param handle            Handle of instance to read
     * @param instance_bad_code Expected result of calls to `read/take_instance`
     * @param wrong_loan_code   Expected result of calls to `return_loan`
     * @param data_reader       DataReader on which to perform calls
     * @param data_values       The data collection to use
     * @param infos             The sample_info collection to use
     * @param max_samples       The value to pass as `max_samples` on calls to `read/take_instance`
     */
    void check_wrong_instance_methods(
            const InstanceHandle_t& handle,
            const ReturnCode_t& instance_bad_code,
            const ReturnCode_t& wrong_loan_code,
            DataReader* data_reader,
            LoanableCollection& data_values,
            SampleInfoSeq& infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            int32_t seq_max = 0)
    {
        EXPECT_EQ(instance_bad_code, data_reader->read_instance(data_values, infos, max_samples, handle));
        check_return_loan(wrong_loan_code, data_reader, data_values, infos, seq_max);
        EXPECT_EQ(instance_bad_code, data_reader->take_instance(data_values, infos, max_samples, handle));
        check_return_loan(wrong_loan_code, data_reader, data_values, infos, seq_max);
    }

    /**
     * @brief Test calls to `read_instance` and `take_instance`
     *
     * @param instance_ok_code    Expected result of calls to `read/take_instance` for valid instance handles
     * @param instance_bad_code   Expected result of calls to `read/take_instance` for invalid instance handles
     * @param loan_return_code    Expected result of calls to `return_loan` for valid instance handles
     * @param data_reader         DataReader on which to perform calls
     * @param data_values         The data collection to use
     * @param infos               The sample_info collection to use
     * @param max_samples         The value to pass as `max_samples` on calls to `read/take_instance`
     * @param two_valid_instances Whether `handle_wrong_` is considered a valid instance
     * @param seq_max             The value to expect as `maximum` on the collections after return_loan returns OK.
     */
    void check_instance_methods(
            const ReturnCode_t& instance_ok_code,
            const ReturnCode_t& instance_bad_code,
            const ReturnCode_t& loan_return_code,
            DataReader* data_reader,
            LoanableCollection& data_values,
            SampleInfoSeq& infos,
            int32_t max_samples = LENGTH_UNLIMITED,
            bool two_valid_instances = false,
            int32_t seq_max = 0)
    {
        // Calc expected result of `return_loan` for calls with a wrong instance handle.
        ReturnCode_t wrong_loan_code = RETCODE_PRECONDITION_NOT_MET;
        if (RETCODE_NOT_ENABLED == instance_bad_code)
        {
            wrong_loan_code = instance_bad_code;
        }
        else if (RETCODE_OK == loan_return_code)
        {
            wrong_loan_code = RETCODE_OK;
        }

        // Trying to get data for HANDLE_NIL should always use instance_bad_code.
        check_wrong_instance_methods(HANDLE_NIL, instance_bad_code, wrong_loan_code,
                data_reader, data_values, infos, max_samples, seq_max);

        // Trying to get data for handle_wrong_ depends on `two_instances`
        if (two_valid_instances)
        {
            check_correct_instance_methods(handle_wrong_, instance_ok_code, loan_return_code,
                    data_reader, data_values, infos, max_samples, seq_max);
        }
        else
        {
            check_wrong_instance_methods(handle_wrong_, instance_bad_code, wrong_loan_code,
                    data_reader, data_values, infos, max_samples, seq_max);
        }

        // Trying to get data for handle_ok_ should always use instance_ok_code
        check_correct_instance_methods(handle_ok_, instance_ok_code, loan_return_code,
                data_reader, data_values, infos, max_samples, seq_max);
    }

    /**
     * @brief This test checks all variants of read / take on a specific state of the reader.
     *
     * This method is designed to be used inside `read_take_apis_test`, and may require changes if used on new tests.
     *
     * The APIs tested are:
     * - read_next_sample / take_next_sample
     * - read / take
     * - read_next_instance / take_next_instance
     * - read_instance / take_instance
     * - return_loan
     *
     * @param code                 Expected return from read/take_xxx APIs
     * @param data_reader          DataReader on which to perform the test
     * @param two_valid_instances  Whether `handle_wrong_` is considered a valid instance
     *
     * @see check_instance_methods to see how read_instance / take_instance are tested.
     */
    template<typename DataType, typename DataSeq>
    void basic_read_apis_check(
            const ReturnCode_t& code,
            DataReader* data_reader,
            bool two_valid_instances = false)
    {
        static const Duration_t time_to_wait(1, 0);

        // Check read_next_sample / take_next_sample
        {
            DataType data;
            SampleInfo info;

            EXPECT_EQ(code, data_reader->take_next_sample(&data, &info));
            if (RETCODE_OK == code)
            {
                // Send taken sample so it can be read again
                data_writer_->write(&data);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            EXPECT_EQ(code, data_reader->read_next_sample(&data, &info));
        }

        // Return code when requesting a bad instance
        ReturnCode_t instance_bad_code = RETCODE_BAD_PARAMETER;
        if (RETCODE_NOT_ENABLED == code)
        {
            instance_bad_code = code;
        }

        // Return code when requesting a correct instance
        ReturnCode_t instance_ok_code = instance_bad_code;
        if (RETCODE_OK == code && type_->is_compute_key_provided)
        {
            instance_ok_code = code;
        }

        // Check read/take and variants with loan
        {
            DataSeq data_values;
            SampleInfoSeq infos;

            ReturnCode_t expected_return_loan_ret = code;
            if (RETCODE_NO_DATA == code)
            {
                // Even when read returns data, no loan will be performed
                expected_return_loan_ret = RETCODE_OK;
            }

            EXPECT_EQ(code, data_reader->read(data_values, infos));
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, 0);
            reset_lengths_if_ok(code, data_values, infos);
            EXPECT_EQ(code, data_reader->read_next_instance(data_values, infos));
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, 0);
            reset_lengths_if_ok(code, data_values, infos);

            EXPECT_EQ(code, data_reader->take(data_values, infos));
            if (RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, 0);
            reset_lengths_if_ok(code, data_values, infos);

            EXPECT_EQ(code, data_reader->take_next_instance(data_values, infos));
            if (RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, 0);
            reset_lengths_if_ok(code, data_values, infos);

            check_instance_methods(instance_ok_code, instance_bad_code, expected_return_loan_ret,
                    data_reader, data_values, infos, LENGTH_UNLIMITED, two_valid_instances, 0);
        }

        // Check read/take and variants without loan
        {
            DataSeq data_values(1);
            SampleInfoSeq infos(1);

            ReturnCode_t expected_return_loan_ret = code;
            if (RETCODE_NO_DATA == code)
            {
                expected_return_loan_ret = RETCODE_OK;
            }

            EXPECT_EQ(code, data_reader->read(data_values, infos));
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, data_values.maximum());
            reset_lengths_if_ok(code, data_values, infos);
            EXPECT_EQ(code, data_reader->read_next_instance(data_values, infos));
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, data_values.maximum());
            reset_lengths_if_ok(code, data_values, infos);

            EXPECT_EQ(code, data_reader->take(data_values, infos));
            if (RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, data_values.maximum());
            reset_lengths_if_ok(code, data_values, infos);

            EXPECT_EQ(code, data_reader->take_next_instance(data_values, infos));
            if (RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos, data_values.maximum());
            reset_lengths_if_ok(code, data_values, infos);

            check_instance_methods(instance_ok_code, instance_bad_code, expected_return_loan_ret,
                    data_reader, data_values, infos, LENGTH_UNLIMITED, two_valid_instances, data_values.maximum());
        }
    }

    /*
     * This test checks all variants of read / take in several situations.
     *
     * The APIs tested are:
     * - read_next_sample / take_next_sample
     * - read / take
     * - read_next_instance / take_next_instance
     * - read_instance / take_instance
     * - return_loan
     *
     * The test checks that:
     * - Calling the APIs on a disabled reader return NOT_ENABLED
     * - Calling the APIs on an enabled reader with no data return NO_DATA
     * - Calling the xxx_instance APIs with a wrong instance handle return BAD_PARAMETER
     * - Calling the APIs when data has been received return OK
     *
     * Checks are done both with and without loans. A call to return_loan is always performed, and its return value is
     * checked to be OK when a loan was performed and PRECONDITION_NOT_MET when not.
     *
     * @see basic_read_apis_check for how the checks are done on each reader state.
     */
    template<typename DataType, typename DataSeq>
    void read_take_apis_test()
    {
        create_instance_handles();

        // We need depth = 2 for the disposed instance test
        DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
        reader_qos.history().depth = 2;

        // We will create a disabled DataReader, so we can check RETCODE_NOT_ENABLED
        SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
        subscriber_qos.entity_factory().autoenable_created_entities = false;

        create_entities(nullptr, reader_qos, subscriber_qos);
        EXPECT_FALSE(data_reader_->is_enabled());
        EXPECT_EQ(0ull, data_reader_->get_unread_count());

        // Read / take operations should all return NOT_ENABLED
        basic_read_apis_check<DataType, DataSeq>(RETCODE_NOT_ENABLED, data_reader_);

        // Enable the DataReader and check NO_DATA should be returned
        EXPECT_EQ(RETCODE_OK, data_reader_->enable());
        EXPECT_TRUE(data_reader_->is_enabled());
        basic_read_apis_check<DataType, DataSeq>(RETCODE_NO_DATA, data_reader_);

        // Send data
        DataType data;
        data.index(0);
        EXPECT_EQ(RETCODE_OK, data_writer_->write(&data, HANDLE_NIL));

        // Wait for data to arrive and check OK should be returned
        Duration_t wait_time(1, 0);
        EXPECT_TRUE(data_reader_->wait_for_unread_message(wait_time));
        basic_read_apis_check<DataType, DataSeq>(RETCODE_OK, data_reader_);

        // Check with data on second instance
        data.index(2u);
        EXPECT_EQ(RETCODE_OK, data_writer_->write(&data, HANDLE_NIL));
        basic_read_apis_check<DataType, DataSeq>(RETCODE_OK, data_reader_, true);

        // Check with disposed instance
        if (type_->is_compute_key_provided)
        {
            EXPECT_EQ(RETCODE_OK, data_writer_->dispose(&data, handle_wrong_));
            basic_read_apis_check<DataType, DataSeq>(RETCODE_OK, data_reader_, true);
        }
    }

    DomainParticipant* participant_ = nullptr;
    Subscriber* subscriber_ = nullptr;
    Publisher* publisher_ = nullptr;
    Topic* topic_ = nullptr;
    DataReader* data_reader_ = nullptr;
    DataWriter* data_writer_ = nullptr;
    TypeSupport type_;
    bool destroy_entities_ = true;

    std::string topic_name;

    InstanceHandle_t handle_ok_ = HANDLE_NIL;
    InstanceHandle_t handle_wrong_ = HANDLE_NIL;

};

/*!
 * This test checks `DataReader::get_guid` function works when the entity was created but not enabled.
 */
TEST_F(DataReaderTests, get_guid)
{
    class DiscoveryListener : public DomainParticipantListener
    {
    public:

        void on_data_reader_discovery(
                DomainParticipant*,
                ReaderDiscoveryStatus reason,
                const SubscriptionBuiltinTopicData& info,
                bool& /*should_be_ignored*/) override
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (ReaderDiscoveryStatus::DISCOVERED_READER == reason)
            {
                guid = info.guid;
                cv.notify_one();
            }
        }

        GUID_t guid;
        std::mutex mutex;
        std::condition_variable cv;
    }
    discovery_listener;

    DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
    participant_qos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
            static_cast<ParticipantFilteringFlags>(
        ParticipantFilteringFlags::FILTER_DIFFERENT_HOST |
        ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS);

    DomainParticipant* listener_participant =
            DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230, participant_qos,
        &discovery_listener,
        StatusMask::none());

    DomainParticipantFactoryQos factory_qos;
    DomainParticipantFactory::get_instance()->get_qos(factory_qos);
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230, participant_qos);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic(topic_name, type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(datareader, nullptr);

    GUID_t guid = datareader->guid();

    participant->enable();

    factory_qos.entity_factory().autoenable_created_entities = true;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    {
        std::unique_lock<std::mutex> lock(discovery_listener.mutex);
        discovery_listener.cv.wait(lock, [&]()
                {
                    return GUID_t::unknown() != discovery_listener.guid;
                });
    }
    ASSERT_EQ(guid, discovery_listener.guid);

    ASSERT_TRUE(subscriber->delete_datareader(datareader) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_subscriber(subscriber) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(
                listener_participant) == RETCODE_OK);
}

TEST_F(DataReaderTests, InvalidQos)
{
    DataReaderQos qos;

    create_entities();

    ASSERT_TRUE(data_reader_->is_enabled());
    ASSERT_EQ(RETCODE_OK, data_reader_->get_qos(qos));
    ASSERT_EQ(qos, DATAREADER_QOS_DEFAULT);

    /* Unsupported QoS */
    const ReturnCode_t unsupported_code = RETCODE_UNSUPPORTED;

    qos = DATAREADER_QOS_DEFAULT;
    qos.destination_order().kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    EXPECT_EQ(unsupported_code, data_reader_->set_qos(qos));

    /* Inconsistent QoS */
    const ReturnCode_t inconsistent_code = RETCODE_INCONSISTENT_POLICY;

    qos = DATAREADER_QOS_DEFAULT;
    qos.reader_resource_limits().max_samples_per_read = -1;
    EXPECT_EQ(inconsistent_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    Locator_t locator;
    qos.endpoint().unicast_locator_list.push_back(locator);
    qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
    EXPECT_EQ(inconsistent_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.endpoint().multicast_locator_list.push_back(locator);
    qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
    EXPECT_EQ(inconsistent_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.endpoint().remote_locator_list.push_back(locator);
    qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
    EXPECT_EQ(inconsistent_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 0;
    EXPECT_EQ(inconsistent_code, data_reader_->set_qos(qos)); // KEEP LAST 0 is inconsistent
    // KEEP LAST 2000 but max_samples_per_instance default (400) is inconsistent but right now it only shows a warning
    // In the reader, this returns RETCODE_INMUTABLE_POLICY, because the depth cannot be changed on run time.
    // Because of the implementation, we know de consistency is checked before the inmutability, so by checking the
    // return against RETCODE_INMUTABLE_POLICY we are testing that the setting are not considered inconsistent yet.
    // This test will fail whenever we enforce the consistency between depth and max_samples_per_instance.
    qos.history().depth = 2000;
    EXPECT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader_->set_qos(qos));

    /* Inmutable QoS */
    const ReturnCode_t inmutable_code = RETCODE_IMMUTABLE_POLICY;

    qos = DATAREADER_QOS_DEFAULT;
    qos.durability().kind = PERSISTENT_DURABILITY_QOS;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.resource_limits().max_samples = 5000;
    qos.resource_limits().max_instances = 2;
    qos.resource_limits().max_samples_per_instance = 100;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.history().kind = KEEP_ALL_HISTORY_QOS;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.history().depth++;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.liveliness().kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.liveliness().lease_duration.seconds = -131;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.liveliness().announcement_period.seconds = -131;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.reader_resource_limits().matched_publisher_allocation.initial++;
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.data_sharing().off();
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    uint16_t datasharing_domain = 131u;
    qos.data_sharing().add_domain_id(datasharing_domain);
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));

    qos = DATAREADER_QOS_DEFAULT;
    qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
    EXPECT_EQ(inmutable_code, data_reader_->set_qos(qos));
}

TEST_F(DataReaderTests, PersistentDurabilityIsAValidQoS)
{
    DataReaderQos qos;
    qos = DATAREADER_QOS_DEFAULT;
    qos.durability().kind = PERSISTENT_DURABILITY_QOS;

    create_entities(
        nullptr,
        qos
        );

    // PERSISTENT DataReader behaves as TRANSIENT
    EXPECT_NE(nullptr, data_reader_);
}

/**
 * This test checks all variants of read / take in several situations for a keyed plain type.
 */
TEST_F(DataReaderTests, read_take_apis)
{
    read_take_apis_test<FooType, FooSeq>();
}

/**
 * This test checks all variants of read / take in several situations for a non-keyed not-plain type.
 */
TEST_F(DataReaderTests, read_take_apis_not_plain)
{
    type_.reset(new FooBoundedTypeSupport());

    read_take_apis_test<FooBoundedType, FooBoundedSeq>();
}

void check_collection(
        const LoanableCollection& collection,
        bool owns,
        LoanableCollection::size_type max,
        LoanableCollection::size_type len)
{
    EXPECT_EQ(owns, collection.has_ownership());
    EXPECT_LE(max, collection.maximum());
    EXPECT_EQ(len, collection.length());
    EXPECT_LE(len, collection.maximum());
}

/*
 * This test checks the preconditions on the data_values and sample_infos arguments to the read / take APIs
 *
 * The APIs tested are:
 * - read / take
 * - read_next_instance / take_next_instance
 * - read_instance / take_instance
 * - return_loan
 *
 * All possible combinations of owns, max_len, and len are used as input on an enabled reader with no data.
 * For collections where the preconditions are met, NO_DATA will be returned, except for read_instance and
 * take_instance, where no instance would exist and the return will be BAD_PARAMETER. Otherwise, PRECONDITION_NOT_MET
 * should be returned.
 *
 * As no data will ever be returned, return_loan will always return OK.
 */
TEST_F(DataReaderTests, collection_preconditions)
{
    create_entities();
    create_instance_handles();

    const ReturnCode_t& no_data_code = RETCODE_NO_DATA;
    const ReturnCode_t& wrong_code = RETCODE_PRECONDITION_NOT_MET;
    const ReturnCode_t& return_loan_code = RETCODE_OK;

    // Helper buffers to create loaned sequences
    FooArray arr;
    SampleInfoArray info_arr;

    // This variables are named following the convention owns_len_max
    FooSeq true_0_0;
    FooSeq true_10_0(num_test_elements);
    FooStack true_10_1;
    FooSeq false_10_0;
    FooSeq false_10_1;
    SampleInfoSeq info_true_0_0;
    SampleInfoSeq info_true_10_0(num_test_elements);
    SampleInfoSeq info_true_10_1(num_test_elements);
    SampleInfoSeq info_false_10_0;
    SampleInfoSeq info_false_10_1;

    // Make the sequences have the corresponding values
    true_10_1.length(1);
    false_10_0.loan(arr.buffer_for_loans(), num_test_elements, 0);
    false_10_1.loan(arr.buffer_for_loans(), num_test_elements, 1);
    info_true_10_1.length(1);
    info_false_10_0.loan(info_arr.buffer_for_loans(), num_test_elements, 0);
    info_false_10_1.loan(info_arr.buffer_for_loans(), num_test_elements, 1);

    // Check we did it right
    check_collection(true_0_0, true, 0, 0);
    check_collection(true_10_0, true, 10, 0);
    check_collection(true_10_1, true, 10, 1);
    check_collection(false_10_0, false, 10, 0);
    check_collection(false_10_1, false, 10, 1);
    check_collection(info_true_0_0, true, 0, 0);
    check_collection(info_true_10_0, true, 10, 0);
    check_collection(info_true_10_1, true, 10, 1);
    check_collection(info_false_10_0, false, 10, 0);
    check_collection(info_false_10_1, false, 10, 1);

    // Check all wrong combinations
    using test_case_t = std::pair<LoanableCollection&, SampleInfoSeq&>;
    std::vector<test_case_t> wrong_cases
    {
        // true_0_0
        {true_0_0, info_true_10_0},
        {true_0_0, info_true_10_1},
        {true_0_0, info_false_10_0},
        {true_0_0, info_false_10_1},
        // true_10_0
        {true_10_0, info_true_0_0},
        {true_10_0, info_true_10_1},
        {true_10_0, info_false_10_0},
        {true_10_0, info_false_10_1},
        // true_10_1
        {true_10_1, info_true_10_0},
        {true_10_1, info_true_0_0},
        {true_10_1, info_false_10_0},
        {true_10_1, info_false_10_1},
        // false_10_0
        {false_10_0, info_true_10_0},
        {false_10_0, info_true_10_1},
        {false_10_0, info_true_0_0},
        {false_10_0, info_false_10_1},
        // false_10_1
        {false_10_1, info_true_10_0},
        {false_10_1, info_true_10_1},
        {false_10_1, info_false_10_0},
        {false_10_1, info_true_0_0},
    };

    for (const test_case_t& test : wrong_cases)
    {
        EXPECT_EQ(wrong_code, data_reader_->read(test.first, test.second));
        EXPECT_EQ(wrong_code, data_reader_->read_next_instance(test.first, test.second));
        EXPECT_EQ(wrong_code, data_reader_->take(test.first, test.second));
        EXPECT_EQ(wrong_code, data_reader_->take_next_instance(test.first, test.second));

        check_instance_methods(wrong_code, wrong_code, wrong_code,
                data_reader_, test.first, test.second);
    }

    // Check compatible combinations
    using ok_test_case_t = std::pair<test_case_t, std::pair<const ReturnCode_t&, const ReturnCode_t&>>;
    std::vector<ok_test_case_t> ok_cases
    {
        // max == 0. Loaned data will be returned.
        { {true_0_0, info_true_0_0}, {no_data_code, return_loan_code}},
        // max > 0 && owns == true. Data will be copied.
        { {true_10_0, info_true_10_0}, {no_data_code, return_loan_code}},
        { {true_10_1, info_true_10_1}, {no_data_code, return_loan_code}},
        // max > 0 && owns == false. Precondition not met.
        { {false_10_0, info_false_10_0}, {wrong_code, wrong_code}},
        { {false_10_1, info_false_10_1}, {wrong_code, wrong_code}}
    };

    const ReturnCode_t& instance_bad_code = RETCODE_BAD_PARAMETER;
    for (const ok_test_case_t& test : ok_cases)
    {
        EXPECT_EQ(test.second.first, data_reader_->read(test.first.first, test.first.second));
        EXPECT_EQ(test.second.second, data_reader_->return_loan(test.first.first, test.first.second));
        EXPECT_EQ(test.second.first, data_reader_->read_next_instance(test.first.first, test.first.second));
        EXPECT_EQ(test.second.second, data_reader_->return_loan(test.first.first, test.first.second));
        EXPECT_EQ(test.second.first, data_reader_->take(test.first.first, test.first.second));
        EXPECT_EQ(test.second.second, data_reader_->return_loan(test.first.first, test.first.second));
        EXPECT_EQ(test.second.first, data_reader_->take_next_instance(test.first.first, test.first.second));
        EXPECT_EQ(test.second.second, data_reader_->return_loan(test.first.first, test.first.second));

        // When collection preconditions are ok, as the reader has no data, BAD_PARAMETER will be returned
        const ReturnCode_t& instance_code = (test.second.first == no_data_code) ? instance_bad_code : test.second.first;
        check_instance_methods(instance_code, instance_code, test.second.second,
                data_reader_, test.first.first, test.first.second, LENGTH_UNLIMITED, false, test.first.first.maximum());
    }

    // Check for  max_samples > max_len
    {
        EXPECT_EQ(wrong_code, data_reader_->read(true_10_0, info_true_10_0, 20));
        EXPECT_EQ(wrong_code, data_reader_->read_next_instance(true_10_0, info_true_10_0, 20));
        EXPECT_EQ(wrong_code, data_reader_->take(true_10_0, info_true_10_0, 20));
        EXPECT_EQ(wrong_code, data_reader_->take_next_instance(true_10_0, info_true_10_0, 20));

        check_instance_methods(wrong_code, wrong_code, return_loan_code,
                data_reader_, true_10_0, info_true_10_0, 20, false, true_10_0.maximum());

    }

    false_10_0.unloan();
    false_10_1.unloan();
    info_false_10_0.unloan();
    info_false_10_1.unloan();
}

void forward_loan(
        LoanableCollection& to,
        const LoanableCollection& from)
{
    assert("Cannot forward loan if there is no loan" && (from.has_ownership() == false));
    auto buf = const_cast<LoanableCollection::element_type*>(from.buffer());
    to.loan(buf, from.maximum(), from.length());
}

/*
 * This test checks the preconditions on the data_values and sample_infos arguments to the return_loan API.
 */
TEST_F(DataReaderTests, return_loan)
{
    static constexpr int32_t num_samples = 10;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    writer_qos.history().depth = num_samples;
    writer_qos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    subscriber_qos.entity_factory().autoenable_created_entities = false;

    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    reader_qos.resource_limits().max_instances = 2;
    reader_qos.resource_limits().max_samples_per_instance = num_samples;
    reader_qos.resource_limits().max_samples = num_samples * 2;

    create_instance_handles();
    create_entities(nullptr, reader_qos, subscriber_qos, writer_qos);
    DataReader* reader2 = subscriber_->create_datareader(topic_, reader_qos);

    FooSeq data_values;
    SampleInfoSeq infos;
    FooSeq data_values_2;
    SampleInfoSeq infos_2;

    const ReturnCode_t& ok_code = RETCODE_OK;
    const ReturnCode_t& precondition_code = RETCODE_PRECONDITION_NOT_MET;

    // Calling return loan on disabled reader should return NOT_ENABLED
    EXPECT_EQ(RETCODE_NOT_ENABLED, data_reader_->return_loan(data_values, infos));

    // Enable both readers
    EXPECT_EQ(ok_code, data_reader_->enable());
    EXPECT_EQ(ok_code, reader2->enable());

    // Calling return loan with empty sequences on an enabled reader should return OK
    EXPECT_EQ(RETCODE_OK, data_reader_->return_loan(data_values, infos));

    FooType data;
    data.index(0);

    // Send a bunch of samples
    for (int32_t i = 0; i < num_samples; ++i)
    {
        EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
    }

    // Read with loan from both readers
    EXPECT_EQ(ok_code, data_reader_->read(data_values, infos));
    EXPECT_EQ(ok_code, reader2->read(data_values_2, infos_2));
    check_collection(data_values, false, num_samples, num_samples);
    check_collection(infos, false, num_samples, num_samples);
    check_collection(data_values_2, false, num_samples, num_samples);
    check_collection(infos_2, false, num_samples, num_samples);

    // Mixing collections on return_loan should fail and keep collections state
    EXPECT_EQ(precondition_code, data_reader_->return_loan(data_values, infos_2));
    check_collection(data_values, false, num_samples, num_samples);
    check_collection(infos_2, false, num_samples, num_samples);

    EXPECT_EQ(precondition_code, data_reader_->return_loan(data_values_2, infos));
    check_collection(infos, false, num_samples, num_samples);
    check_collection(data_values_2, false, num_samples, num_samples);

    EXPECT_EQ(precondition_code, reader2->return_loan(data_values, infos_2));
    check_collection(data_values, false, num_samples, num_samples);
    check_collection(infos_2, false, num_samples, num_samples);

    EXPECT_EQ(precondition_code, reader2->return_loan(data_values_2, infos));
    check_collection(infos, false, num_samples, num_samples);
    check_collection(data_values_2, false, num_samples, num_samples);

    // Return loan to the other reader should fail and keep collections state
    EXPECT_EQ(precondition_code, reader2->return_loan(data_values, infos));
    check_collection(data_values, false, num_samples, num_samples);
    check_collection(infos, false, num_samples, num_samples);

    EXPECT_EQ(precondition_code, data_reader_->return_loan(data_values_2, infos_2));
    check_collection(data_values_2, false, num_samples, num_samples);
    check_collection(infos_2, false, num_samples, num_samples);

    // Return loan to original reader should reset collections
    EXPECT_EQ(ok_code, data_reader_->return_loan(data_values, infos));
    check_collection(data_values, true, 0, 0);
    check_collection(infos, true, 0, 0);

    EXPECT_EQ(ok_code, reader2->return_loan(data_values_2, infos_2));
    check_collection(data_values_2, true, 0, 0);
    check_collection(infos_2, true, 0, 0);

    // Take with loan from both readers
    EXPECT_EQ(ok_code, data_reader_->take(data_values, infos));
    EXPECT_EQ(ok_code, reader2->take(data_values_2, infos_2));
    check_collection(data_values, false, num_samples, num_samples);
    check_collection(infos, false, num_samples, num_samples);
    check_collection(data_values_2, false, num_samples, num_samples);
    check_collection(infos_2, false, num_samples, num_samples);

    // Save a copy of the loaned buffers to try returning the same loan twice
    FooSeq aux_values;
    SampleInfoSeq aux_infos;
    FooSeq aux_values_2;
    SampleInfoSeq aux_infos_2;

    forward_loan(aux_values, data_values);
    forward_loan(aux_infos, infos);
    forward_loan(aux_values_2, data_values_2);
    forward_loan(aux_infos_2, infos_2);
    check_collection(aux_values, false, num_samples, num_samples);
    check_collection(aux_infos, false, num_samples, num_samples);
    check_collection(aux_values_2, false, num_samples, num_samples);
    check_collection(aux_infos_2, false, num_samples, num_samples);

    // Deleting a reader with an outstanding loan should fail
    ASSERT_EQ(precondition_code, subscriber_->delete_datareader(reader2));

    // Return loan to original reader should reset collections
    EXPECT_EQ(ok_code, data_reader_->return_loan(data_values, infos));
    check_collection(data_values, true, 0, 0);
    check_collection(infos, true, 0, 0);

    EXPECT_EQ(ok_code, reader2->return_loan(data_values_2, infos_2));
    check_collection(data_values_2, true, 0, 0);
    check_collection(infos_2, true, 0, 0);

    // Returning the same loans again should fail
    EXPECT_EQ(precondition_code, data_reader_->return_loan(aux_values, aux_infos));
    check_collection(aux_values, false, num_samples, num_samples);
    check_collection(aux_infos, false, num_samples, num_samples);

    EXPECT_EQ(precondition_code, reader2->return_loan(aux_values_2, aux_infos_2));
    check_collection(aux_values_2, false, num_samples, num_samples);
    check_collection(aux_infos_2, false, num_samples, num_samples);

    // Return forward loans to avoid warnings when destroying them
    aux_values.unloan();
    aux_infos.unloan();
    aux_values_2.unloan();
    aux_infos_2.unloan();

    EXPECT_EQ(ok_code, subscriber_->delete_datareader(reader2));
}

/*
 * This test checks that the limits imposed by reader_resource_limits QoS are taken into account when performing loans.
 */
TEST_F(DataReaderTests, resource_limits)
{
    static constexpr int32_t num_samples = 100;

    const ReturnCode_t& ok_code = RETCODE_OK;
    const ReturnCode_t& resources_code = RETCODE_OUT_OF_RESOURCES;
    const ReturnCode_t& no_data_code = RETCODE_NO_DATA;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    writer_qos.history().depth = num_samples;
    writer_qos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    reader_qos.resource_limits().max_instances = 1;
    reader_qos.resource_limits().max_samples_per_instance = num_samples;
    reader_qos.resource_limits().max_samples = LENGTH_UNLIMITED;
    reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    // Specify resource limits for this test
    // - max_samples_per_read = 10. This means that even if the max_samples parameter is greater than 10 (or even
    //   LENGTH_UNLIMITED, no more than 10 samples will be returned by a single call to read / take
    // - A maximum of 3 outstanding reads. The 4th call to read / take without a call to return_loan will fail
    // - A maximum of 15 total SampleInfo structures can be loaned. We use this value so that after a first call to
    //   read / take returns max_samples_per_read (10), a second one should only return 5 due to this limit
    reader_qos.reader_resource_limits().max_samples_per_read = 10;
    reader_qos.reader_resource_limits().outstanding_reads_allocation = { 1, 3, 1 };
    reader_qos.reader_resource_limits().sample_infos_allocation = { 1, 15, 1 };

    create_instance_handles();
    create_entities(nullptr, reader_qos, SUBSCRIBER_QOS_DEFAULT, writer_qos);

    FooType data;
    data.index(0);

    // Send a bunch of samples
    for (int32_t i = 0; i < num_samples; ++i)
    {
        EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
    }

    // Check outstanding reads maximum
    {
        FooSeq data_seqs[4];
        SampleInfoSeq info_seqs[4];

        // Loan 1 sample each time, so the other limits are not surpassed
        for (int32_t i = 0; i < 3; ++i)
        {
            EXPECT_EQ(ok_code, data_reader_->read(data_seqs[i], info_seqs[i], 1));
            check_collection(data_seqs[i], false, 1, 1);
            check_collection(info_seqs[i], false, 1, 1);
        }
        // The 4th should fail
        EXPECT_EQ(resources_code, data_reader_->read(data_seqs[3], info_seqs[3], 1));
        check_collection(data_seqs[3], true, 0, 0);
        check_collection(info_seqs[3], true, 0, 0);

        // Returning a loan will allow a new loan
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[2], info_seqs[2]));
        EXPECT_EQ(ok_code, data_reader_->read(data_seqs[3], info_seqs[3], 1));
        // Return all remaining loans
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[0], info_seqs[0]));
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[1], info_seqs[1]));
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[3], info_seqs[3]));
    }

    // Check max_samples and max_samples_per_read
    {
        FooSeq data_seq;
        SampleInfoSeq info_seq;

        //  The standard is not clear on what should be done if max_samples is 0. NO_DATA? OK with length = 0?
        // We have assumed the correct interpretation is the first one.
        // This test should change whenever this interpretation becomes invalid.
        EXPECT_EQ(no_data_code, data_reader_->read(data_seq, info_seq, 0));

        // Up to max_samples_per_read, max_samples will be returned
        for (int32_t i = 1; i <= 10; ++i)
        {
            EXPECT_EQ(ok_code, data_reader_->read(data_seq, info_seq, i));
            check_collection(data_seq, false, i, i);
            check_collection(info_seq, false, i, i);
            EXPECT_EQ(ok_code, data_reader_->return_loan(data_seq, info_seq));
        }

        // For values greater than max_samples_per_read, only max_samples_per_read are returned
        for (int32_t i = 11; i <= 20; ++i)
        {
            EXPECT_EQ(ok_code, data_reader_->read(data_seq, info_seq, i));
            check_collection(data_seq, false, 10, 10);
            check_collection(info_seq, false, 10, 10);
            EXPECT_EQ(ok_code, data_reader_->return_loan(data_seq, info_seq));
        }

        // For LENGTH_UNLIMITED, max_samples_per_read are returned
        EXPECT_EQ(ok_code, data_reader_->read(data_seq, info_seq, LENGTH_UNLIMITED));
        check_collection(data_seq, false, 10, 10);
        check_collection(info_seq, false, 10, 10);
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seq, info_seq));
    }

    // Check SampleInfo allocation limits
    {
        FooSeq data_seqs[3];
        SampleInfoSeq info_seqs[3];

        // On the first call, max_samples_per_read should be returned
        EXPECT_EQ(ok_code, data_reader_->read(data_seqs[0], info_seqs[0], LENGTH_UNLIMITED));
        check_collection(data_seqs[0], false, 10, 10);
        check_collection(info_seqs[0], false, 10, 10);

        // On the second call, sample_infos_max - max_samples_per_read should be returned
        EXPECT_EQ(ok_code, data_reader_->read(data_seqs[1], info_seqs[1], LENGTH_UNLIMITED));
        check_collection(data_seqs[1], false, 5, 5);
        check_collection(info_seqs[1], false, 5, 5);

        // On the third call, no sample_info will be available, and should fail with OUT_OF_RESOURCES
        EXPECT_EQ(resources_code, data_reader_->read(data_seqs[2], info_seqs[2], LENGTH_UNLIMITED));

        // Return the first loan. Now max_samples_per_read infos are available
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[0], info_seqs[0]));

        // Loan max_samples_per_read - 1. 1 sample_info still available
        EXPECT_EQ(ok_code, data_reader_->read(data_seqs[0], info_seqs[0], 9));
        check_collection(data_seqs[0], false, 9, 9);
        check_collection(info_seqs[0], false, 9, 9);

        // This call should loan the last available info
        EXPECT_EQ(ok_code, data_reader_->read(data_seqs[2], info_seqs[2], LENGTH_UNLIMITED));
        check_collection(data_seqs[2], false, 1, 1);
        check_collection(info_seqs[2], false, 1, 1);

        // Return all loans
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[0], info_seqs[0]));
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[1], info_seqs[1]));
        EXPECT_EQ(ok_code, data_reader_->return_loan(data_seqs[2], info_seqs[2]));
    }

    // Check resource_limits.max_samples
    {
        static constexpr int32_t additional_samples = 10;

        // Create a second reader with unlimited loans and sample infos and wait for data
        reader_qos.resource_limits().max_samples = num_samples;
        reader_qos.resource_limits().allocated_samples = num_samples / 2;
        reader_qos.reader_resource_limits().max_samples_per_read = num_samples;
        reader_qos.reader_resource_limits().outstanding_reads_allocation.maximum = 2 * num_samples;
        reader_qos.reader_resource_limits().sample_infos_allocation.maximum = 2 * num_samples;

        DataReader* reader2 = subscriber_->create_datareader(topic_, reader_qos);
        EXPECT_TRUE(reader2->wait_for_unread_message({ 10, 0 }));

        // First take all samples without returning the loan
        FooSeq data_seq;
        SampleInfoSeq info_seq;
        EXPECT_EQ(ok_code, reader2->read(data_seq, info_seq));
        while (data_seq.length() != num_samples)
        {
            EXPECT_EQ(ok_code, reader2->return_loan(data_seq, info_seq));
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_EQ(ok_code, reader2->read(data_seq, info_seq));
        }
        EXPECT_EQ(ok_code, reader2->return_loan(data_seq, info_seq));
        EXPECT_EQ(ok_code, reader2->take(data_seq, info_seq));
        check_collection(data_seq, false, num_samples, num_samples);
        check_collection(info_seq, false, num_samples, num_samples);

        // Write some more samples
        for (int32_t i = 0; i < additional_samples; ++i)
        {
            EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
        }

        // OUT_OF_RESOURCES should be returned due to resource_limits.max_samples
        FooSeq data_seq2;
        SampleInfoSeq info_seq2;
        EXPECT_EQ(resources_code, reader2->take(data_seq2, info_seq2));
        check_collection(data_seq2, true, 0, 0);
        check_collection(info_seq2, true, 0, 0);

        // Should succeed after returning the loan
        EXPECT_EQ(ok_code, reader2->return_loan(data_seq, info_seq));
        check_collection(data_seq, true, 0, 0);
        check_collection(info_seq, true, 0, 0);
        EXPECT_EQ(ok_code, reader2->take(data_seq2, info_seq2));
        check_collection(data_seq2, false, additional_samples, additional_samples);
        check_collection(info_seq2, false, additional_samples, additional_samples);
        EXPECT_EQ(ok_code, reader2->return_loan(data_seq2, info_seq2));

        EXPECT_EQ(ok_code, subscriber_->delete_datareader(reader2));
    }
}

void check_sample_values(
        const FooSeq& data,
        const std::string& values)
{
    EXPECT_EQ(static_cast<size_t>(data.length()), values.size());

    for (FooSeq::size_type i = 0; i < data.length(); ++i)
    {
        EXPECT_EQ(values[i], data[i].message()[0]);
    }
}

/*
 * This test checks that the behaviour of the sample_states parameter of read/take calls.
 */
TEST_F(DataReaderTests, read_unread)
{
    static const Duration_t time_to_wait(0, 100 * 1000 * 1000);
    static constexpr int32_t num_samples = 10;
    static constexpr uint64_t num_samples_check = static_cast<uint64_t>(num_samples);

    const ReturnCode_t& ok_code = RETCODE_OK;
    const ReturnCode_t& no_data_code = RETCODE_NO_DATA;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    writer_qos.history().depth = num_samples;
    writer_qos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    reader_qos.resource_limits().max_instances = 1;
    reader_qos.resource_limits().max_samples_per_instance = num_samples;
    reader_qos.resource_limits().max_samples = 3 * num_samples;

    create_instance_handles();
    create_entities(nullptr, reader_qos, SUBSCRIBER_QOS_DEFAULT, writer_qos);

    FooType data;
    data.index(0);
    data.message()[1] = '\0';

    // Send a bunch of samples
    for (char i = 0; i < num_samples; ++i)
    {
        data.message()[0] = i + '0';
        EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
    }

    // Reader should have 10 samples with the following states (R = read, N = not-read, / = removed from history)
    // {N, N, N, N, N, N, N, N, N, N}

    // There are unread samples, so wait_for_unread should be ok
    EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));
    EXPECT_EQ(num_samples_check, data_reader_->get_unread_count());

    // Trying to get READ samples should return NO_DATA
    {
        FooSeq data_seq;
        SampleInfoSeq info_seq;

        EXPECT_EQ(no_data_code, data_reader_->read(data_seq, info_seq, LENGTH_UNLIMITED, READ_SAMPLE_STATE));
        EXPECT_EQ(no_data_code,
                data_reader_->read_instance(data_seq, info_seq, LENGTH_UNLIMITED, handle_ok_, READ_SAMPLE_STATE));
        EXPECT_EQ(no_data_code,
                data_reader_->read_next_instance(data_seq, info_seq, LENGTH_UNLIMITED, HANDLE_NIL,
                READ_SAMPLE_STATE));

        EXPECT_EQ(no_data_code, data_reader_->take(data_seq, info_seq, LENGTH_UNLIMITED, READ_SAMPLE_STATE));
        EXPECT_EQ(no_data_code,
                data_reader_->take_instance(data_seq, info_seq, LENGTH_UNLIMITED, handle_ok_, READ_SAMPLE_STATE));
        EXPECT_EQ(no_data_code,
                data_reader_->take_next_instance(data_seq, info_seq, LENGTH_UNLIMITED, HANDLE_NIL, READ_SAMPLE_STATE));
    }

    // There are unread samples, so wait_for_unread should be ok
    EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));

    // Checks with read API
    {
        FooSeq data_seq[6];
        SampleInfoSeq info_seq[6];

        // Current state: {R, N, N, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check, data_reader_->get_unread_count());

        // This should return the first sample
        EXPECT_EQ(ok_code, data_reader_->read(data_seq[0], info_seq[0], 1, NOT_READ_SAMPLE_STATE));
        check_collection(data_seq[0], false, 1, 1);
        check_sample_values(data_seq[0], "0");

        // Current state: {R, N, N, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 1, data_reader_->get_unread_count());

        // This should return the first sample
        EXPECT_EQ(ok_code, data_reader_->read(data_seq[1], info_seq[1], 1, READ_SAMPLE_STATE));
        check_collection(data_seq[1], false, 1, 1);
        check_sample_values(data_seq[1], "0");

        // Current state: {R, N, N, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 1, data_reader_->get_unread_count());

        // This should return the first sample
        EXPECT_EQ(ok_code, data_reader_->read(data_seq[2], info_seq[2], LENGTH_UNLIMITED, READ_SAMPLE_STATE));
        check_collection(data_seq[2], false, 1, 1);
        check_sample_values(data_seq[2], "0");

        // Current state: {R, N, N, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 1, data_reader_->get_unread_count());

        // This should return the second sample
        EXPECT_EQ(ok_code, data_reader_->read(data_seq[3], info_seq[3], 1, NOT_READ_SAMPLE_STATE));
        check_collection(data_seq[3], false, 1, 1);
        check_sample_values(data_seq[3], "1");

        // Current state: {R, R, N, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 2, data_reader_->get_unread_count());

        // This should return the first sample
        EXPECT_EQ(ok_code, data_reader_->read(data_seq[4], info_seq[4], 1, READ_SAMPLE_STATE));
        check_collection(data_seq[4], false, 1, 1);
        check_sample_values(data_seq[4], "0");

        // Current state: {R, R, N, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 2, data_reader_->get_unread_count());

        // This should return the first and second samples
        EXPECT_EQ(ok_code, data_reader_->read(data_seq[5], info_seq[5], LENGTH_UNLIMITED, READ_SAMPLE_STATE));
        check_collection(data_seq[5], false, 2, 2);
        check_sample_values(data_seq[5], "01");

        // Return all loans
        for (size_t i = 0; i < 6; ++i)
        {
            EXPECT_EQ(ok_code, data_reader_->return_loan(data_seq[i], info_seq[i]));
        }
    }

    // There are unread samples, so wait_for_unread should be ok
    EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));

    // Checks with take API
    {
        FooSeq data_seq[6];
        SampleInfoSeq info_seq[6];

        // Current state: {R, R, N, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 2, data_reader_->get_unread_count());

        // This should return the third sample
        EXPECT_EQ(ok_code, data_reader_->take(data_seq[0], info_seq[0], 1, NOT_READ_SAMPLE_STATE));
        check_collection(data_seq[0], false, 1, 1);
        check_sample_values(data_seq[0], "2");

        // Current state: {R, R, /, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 3, data_reader_->get_unread_count());

        // This should return the first sample
        EXPECT_EQ(ok_code, data_reader_->take(data_seq[1], info_seq[1], 1, READ_SAMPLE_STATE));
        check_collection(data_seq[1], false, 1, 1);
        check_sample_values(data_seq[1], "0");

        // Current state: {/, R, /, N, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 3, data_reader_->get_unread_count());

        // This should return samples 2 and 4
        EXPECT_EQ(ok_code, data_reader_->take(data_seq[2], info_seq[2], 2));
        check_collection(data_seq[2], false, 2, 2);
        check_sample_values(data_seq[2], "13");

        // Current state: {/, /, /, /, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 4, data_reader_->get_unread_count());

        // This should return no data
        EXPECT_EQ(no_data_code, data_reader_->take(data_seq[3], info_seq[3], LENGTH_UNLIMITED, READ_SAMPLE_STATE));
        check_collection(data_seq[3], true, 0, 0);

        // Current state: {/, /, /, /, N, N, N, N, N, N}
        EXPECT_EQ(num_samples_check - 4, data_reader_->get_unread_count());

        // This should return samples 5 and 6
        EXPECT_EQ(ok_code, data_reader_->read(data_seq[3], info_seq[3], 2));
        check_collection(data_seq[3], false, 2, 2);
        check_sample_values(data_seq[3], "45");

        // Current state: {/, /, /, /, R, R, N, N, N, N}
        EXPECT_EQ(num_samples_check - 6, data_reader_->get_unread_count());

        // This should return samples 7, ... num_samples
        EXPECT_EQ(ok_code, data_reader_->take(data_seq[4], info_seq[4], LENGTH_UNLIMITED, NOT_READ_SAMPLE_STATE));
        check_collection(data_seq[4], false, num_samples - 6, num_samples - 6);
        check_sample_values(data_seq[4], "6789");

        // Current state: {/, /, /, /, R, R, /, /, /, /}
        EXPECT_EQ(num_samples_check - 10, data_reader_->get_unread_count());

        // There are not unread samples, so wait_for_unread should return false
        EXPECT_FALSE(data_reader_->wait_for_unread_message(time_to_wait));

        // Current state: {/, /, /, /, R, R, /, /, /, /}
        EXPECT_EQ(num_samples_check - 10, data_reader_->get_unread_count());

        // Add a new sample to have a NOT_READ one
        data.message()[0] = 'A';
        EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));

        // Wait for new sample to arrive
        EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));

        // Current state: {/, /, /, /, R, R, /, /, /, /, N}
        EXPECT_EQ(num_samples_check - 10 + 1, data_reader_->get_unread_count());

        // This should return samples 5, 6 and new
        EXPECT_EQ(ok_code, data_reader_->take(data_seq[5], info_seq[5]));
        check_collection(data_seq[5], false, 3, 3);
        check_sample_values(data_seq[5], "45A");

        // Current state: {/, /, /, /, /, /, /, /, /, /, /}
        EXPECT_EQ(num_samples_check - 10 + 1 - 1, data_reader_->get_unread_count());

        // There are not unread samples, so wait_for_unread should return false
        EXPECT_FALSE(data_reader_->wait_for_unread_message(time_to_wait));

        // Return all loans
        for (size_t i = 0; i < 6; ++i)
        {
            EXPECT_EQ(ok_code, data_reader_->return_loan(data_seq[i], info_seq[i]));
        }
    }

    // Check read_next_sample / take_next_sample
    {
        // Send a bunch of samples
        for (char i = 0; i < num_samples; ++i)
        {
            data.message()[0] = i + '0';
            EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
        }

        // Reader should have 10 samples with the following states (R = read, N = not-read, / = removed from history)
        // {N, N, N, N, N, N, N, N, N, N}

        // Read a sample and take another
        for (char i = 0; i < num_samples; i += 2)
        {
            FooType read_data;
            FooType take_data;
            SampleInfo read_info;
            SampleInfo take_info;

            EXPECT_EQ(ok_code, data_reader_->read_next_sample(&read_data, &read_info));
            EXPECT_EQ(read_data.message()[0], i + '0');

            EXPECT_EQ(ok_code, data_reader_->take_next_sample(&take_data, &take_info));
            EXPECT_EQ(take_data.message()[0], i + '1');

            EXPECT_FALSE(read_data == take_data);
            EXPECT_NE(read_info.sample_identity, take_info.sample_identity);
        }

        // Reader sample states should be
        // {R, /, R, /, R, /, R, /, R, /}

        // As all samples are read, read_next_sample and take_next_sample should not return data
        {
            FooType read_data;
            SampleInfo read_info;

            EXPECT_EQ(no_data_code, data_reader_->read_next_sample(&read_data, &read_info));
            EXPECT_EQ(no_data_code, data_reader_->take_next_sample(&read_data, &read_info));
        }
    }
}

/*
 * This test checks the behaviour of the two overloads of get_unread_count.
 */
TEST_F(DataReaderTests, get_unread_count)
{
    static const Duration_t time_to_wait(0, 100 * 1000 * 1000);
    static constexpr int32_t num_samples = 10;
    static constexpr uint64_t num_samples_check = static_cast<uint64_t>(num_samples);

    const ReturnCode_t& ok_code = RETCODE_OK;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    writer_qos.history().depth = num_samples;
    writer_qos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    reader_qos.resource_limits().max_instances = 1;
    reader_qos.resource_limits().max_samples_per_instance = num_samples;
    reader_qos.resource_limits().max_samples = 3 * num_samples;

    create_instance_handles();
    create_entities(nullptr, reader_qos, SUBSCRIBER_QOS_DEFAULT, writer_qos);

    FooType data;
    data.index(0);
    data.message()[1] = '\0';

    // Send a bunch of samples
    for (char i = 0; i < num_samples; ++i)
    {
        data.message()[0] = i + '0';
        EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
    }

    // Reader should have 10 unread samples

    // There are unread samples, so wait_for_unread should be ok
    EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));

    // Calling get_unread_count() several times should always return the same value
    for (char i = 0; i < num_samples; ++i)
    {
        EXPECT_EQ(num_samples_check, data_reader_->get_unread_count());
    }

    SampleInfo sample_info;
    ASSERT_EQ(RETCODE_OK, data_reader_->get_first_untaken_info(&sample_info));
    ASSERT_EQ(SampleStateKind::NOT_READ_SAMPLE_STATE, sample_info.sample_state);

    // Calling get_unread_count(false) several times should always return the same value
    for (char i = 0; i < num_samples; ++i)
    {
        EXPECT_EQ(num_samples_check, data_reader_->get_unread_count(false));
    }

    ASSERT_EQ(RETCODE_OK, data_reader_->get_first_untaken_info(&sample_info));
    ASSERT_EQ(SampleStateKind::NOT_READ_SAMPLE_STATE, sample_info.sample_state);

    // Calling get_unread_count(true) once will return the correct value
    EXPECT_EQ(num_samples_check, data_reader_->get_unread_count(true));

    ASSERT_EQ(RETCODE_OK, data_reader_->get_first_untaken_info(&sample_info));
    ASSERT_EQ(SampleStateKind::READ_SAMPLE_STATE, sample_info.sample_state);

    // All variants should then return 0
    EXPECT_EQ(0, data_reader_->get_unread_count(true));
    EXPECT_EQ(0, data_reader_->get_unread_count(false));
    EXPECT_EQ(0, data_reader_->get_unread_count());
}

template<typename DataType>
void lookup_instance_test(
        DataType& data,
        DataWriter* writer,
        DataReader* reader,
        const InstanceHandle_t& handle_ok)
{
    // Send sample with key value 0
    data.index(0);
    EXPECT_EQ(RETCODE_OK, writer->write(&data));
    // Ensure it arrived to the DataReader
    EXPECT_TRUE(reader->wait_for_unread_message({ 1, 0 }));

    // DataReader should have a single sample on instance handle_ok_

    // Wrong parameter should return HANDLE_NIL
    EXPECT_EQ(HANDLE_NIL, reader->lookup_instance(nullptr));
    // Querying with the correct key value should return handle_ok_, but only if the type has keys
    EXPECT_EQ(data.isKeyDefined() ? handle_ok : HANDLE_NIL, reader->lookup_instance(&data));
    // Querying with another key should return HANDLE_NIL
    data.index(37);
    EXPECT_EQ(HANDLE_NIL, reader->lookup_instance(&data));
}

TEST_F(DataReaderTests, lookup_instance)
{
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    reader_qos.history().depth = 1;
    reader_qos.resource_limits().max_instances = 1;
    reader_qos.resource_limits().max_samples_per_instance = 1;
    reader_qos.resource_limits().max_samples = 1;

    create_instance_handles();

    // Perform test on type with keys
    {
        create_entities(nullptr, reader_qos);
        FooType data;
        lookup_instance_test(data, data_writer_, data_reader_, handle_ok_);
    }

    // Destroy entities
    TearDown();

    // Perform test on type without keys
    {
        type_.reset(new FooBoundedTypeSupport());
        create_entities(nullptr, reader_qos);
        FooBoundedType data;
        lookup_instance_test(data, data_writer_, data_reader_, handle_ok_);
    }
}

TEST_F(DataReaderTests, sample_info)
{
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    reader_qos.history().depth = 1;
    reader_qos.resource_limits().max_instances = 2;
    reader_qos.resource_limits().max_samples_per_instance = 1;
    reader_qos.resource_limits().max_samples = 2;

    create_entities(nullptr, reader_qos);
    publisher_->delete_datawriter(data_writer_);
    data_writer_ = nullptr;

    struct TestCmd
    {
        enum Operation
        {
            WRITE, UNREGISTER, DISPOSE, CLOSE
        };

        size_t writer_index;
        Operation operation;
        size_t instance_index;
    };

    struct TestInstanceResult
    {
        ReturnCode_t ret_code;
        ViewStateKind view_state;
        InstanceStateKind instance_state;
        int32_t disposed_generation_count;
        int32_t no_writers_generation_count;
    };

    struct TestStep
    {
        std::vector<TestCmd> operations;
        TestInstanceResult instance_state[2];
    };

    struct TestState
    {
        TestState(
                TypeSupport& type,
                Topic* topic,
                Publisher* publisher)
            : topic_(topic)
            , publisher_(publisher)
        {
            writer_qos_ = DATAWRITER_QOS_DEFAULT;
            writer_qos_.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
            writer_qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;
            writer_qos_.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            writer_qos_.history().kind = KEEP_LAST_HISTORY_QOS;
            writer_qos_.history().depth = 1;
            writer_qos_.resource_limits().max_instances = 2;
            writer_qos_.resource_limits().max_samples_per_instance = 1;
            writer_qos_.resource_limits().max_samples = 2;
            writer_qos_.writer_data_lifecycle().autodispose_unregistered_instances = false;

            data_[0].index(1);
            data_[1].index(2);

            type.compute_key(&data_[0], handles_[0]);
            type.compute_key(&data_[1], handles_[1]);
        }

        ~TestState()
        {
            close_writer(0);
            close_writer(1);
        }

        void run_test(
                DataReader* reader,
                const std::vector<TestStep>& steps)
        {
            for (const TestStep& step : steps)
            {
                for (const TestCmd& cmd : step.operations)
                {
                    execute(cmd);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                check(reader, 0, step.instance_state[0]);
                check(reader, 1, step.instance_state[1]);
            }

        }

        void execute(
                const TestCmd& cmd)
        {
            DataWriter* writer = nullptr;
            ReturnCode_t ret_code;

            switch (cmd.operation)
            {
                case TestCmd::CLOSE:
                    close_writer(cmd.writer_index);
                    break;

                case TestCmd::DISPOSE:
                    writer = open_writer(cmd.writer_index);
                    ret_code = writer->dispose(&data_[cmd.instance_index], handles_[cmd.instance_index]);
                    EXPECT_EQ(RETCODE_OK, ret_code);
                    break;

                case TestCmd::UNREGISTER:
                    writer = open_writer(cmd.writer_index);
                    ret_code = writer->unregister_instance(&data_[cmd.instance_index], handles_[cmd.instance_index]);
                    EXPECT_EQ(RETCODE_OK, ret_code);
                    break;

                case TestCmd::WRITE:
                    writer = open_writer(cmd.writer_index);
                    ret_code = writer->write(&data_[cmd.instance_index], handles_[cmd.instance_index]);
                    EXPECT_EQ(RETCODE_OK, ret_code);
                    break;
            }
        }

        void check(
                DataReader* reader,
                size_t instance_index,
                const TestInstanceResult& instance_result)
        {
            FooSeq values;
            SampleInfoSeq infos;
            ReturnCode_t ret_code;

            ret_code = reader->read_instance(values, infos, LENGTH_UNLIMITED, handles_[instance_index]);
            EXPECT_EQ(ret_code, instance_result.ret_code);
            if (RETCODE_OK == ret_code)
            {
                EXPECT_EQ(instance_result.instance_state, infos[0].instance_state);
                EXPECT_EQ(instance_result.view_state, infos[0].view_state);
                EXPECT_EQ(instance_result.disposed_generation_count, infos[0].disposed_generation_count);
                EXPECT_EQ(instance_result.no_writers_generation_count, infos[0].no_writers_generation_count);
                EXPECT_EQ(RETCODE_OK, reader->return_loan(values, infos));

                EXPECT_EQ(handles_[instance_index], reader->lookup_instance(&data_[instance_index]));
            }
            else
            {
                EXPECT_EQ(HANDLE_NIL, reader->lookup_instance(&data_[instance_index]));
            }
        }

    private:

        Topic* topic_;
        Publisher* publisher_;
        DataWriterQos writer_qos_;
        DataWriter* writers_[2] = { nullptr, nullptr };

        InstanceHandle_t handles_[2];
        FooType data_[2];

        void close_writer(
                size_t index)
        {
            DataWriter*& writer = writers_[index];
            if (writer != nullptr)
            {
                publisher_->delete_datawriter(writer);
                writer = nullptr;
            }
        }

        DataWriter* open_writer(
                size_t index)
        {
            DataWriter*& writer = writers_[index];
            if (writer == nullptr)
            {
                writer = publisher_->create_datawriter(topic_, writer_qos_);
            }
            return writer;
        }

    };

    static const std::vector<TestStep> steps =
    {
        {
            // Instances have never been written
            {},
            {
                {RETCODE_BAD_PARAMETER, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
                {RETCODE_BAD_PARAMETER, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // One writer writes on first instance => that instance should be NEW and ALIVE
            { {0, TestCmd::WRITE, 0} },
            {
                {RETCODE_OK, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
                {RETCODE_BAD_PARAMETER, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // Same writer writes on first instance => instance becomes NOT_NEW
            { {0, TestCmd::WRITE, 0} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
                {RETCODE_BAD_PARAMETER, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // Same writer disposes first instance => instance becomes NOT_ALIVE_DISPOSED
            { {0, TestCmd::DISPOSE, 0} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_DISPOSED_INSTANCE_STATE, 0, 0},
                {RETCODE_BAD_PARAMETER, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // First writer writes second instance => NEW and ALIVE
            // Second writer writes first instance => NEW and ALIVE
            { {0, TestCmd::WRITE, 1}, {1, TestCmd::WRITE, 0} },
            {
                {RETCODE_OK, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 1, 0},
                {RETCODE_OK, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // Both writers write on second instance => NOT_NEW and ALIVE
            { {0, TestCmd::WRITE, 1}, {1, TestCmd::WRITE, 1} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 1, 0},
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // Second writer closes => first instance becomes NOT_ALIVE_NO_WRITERS
            { {1, TestCmd::CLOSE, 0} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, 1, 0},
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // First writer unregisters second instance => NOT_ALIVE_NO_WRITERS
            { {0, TestCmd::UNREGISTER, 1} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, 1, 0},
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, 0, 0},
            }
        },
        {
            // Both writers write both instances
            { {0, TestCmd::WRITE, 0}, {1, TestCmd::WRITE, 0}, {0, TestCmd::WRITE, 1}, {1, TestCmd::WRITE, 1} },
            {
                {RETCODE_OK, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 1, 1},
                {RETCODE_OK, NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 1},
            }
        },
        {
            // Reading twice should return NOT_NEW
            {},
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 1, 1},
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 0, 1},
            }
        },
        {
            // 0 - Unregistering while having another alive writer should not change state
            // 1 - Disposing while having another alive writer is always done
            { {0, TestCmd::UNREGISTER, 0}, {1, TestCmd::DISPOSE, 1} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 1, 1},
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_DISPOSED_INSTANCE_STATE, 0, 1},
            }
        },
        {
            // 0 - Writing and unregistering while having another alive writer should not change state
            // 1 - Unregister a disposed instance should not change state
            { {0, TestCmd::WRITE, 0}, {0, TestCmd::UNREGISTER, 1}, {1, TestCmd::UNREGISTER, 0} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, ALIVE_INSTANCE_STATE, 1, 1},
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_DISPOSED_INSTANCE_STATE, 0, 1},
            }
        },
        {
            // 0 - Closing both writers should return NOT_ALIVE_NO_WRITERS
            // 1 - Closing both writers on a disposed instance should not change state
            { {0, TestCmd::CLOSE, 0}, {1, TestCmd::CLOSE, 0} },
            {
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, 1, 1},
                {RETCODE_OK, NOT_NEW_VIEW_STATE, NOT_ALIVE_DISPOSED_INSTANCE_STATE, 0, 1},
            }
        },
    };

    // Run test once
    TestState state(type_, topic_, publisher_);
    state.run_test(data_reader_, steps);

    // Taking all data should remove instance information
    FooSeq data;
    SampleInfoSeq infos;
    EXPECT_EQ(RETCODE_OK, data_reader_->take(data, infos));
    EXPECT_EQ(RETCODE_OK, data_reader_->return_loan(data, infos));

    // Run test again
    state.run_test(data_reader_, steps);
}

struct arraybuf : public std::streambuf
{
    template <std::size_t Size> arraybuf(
            std::array<char, Size>& array)
    {
        this->setp(array.data(), array.data() + Size - 1);
        array.fill(0);
    }

};

struct oarraystream : virtual arraybuf, std::ostream
{
    template <std::size_t Size> oarraystream(
            std::array<char, Size>& array)
        : arraybuf(array)
        , std::ostream(this)
    {
    }

};

/*
 *  This test deals with issues covered on PR #3044.
 *  It checks (read|take)_next_instance methods iterate properly over all
 *  instances in the history.
 */
TEST_F(DataReaderTests, check_read_take_iteration)
{
    const std::size_t max_handles = 100;
    std::array<InstanceHandle_t, max_handles> handles;
    auto loop_timeout = std::chrono::seconds(5);

    // Allocate resources
    DataReaderQos reader_qos;
    DataWriterQos writer_qos;
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    PublisherQos publisher_qos = PUBLISHER_QOS_DEFAULT;

    reader_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    reader_qos.history().depth = 1;
    writer_qos.history(reader_qos.history());

    reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.durability(reader_qos.durability());

    reader_qos.resource_limits().max_instances = max_handles;
    reader_qos.resource_limits().max_samples_per_instance = 1;
    writer_qos.resource_limits(reader_qos.resource_limits());

    create_entities(nullptr, reader_qos, subscriber_qos, writer_qos, publisher_qos);

    {
        // Populate the test handles and write on all instances
        FooType data;
        for (uint32_t i = 0; i < max_handles; ++i )
        {
            // calculate key
            data.index(i);
            type_.compute_key(&data, handles[i]);

            // write the index as message
            oarraystream out(data.message());
            out << i;

            EXPECT_EQ(data_writer_->write(&data, handles[i]), RETCODE_OK);
        }
    }

    // Loop till all samples are received
    std::size_t received = 0;
    auto start = std::chrono::system_clock::now();
    bool timeout = false;

    do
    {
        FooSeq data;
        SampleInfoSeq infos;

        auto ret = data_reader_->read(data, infos, max_handles,
                        NOT_READ_SAMPLE_STATE,
                        NEW_VIEW_STATE,
                        ALIVE_INSTANCE_STATE);

        if ( ret == RETCODE_OK)
        {
            received += data.length();
            EXPECT_EQ(RETCODE_OK, data_reader_->return_loan(data, infos));
        }
        else
        {
            timeout = (std::chrono::system_clock::now() - start) < loop_timeout;
            std::this_thread::yield();
        }

    }
    while ( received < max_handles && !timeout);

    EXPECT_FALSE(timeout);
    received = 0;

    // Take only the even handles
    for (typename std::remove_const<decltype(max_handles)>::type i = 0; i < max_handles; i += 2, ++received )
    {
        FooSeq data;
        SampleInfoSeq infos;

        EXPECT_EQ(data_reader_->take_instance(data, infos, 1, handles[i]),
                RETCODE_OK);

        EXPECT_EQ(static_cast<int>(i), std::atoi(data[0].message().data()));

        EXPECT_EQ(RETCODE_OK, data_reader_->return_loan(data, infos));
    }

    // Iterate over available instances with data and check all are retrieved
    auto pending = received;
    InstanceHandle_t handle = HANDLE_NIL;
    ReturnCode_t ret;

    do
    {
        FooSeq data;
        SampleInfoSeq infos;

        ret = data_reader_->read_next_instance(
            data,
            infos,
            1,
            handle);

        if (RETCODE_OK == ret)
        {
            received += data.length();
            handle = infos[0].instance_handle;
            EXPECT_TRUE(std::atoi(data[0].message().data()) % 2 == 1);
            EXPECT_EQ(RETCODE_OK, data_reader_->return_loan(data, infos));
        }
    }
    while (ret == RETCODE_OK);

    EXPECT_EQ(received, max_handles);

    // Iterate over available instances and check all are removed
    received = pending;
    handle = HANDLE_NIL;

    do
    {
        FooSeq data;
        SampleInfoSeq infos;

        ret = data_reader_->take_next_instance(
            data,
            infos,
            1,
            handle);

        if (RETCODE_OK == ret)
        {
            received += data.length();
            handle = infos[0].instance_handle;
            EXPECT_TRUE(std::atoi(data[0].message().data()) % 2 == 1);
            EXPECT_EQ(RETCODE_OK, data_reader_->return_loan(data, infos));
        }
    }
    while (ret == RETCODE_OK);

    EXPECT_EQ(received, max_handles);
}

/*
 * This type fails deserialization on odd samples
 */
class FailingFooTypeSupport : public FooTypeSupport
{

public:

    FailingFooTypeSupport()
        : FooTypeSupport()
    {
    }

    bool deserialize(
            SerializedPayload_t& payload,
            void* data) override
    {
        //Convert DATA to pointer of your type
        FooType* p_type = static_cast<FooType*>(data);

        bool ret_val = FooTypeSupport::deserialize(payload, p_type);
        if (p_type->message()[0] % 2)
        {
            return false;
        }
        return ret_val;
    }

};

/*
 * This test checks that the behaviour of the read/take calls when deserialization fails.
 */
TEST_F(DataReaderTests, Deserialization_errors)
{
    type_.reset(new FailingFooTypeSupport());

    static const Duration_t time_to_wait(0, 100 * 1000 * 1000);
    static constexpr int32_t num_samples = 10;

    const ReturnCode_t& ok_code = RETCODE_OK;
    const ReturnCode_t& no_data_code = RETCODE_NO_DATA;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
    writer_qos.history().depth = num_samples;
    writer_qos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    reader_qos.resource_limits().max_instances = 1;
    reader_qos.resource_limits().max_samples_per_instance = num_samples;
    reader_qos.resource_limits().max_samples = 3 * num_samples;

    create_instance_handles();
    create_entities(nullptr, reader_qos, SUBSCRIBER_QOS_DEFAULT, writer_qos);

    FooType data;
    data.index(0);
    data.message()[1] = '\0';

    // Check deserialization errors without loans
    {
        // Send a bunch of samples
        for (char i = 0; i < num_samples; ++i)
        {
            data.message()[0] = i + '0';
            EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
        }

        // There are unread samples, so wait_for_unread should be ok
        EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));

        {
            FooSeq data_seq(num_samples);
            SampleInfoSeq info_seq(num_samples);

            // Reader should have 10 samples with the following states (R = read, N = not-read, / = removed from history)
            // {N, N, N, N, N, N, N, N, N, N}

            // This should return samples 0, 2, 4, 6, and 8
            EXPECT_EQ(ok_code, data_reader_->read(data_seq, info_seq, num_samples));
            check_collection(data_seq, true, num_samples, num_samples / 2);
            check_sample_values(data_seq, "02468");
        }

        {
            FooSeq data_seq(num_samples);
            SampleInfoSeq info_seq(num_samples);

            // Reader sample states should be
            // {R, /, R, /, R, /, R, /, R, /}

            // There are not unread samples in the history
            EXPECT_EQ(no_data_code, data_reader_->take(data_seq, info_seq, num_samples, NOT_READ_SAMPLE_STATE));
        }

        {
            FooSeq data_seq(num_samples);
            SampleInfoSeq info_seq(num_samples);

            // This should return samples 0, 2, 4, 6, and 8 (just for cleaning)
            EXPECT_EQ(ok_code, data_reader_->take(data_seq, info_seq, num_samples, READ_SAMPLE_STATE));
            check_collection(data_seq, true, num_samples, num_samples / 2);
            check_sample_values(data_seq, "02468");
        }
    }

    // Check deserialization errors with loans (loaned samples are not deserialized)
    {
        // Send a bunch of samples
        for (char i = 0; i < num_samples; ++i)
        {
            data.message()[0] = i + '0';
            EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
        }

        // There are unread samples, so wait_for_unread should be ok
        EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));

        {
            FooSeq data_seq;
            SampleInfoSeq info_seq;

            // Reader should have 10 samples with the following states (R = read, N = not-read, / = removed from history)
            // {N, N, N, N, N, N, N, N, N, N}

            // This should return samples 0 to 9
            EXPECT_EQ(ok_code, data_reader_->read(data_seq, info_seq, num_samples));
            check_collection(data_seq, false, num_samples, num_samples);
            check_sample_values(data_seq, "0123456789");
            EXPECT_EQ(ok_code, data_reader_->return_loan(data_seq, info_seq));
        }

        {
            FooSeq data_seq;
            SampleInfoSeq info_seq;

            // Reader should have 10 samples with the following states (R = read, N = not-read, / = removed from history)
            // {N, N, N, N, N, N, N, N, N, N}

            // This should return samples 0 to 9
            EXPECT_EQ(ok_code, data_reader_->take(data_seq, info_seq, num_samples, READ_SAMPLE_STATE));
            check_collection(data_seq, false, num_samples, num_samples);
            check_sample_values(data_seq, "0123456789");
            EXPECT_EQ(ok_code, data_reader_->return_loan(data_seq, info_seq));
        }
    }

    // Check deserialization errors for read_next_sample and take_next_sample.
    // Regression test for #12129
    {
        // Send two samples
        for (char i = 0; i < 2; ++i)
        {
            data.message()[0] = 1;
            EXPECT_EQ(ok_code, data_writer_->write(&data, handle_ok_));
        }

        // There are unread samples, so wait_for_unread should be ok
        EXPECT_TRUE(data_reader_->wait_for_unread_message(time_to_wait));

        {
            SampleInfo info;
            EXPECT_EQ(no_data_code, data_reader_->take_next_sample(&data, &info));
        }

        {
            SampleInfo info;
            EXPECT_EQ(no_data_code, data_reader_->read_next_sample(&data, &info));
        }
    }

}

TEST_F(DataReaderTests, TerminateWithoutDestroyingReader)
{
    destroy_entities_ = false;
    create_entities();
}

void set_listener_test (
        DataReader* reader,
        DataReaderListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(reader->set_listener(listener, mask), RETCODE_OK);
    ASSERT_EQ(reader->get_status_mask(), mask);
}

class CustomListener : public DataReaderListener
{

};

/*
 * This test checks the calls to set_listener and get_status_mask
 */
TEST_F(DataReaderTests, SetListener)
{
    CustomListener listener;
    create_entities(&listener);

    ASSERT_EQ(data_reader_->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<DataReader*, DataReaderListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { data_reader_, &listener, StatusMask::data_available() },
        { data_reader_, &listener, StatusMask::sample_rejected() },
        { data_reader_, &listener, StatusMask::liveliness_changed() },
        { data_reader_, &listener, StatusMask::requested_deadline_missed() },
        { data_reader_, &listener, StatusMask::requested_incompatible_qos() },
        { data_reader_, &listener, StatusMask::subscription_matched() },
        { data_reader_, &listener, StatusMask::sample_lost() },
        //all except one
        { data_reader_, &listener, StatusMask::all() >> StatusMask::data_available() },
        { data_reader_, &listener, StatusMask::all() >> StatusMask::sample_rejected() },
        { data_reader_, &listener, StatusMask::all() >> StatusMask::liveliness_changed() },
        { data_reader_, &listener, StatusMask::all() >> StatusMask::requested_deadline_missed() },
        { data_reader_, &listener, StatusMask::all() >> StatusMask::requested_incompatible_qos() },
        { data_reader_, &listener, StatusMask::all() >> StatusMask::subscription_matched() },
        { data_reader_, &listener, StatusMask::all() >> StatusMask::sample_lost() },
        //all and none
        { data_reader_, &listener, StatusMask::all() },
        { data_reader_, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }
}

TEST_F(DataReaderTests, get_listening_locators)
{
    // Prepare specific listening locators
    Locator unicast_locator;
    IPLocator::setIPv4(unicast_locator, 127, 0, 0, 1);
    IPLocator::setPortRTPS(unicast_locator, 7399);

    Locator multicast_locator;
    IPLocator::setIPv4(multicast_locator, 239, 127, 0, 1);
    IPLocator::setPortRTPS(multicast_locator, 7398);

    // Set specific locators on DataReader QoS
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.endpoint().unicast_locator_list.push_back(unicast_locator);
    reader_qos.endpoint().multicast_locator_list.push_back(multicast_locator);

    // We will create a disabled DataReader, so we can check RETCODE_NOT_ENABLED
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    subscriber_qos.entity_factory().autoenable_created_entities = false;

    create_entities(nullptr, reader_qos, subscriber_qos);
    EXPECT_FALSE(data_reader_->is_enabled());

    // Calling on disabled reader should return NOT_ENABLED
    LocatorList locator_list;
    EXPECT_EQ(RETCODE_NOT_ENABLED, data_reader_->get_listening_locators(locator_list));

    // Enable and try again
    EXPECT_EQ(RETCODE_OK, data_reader_->enable());
    EXPECT_EQ(RETCODE_OK, data_reader_->get_listening_locators(locator_list));

    EXPECT_EQ(locator_list.size(), 2u);
    bool unicast_found = false;
    bool multicast_found = false;
    for (const Locator& locator : locator_list)
    {
        unicast_found |= (locator == unicast_locator);
        multicast_found |= (locator == multicast_locator);
    }
    EXPECT_TRUE(unicast_found);
    EXPECT_TRUE(multicast_found);
}

/*
 * Issue https://github.com/eProsima/Fast-DDS/issues/2044 highlighted that a DataWriter removal may left DataReaders
 * Histories in an invalid state. DataWriter removal triggers the clean up of any related sample in DataReader's
 * History. Because the key related internal state was not updated and kept sample dangling references.
 * */

TEST_F(DataReaderTests, check_key_history_wholesomeness_on_unmatch)
{
    // handle_ok_ (1)
    create_instance_handles();

    // Set up entities
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;

    // defaults to footopic using type FooType
    create_entities(
        nullptr,
        reader_qos,
        SUBSCRIBER_QOS_DEFAULT,
        writer_qos);

    // add a key sample
    FooType sample;
    std::array<char, 256> msg = {"checking robustness"};

    sample.index(0);
    sample.message(msg);

    ASSERT_EQ(RETCODE_OK, data_writer_->write(&sample));

    // wait till the DataReader receives the data
    ASSERT_TRUE(data_reader_->wait_for_unread_message(Duration_t(3, 0)));

    // now the writer is removed
    ASSERT_EQ(publisher_->delete_datawriter(data_writer_), RETCODE_OK);
    data_writer_ = nullptr;

    // here the DataReader History state must be coherent and don't loop endlessly
    ReturnCode_t res;
    std::thread query([this, &res]()
            {
                FooSeq samples;
                SampleInfoSeq infos;

                res = data_reader_->take_instance(samples, infos, LENGTH_UNLIMITED, handle_ok_);

                // If the DataWriter is destroyed only the non-notified samples must be removed
                // this operation MUST succeed
                ASSERT_EQ(res, RETCODE_OK);

                data_reader_->return_loan(samples, infos);
            });

    // Check if the thread hangs
    // wait for termination
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    query.join();
}

class DataReaderUnsupportedTests : public ::testing::Test
{
public:

    DataReaderUnsupportedTests()
    {
        Reset();
    }

    ~DataReaderUnsupportedTests()
    {
        Log::Reset();
        Log::KillThread();
    }

    void Reset()
    {
        Log::ClearConsumers();
        // Only listen for logWarnings generated by the tested class
        mockConsumer = new MockConsumer("DATA_READER");
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Warning);
    }

    MockConsumer* mockConsumer;

    const uint32_t AsyncTries = 5;
    const uint32_t AsyncWaitMs = 25;

    void HELPER_WaitForEntries(
            uint32_t amount)
    {
        size_t entries = 0;
        for (uint32_t i = 0; i != AsyncTries; i++)
        {
            entries = mockConsumer->ConsumedEntries().size();
            if (entries == amount)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(AsyncWaitMs));
        }

        ASSERT_EQ(amount, mockConsumer->ConsumedEntries().size());
    }

};

/*
 * This test checks that the DataReader methods defined in the standard not yet implemented in FastDDS return
 * RETCODE_UNSUPPORTED. The following methods are checked:
 * 1. create_querycondition
 * 2. get_key_value
 * 3. wait_for_historical_data
 */
TEST_F(DataReaderUnsupportedTests, UnsupportedDataReaderMethods)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);

    {
        SampleStateMask sample_states = ANY_SAMPLE_STATE;
        ViewStateMask view_states = ANY_VIEW_STATE;
        InstanceStateMask instance_states = ANY_INSTANCE_STATE;
        std::string query_expression;
        std::vector<std::string> query_parameters;
        EXPECT_EQ(
            nullptr,
            data_reader->create_querycondition(
                sample_states,
                view_states,
                instance_states,
                query_expression,
                query_parameters));
    }

    InstanceHandle_t key_handle;
    EXPECT_EQ(RETCODE_UNSUPPORTED, data_reader->get_key_value(nullptr, key_handle));

    EXPECT_EQ(RETCODE_UNSUPPORTED, data_reader->wait_for_historical_data({0, 1}));

    // Expected logWarnings: create_querycondition
    HELPER_WaitForEntries(1);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

// Regression test for #12133.
TEST_F(DataReaderTests, read_samples_with_future_changes)
{
    eprosima::fastdds::LibrarySettings att;
    att.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
    DomainParticipantFactory::get_instance()->set_library_settings(att);
    static constexpr int32_t num_samples = 8;
    static constexpr int32_t expected_samples = 4;
    const ReturnCode_t& ok_code = RETCODE_OK;
    bool start_dropping_acks = false;
    bool start_dropping_datas = false;
    static const Duration_t time_to_wait(0, 100 * 1000 * 1000);
    std::shared_ptr<test_UDPv4TransportDescriptor> test_descriptor =
            std::make_shared<test_UDPv4TransportDescriptor>();
    test_descriptor->drop_ack_nack_messages_filter_ = [&](CDRMessage_t&) -> bool
            {
                return start_dropping_acks;
            };
    test_descriptor->drop_data_messages_filter_ = [&](CDRMessage_t&) -> bool
            {
                return start_dropping_datas;
            };

    DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
    participant_qos.transport().use_builtin_transports = false;
    participant_qos.transport().user_transports.push_back(test_descriptor);

    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;

    create_entities(
        nullptr,
        reader_qos,
        SUBSCRIBER_QOS_DEFAULT,
        writer_qos,
        PUBLISHER_QOS_DEFAULT,
        TOPIC_QOS_DEFAULT,
        participant_qos);

    DataWriter* data_writer2 = publisher_->create_datawriter(topic_, writer_qos);

    create_instance_handles();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait discovery

    FooType data;
    data.index(0);
    data.message()[0] = '\0';
    data.message()[1] = '\0';

    for (int i = 0; i < 2; ++i)
    {
        data_writer_->write(&data, handle_ok_);
    }

    start_dropping_datas = true;
    start_dropping_acks = true;

    for (int i = 0; i < 2; ++i)
    {
        data_writer2->write(&data, handle_ok_);
    }

    start_dropping_datas = false;

    for (int i = 0; i < 2; ++i)
    {
        data_writer2->write(&data, handle_ok_);
    }

    for (int i = 0; i < 2; ++i)
    {
        data_writer_->write(&data, handle_ok_);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait all received

    FooSeq data_seq(num_samples);
    SampleInfoSeq info_seq(num_samples);

    EXPECT_EQ(ok_code, data_reader_->take(data_seq, info_seq, num_samples, NOT_READ_SAMPLE_STATE));
    check_collection(data_seq, true, num_samples, expected_samples);

    ASSERT_EQ(publisher_->delete_datawriter(data_writer2), RETCODE_OK);
}

// Delete contained entities test
TEST_F(DataReaderTests, delete_contained_entities)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic(topic_name, type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);

    SampleStateMask mock_sample_state_kind = ANY_SAMPLE_STATE;
    ViewStateMask mock_view_state_kind = ANY_VIEW_STATE;
    InstanceStateMask mock_instance_states = ANY_INSTANCE_STATE;

    ReadCondition* read_condition = data_reader->create_readcondition(
        mock_sample_state_kind,
        mock_view_state_kind,
        mock_instance_states);
    EXPECT_NE(read_condition, nullptr);

    const std::string mock_query_expression;
    const std::vector<std::string> mock_query_parameters;

    QueryCondition* query_condition = data_reader->create_querycondition(
        mock_sample_state_kind,
        mock_view_state_kind,
        mock_instance_states,
        mock_query_expression,
        mock_query_parameters
        );

    // To be updated when Query Conditions are available
    ASSERT_EQ(query_condition, nullptr);

    // Should fail with outstanding ReadConditions
    ASSERT_EQ(subscriber->delete_datareader(data_reader), RETCODE_PRECONDITION_NOT_MET);

    // Should not fail with outstanding ReadConditions
    ASSERT_EQ(data_reader->delete_contained_entities(), RETCODE_OK);
}

TEST_F(DataReaderTests, read_conditions_management)
{
    create_entities();
    DataReader& reader = *data_reader_;

    // Condition masks
    SampleStateMask sample_states = 0;
    ViewStateMask view_states = 0;
    InstanceStateMask instance_states = 0;

    // Create and destroy testing

    // 1- cannot create a ReadConditon that cannot be triggered
    ReadCondition* cond = reader.create_readcondition( sample_states, view_states, instance_states);
    EXPECT_EQ(cond, nullptr);

    // 2- create a ReadCondition and destroy it
    sample_states = ANY_SAMPLE_STATE;
    cond = reader.create_readcondition( sample_states, view_states, instance_states);
    EXPECT_NE(cond, nullptr);
    ReturnCode_t res = reader.delete_readcondition(cond);
    EXPECT_EQ(res, RETCODE_OK);

    // 3- Create several ReadConditions associated to the same masks (share implementation)
    std::forward_list<ReadCondition*> conds;

    for (int i = 0; i < 10; ++i )
    {
        conds.push_front(reader.create_readcondition( sample_states, view_states, instance_states));
    }

    for (ReadCondition* c : conds)
    {
        EXPECT_EQ(reader.delete_readcondition(c), RETCODE_OK);
    }
    conds.clear();

    // 4- Create several ReadConditions associated to different masks
    sample_states = 0;
    view_states = 0;
    instance_states = 0;

    for (int i = 0; i < 10; ++i )
    {
        conds.push_front(reader.create_readcondition( ++sample_states, ++view_states, ++instance_states));
    }

    for (ReadCondition* c : conds)
    {
        EXPECT_EQ(reader.delete_readcondition(c), RETCODE_OK);
    }
    conds.clear();

    // 5- Create several ReadConditions and destroy them using delete_contained_entities
    sample_states = 0;
    view_states = 0;
    instance_states = 0;

    for (int i = 0; i < 10; ++i )
    {
        conds.push_front(reader.create_readcondition( ++sample_states, ++view_states, ++instance_states));
    }

    EXPECT_EQ(reader.delete_contained_entities(), RETCODE_OK);
    conds.clear();

    // 6- Check a DataReader only handles its own ReadConditions
    DataReader* another_reader = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(another_reader, nullptr);

    cond = another_reader->create_readcondition(sample_states, view_states, instance_states);
    EXPECT_NE(cond, nullptr);
    EXPECT_EQ(reader.delete_readcondition(cond), RETCODE_PRECONDITION_NOT_MET);

    // 7- Check the DataReader cannot be deleted with outstanding conditions
    EXPECT_EQ(subscriber_->delete_datareader(another_reader), RETCODE_PRECONDITION_NOT_MET);
    // but delete_contained_entities() succeeds with outstanding ReadConditions
    EXPECT_EQ(another_reader->delete_contained_entities(), RETCODE_OK);
    // no outstanding conditions (killed above)
    EXPECT_EQ(subscriber_->delete_datareader(another_reader), RETCODE_OK);
}

TEST_F(DataReaderTests, read_conditions_wait_on_SampleStateMask)
{
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    create_entities(nullptr, reader_qos, SUBSCRIBER_QOS_DEFAULT, writer_qos);
    DataReader& data_reader = *data_reader_;
    DataWriter& data_writer = *data_writer_;

    // Condition masks
    ViewStateMask view_states = ANY_VIEW_STATE;
    InstanceStateMask instance_states = ANY_INSTANCE_STATE;

    // Create the read conditions
    ReadCondition* read_cond = data_reader.create_readcondition(READ_SAMPLE_STATE, view_states, instance_states);
    EXPECT_NE(read_cond, nullptr);

    ReadCondition* not_read_cond =
            data_reader.create_readcondition(NOT_READ_SAMPLE_STATE, view_states, instance_states);
    EXPECT_NE(not_read_cond, nullptr);

    ReadCondition* any_read_cond = data_reader.create_readcondition(ANY_SAMPLE_STATE, view_states, instance_states);
    EXPECT_NE(any_read_cond, nullptr);

    // Create the waitset and associate
    WaitSet ws;
    EXPECT_EQ(ws.attach_condition(*read_cond), RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*not_read_cond), RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*any_read_cond), RETCODE_OK);

    // 1- Check NOT_READ_SAMPLE_STATE
    // Send sample from a background thread
    std::array<char, 256> test_message = {"Testing sample state"};
    std::thread bw([&]
            {
                FooType msg;
                msg.index(1);
                msg.message(test_message);

                // Allow main thread entering wait state, before sending
                std::this_thread::sleep_for(std::chrono::seconds(1));
                data_writer.write(&msg);
            });

    ConditionSeq triggered;
    EXPECT_EQ(ws.wait(triggered, 2.0), RETCODE_OK);
    bw.join();

    // Check the data is there
    EXPECT_EQ(data_reader.get_unread_count(), 1);

    // Check the conditions triggered were the expected ones
    ASSERT_FALSE(read_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), read_cond), triggered.end());
    ASSERT_TRUE(not_read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), not_read_cond), triggered.end());
    ASSERT_TRUE(any_read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_read_cond), triggered.end());

    // 2- Check READ_SAMPLE_STATE
    // Read sample from a background thread
    FooSeq datas;
    SampleInfoSeq infos;

    EXPECT_EQ(data_reader.read_w_condition(
                datas,
                infos,
                1,
                not_read_cond), RETCODE_OK);

    triggered.clear();
    EXPECT_EQ(ws.wait(triggered, 1.0), RETCODE_OK);

    // Check data is good
    ASSERT_TRUE(infos[0].valid_data);
    EXPECT_EQ(datas[0].index(), 1u);
    EXPECT_EQ(datas[0].message(), test_message);
    EXPECT_EQ(data_reader.return_loan(datas, infos), RETCODE_OK);

    // Check the conditions triggered were the expected ones
    ASSERT_TRUE(read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), read_cond), triggered.end());
    ASSERT_FALSE(not_read_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), not_read_cond), triggered.end());
    ASSERT_TRUE(any_read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_read_cond), triggered.end());

    // take the sample to check the API works
    EXPECT_EQ(data_reader.take_w_condition(
                datas,
                infos,
                1,
                read_cond), RETCODE_OK);

    // Check data is good
    ASSERT_TRUE(infos[0].valid_data);
    EXPECT_EQ(datas[0].index(), 1u);
    EXPECT_EQ(datas[0].message(), test_message);
    EXPECT_EQ(data_reader.return_loan(datas, infos), RETCODE_OK);

    // Detach conditions & destroy
    EXPECT_EQ(ws.detach_condition(*read_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(read_cond), RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*not_read_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(not_read_cond), RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*any_read_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(any_read_cond), RETCODE_OK);
}

TEST_F(DataReaderTests, read_conditions_wait_on_ViewStateMask)
{
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    create_entities(nullptr, reader_qos, SUBSCRIBER_QOS_DEFAULT, writer_qos);
    DataReader& data_reader = *data_reader_;
    DataWriter& data_writer = *data_writer_;

    // Condition masks
    SampleStateMask sample_states = ANY_SAMPLE_STATE;
    InstanceStateMask instance_states = ANY_INSTANCE_STATE;

    // Create the read conditions
    ReadCondition* view_cond = data_reader.create_readcondition(sample_states, NEW_VIEW_STATE, instance_states);
    EXPECT_NE(view_cond, nullptr);

    ReadCondition* not_view_cond =
            data_reader.create_readcondition(sample_states, NOT_NEW_VIEW_STATE, instance_states);
    EXPECT_NE(not_view_cond, nullptr);

    ReadCondition* any_view_cond = data_reader.create_readcondition(sample_states, ANY_VIEW_STATE, instance_states);
    EXPECT_NE(any_view_cond, nullptr);

    // Create the waitset and associate
    WaitSet ws;
    EXPECT_EQ(ws.attach_condition(*view_cond), RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*not_view_cond), RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*any_view_cond), RETCODE_OK);

    // 1- Check NEW_VIEW_STATE
    // Send sample from a background thread
    std::array<char, 256> test_message = {"Testing sample state"};
    std::thread bw([&]
            {
                FooType msg;
                msg.index(1);
                msg.message(test_message);

                // Allow main thread entering wait state, before sending
                std::this_thread::sleep_for(std::chrono::seconds(1));
                data_writer.write(&msg);
            });

    ConditionSeq triggered;
    EXPECT_EQ(ws.wait(triggered, 2.0), RETCODE_OK);
    bw.join();

    // Check the data is there
    EXPECT_EQ(data_reader.get_unread_count(), 1);

    // Check the conditions triggered were the expected ones
    ASSERT_TRUE(view_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), view_cond), triggered.end());
    ASSERT_FALSE(not_view_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), not_view_cond), triggered.end());
    ASSERT_TRUE(any_view_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_view_cond), triggered.end());

    // 2- Check NOT_NEW_VIEW_STATE
    // Read the sample in order to change instance view state
    FooSeq datas;
    SampleInfoSeq infos;

    EXPECT_EQ(data_reader.read_w_condition(
                datas,
                infos,
                1,
                view_cond), RETCODE_OK);

    triggered.clear();
    EXPECT_EQ(ws.wait(triggered, 1.0), RETCODE_OK);

    // Check data is good
    ASSERT_TRUE(infos[0].valid_data);
    EXPECT_EQ(datas[0].index(), 1u);
    EXPECT_EQ(datas[0].message(), test_message);
    EXPECT_EQ(data_reader.return_loan(datas, infos), RETCODE_OK);

    // Check the conditions triggered were the expected ones
    ASSERT_FALSE(view_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), view_cond), triggered.end());
    ASSERT_TRUE(not_view_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), not_view_cond), triggered.end());
    ASSERT_TRUE(any_view_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_view_cond), triggered.end());

    // Detach conditions & destroy
    EXPECT_EQ(ws.detach_condition(*view_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(view_cond), RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*not_view_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(not_view_cond), RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*any_view_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(any_view_cond), RETCODE_OK);
}

TEST_F(DataReaderTests, read_conditions_wait_on_InstanceStateMask)
{
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    create_entities(nullptr, reader_qos, SUBSCRIBER_QOS_DEFAULT, writer_qos);
    DataReader& data_reader = *data_reader_;
    DataWriter& data_writer = *data_writer_;

    // Condition masks
    SampleStateMask sample_states = ANY_SAMPLE_STATE;
    ViewStateMask view_states = ANY_VIEW_STATE;

    // Create the read conditions
    ReadCondition* alive_cond = data_reader.create_readcondition(sample_states, view_states, ALIVE_INSTANCE_STATE);
    EXPECT_NE(alive_cond, nullptr);

    ReadCondition* disposed_cond =
            data_reader.create_readcondition(sample_states, view_states, NOT_ALIVE_DISPOSED_INSTANCE_STATE);
    EXPECT_NE(disposed_cond, nullptr);

    ReadCondition* no_writer_cond = data_reader.create_readcondition(sample_states, view_states,
                    NOT_ALIVE_NO_WRITERS_INSTANCE_STATE);
    EXPECT_NE(no_writer_cond, nullptr);

    ReadCondition* any_cond = data_reader.create_readcondition(sample_states, view_states, ANY_INSTANCE_STATE);
    EXPECT_NE(any_cond, nullptr);

    // Create the waitset and associate
    WaitSet ws;
    EXPECT_EQ(ws.attach_condition(*alive_cond), RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*disposed_cond), RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*no_writer_cond), RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*any_cond), RETCODE_OK);

    // 1- Check ALIVE_INSTANCE_STATE
    // Send sample from a background thread
    std::array<char, 256> test_message = {"Testing sample state"};

    FooType msg;
    msg.index(1);
    msg.message(test_message);

    std::thread bw([&]
            {
                // Allow main thread entering wait state, before sending
                std::this_thread::sleep_for(std::chrono::seconds(1));
                data_writer.write(&msg);
            });

    ConditionSeq triggered;
    EXPECT_EQ(ws.wait(triggered, 2.0), RETCODE_OK);
    bw.join();

    // Check the data is there
    EXPECT_EQ(data_reader.get_unread_count(), 1);

    // Check the conditions triggered were the expected ones
    ASSERT_TRUE(alive_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), alive_cond), triggered.end());
    ASSERT_FALSE(disposed_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), disposed_cond), triggered.end());
    ASSERT_FALSE(no_writer_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), no_writer_cond), triggered.end());
    ASSERT_TRUE(any_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_cond), triggered.end());

    // 2 - Check NOT_ALIVE_DISPOSED_INSTANCE_STATE
    // unregister the instance
    EXPECT_EQ(data_writer.unregister_instance(&msg, HANDLE_NIL), RETCODE_OK);

    triggered.clear();
    EXPECT_EQ(ws.wait(triggered, 1.0), RETCODE_OK);

    // Check the conditions triggered were the expected ones
    ASSERT_FALSE(alive_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), alive_cond), triggered.end());
    ASSERT_TRUE(disposed_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), disposed_cond), triggered.end());
    ASSERT_FALSE(no_writer_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), no_writer_cond), triggered.end());
    ASSERT_TRUE(any_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_cond), triggered.end());

    // 4 - Check (read|take)_next_instance_w_condition() APIs
    // take the sample to check the API works
    FooSeq datas;
    SampleInfoSeq infos;
    EXPECT_EQ(data_reader.take_next_instance_w_condition(
                datas,
                infos,
                1,
                HANDLE_NIL,
                disposed_cond), RETCODE_OK);

    // Check data is bad because the sample for instance 1 was unregistered
    ASSERT_FALSE(infos[0].valid_data);
    InstanceHandle_t prev_handle = infos[0].instance_handle;
    EXPECT_EQ(data_reader.return_loan(datas, infos), RETCODE_OK);

    // new instance
    msg.index(2u);
    data_writer.write(&msg);

    EXPECT_EQ(data_reader.read_next_instance_w_condition(
                datas,
                infos,
                1,
                prev_handle,
                alive_cond), RETCODE_OK);

    // Check data is good
    ASSERT_TRUE(infos[0].valid_data);
    EXPECT_EQ(datas[0].index(), 2u);
    EXPECT_EQ(datas[0].message(), test_message);
    EXPECT_EQ(data_reader.return_loan(datas, infos), RETCODE_OK);

    // 5 - Check NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
    // delete the writer to remove all writers from a new instance
    ASSERT_EQ(publisher_->delete_datawriter(data_writer_), RETCODE_OK);
    data_writer_ = nullptr;

    triggered.clear();
    EXPECT_EQ(ws.wait(triggered, 1.0), RETCODE_OK);

    // Check the conditions triggered were the expected ones
    ASSERT_FALSE(alive_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), alive_cond), triggered.end());
    ASSERT_TRUE(disposed_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), disposed_cond), triggered.end());
    ASSERT_TRUE(no_writer_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), no_writer_cond), triggered.end());
    ASSERT_TRUE(any_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_cond), triggered.end());

    // Detach conditions & destroy
    EXPECT_EQ(ws.detach_condition(*alive_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(alive_cond), RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*disposed_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(disposed_cond), RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*no_writer_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(no_writer_cond), RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*any_cond), RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(any_cond), RETCODE_OK);
}

/*
 * This test checks the allocation consistency when NOT using instances.
 * If the topic is keyed,
 * max_samples should be greater or equal than max_instances * max_samples_per_instance.
 * If that condition is not met, the endpoint creation should fail.
 * If not keyed (not using instances), the only property that is used is max_samples,
 * thus, should not fail with the previously mentioned configuration.
 * The following method is checked:
 * 1. Subscriber::create_datareader
 * 2. DataReader::set_qos
 */
TEST_F(DataReaderTests, InstancePolicyAllocationConsistencyNotKeyed)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    // This test pretends to use topic with no instances, so the following flag is set false.
    type.get()->is_compute_key_provided = false;

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Next QoS config checks the default qos configuration,
    // create_datareader() should NOT return nullptr.
    DataReaderQos qos = DATAREADER_QOS_DEFAULT;

    DataReader* data_reader1 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader1, nullptr);

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value, and does not make any change.
    // Updated to check negative values (Redmine ticket #20722)
    qos.resource_limits().max_samples = -1;
    qos.resource_limits().max_instances = -1;
    qos.resource_limits().max_samples_per_instance = -1;

    DataReader* data_reader2 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader2, nullptr);

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // create_datareader() should NOT return nullptr.
    // By not using instances, instance allocation consistency is not checked.
    qos.resource_limits().max_samples = 4999;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataReader* data_reader3 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader3, nullptr);

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // create_datareader() should NOT return nullptr.
    // By not using instances, instance allocation consistency is not checked.
    qos.resource_limits().max_samples = 5001;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataReader* data_reader4 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader4, nullptr);

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // create_datareader() should NOT return nullptr.
    // By not using instances, instance allocation consistency is not checked.
    qos.resource_limits().max_samples = 0;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataReader* data_reader5 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader5, nullptr);

    // It is needed to disable the creation of enabled entities from the subscriber for following checks.
    // This allows to change inmutable policies
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    subscriber_qos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(RETCODE_OK, subscriber->set_qos(subscriber_qos));

    // Next QoS config checks the default qos configuration,
    // set_qos() should return RETCODE_OK = 0
    DataReaderQos qos2 = DATAREADER_QOS_DEFAULT;
    DataReader* default_data_reader2 = subscriber->create_datareader(topic, qos2);
    ASSERT_NE(default_data_reader2, nullptr);

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value.
    // By not using instances, instance allocation consistency is not checked.
    // Updated to check negative values (Redmine ticket #20722)
    qos2.resource_limits().max_samples = -1;
    qos2.resource_limits().max_instances = -1;
    qos2.resource_limits().max_samples_per_instance = -1;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0
    // By not using instances, instance allocation consistency is not checked.
    qos2.resource_limits().max_samples = 4999;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0
    // By not using instances, instance allocation consistency is not checked.
    qos2.resource_limits().max_samples = 5001;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // set_qos() should return RETCODE_OK = 0
    // By not using instances, instance allocation consistency is not checked.
    qos2.resource_limits().max_samples = 0;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));
}

/*
 * This test checks the allocation consistency when USING instances.
 * If the topic is keyed,
 * max_samples should be greater or equal than max_instances * max_samples_per_instance.
 * If that condition is not met, the endpoint creation should fail.
 * If not keyed (not using instances), the only property that is used is max_samples,
 * thus, should not fail with the previously mentioned configuration.
 * The following method is checked:
 * 1. Subscriber::create_datareader
 * 2. DataReader::set_qos
 */
TEST_F(DataReaderTests, InstancePolicyAllocationConsistencyKeyed)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    // This test pretends to use topic with instances, so the following flag is set.
    type.get()->is_compute_key_provided = true;

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Next QoS config checks the default qos configuration,
    // create_datareader() should not return nullptr.
    DataReaderQos qos = DATAREADER_QOS_DEFAULT;

    DataReader* data_reader1 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader1, nullptr);

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value.
    // Updated to check negative values (Redmine ticket #20722)
    qos.resource_limits().max_samples = -1;
    qos.resource_limits().max_instances = -1;
    qos.resource_limits().max_samples_per_instance = -1;

    DataReader* data_reader2 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader2, nullptr);

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // create_datareader() should return nullptr.
    qos.resource_limits().max_samples = 4999;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataReader* data_reader3 = subscriber->create_datareader(topic, qos);
    ASSERT_EQ(data_reader3, nullptr);

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // create_datareader() should not return nullptr.
    qos.resource_limits().max_samples = 5001;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataReader* data_reader4 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader4, nullptr);

    // Next QoS config checks that if user sets max_samples = ( max_instances * max_samples_per_instance ) ,
    // create_datareader() should not return nullptr.
    qos.resource_limits().max_samples = 5000;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataReader* data_reader5 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader5, nullptr);

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // create_datareader() should not return nullptr.
    qos.resource_limits().max_samples = 0;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataReader* data_reader6 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(data_reader6, nullptr);

    // It is needed to disable the creation of enabled entities from the subscriber for following checks.
    // This allows to change inmutable policies
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    subscriber_qos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(RETCODE_OK, subscriber->set_qos(subscriber_qos));

    // Next QoS config checks the default qos configuration,
    // set_qos() should return RETCODE_OK = 0, as the by default values are already infinite.
    DataReaderQos qos2 = DATAREADER_QOS_DEFAULT;
    DataReader* default_data_reader2 = subscriber->create_datareader(topic, qos2);
    ASSERT_NE(default_data_reader2, nullptr);

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value.
    // Updated to check negative values (Redmine ticket #20722)
    qos2.resource_limits().max_samples = -1;
    qos2.resource_limits().max_instances = -1;
    qos2.resource_limits().max_samples_per_instance = -1;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return a value != 0 (not OK)
    qos2.resource_limits().max_samples = 4999;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_NE(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0.
    qos2.resource_limits().max_samples = 5001;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples = ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0.
    qos2.resource_limits().max_samples = 5000;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // set_qos() should return RETCODE_OK = 0.
    qos2.resource_limits().max_samples = 0;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_reader2->set_qos(qos2));
}

/*
 * This test checks the proper behavior of the custom payload pool DataReader overload.
 */
TEST_F(DataReaderTests, CustomPoolCreation)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Next QoS config checks the default qos configuration,
    // create_datareader() should not return nullptr.
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;

    std::shared_ptr<CustomPayloadPool> payload_pool = std::make_shared<CustomPayloadPool>();

    DataReader* data_reader =
            subscriber->create_datareader(topic, reader_qos, nullptr, StatusMask::all(), payload_pool);

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;

    DataWriter* data_writer = publisher->create_datawriter(topic, writer_qos);

    FooType data;
    data.index(0);
    data.message()[0] = '\0';
    data.message()[1] = '\0';

    data_writer->write(&data, HANDLE_NIL);

    ASSERT_EQ(payload_pool->requested_payload_count, 1u);

    ASSERT_NE(data_reader, nullptr);

    participant->delete_contained_entities();

    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

// Check DataReaderQos inmutabilities
TEST_F(DataReaderTests, UpdateInmutableQos)
{
    /* Test setup */
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);

    /* Test actions */
    // Resource limits
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.resource_limits().max_samples = reader_qos.resource_limits().max_samples - 1;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // History
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.history().depth = reader_qos.history().depth + 1;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // Durability
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // Liveliness
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.liveliness().kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.liveliness().lease_duration = Duration_t{123, 123};
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.liveliness().announcement_period = Duration_t{123, 123};
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // Relibility
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // Ownsership
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // Destination order (currently reports unsupported)
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.destination_order().kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    ASSERT_EQ(RETCODE_UNSUPPORTED, data_reader->set_qos(reader_qos));

    // Reader resource limits
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reader_resource_limits().matched_publisher_allocation.maximum =
            reader_qos.reader_resource_limits().matched_publisher_allocation.maximum - 1;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // Datasharing
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.data_sharing().off();
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.data_sharing().automatic(".");
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.data_sharing().add_domain_id(static_cast<uint16_t>(12));
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.data_sharing().data_sharing_listener_thread().priority =
            reader_qos.data_sharing().data_sharing_listener_thread().priority + 1;
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    // Unique network flows
    reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.properties().properties().push_back({"fastdds.unique_network_flows", "true"});
    ASSERT_EQ(RETCODE_IMMUTABLE_POLICY, data_reader->set_qos(reader_qos));

    /* Cleanup */
    participant->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

TEST_F(DataReaderTests, history_depth_max_samples_per_instance_warning)
{

    /* Setup log so it may catch the expected warning */
    Log::ClearConsumers();
    MockConsumer* mockConsumer = new MockConsumer("RTPS_QOS_CHECK");
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
    Log::SetVerbosity(Log::Warning);

    /* Create a participant, topic, and a subscriber */
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0,
                    PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    /* Create a datareader with the QoS that should generate a warning */
    DataReaderQos qos;
    qos.history().depth = 10;
    qos.resource_limits().max_samples_per_instance = 5;
    DataReader* datareader_1 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(datareader_1, nullptr);

    /* Check that the config generated a warning */
    auto wait_for_log_entries =
            [&mockConsumer](const uint32_t amount, const uint32_t retries, const uint32_t wait_ms) -> size_t
            {
                size_t entries = 0;
                for (uint32_t i = 0; i < retries; i++)
                {
                    entries = mockConsumer->ConsumedEntries().size();
                    if (entries >= amount)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
                }
                return entries;
            };

    const size_t expected_entries = 1;
    const uint32_t retries = 4;
    const uint32_t wait_ms = 25;
    ASSERT_EQ(wait_for_log_entries(expected_entries, retries, wait_ms), expected_entries);

    /* Check that a correctly initialized datareader does not produce any warning */
    qos.history().depth = 10;
    qos.resource_limits().max_samples_per_instance = 10;
    DataReader* datareader_2 = subscriber->create_datareader(topic, qos);
    ASSERT_NE(datareader_2, nullptr);
    ASSERT_EQ(wait_for_log_entries(expected_entries, retries, wait_ms), expected_entries);

    /* Tear down */
    participant->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant);
    Log::KillThread();
}

struct LoanableType
{
    static constexpr uint32_t initialization_value()
    {
        return 27u;
    }

    uint32_t index = initialization_value();
};

class DataRepresentationTestsTypeSupport : public TopicDataType
{
public:

    typedef LoanableType type;

    DataRepresentationTestsTypeSupport()
        : TopicDataType()
    {
        max_serialized_type_size = 4u + sizeof(LoanableType);
        set_name("LoanableType");
    }

    bool serialize(
            const void* const /*data*/,
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return max_serialized_type_size;
    }

    void* create_data() override
    {
        return nullptr;
    }

    void delete_data(
            void* /*data*/) override
    {
    }

    bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool is_bounded() const override
    {
        return true;
    }

    MOCK_CONST_METHOD1(custom_is_plain_with_rep, bool(DataRepresentationId_t data_representation_id));

    bool is_plain(
            DataRepresentationId_t data_representation_id) const override
    {
        return custom_is_plain_with_rep(data_representation_id);
    }

};

TEST_F(DataReaderTests, data_type_is_plain_data_representation)
{
    /* Create a participant, topic, and a subscriber */
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0,
                    PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DataRepresentationTestsTypeSupport* type = new DataRepresentationTestsTypeSupport();
    TypeSupport ts (type);
    ts.register_type(participant);

    Topic* topic = participant->create_topic("plain_topic", "LoanableType", TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    /* Define XCDR1 only data representation QoS to force "is_plain" call */
    DataReaderQos qos_xcdr = DATAREADER_QOS_DEFAULT;
    qos_xcdr.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    qos_xcdr.representation().m_value.clear();
    qos_xcdr.representation().m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);

    /* Expect the "is_plain" method called with default data representation (XCDR1) */
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR_DATA_REPRESENTATION)).Times(
        testing::AtLeast(1)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION)).Times(0);

    /* Create a datareader will trigger the "is_plain" call */
    DataReader* datareader_xcdr = subscriber->create_datareader(topic, qos_xcdr);
    ASSERT_NE(datareader_xcdr, nullptr);

    testing::Mock::VerifyAndClearExpectations(&type);

    /* Define XCDR2 data representation QoS to force "is_plain" call */
    DataReaderQos qos_xcdr2 = DATAREADER_QOS_DEFAULT;
    qos_xcdr2.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    qos_xcdr2.representation().m_value.clear();
    qos_xcdr2.representation().m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    /* Expect the "is_plain" method called with XCDR2 data representation */
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR_DATA_REPRESENTATION)).Times(0);
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION)).Times(
        testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    /* Create a datareader will trigger the "is_plain" call */
    DataReader* datareader_xcdr2 = subscriber->create_datareader(topic, qos_xcdr2);
    ASSERT_NE(datareader_xcdr2, nullptr);

    /* NOT Define data representation QoS to force "is_plain" call */
    DataReaderQos qos_no_xcdr = DATAREADER_QOS_DEFAULT;
    qos_no_xcdr.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    qos_no_xcdr.representation().m_value.clear();

    /* Expect the "is_plain" method called with both data representation */
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR_DATA_REPRESENTATION)).Times(
        testing::AtLeast(1)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION)).Times(
        testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    /* Create a datareader will trigger the "is_plain" call */
    DataReader* datareader_no_xcdr = subscriber->create_datareader(topic, qos_no_xcdr);
    ASSERT_NE(datareader_no_xcdr, nullptr);

    /* Tear down */
    participant->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
