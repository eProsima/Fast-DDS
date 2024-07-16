#include <cassert>
#include <chrono>
#include <future>

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#include "mutex_testing_tool/TMutex.hpp"
#include <TimeConversion.hpp>

#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

const char* SLOW_FLOW_NAME = "SlowFlow";

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
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t* payload)
    {
        const DummyType* sample = static_cast<const DummyType*>(data);
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);
        payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        // Serialize encapsulation
        ser.serialize_encapsulation();
        //serialize the object:
        ser.serialize(sample->value_);
        payload->length = (uint32_t)ser.get_serialized_data_length();
        return true;
    }

    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t* payload,
            void* data)
    {
        DummyType* sample = reinterpret_cast<DummyType*>(data);
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer       // Deserialize encapsulation.
                );
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        //serialize the object:
        deser.deserialize(sample->value_);
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            const void* const)
    {
        return []() -> uint32_t
               {
                   return 4 + 4 /*encapsulation*/;
               };
    }

    bool getKey(
            const void* const,
            eprosima::fastdds::rtps::InstanceHandle_t*,
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

class UserThreadNonBlockedTest : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        // Disable SHM transport, DataSharing and Intraprocess
        auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        participant_qos_.transport().user_transports.push_back(udp_transport);
        participant_qos_.transport().use_builtin_transports = false;
        datawriter_qos_.data_sharing().off();
        datareader_qos_.data_sharing().off();
        eprosima::fastdds::LibrarySettings library_attributes;
        library_attributes.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_attributes);

        datareader_qos_.reliable_reader_qos().times.initial_acknack_delay.seconds = 10;
        datareader_qos_.reliable_reader_qos().times.heartbeat_response_delay.seconds = 10;

        // Slow flow-controller used in some test
        auto slow_flowcontroller = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
        slow_flowcontroller->name = SLOW_FLOW_NAME;
        slow_flowcontroller->period_ms = 10000;
        slow_flowcontroller->max_bytes_per_period = 1;
        participant_qos_.flow_controllers().push_back(slow_flowcontroller);
    }

    virtual void TearDown()
    {
        assert(participant_);
        participant_->delete_contained_entities();
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }

    void init()
    {
        // Create participant
        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            GET_PID() % 230, participant_qos_);
        assert(participant_);

        // Register type
        type_.register_type(participant_);

        eprosima::fastdds::dds::Publisher* publisher = participant_->create_publisher(
            eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
        assert(publisher);

        eprosima::fastdds::dds::Subscriber* subscriber = participant_->create_subscriber(
            eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
        assert(subscriber);

        eprosima::fastdds::dds::Topic* topic = participant_->create_topic("DummyType", type_->getName(),
                        eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        assert(topic);

        datawriter_ = publisher->create_datawriter(topic, datawriter_qos_);

        assert(datawriter_);

        datareader_ = subscriber->create_datareader(topic, datareader_qos_);

        assert(datareader_);
    }

public:

    UserThreadNonBlockedTest() = default;

    eprosima::fastdds::dds::DomainParticipantQos participant_qos_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::TypeSupport type_ {new DummyType()};

    eprosima::fastdds::dds::DataWriterQos datawriter_qos_;

    eprosima::fastdds::dds::DataWriter* datawriter_;

    eprosima::fastdds::dds::DataReaderQos datareader_qos_;

    eprosima::fastdds::dds::DataReader* datareader_;
};

/*!
 * @test Tests the mutexes involved in calling `DataWriter::write()` for publishing a new sample when there is another
 * one in the process of being sent
 * and it has to be removed.
 */
TEST_F(UserThreadNonBlockedTest, remove_previous_sample_on_history)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datawriter_qos_.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    datawriter_qos_.publish_mode().flow_controller_name = SLOW_FLOW_NAME;
    init();

    DummyType sample{1};

    datawriter_->write(reinterpret_cast<void*>(&sample));

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datawriter_->write(reinterpret_cast<void*>(&sample));

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<bool, std::chrono::microseconds>> promise;
            std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        bool returned_value = (RETCODE_OK == datawriter_->write(reinterpret_cast<void*>(&sample)));
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count <= 3 ? false : true, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datawriter_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(5, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(5, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataWriter::write()` for publishing a new sample using best-effort
 * reliability.
 */
TEST_F(UserThreadNonBlockedTest, write_sample_besteffort)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datawriter_->write(reinterpret_cast<void*>(&sample));

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<bool, std::chrono::microseconds>> promise;
            std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        bool returned_value = (RETCODE_OK == datawriter_->write(reinterpret_cast<void*>(&sample)));
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count == 0 ? false : true, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datawriter_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(2, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(2, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataWriter::write()` for publishing a new sample using reliable
 * reliability.
 */
TEST_F(UserThreadNonBlockedTest, write_sample_reliable)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datawriter_->write(reinterpret_cast<void*>(&sample));

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < 2; ++count)
    {
        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<bool, std::chrono::microseconds>> promise;
            std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        bool returned_value = (RETCODE_OK == datawriter_->write(reinterpret_cast<void*>(&sample)));
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count == 0 ? false : true, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datawriter_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(3, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(3, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataWriter::write()` for publishing a new sample using best-effort
 * reliability and asynchronous publication mode.
 */
TEST_F(UserThreadNonBlockedTest, write_async_sample_besteffort)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datawriter_qos_.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datawriter_->write(reinterpret_cast<void*>(&sample));

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<bool, std::chrono::microseconds>> promise;
            std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        bool returned_value = (RETCODE_OK == datawriter_->write(reinterpret_cast<void*>(&sample)));
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count == 0 ? false : true, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datawriter_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(3, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(3, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataWriter::write()` for publishing a new sample using reliable
 * reliability and asynchronous publication mode.
 */
TEST_F(UserThreadNonBlockedTest, write_async_sample_reliable)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datawriter_qos_.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1};

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datawriter_->write(reinterpret_cast<void*>(&sample));

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < 2; ++count)
    {
        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<bool, std::chrono::microseconds>> promise;
            std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        bool returned_value = (RETCODE_OK == datawriter_->write(reinterpret_cast<void*>(&sample)));
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count == 0 ? false : true, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datawriter_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(3, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(3, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataReader::read_next_sample()` for reading a new sample using
 * best-effort reliability.
 */
TEST_F(UserThreadNonBlockedTest, read_sample_besteffort)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter_->write(reinterpret_cast<void*>(&sample));
    datawriter_->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datareader_->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        datawriter_->write(reinterpret_cast<void*>(&sample));
        datawriter_->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<eprosima::fastdds::dds::ReturnCode_t, std::chrono::microseconds>> promise;
            std::future<std::pair<eprosima::fastdds::dds::ReturnCode_t,
                    std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        eprosima::fastdds::dds::ReturnCode_t returned_value =
                        datareader_->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit(std::pair<eprosima::fastdds::dds::ReturnCode_t,
                        std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count == 0 ? eprosima::fastdds::dds::RETCODE_NO_DATA :
                    eprosima::fastdds::dds::RETCODE_OK, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datareader_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataReader::read_next_sample()` for reading a new sample using
 * reliable reliability.
 */
TEST_F(UserThreadNonBlockedTest, read_sample_reliable)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter_->write(reinterpret_cast<void*>(&sample));
    datawriter_->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datareader_->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        datawriter_->write(reinterpret_cast<void*>(&sample));
        datawriter_->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<eprosima::fastdds::dds::ReturnCode_t, std::chrono::microseconds>> promise;
            std::future<std::pair<eprosima::fastdds::dds::ReturnCode_t,
                    std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        eprosima::fastdds::dds::ReturnCode_t returned_value =
                        datareader_->read_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<eprosima::fastdds::dds::ReturnCode_t,
                        std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(
                count == 0 ? eprosima::fastdds::dds::RETCODE_NO_DATA : eprosima::fastdds::dds::RETCODE_OK,
                returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datareader_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataReader::take_next_sample()` for taking a new sample using
 * best-effort reliability.
 */
TEST_F(UserThreadNonBlockedTest, take_sample_besteffort)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter_->write(reinterpret_cast<void*>(&sample));
    datawriter_->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datareader_->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        datawriter_->write(reinterpret_cast<void*>(&sample));
        datawriter_->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<eprosima::fastdds::dds::ReturnCode_t, std::chrono::microseconds>> promise;
            std::future<std::pair<eprosima::fastdds::dds::ReturnCode_t,
                    std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        eprosima::fastdds::dds::ReturnCode_t returned_value =
                        datareader_->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<eprosima::fastdds::dds::ReturnCode_t,
                        std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(
                count == 0 ? eprosima::fastdds::dds::RETCODE_NO_DATA : eprosima::fastdds::dds::RETCODE_OK,
                returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datareader_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataReader::take_next_sample()` for taking a new sample using
 * reliable reliability.
 */
TEST_F(UserThreadNonBlockedTest, take_sample_reliable)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter_->write(reinterpret_cast<void*>(&sample));
    datawriter_->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datareader_->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        datawriter_->write(reinterpret_cast<void*>(&sample));
        datawriter_->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<eprosima::fastdds::dds::ReturnCode_t, std::chrono::microseconds>> promise;
            std::future<std::pair<eprosima::fastdds::dds::ReturnCode_t,
                    std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        eprosima::fastdds::dds::ReturnCode_t returned_value =
                        datareader_->take_next_sample(reinterpret_cast<void*>(&read_sample), &read_info);
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<eprosima::fastdds::dds::ReturnCode_t,
                        std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(
                count == 0 ? eprosima::fastdds::dds::RETCODE_NO_DATA : eprosima::fastdds::dds::RETCODE_OK,
                returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datareader_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataReader::wait_for_unread_message()` for waiting a new sample using
 * best-effort reliability.
 */
TEST_F(UserThreadNonBlockedTest, wait_for_sample_besteffort)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter_->write(reinterpret_cast<void*>(&sample));
    datawriter_->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datareader_->wait_for_unread_message({0, 100000000});

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        datawriter_->write(reinterpret_cast<void*>(&sample));
        datawriter_->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<bool, std::chrono::microseconds>> promise;
            std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        bool returned_value = datareader_->wait_for_unread_message({0, 100000000});
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count == 0 ? false : true, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datareader_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

/*!
 * @test Tests the mutexes involved in calling `DataReader::wait_for_unread_message()` for waiting a new sample using
 * reliable reliability.
 */
TEST_F(UserThreadNonBlockedTest, wait_for_sample_reliable)
{
    datawriter_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos_.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    init();

    DummyType sample{1}, read_sample;
    eprosima::fastdds::dds::SampleInfo read_info;

    datawriter_->write(reinterpret_cast<void*>(&sample));
    datawriter_->wait_for_acknowledgments({0, 100000000});

    // Record the mutexes.
    eprosima::fastdds::tmutex_start_recording();

    datareader_->wait_for_unread_message({0, 100000000});

    eprosima::fastdds::tmutex_stop_recording();

    for (size_t count = 0; count < eprosima::fastdds::tmutex_get_num_mutexes(); ++count)
    {
        datawriter_->write(reinterpret_cast<void*>(&sample));
        datawriter_->wait_for_acknowledgments({0, 100000000});

        std::cout << "Testing mutex " << count << std::endl;
        // Start testing locking the mutexes.
        if (eprosima::fastdds::tmutex_lock_mutex(count))
        {
            std::promise<std::pair<bool, std::chrono::microseconds>> promise;
            std::future<std::pair<bool, std::chrono::microseconds>> future = promise.get_future();
            std::thread([&]
                    {
                        auto now = std::chrono::steady_clock::now();
                        bool returned_value = datareader_->wait_for_unread_message({0, 100000000});
                        auto end = std::chrono::steady_clock::now();
                        promise.set_value_at_thread_exit( std::pair<bool, std::chrono::microseconds>(returned_value,
                        std::chrono::duration_cast<std::chrono::microseconds>(end - now)));
                    }).detach();
            future.wait();
            auto returned_value = future.get();
            // If main mutex cannot be taken, the write fails.
            // But for the rest the information is stored and it is as if the samples was sent.
            ASSERT_EQ(count == 0 ? false : true, returned_value.first);
            std::chrono::microseconds max_w(eprosima::fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(
                        datareader_qos_.reliability().max_blocking_time));
            ASSERT_GE(returned_value.second, max_w);
            ASSERT_LE(returned_value.second - max_w, std::chrono::milliseconds(1));

            eprosima::fastdds::tmutex_unlock_mutex(count);
        }
        else
        {
            std::cout << "Mutex " << count << " is not a timed lock. Pass.." << std::endl;
        }
    }

    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_mutexes());
    ASSERT_EQ(0, eprosima::fastdds::tmutex_get_num_lock_type());
    ASSERT_EQ(1, eprosima::fastdds::tmutex_get_num_timedlock_type());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
