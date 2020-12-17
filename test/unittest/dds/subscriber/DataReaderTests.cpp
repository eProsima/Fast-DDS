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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>

#include <fastdds/dds/core/LoanableArray.hpp>
#include <fastdds/dds/core/StackAllocatedSequence.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

#include "./FooType.hpp"
#include "./FooTypeSupport.hpp"

static constexpr LoanableCollection::size_type num_test_elements = 10u;

FASTDDS_SEQUENCE(FooSeq, FooType);
using FooArray = LoanableArray<FooType, num_test_elements>;
using FooStack = StackAllocatedSequence<FooType, num_test_elements>;
using SampleInfoArray = LoanableArray<SampleInfo, num_test_elements>;

class DataReaderTests : public testing::Test
{
public:

    void SetUp() override
    {
        type_.reset(new FooTypeSupport());
    }

    void TearDown() override
    {
        if (data_writer_)
        {
            ASSERT_EQ(publisher_->delete_datawriter(data_writer_), ReturnCode_t::RETCODE_OK);
        }
        if (data_reader_)
        {
            ASSERT_EQ(subscriber_->delete_datareader(data_reader_), ReturnCode_t::RETCODE_OK);
        }
        if (topic_)
        {
            ASSERT_EQ(participant_->delete_topic(topic_), ReturnCode_t::RETCODE_OK);
        }
        if (publisher_)
        {
            ASSERT_EQ(participant_->delete_publisher(publisher_), ReturnCode_t::RETCODE_OK);
        }
        if (subscriber_)
        {
            ASSERT_EQ(participant_->delete_subscriber(subscriber_), ReturnCode_t::RETCODE_OK);
        }
        if (participant_)
        {
            auto factory = DomainParticipantFactory::get_instance();
            ASSERT_EQ(factory->delete_participant(participant_), ReturnCode_t::RETCODE_OK);
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
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, part_qos);
        ASSERT_NE(participant_, nullptr);

        subscriber_ = participant_->create_subscriber(sqos);
        ASSERT_NE(subscriber_, nullptr);

        publisher_ = participant_->create_publisher(pqos);
        ASSERT_NE(publisher_, nullptr);

        type_.register_type(participant_);

        topic_ = participant_->create_topic("footopic", type_.get_type_name(), tqos);
        ASSERT_NE(topic_, nullptr);

        data_reader_ = subscriber_->create_datareader(topic_, rqos, rlistener);
        ASSERT_NE(data_reader_, nullptr);

        data_writer_ = publisher_->create_datawriter(topic_, wqos);
        ASSERT_NE(data_reader_, nullptr);
    }

    void create_instance_handles()
    {
        FooType data;

        data.index(1);
        type_.get_key(&data, &handle_ok_);

        data.index(2);
        type_.get_key(&data, &handle_wrong_);
    }

    DomainParticipant* participant_ = nullptr;
    Subscriber* subscriber_ = nullptr;
    Publisher* publisher_ = nullptr;
    Topic* topic_ = nullptr;
    DataReader* data_reader_ = nullptr;
    DataWriter* data_writer_ = nullptr;
    TypeSupport type_;

    InstanceHandle_t handle_ok_ = HANDLE_NIL;
    InstanceHandle_t handle_wrong_ = HANDLE_NIL;

    void reset_lengths(
            LoanableCollection& data_values,
            SampleInfoSeq& infos)
    {
        data_values.length(0u);
        infos.length(0u);
    }

    void send_data(
            LoanableCollection& data_values,
            SampleInfoSeq& infos)
    {
        LoanableCollection::size_type n = infos.length();
        auto buffer = data_values.buffer();
        for (LoanableCollection::size_type i = 0u; i < n; ++i)
        {
            if (infos[i].valid_data)
            {
                EXPECT_EQ(ReturnCode_t::RETCODE_OK, data_writer_->write(buffer[i], HANDLE_NIL));
            }
        }
    }

    void check_return_loan(
            const ReturnCode_t& code,
            DataReader* data_reader,
            LoanableCollection& data_values,
            SampleInfoSeq& infos)
    {
        ReturnCode_t expected_return_loan_ret = code;
        if (ReturnCode_t::RETCODE_NO_DATA == code)
        {
            // When read returns NO_DATA, no loan will be performed
            expected_return_loan_ret = ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }
        EXPECT_EQ(expected_return_loan_ret, data_reader->return_loan(data_values, infos));

        if (ReturnCode_t::RETCODE_OK == expected_return_loan_ret)
        {
            EXPECT_TRUE(data_values.has_ownership());
            EXPECT_EQ(0u, data_values.maximum());
            EXPECT_TRUE(infos.has_ownership());
            EXPECT_EQ(0u, infos.maximum());
        }
    }

    void check_instance_methods(
            const ReturnCode_t& instance_ok_code,
            const ReturnCode_t& instance_bad_code,
            const ReturnCode_t& loan_return_code,
            DataReader* data_reader,
            LoanableCollection& data_values,
            SampleInfoSeq& infos)
    {
        EXPECT_EQ(instance_bad_code, data_reader->read_instance(data_values, infos, LENGTH_UNLIMITED, HANDLE_NIL));
        check_return_loan(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, data_reader, data_values, infos);
        EXPECT_EQ(instance_bad_code, data_reader->read_instance(data_values, infos, LENGTH_UNLIMITED, handle_wrong_));
        check_return_loan(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, data_reader, data_values, infos);
        EXPECT_EQ(instance_bad_code, data_reader->take_instance(data_values, infos, LENGTH_UNLIMITED, HANDLE_NIL));
        check_return_loan(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, data_reader, data_values, infos);
        EXPECT_EQ(instance_bad_code, data_reader->take_instance(data_values, infos, LENGTH_UNLIMITED, handle_wrong_));
        check_return_loan(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, data_reader, data_values, infos);

        EXPECT_EQ(instance_ok_code, data_reader->read_instance(data_values, infos, LENGTH_UNLIMITED, handle_ok_));
        check_return_loan(loan_return_code, data_reader, data_values, infos);
        reset_lengths(data_values, infos);

        EXPECT_EQ(instance_ok_code, data_reader->take_instance(data_values, infos, LENGTH_UNLIMITED, handle_ok_));
        if (ReturnCode_t::RETCODE_OK == instance_ok_code)
        {
            // Write received data so it can be taken again
            send_data(data_values, infos);
        }
        check_return_loan(loan_return_code, data_reader, data_values, infos);
        reset_lengths(data_values, infos);
    }

    void basic_read_apis_check(
            const ReturnCode_t& code,
            DataReader* data_reader)
    {
        static const Duration_t time_to_wait(1, 0);

        // Check read_next_sample / take_next_sample
        {
            FooType data;
            SampleInfo info;

            EXPECT_EQ(code, data_reader->read_next_sample(&data, &info));
            EXPECT_EQ(code, data_reader->take_next_sample(&data, &info));
            if (ReturnCode_t::RETCODE_OK == code)
            {
                // Send taken sample so it can be read again
                data_writer_->write(&data);
                data_reader->wait_for_unread_message(time_to_wait);
            }
        }

        // Return code when requesting a bad instance
        ReturnCode_t instance_bad_code = ReturnCode_t::RETCODE_BAD_PARAMETER;
        if (ReturnCode_t::RETCODE_NOT_ENABLED == code)
        {
            instance_bad_code = code;
        }

        // Return code when requesting a correct instance
        ReturnCode_t instance_ok_code = instance_bad_code;
        if (ReturnCode_t::RETCODE_OK == code)
        {
            instance_ok_code = code;
        }

        // Check read/take and variants with loan
        {
            FooSeq data_values;
            SampleInfoSeq infos;

            EXPECT_EQ(code, data_reader->read(data_values, infos));
            check_return_loan(code, data_reader, data_values, infos);
            reset_lengths(data_values, infos);
            EXPECT_EQ(code, data_reader->read_next_instance(data_values, infos));
            check_return_loan(code, data_reader, data_values, infos);
            reset_lengths(data_values, infos);

            EXPECT_EQ(code, data_reader->take(data_values, infos));
            if (ReturnCode_t::RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(code, data_reader, data_values, infos);
            reset_lengths(data_values, infos);

            EXPECT_EQ(code, data_reader->take_next_instance(data_values, infos));
            if (ReturnCode_t::RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(code, data_reader, data_values, infos);
            reset_lengths(data_values, infos);

            check_instance_methods(instance_ok_code, instance_bad_code, instance_ok_code,
                    data_reader, data_values, infos);
        }

        // Check read/take and variants without loan
        {
            FooSeq data_values(1u);
            SampleInfoSeq infos(1u);

            ReturnCode_t expected_return_loan_ret = code;
            if (ReturnCode_t::RETCODE_OK == code)
            {
                // Even when read returns data, no loan will be performed
                expected_return_loan_ret = ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
            }

            EXPECT_EQ(code, data_reader->read(data_values, infos));
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos);
            reset_lengths(data_values, infos);
            EXPECT_EQ(code, data_reader->read_next_instance(data_values, infos));
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos);
            reset_lengths(data_values, infos);

            EXPECT_EQ(code, data_reader->take(data_values, infos));
            if (ReturnCode_t::RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos);
            reset_lengths(data_values, infos);

            EXPECT_EQ(code, data_reader->take_next_instance(data_values, infos));
            if (ReturnCode_t::RETCODE_OK == code)
            {
                send_data(data_values, infos);
                data_reader->wait_for_unread_message(time_to_wait);
            }
            check_return_loan(expected_return_loan_ret, data_reader, data_values, infos);
            reset_lengths(data_values, infos);

            check_instance_methods(instance_ok_code, instance_bad_code, expected_return_loan_ret,
                    data_reader, data_values, infos);
        }
    }

};

TEST_F(DataReaderTests, ReadData)
{
    // We will create a disabled DataReader, so we can check RETCODE_NOT_ENABLED
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    subscriber_qos.entity_factory().autoenable_created_entities = false;
    create_entities(nullptr, DATAREADER_QOS_DEFAULT, subscriber_qos);
    EXPECT_FALSE(data_reader_->is_enabled());

    // Read / take operations should all return NOT_ENABLED
    basic_read_apis_check(ReturnCode_t::RETCODE_NOT_ENABLED, data_reader_);

    // Enable the DataReader and check NO_DATA should be returned
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, data_reader_->enable());
    EXPECT_TRUE(data_reader_->is_enabled());
    basic_read_apis_check(ReturnCode_t::RETCODE_NO_DATA, data_reader_);

    // Send data
    FooType data;
    data.index(1u);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, data_writer_->write(&data, HANDLE_NIL));

    // Wait for data to arrive and check OK should be returned
    Duration_t wait_time(1, 0);
    EXPECT_TRUE(data_reader_->wait_for_unread_message(wait_time));
    basic_read_apis_check(ReturnCode_t::RETCODE_OK, data_reader_);
}

void check_collection(
        const LoanableCollection& collection,
        bool owns,
        LoanableCollection::size_type max,
        LoanableCollection::size_type len)
{
    EXPECT_EQ(owns, collection.has_ownership());
    EXPECT_EQ(max, collection.maximum());
    EXPECT_EQ(len, collection.length());
}

TEST_F(DataReaderTests, collection_preconditions)
{
    create_entities();
    create_instance_handles();

    const ReturnCode_t& ok_code = ReturnCode_t::RETCODE_NO_DATA;
    const ReturnCode_t& wrong_code = ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;

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
    true_10_1.length(1u);
    false_10_0.loan(arr.buffer_for_loans(), num_test_elements, 0u);
    false_10_1.loan(arr.buffer_for_loans(), num_test_elements, 1u);
    info_true_10_1.length(1u);
    info_false_10_0.loan(info_arr.buffer_for_loans(), num_test_elements, 0u);
    info_false_10_1.loan(info_arr.buffer_for_loans(), num_test_elements, 1u);

    // Check we did it right
    check_collection(true_0_0, true, 0u, 0u);
    check_collection(true_10_0, true, 10u, 0u);
    check_collection(true_10_1, true, 10u, 1u);
    check_collection(false_10_0, false, 10u, 0u);
    check_collection(false_10_1, false, 10u, 1u);
    check_collection(info_true_0_0, true, 0u, 0u);
    check_collection(info_true_10_0, true, 10u, 0u);
    check_collection(info_true_10_1, true, 10u, 1u);
    check_collection(info_false_10_0, false, 10u, 0u);
    check_collection(info_false_10_1, false, 10u, 1u);

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
    using ok_test_case_t = std::pair<test_case_t, const ReturnCode_t&>;
    std::vector<ok_test_case_t> ok_cases
    {
        // max == 0. Loaned data will be returned.
        { {true_0_0, info_true_0_0}, ok_code},
        // max > 0 && owns == true. Data will be copied.
        { {true_10_0, info_true_10_0}, ok_code},
        { {true_10_1, info_true_10_1}, ok_code},
        // max > 0 && owns == false. Precondition not met.
        { {false_10_0, info_false_10_0}, wrong_code},
        { {false_10_1, info_false_10_1}, wrong_code}
    };

    const ReturnCode_t& instance_bad_code = ReturnCode_t::RETCODE_BAD_PARAMETER;
    for (const ok_test_case_t& test : ok_cases)
    {
        EXPECT_EQ(test.second, data_reader_->read(test.first.first, test.first.second));
        EXPECT_EQ(wrong_code, data_reader_->return_loan(test.first.first, test.first.second));
        EXPECT_EQ(test.second, data_reader_->read_next_instance(test.first.first, test.first.second));
        EXPECT_EQ(wrong_code, data_reader_->return_loan(test.first.first, test.first.second));
        EXPECT_EQ(test.second, data_reader_->take(test.first.first, test.first.second));
        EXPECT_EQ(wrong_code, data_reader_->return_loan(test.first.first, test.first.second));
        EXPECT_EQ(test.second, data_reader_->take_next_instance(test.first.first, test.first.second));
        EXPECT_EQ(wrong_code, data_reader_->return_loan(test.first.first, test.first.second));

        // When collection preconditions are ok, as the reader has no data, BAD_PARAMETER will be returned
        const ReturnCode_t& instance_code = (test.second == ok_code) ? instance_bad_code : test.second;
        check_instance_methods(instance_code, instance_code, wrong_code,
                data_reader_, test.first.first, test.first.second);
    }

    false_10_0.unloan();
    false_10_1.unloan();
    info_false_10_0.unloan();
    info_false_10_1.unloan();
}

void set_listener_test (
        DataReader* reader,
        DataReaderListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(reader->set_listener(listener, mask), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(reader->get_status_mask(), mask);
}

class CustomListener : public DataReaderListener
{

};

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

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
