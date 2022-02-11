#include "mutex_testing_tool/TMutex.hpp"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastrtps/utils/TimeConversion.h>
#include <fastcdr/Cdr.h>

#include <cassert>
#include <future>
#include <chrono>
#include <gtest/gtest.h>

class DummyType : public eprosima::fastdds::dds::TopicDataType
{
public:

    DummyType()
    {
        setName("DummyType");
        m_typeSize = 4 + 4 /*encapsulation*/;
        m_isGetKeyDefined = false;
    }

    DummyType(
            int32_t value)
        : DummyType()
    {
        value_ = value;
    }

    virtual ~DummyType() = default;

    bool serialize(
            void* data,
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
            void* data)
    {
        DummyType* sample = reinterpret_cast<DummyType*>(data);
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastcdr::Cdr::DDS_CDR);     // Object that deserializes the data.
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        //serialize the object:
        deser.deserialize(sample->value_);
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void*)
    {
        return []() -> uint32_t
               {
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

    void deleteData(
            void* data)
    {
        delete(reinterpret_cast<DummyType*>(data));
    }

private:

    int32_t value_;
};

static const char* const topic_name = "Dummy";

class UserThreadNonBlockedTest : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        datareader_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        datareader_qos.reliable_reader_qos().times.initialAcknackDelay.seconds = 10;
        datareader_qos.reliable_reader_qos().times.heartbeatResponseDelay.seconds = 10;
    }

    virtual void TearDown()
    {
        assert(participant);
        participant->delete_contained_entities();
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
        participant = nullptr;
    }

    void init()
    {
        // Create participant
        participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0,
                        participant_qos);
        assert(participant);

        // Register type
        assert(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK == participant->register_type(type));

        topic = participant->create_topic(topic_name, type.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        assert(topic);

        publisher = participant->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
        assert(publisher);

        subscriber = participant->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
        assert(subscriber);

        datawriter = publisher->create_datawriter(topic, datawriter_qos);
        assert(datawriter);

        datareader = subscriber->create_datareader(topic, datareader_qos);
        assert(datareader);
    }

public:

    UserThreadNonBlockedTest() = default;

    eprosima::fastdds::dds::DomainParticipantQos participant_qos;

    eprosima::fastdds::dds::DomainParticipant* participant = nullptr;

    eprosima::fastdds::dds::Publisher* publisher = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber = nullptr;

    eprosima::fastdds::dds::TypeSupport type = eprosima::fastdds::dds::TypeSupport(new DummyType());

    eprosima::fastdds::dds::Topic* topic = nullptr;

    eprosima::fastdds::dds::DataWriterQos datawriter_qos;

    eprosima::fastdds::dds::DataWriter* datawriter = nullptr;

    eprosima::fastdds::dds::DataReaderQos datareader_qos;

    eprosima::fastdds::dds::DataReader* datareader = nullptr;
};

TEST_F(UserThreadNonBlockedTest, write_sample_besteffort)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(5, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(5, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes(); ++count)
    {
        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<bool, std::chrono::microseconds>> promise;
        std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    bool returned_value = datawriter->write(reinterpret_cast<void*>(&sample));
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If one of the two first mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        ASSERT_EQ(count < 2 ? false : true, returned_value.first);
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datawriter_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);
    }
}

TEST_F(UserThreadNonBlockedTest, write_sample_reliable)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(6, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(6, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes(); ++count)
    {
        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<bool, std::chrono::microseconds>> promise;
        std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    bool returned_value = datawriter->write(reinterpret_cast<void*>(&sample));
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If one of the two first mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        ASSERT_EQ(count < 2 ? false : true, returned_value.first);
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datawriter_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);
    }
}

TEST_F(UserThreadNonBlockedTest, write_async_sample_besteffort)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datawriter_qos.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes(); ++count)
    {
        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<bool, std::chrono::microseconds>> promise;
        std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    bool returned_value = datawriter->write(reinterpret_cast<void*>(&sample));
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If one of the two first mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        ASSERT_EQ(count < 2 ? false : true, returned_value.first);
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datawriter_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);
    }
}

TEST_F(UserThreadNonBlockedTest, write_async_sample_reliable)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datawriter_qos.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < 2; ++count)
    {
        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<bool, std::chrono::microseconds>> promise;
        std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    bool returned_value = datawriter->write(reinterpret_cast<void*>(&sample));
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If one of the two first mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        ASSERT_EQ(count < 2 ? false : true, returned_value.first);
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datawriter_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);
    }
}

void wait_sample_received(
        eprosima::fastdds::dds::DataReader* data_reader)
{
    eprosima::fastdds::dds::WaitSet wait_set;
    eprosima::fastdds::dds::StatusCondition& condition = data_reader->get_statuscondition();
    condition.set_enabled_statuses(eprosima::fastdds::dds::StatusMask::data_available());
    wait_set.attach_condition(condition);

    eprosima::fastdds::dds::ConditionSeq active_conditions;
    if (eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK == wait_set.wait(active_conditions, {10, 0}))
    {
        ASSERT_EQ(&condition, active_conditions.at(0));
        ASSERT_TRUE(active_conditions.at(0)->get_trigger_value());
    }
    else
    {
        std::cout << "No data this time" << std::endl;
    }
}

TEST_F(UserThreadNonBlockedTest, read_sample_besteffort)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
    wait_sample_received(datareader);

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK,
            datareader->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes(); ++count)
    {
        ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
        wait_sample_received(datareader);

        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<eprosima::fastrtps::types::ReturnCode_t, std::chrono::microseconds>> promise;
        std::future<std::pair<eprosima::fastrtps::types::ReturnCode_t,
                std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    eprosima::fastrtps::types::ReturnCode_t returned_value =
                    datareader->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<eprosima::fastrtps::types::ReturnCode_t,
                    std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If main mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        if (0 == count)
        {
            ASSERT_NE(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        else
        {
            ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datareader_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);

        // If failed in the test, read the data for next loop.
        datareader->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
    }
}

TEST_F(UserThreadNonBlockedTest, read_sample_reliable)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
    wait_sample_received(datareader);

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK,
            datareader->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(3, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes(); ++count)
    {
        ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
        wait_sample_received(datareader);

        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<eprosima::fastrtps::types::ReturnCode_t, std::chrono::microseconds>> promise;
        std::future<std::pair<eprosima::fastrtps::types::ReturnCode_t,
                std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    eprosima::fastrtps::types::ReturnCode_t returned_value =
                    datareader->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<eprosima::fastrtps::types::ReturnCode_t,
                    std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If main mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        if (0 == count)
        {
            ASSERT_NE(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        else
        {
            ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datareader_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);

        // If failed in the test, read the data for next loop.
        datareader->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
    }
}

TEST_F(UserThreadNonBlockedTest, take_sample_besteffort)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
    wait_sample_received(datareader);

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK,
            datareader->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(4, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(4, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    // Not run last one because cause an assertion on TopicPayloadPool because one change was not correctly free.
    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes() - 1; ++count)
    {
        ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
        wait_sample_received(datareader);

        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<eprosima::fastrtps::types::ReturnCode_t, std::chrono::microseconds>> promise;
        std::future<std::pair<eprosima::fastrtps::types::ReturnCode_t,
                std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    eprosima::fastrtps::types::ReturnCode_t returned_value =
                    datareader->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<eprosima::fastrtps::types::ReturnCode_t,
                    std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If main mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        if (0 == count)
        {
            ASSERT_NE(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        else
        {
            ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datareader_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);

        // If failed in the test, read the data for next loop.
        datareader->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
    }
}

TEST_F(UserThreadNonBlockedTest, take_sample_reliable)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
    wait_sample_received(datareader);

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK,
            datareader->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info));

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(4, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(4, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    // Not run last one because cause an assertion on TopicPayloadPool because one change was not correctly free.
    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes() - 1; ++count)
    {
        ASSERT_TRUE(datawriter->write(reinterpret_cast<void*>(&sample)));
        wait_sample_received(datareader);

        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<eprosima::fastrtps::types::ReturnCode_t, std::chrono::microseconds>> promise;
        std::future<std::pair<eprosima::fastrtps::types::ReturnCode_t,
                std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    eprosima::fastrtps::types::ReturnCode_t returned_value =
                    datareader->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<eprosima::fastrtps::types::ReturnCode_t,
                    std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        // If main mutex cannot be taken, the write fails.
        // But for the rest the information is stored and it is as if the samples was sent.
        if (0 == count)
        {
            ASSERT_NE(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        else
        {
            ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK, returned_value.first);
        }
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datareader_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);

        // If failed in the test, read the data for next loop.
        datareader->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
    }
}

TEST_F(UserThreadNonBlockedTest, wait_for_sample_besteffort)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter->write(reinterpret_cast<void*>(&sample));
    datawriter->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    datareader->wait_for_unread_message({0, 100000000});

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(1, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes(); ++count)
    {
        datawriter->write(reinterpret_cast<void*>(&sample));
        datawriter->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<bool, std::chrono::microseconds>> promise;
        std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    bool returned_value = datareader->wait_for_unread_message({0, 100000000});
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        ASSERT_FALSE(returned_value.first);
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datareader_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);
    }
}

TEST_F(UserThreadNonBlockedTest, wait_for_sample_reliable)
{
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter->write(reinterpret_cast<void*>(&sample));
    datawriter->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastrtps::tmutex_start_recording();

    datareader->wait_for_unread_message({0, 100000000});

    eprosima::fastrtps::tmutex_stop_recording();

    ASSERT_EQ(1, eprosima::fastrtps::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastrtps::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastrtps::tmutex_get_num_timedlock_type());

    for (size_t count = 0; count < eprosima::fastrtps::tmutex_get_num_mutexes(); ++count)
    {
        datawriter->write(reinterpret_cast<void*>(&sample));
        datawriter->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << ": " << eprosima::fastrtps::tmux_get_mutex_trace(count) << std::endl;
        // Start testing locking the mutexes.
        eprosima::fastrtps::tmutex_lock_mutex(count);

        std::promise<std::pair<bool, std::chrono::microseconds>> promise;
        std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
        std::thread([&]
                {
                    auto now = std::chrono::steady_clock::now();
                    bool returned_value = datareader->wait_for_unread_message({0, 100000000});
                    auto end = std::chrono::steady_clock::now();
                    promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                    std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                }).detach();
        future.wait();
        auto returned_value = future.get();
        ASSERT_FALSE(returned_value.first);
        std::chrono::microseconds max_w(eprosima::fastrtps::rtps::TimeConv::Time_t2MicroSecondsInt64(
                    datareader_qos.reliability().max_blocking_time));
        ASSERT_GE(returned_value.second, max_w);
        ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

        eprosima::fastrtps::tmutex_unlock_mutex(count);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
