#include "mutex_testing_tool/TMutex.hpp"
#include <fastrtps/Domain.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastcdr/Cdr.h>

#include <cassert>
#include <future>
#include <chrono>
#include <gtest/gtest.h>

class DummyType:public eprosima::fastrtps::TopicDataType
{
    public:

        DummyType()
        {
            setName("DummyType");
            m_typeSize = 4 + 4 /*encapsulation*/;
            m_isGetKeyDefined = false;
        }

        DummyType(int32_t value) : DummyType()
        {
            value_ = value;
        }

        virtual ~DummyType() = default;

        bool serialize(
                void*data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload)
        {
            DummyType* sample = reinterpret_cast<DummyType*>(data);
            // Object that manages the raw buffer.
            eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
            // Object that serializes the data.
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                    eprosima::fastcdr::Cdr::DDS_CDR);
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            // Serialize encapsulation
            ser.serialize_encapsulation();
            //serialize the object:
            ser.serialize(sample->value_);
            payload->length = (uint32_t)ser.getSerializedDataLength();
            return true;
        }

        bool deserialize(
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                void * data)
        {
            DummyType* sample = reinterpret_cast<DummyType*>(data);
            // Object that manages the raw buffer.
            eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
            // Object that serializes the data.
            eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                    eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
            // Deserialize encapsulation.
            deser.read_encapsulation();
            payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            //serialize the object:
            deser.deserialize(sample->value_);
            return true;
        }

        std::function<uint32_t()> getSerializedSizeProvider(void*)
        {
            return []() -> uint32_t {
                return 4 + 4 /*encapsulation*/;
            };
        }

        bool getKey(
                void*,
                eprosima::fastrtps::rtps::InstanceHandle_t*,
                bool)
        {
            return false;
        }

        void* createData()
        {
            return reinterpret_cast<void*>(new DummyType());
        }

        void deleteData(void* data)
        {
            delete(reinterpret_cast<DummyType*>(data));
        }

    private:

        int32_t value_;
};

class UserThreadNonBlockedTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
            // Create participant
            eprosima::fastrtps::ParticipantAttributes participant_attr;
            participant_ = eprosima::fastrtps::Domain::createParticipant(participant_attr);
            assert(participant_);

            // Register type
            eprosima::fastrtps::Domain::registerType(participant_, &type_);

            // Create publisher
            publisher_attr_.topic.topicDataType = type_.getName();
            publisher_attr_.topic.topicName = "Dummy";
            publisher_ = eprosima::fastrtps::Domain::createPublisher(participant_, publisher_attr_, nullptr);
            assert(publisher_);

            // Create subscriber
            eprosima::fastrtps::SubscriberAttributes subscriber_attr;
            subscriber_attr.topic.topicDataType = type_.getName();
            subscriber_attr.topic.topicName = "Dummy";
            subscriber_ = eprosima::fastrtps::Domain::createSubscriber(participant_, subscriber_attr, nullptr);
            assert(subscriber_);

        }

        virtual void TearDown()
        {
            assert(participant_);
            eprosima::fastrtps::Domain::removeParticipant(participant_);
            participant_ = nullptr;
        }

    public:

        UserThreadNonBlockedTest() = default;

        eprosima::fastrtps::Participant* participant_;

        DummyType type_;

        eprosima::fastrtps::PublisherAttributes publisher_attr_;

        eprosima::fastrtps::Publisher* publisher_;

        eprosima::fastrtps::Subscriber* subscriber_;
};

TEST_F(UserThreadNonBlockedTest, write_sample)
{
    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    publisher_->write(reinterpret_cast<void*>(&sample));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(5, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(5, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    // Start testing locking the mutexes.
    eprosima::fastrtps::tmutex_lock_mutex(0);

    std::promise<std::pair<bool, std::chrono::microseconds>> promise;
    std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
    std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    bool returned_value = publisher_->write(reinterpret_cast<void*>(&sample));
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                                std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
    future.wait();
    auto returned_value = future.get();
    ASSERT_EQ(false, returned_value.first);
    std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                publisher_attr_.qos.m_reliability.max_blocking_time));
    ASSERT_GE(returned_value.second, max_w);
    ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
