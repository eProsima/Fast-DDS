/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <mutex>
#include <thread>
#include <utility>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#include <rtps/flowcontrol/GrainedFlowController.hpp>

#include "FlowControllerFactoryMock.hpp"
#include <HelloWorldPubSubTypes.hpp>
#include <Data1mbPubSubTypes.hpp>

#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

using namespace ::testing;

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

uint32_t domain_id {0};
const char* certs_path {nullptr};

class WListener : public DataWriterListener
{
public:

    void on_publication_matched(
            DataWriter*,
            const PublicationMatchedStatus& info) override
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            matched_readers_ = info.total_count;
        }
        cv_.notify_one();
    }

    bool wait_for_matched_readers(
            uint32_t expected_readers,
            uint32_t timeout_ms)
    {
        bool ret {false};
        std::unique_lock<std::mutex> lock(mutex_);
        auto cv_ret = cv_.wait_for(
            lock,
            std::chrono::milliseconds(timeout_ms),
            [this, expected_readers]()
            {
                return matched_readers_ >= expected_readers;
            });
        if (cv_ret)
        {
            ret = true;
        }
        else
        {
            std::cout << "Timeout reached. Matched readers: " << matched_readers_ << std::endl;
        }
        return ret;
    }

private:

    std::mutex mutex_;

    std::condition_variable cv_;

    uint32_t matched_readers_ {0};
};

template<typename T>
class Listener : public DataReaderListener
{
public:

    void print_data(
            HelloWorld& hello)
    {
        std::cout << "Message: '" << hello.message() << "' with index: '" << hello.index() << "' RECEIVED"
                  << std::endl;
    }

    void print_data(
            Data1mb& data)
    {
        std::cout << "Data: '" << static_cast<uint16_t>(data.data().at(0)) << "' RECEIVED"
                  << std::endl;
    }

    void on_data_available(
            DataReader* reader) override
    {
        SampleInfo info;
        T data;
        while (reader->take_next_sample(&data, &info) == RETCODE_OK)
        {
            if (info.valid_data)
            {
                print_data(data);
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (RELIABLE_RELIABILITY_QOS == reader->get_qos().reliability().kind)
                    {
                        ++reliable_samples_received_;
                    }
                    else
                    {
                        ++besteffort_samples_received_;
                    }
                }
                cv_.notify_one();
            }
        }
    }

    uint32_t samples_received()
    {
        uint32_t ret {0};
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ret = reliable_samples_received_ + besteffort_samples_received_;
            reliable_samples_received_ = 0;
            besteffort_samples_received_ = 0;
        }
        return ret;
    }

    uint32_t reliable_samples_received()
    {
        uint32_t ret {0};
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ret = reliable_samples_received_;
            reliable_samples_received_ = 0;
        }
        return ret;
    }

    uint32_t besteffort_samples_received()
    {
        uint32_t ret {0};
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ret = besteffort_samples_received_;
            besteffort_samples_received_ = 0;
        }
        return ret;
    }

    bool wait_for_samples_received(
            uint32_t expected_samples,
            uint32_t timeout_ms)
    {
        bool ret {false};
        std::unique_lock<std::mutex> lock(mutex_);
        auto cv_ret = cv_.wait_for(
            lock,
            std::chrono::milliseconds(timeout_ms),
            [this, expected_samples]()
            {
                return (reliable_samples_received_ + besteffort_samples_received_) >= expected_samples;
            });

        if (cv_ret)
        {
            ret = true;
        }
        else
        {
            std::cout << "Timeout reached. Samples received: reliable (" << reliable_samples_received_
                      << "), besteffort(" << besteffort_samples_received_ << ")" << std::endl;
        }

        return ret;
    }

    bool wait_for_reliable_samples_received(
            uint32_t expected_samples,
            uint32_t timeout_ms)
    {
        bool ret {false};
        std::unique_lock<std::mutex> lock(mutex_);
        auto cv_ret = cv_.wait_for(
            lock,
            std::chrono::milliseconds(timeout_ms),
            [this, expected_samples]()
            {
                return reliable_samples_received_ >= expected_samples;
            });

        if (cv_ret)
        {
            ret = true;
        }
        else
        {
            std::cout << "Timeout reached. Samples received: reliable (" << reliable_samples_received_
                      << "), besteffort(" << besteffort_samples_received_ << ")" << std::endl;
        }

        return ret;
    }

private:

    std::mutex mutex_;

    std::condition_variable cv_;

    uint32_t reliable_samples_received_ {0};

    uint32_t besteffort_samples_received_ {0};

};

template<class T> struct TypeTag;
template<> struct TypeTag<HelloWorldPubSubType>
{
    static std::string name()
    {
        return "HelloWorld";
    }

};
template<> struct TypeTag<Data1mbPubSubType>
{
    static std::string name()
    {
        return "Data1mb";
    }

};

template<typename T, bool B>
struct TypeAndBool
{
    using Type = T;
    static constexpr bool security = B;
};

template<typename Combo>
class GrainedFlowControllerTests : public testing::Test
{
protected:

    using Type = typename Combo::Type;
    static constexpr bool security = Combo::security;

    void SetUp() override
    {
#if HAVE_SECURITY
        certs_path = std::getenv("CERTS_PATH");

        if (certs_path == nullptr)
        {
            std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
            exit(-1);
        }
#endif // if HAVE_SECURITY
    }

    std::pair<DomainParticipant*, DataWriter*> create_datawriter_with_grained_flow_controller(
            WListener* listener,
            bool reliable,
            int32_t depth)
    {
        std::pair<DomainParticipant*, DataWriter*> result {nullptr, nullptr};
        DomainParticipant* participant {nullptr};
        Publisher* publisher {nullptr};
        TypeSupport type(new Type());
        Topic* topic {nullptr};
        DataWriterQos datawriter_qos {DATAWRITER_QOS_DEFAULT};
        DataWriter* datawriter {nullptr};

        if (0 == domain_id)
        {
            domain_id = GET_PID() % 230;
        }

        DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
        auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        participant_qos.transport().user_transports.push_back(udp_transport);
        participant_qos.transport().use_builtin_transports = false;
        if (security)
        {
            participant_qos.properties().properties().emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");
            participant_qos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem");
            participant_qos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem");
            participant_qos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem");
            participant_qos.properties().properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
            participant_qos.properties().properties().emplace_back("dds.sec.access.plugin",
                    "builtin.Access-Permissions");
            participant_qos.properties().properties().emplace_back(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem");
            participant_qos.properties().properties().emplace_back(
                "dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/governance_helloworld_all_enable.smime");
            participant_qos.properties().properties().emplace_back(
                "dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld.smime");
        }
        participant = DomainParticipantFactory::get_instance()->create_participant(domain_id,
                        participant_qos);

        if (nullptr == participant)
        {
            goto fail;
        }

        result.first = participant;

        // Create publisher
        publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        if (nullptr == publisher)
        {
            goto fail;
        }

        type.register_type(participant);

        // Create topic
        topic = participant->create_topic("HelloWorldTopic_GrainedFlow",
                        type.get_type_name(), TOPIC_QOS_DEFAULT);
        if (nullptr == topic)
        {
            goto fail;
        }

        // Create datawriter with the grained flow controller
        datawriter_qos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
        datawriter_qos.publish_mode().flow_controller_name = rtps::test_grained_flow_controller_name;
        datawriter_qos.history().depth = depth;
        datawriter_qos.data_sharing().off();
        if (security)
        {
            datawriter_qos.properties().properties().emplace_back("rtps.endpoint.submessage_protection_kind",
                    "ENCRYPT");
        }

        if (reliable)
        {
            datawriter_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        }
        else
        {
            datawriter_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
        }
        datawriter = publisher->create_datawriter(topic, datawriter_qos, listener, StatusMask::all());
        if (nullptr == datawriter)
        {
            goto fail;
        }

        result.second = datawriter;
        goto sucess;

fail:

        if (datawriter)
        {
            publisher->delete_datawriter(datawriter);
        }

        if (topic)
        {
            participant->delete_topic(topic);
        }

        if (publisher)
        {
            participant->delete_publisher(publisher);
        }

        if (participant)
        {
            DomainParticipantFactory::get_instance()->delete_participant(participant);
        }

        result.first = nullptr;
        result.second = nullptr;

sucess:

        return result;
    }

    std::pair<DomainParticipant*, DataReader*> create_datareader(
            DataReaderListener* listener,
            bool reliable,
            int32_t depth)
    {
        std::pair<DomainParticipant*, DataReader*> result {nullptr, nullptr};
        DomainParticipant* participant {nullptr};
        Subscriber* subscriber {nullptr};
        TypeSupport type(new Type());
        Topic* topic {nullptr};
        DataReaderQos datareader_qos {DATAREADER_QOS_DEFAULT};
        DataReader* datareader {nullptr};

        if (0 == domain_id)
        {
            domain_id = GET_PID() % 230;
        }

        DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
        auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        participant_qos.transport().user_transports.push_back(udp_transport);
        participant_qos.transport().use_builtin_transports = false;
        if (security)
        {
            participant_qos.properties().properties().emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");
            participant_qos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem");
            participant_qos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem");
            participant_qos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem");
            participant_qos.properties().properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
            participant_qos.properties().properties().emplace_back("dds.sec.access.plugin",
                    "builtin.Access-Permissions");
            participant_qos.properties().properties().emplace_back(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem");
            participant_qos.properties().properties().emplace_back(
                "dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/governance_helloworld_all_enable.smime");
            participant_qos.properties().properties().emplace_back(
                "dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld.smime");
        }
        participant = DomainParticipantFactory::get_instance()->create_participant(domain_id,
                        participant_qos);

        if (nullptr == participant)
        {
            goto fail;
        }

        result.first = participant;

        // Create publisher
        subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        if (nullptr == subscriber)
        {
            goto fail;
        }

        type.register_type(participant);

        // Create topic
        topic = participant->create_topic("HelloWorldTopic_GrainedFlow",
                        type.get_type_name(), TOPIC_QOS_DEFAULT);
        if (nullptr == topic)
        {
            goto fail;
        }

        if (reliable)
        {
            datareader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        }
        else
        {
            datareader_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
        }
        datareader_qos.history().depth = depth;
        datareader_qos.data_sharing().off();
        if (security)
        {
            datareader_qos.properties().properties().emplace_back("rtps.endpoint.submessage_protection_kind",
                    "ENCRYPT");
        }

        datareader = subscriber->create_datareader(topic, datareader_qos, listener, StatusMask::all());
        if (nullptr == datareader)
        {
            goto fail;
        }

        result.second = datareader;
        goto sucess;

fail:

        if (datareader)
        {
            subscriber->delete_datareader(datareader);
        }

        if (topic)
        {
            participant->delete_topic(topic);
        }

        if (subscriber)
        {
            participant->delete_subscriber(subscriber);
        }

        if (participant)
        {
            DomainParticipantFactory::get_instance()->delete_participant(participant);
        }

        result.first = nullptr;
        result.second = nullptr;

sucess:

        return result;
    }

    void delete_datawriter_with_grained_flow_controller(
            std::pair<DomainParticipant*, DataWriter*>& datawriter_pair)
    {
        if (datawriter_pair.first)
        {
            datawriter_pair.first->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(datawriter_pair.first);
        }

        datawriter_pair.first = nullptr;
        datawriter_pair.second = nullptr;
    }

    void delete_datareader(
            std::pair<DomainParticipant*, DataReader*>& datareader_pair)
    {
        if (datareader_pair.first)
        {
            datareader_pair.first->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(datareader_pair.first);
        }

        datareader_pair.first = nullptr;
        datareader_pair.second = nullptr;
    }

    void fill_data(
            HelloWorld& data,
            uint16_t index)
    {
        data.index(index);
        data.message("Hello World with Grained Flow Controller");
    }

    void fill_data(
            Data1mb& data,
            uint16_t index)
    {
        data.data() = std::vector<uint8_t>(850, static_cast<uint8_t>(index % 256));
    }

};

using Combos = ::testing::Types<
    TypeAndBool<HelloWorldPubSubType, false>, TypeAndBool<HelloWorldPubSubType, true>,
    TypeAndBool<Data1mbPubSubType, false>, TypeAndBool<Data1mbPubSubType, true>
    >;

struct ComboNameGen
{
    template<typename Combo>
    static std::string GetName(
            int)
    {
        return TypeTag<typename Combo::Type>::name() + (Combo::security ? "_Security" : "_NoSecurity");
    }

};

TYPED_TEST_SUITE(GrainedFlowControllerTests, Combos, ComboNameGen);

TYPED_TEST(GrainedFlowControllerTests, multiple_readers_update_by_step)
{
    using T = typename TestFixture::Type;
    using type = typename T::type;
    static constexpr bool security_flag = TestFixture::security;

    if (security_flag)
    {
    #if !HAVE_SECURITY
        return;
    #endif // if !HAVE_SECURITY
    }

    WListener wlistener;
    Listener<type> listener;

    eprosima::fastdds::LibrarySettings att;
    att.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(att);

    // Datawriter must be created first to store its flow controller statically.
    auto datawriter_pair = this->create_datawriter_with_grained_flow_controller(&wlistener, true, 10);
    auto datareader_pair = this->create_datareader(&listener, true, 10);
    auto datareader2_pair = this->create_datareader(&listener, true, 10);
    auto datareader3_pair = this->create_datareader(&listener, false, 10);

    wlistener.wait_for_matched_readers(3, 2000);

    rtps::FlowControllerFactoryMock::grained_flow_controller->register_remote_reader(
        datareader_pair.second->guid(), 10);              // Zero limitation size
    rtps::FlowControllerFactoryMock::grained_flow_controller->register_remote_reader(
        datareader2_pair.second->guid(), 10);              // Zero limitation size
    rtps::FlowControllerFactoryMock::grained_flow_controller->register_remote_reader(
        datareader3_pair.second->guid(), 10);              // Zero limitation size

    for (uint16_t i = 0; i < 10; ++i)
    {
        type data;
        this->fill_data(data, i);
        datawriter_pair.second->write(&data);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(0u, listener.samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader_pair.second->guid(), 1000);

    EXPECT_TRUE(listener.wait_for_samples_received(10, 30000));
    EXPECT_EQ(10u, listener.samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader2_pair.second->guid(), 1000);

    EXPECT_TRUE(listener.wait_for_samples_received(10, 30000));
    EXPECT_EQ(10u, listener.samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader3_pair.second->guid(), 1000);

    listener.wait_for_samples_received(10, 30000);
    EXPECT_LE(6u, listener.samples_received());

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep to ensure last reader reset the limitation.

    for (uint16_t i = 10; i < 20; ++i)
    {
        type data;
        this->fill_data(data, i);
        datawriter_pair.second->write(&data);
    }

    EXPECT_TRUE(listener.wait_for_samples_received(30, 30000));
    EXPECT_EQ(20u, listener.reliable_samples_received());
    EXPECT_LE(6u, listener.besteffort_samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader_pair.second->guid(), 10);

    for (uint16_t i = 20; i < 30; ++i)
    {
        type data;
        this->fill_data(data, i);
        datawriter_pair.second->write(&data);
    }

    listener.wait_for_samples_received(20, 30000);
    EXPECT_EQ(10u, listener.reliable_samples_received());
    EXPECT_LT(0u, listener.besteffort_samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader2_pair.second->guid(), 10);

    for (uint16_t i = 30; i < 40; ++i)
    {
        type data;
        this->fill_data(data, i);
        datawriter_pair.second->write(&data);
    }

    listener.wait_for_samples_received(10, 30000);
    EXPECT_LT(0u, listener.besteffort_samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader3_pair.second->guid(), 10);

    for (uint16_t i = 40; i < 50; ++i)
    {
        type data;
        this->fill_data(data, i);
        datawriter_pair.second->write(&data);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(0u, listener.samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader_pair.second->guid(), 1000);
    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader2_pair.second->guid(), 1000);
    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader3_pair.second->guid(), 1000);

    listener.wait_for_samples_received(30, 30000);
    EXPECT_EQ(20u, listener.reliable_samples_received());
    EXPECT_LT(0u, listener.besteffort_samples_received());

    rtps::FlowControllerFactoryMock::grained_flow_controller->unregister_remote_reader(
        datareader_pair.second->guid());
    rtps::FlowControllerFactoryMock::grained_flow_controller->unregister_remote_reader(
        datareader2_pair.second->guid());
    rtps::FlowControllerFactoryMock::grained_flow_controller->unregister_remote_reader(
        datareader3_pair.second->guid());

    this->delete_datareader(datareader3_pair);
    this->delete_datareader(datareader2_pair);
    this->delete_datareader(datareader_pair);
    this->delete_datawriter_with_grained_flow_controller(datawriter_pair);
}

TYPED_TEST(GrainedFlowControllerTests, multiple_readers_update_mixed)
{
    using T = typename TestFixture::Type;
    using type = typename T::type;
    static constexpr bool security_flag = TestFixture::security;

    if (security_flag)
    {
    #if !HAVE_SECURITY
        return;
    #endif // if !HAVE_SECURITY
    }

    WListener wlistener;
    Listener<type> listener;

    eprosima::fastdds::LibrarySettings att;
    att.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(att);

    // Datawriter must be created first to store its flow controller statically.
    auto datawriter_pair = this->create_datawriter_with_grained_flow_controller(&wlistener, true, 100);
    auto datareader_pair = this->create_datareader(&listener, true, 100);
    auto datareader2_pair = this->create_datareader(&listener, true, 100);
    auto datareader3_pair = this->create_datareader(&listener, false, 100);

    wlistener.wait_for_matched_readers(3, 2000);

    uint32_t r1_limit = 10;
    uint32_t r2_limit = 10;
    uint32_t r3_limit = 10;
    rtps::FlowControllerFactoryMock::grained_flow_controller->register_remote_reader(
        datareader_pair.second->guid(), r1_limit);              // Zero limitation size
    rtps::FlowControllerFactoryMock::grained_flow_controller->register_remote_reader(
        datareader2_pair.second->guid(), r2_limit);              // Zero limitation size
    rtps::FlowControllerFactoryMock::grained_flow_controller->register_remote_reader(
        datareader3_pair.second->guid(), r3_limit);              // Zero limitation size

    for (uint16_t i = 0; i < 100; ++i)
    {
        if (i % 20 == 0)
        {
            r1_limit = (r1_limit == 10) ? 1000 : 10;
            rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
                datareader_pair.second->guid(), r1_limit);
        }
        if ((i + 5) % 20 == 0)
        {
            r2_limit = (r2_limit == 10) ? 1000 : 10;
            rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
                datareader2_pair.second->guid(), r2_limit);
        }
        if (i % 23 == 0)
        {
            r3_limit = (r3_limit == 10) ? 1000 : 10;
            rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
                datareader3_pair.second->guid(), r3_limit);
        }

        type data;
        this->fill_data(data, i);
        datawriter_pair.second->write(&data);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader_pair.second->guid(), 1000);
    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader2_pair.second->guid(), 1000);
    rtps::FlowControllerFactoryMock::grained_flow_controller->update_remote_reader_bytes_per_period(
        datareader3_pair.second->guid(), 1000);

    EXPECT_TRUE(listener.wait_for_reliable_samples_received(200, 300000));
    EXPECT_LE(250u, listener.samples_received());

    this->delete_datareader(datareader3_pair);
    this->delete_datareader(datareader2_pair);
    this->delete_datareader(datareader_pair);
    this->delete_datawriter_with_grained_flow_controller(datawriter_pair);
}

int main(
        int argc,
        char** argv)
{
    //Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
