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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class FooType
{
public:

    inline uint32_t index() const
    {
        return index_;
    }

    inline uint32_t& index()
    {
        return index_;
    }

    inline void index(
            uint32_t value)
    {
        index_ = value;
    }

    inline const std::array<char, 256>& message() const
    {
        return message_;
    }

    inline std::array<char, 256>& message()
    {
        return message_;
    }

    inline void message(
            const std::array<char, 256>& value)
    {
        message_ = value;
    }

    inline void serialize(
            eprosima::fastcdr::Cdr& scdr) const
    {
        scdr << index_;
        scdr << message_;
    }

    inline void deserialize(
            eprosima::fastcdr::Cdr& dcdr)
    {
        dcdr >> index_;
        dcdr >> message_;
    }

    inline bool isKeyDefined()
    {
        return true;
    }

    inline void serializeKey(
            eprosima::fastcdr::Cdr& scdr) const
    {
        scdr << index_;
    }

private:

    uint32_t index_ = 0;
    std::array<char, 256> message_;
};

class FooTypeSupport : public TopicDataType
{
public:

    FooTypeSupport()
        : TopicDataType()
    {
        setName("FooType");
        m_typeSize = 4u + 4u + 256u; // encapsulation + index + message
        m_isGetKeyDefined = true;
    }

    bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload) override
    {
        FooType* p_type = static_cast<FooType*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fb(reinterpret_cast<char*>(payload->data), payload->max_size);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fb, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
        payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        // Serialize encapsulation
        ser.serialize_encapsulation();

        try
        {
            // Serialize the object.
            p_type->serialize(ser);
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        // Get the serialized length
        payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override
    {
        //Convert DATA to pointer of your type
        FooType* p_type = static_cast<FooType*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fb(reinterpret_cast<char*>(payload->data), payload->length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fb, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        try
        {
            // Deserialize the object.
            p_type->deserialize(deser);
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return [this]
               {
                   return m_typeSize;
               };
    }

    void* createData() override
    {
        return static_cast<void*>(new FooType());
    }

    void deleteData(
            void* data) override
    {
        FooType* p_type = static_cast<FooType*>(data);
        delete p_type;
    }

    bool getKey(
            void* data,
            fastrtps::rtps::InstanceHandle_t* handle,
            bool force_md5) override
    {
        FooType* p_type = static_cast<FooType*>(data);
        char key_buf[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(key_buf, 16);

        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
        p_type->serializeKey(ser);
        if (force_md5)
        {
            MD5 md5;
            md5.init();
            md5.update(key_buf, static_cast<unsigned int>(ser.getSerializedDataLength()));
            md5.finalize();
            for (uint8_t i = 0; i < 16; ++i)
            {
                handle->value[i] = md5.digest[i];
            }
        }
        else
        {
            for (uint8_t i = 0; i < 16; ++i)
            {
                handle->value[i] = key_buf[i];
            }
        }
        return true;
    }

    inline bool is_bounded() const override
    {
        return true;
    }

    inline bool is_plain() const override
    {
        return true;
    }

    inline bool construct_sample(
            void* memory) const override
    {
        new (memory) FooType();
        return true;
    }

};

FASTDDS_SEQUENCE(FooSeq, FooType);

void check_return_loan(
        const ReturnCode_t& code,
        DataReader* data_reader,
        FooSeq& data_values,
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

void basic_read_apis_check(
        const ReturnCode_t& code,
        DataReader* data_reader)
{
    // Check read_next_sample
    {
        FooType data;
        SampleInfo info;

        EXPECT_EQ(code, data_reader->read_next_sample(&data, &info));
    }

    // Check read and variants with loan
    {
        FooSeq data_values;
        SampleInfoSeq infos;

        EXPECT_EQ(code, data_reader->read(data_values, infos));
        check_return_loan(code, data_reader, data_values, infos);
        EXPECT_EQ(code, data_reader->read_next_instance(data_values, infos));
        check_return_loan(code, data_reader, data_values, infos);
    }

    // Check read and variants without loan
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
        EXPECT_EQ(code, data_reader->read_next_instance(data_values, infos));
        check_return_loan(expected_return_loan_ret, data_reader, data_values, infos);
    }
}

TEST(DataReaderTests, ReadData)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // We will create a disabled DataReader, so we can check RETCODE_NOT_ENABLED
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    subscriber_qos.entity_factory().autoenable_created_entities = false;
    Subscriber* subscriber = participant->create_subscriber(subscriber_qos);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);
    EXPECT_FALSE(data_reader->is_enabled());

    // Read / take operations should all return NOT_ENABLED
    basic_read_apis_check(ReturnCode_t::RETCODE_NOT_ENABLED, data_reader);

    // Enable the DataReader and check NO_DATA should be returned
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, data_reader->enable());
    EXPECT_TRUE(data_reader->is_enabled());
    basic_read_apis_check(ReturnCode_t::RETCODE_NO_DATA, data_reader);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
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

TEST(DataReaderTests, SetListener)
{
    CustomListener listener;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new FooTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT, &listener);
    ASSERT_NE(datareader, nullptr);
    ASSERT_EQ(datareader->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<DataReader*, DataReaderListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { datareader, &listener, StatusMask::data_available() },
        { datareader, &listener, StatusMask::sample_rejected() },
        { datareader, &listener, StatusMask::liveliness_changed() },
        { datareader, &listener, StatusMask::requested_deadline_missed() },
        { datareader, &listener, StatusMask::requested_incompatible_qos() },
        { datareader, &listener, StatusMask::subscription_matched() },
        { datareader, &listener, StatusMask::sample_lost() },
        //all except one
        { datareader, &listener, StatusMask::all() >> StatusMask::data_available() },
        { datareader, &listener, StatusMask::all() >> StatusMask::sample_rejected() },
        { datareader, &listener, StatusMask::all() >> StatusMask::liveliness_changed() },
        { datareader, &listener, StatusMask::all() >> StatusMask::requested_deadline_missed() },
        { datareader, &listener, StatusMask::all() >> StatusMask::requested_incompatible_qos() },
        { datareader, &listener, StatusMask::all() >> StatusMask::subscription_matched() },
        { datareader, &listener, StatusMask::all() >> StatusMask::sample_lost() },
        //all and none
        { datareader, &listener, StatusMask::all() },
        { datareader, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }

    ASSERT_EQ(subscriber->delete_datareader(datareader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
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
