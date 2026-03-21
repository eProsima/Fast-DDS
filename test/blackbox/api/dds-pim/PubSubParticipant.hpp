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

/**
 * @file PubSubParticipant.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBPARTICIPANT_HPP_
#define _TEST_BLACKBOX_PUBSUBPARTICIPANT_HPP_

#include <atomic>
#include <condition_variable>
#include <thread>
#include <tuple>
#include <vector>

#include <asio.hpp>
#include <gtest/gtest.h>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>

/**
 * @brief A class with one participant that can have multiple publishers and subscribers
 */
template<class TypeSupport>
class PubSubParticipant
{
public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

private:

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
        friend class PubSubParticipant;

    public:

        PubListener(
                PubSubParticipant* participant)
            : participant_(participant)
        {
        }

        ~PubListener()
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter*,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
        {
            (0 < info.current_count_change) ? participant_->pub_matched() : participant_->pub_unmatched();
        }

        void on_liveliness_lost(
                eprosima::fastdds::dds::DataWriter*,
                const eprosima::fastdds::dds::LivelinessLostStatus& status) override
        {
            (void)status;
            participant_->pub_liveliness_lost();
        }

    private:

        PubListener& operator =(
                const PubListener&) = delete;
        //! A pointer to the participant
        PubSubParticipant* participant_;
    };

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
        friend class PubSubParticipant;

    public:

        SubListener(
                PubSubParticipant* participant)
            : participant_(participant)
        {
        }

        ~SubListener()
        {
        }

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader*,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override
        {
            (0 < info.current_count_change) ? participant_->sub_matched() : participant_->sub_unmatched();
        }

        void on_liveliness_changed(
                eprosima::fastdds::dds::DataReader*,
                const eprosima::fastdds::dds::LivelinessChangedStatus& status) override
        {
            (status.alive_count_change ==
            1) ? participant_->sub_liveliness_recovered() : participant_->sub_liveliness_lost();

        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override
        {
            type data;
            eprosima::fastdds::dds::SampleInfo info;

            while (eprosima::fastdds::dds::RETCODE_OK == reader->take_next_sample(&data, &info))
            {
                participant_->data_received();
            }
        }

    private:

        SubListener& operator =(
                const SubListener&) = delete;
        //! A pointer to the participant
        PubSubParticipant* participant_;
    };

    class ParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
        friend class PubSubParticipant;

    public:

        ParticipantListener(
                PubSubParticipant* participant)
            : participant_(participant)
        {
        }

        ~ParticipantListener() = default;

        void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant*,
                eprosima::fastdds::rtps::ParticipantDiscoveryStatus status,
                const eprosima::fastdds::dds::ParticipantBuiltinTopicData& info,
                bool& should_be_ignored)
        {
            static_cast<void>(should_be_ignored);
            bool expected = false;
            if (status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
            {
                ++participant_->matched_;
                if (nullptr !=  participant_->on_discovery_)
                {
                    participant_->discovery_result_.compare_exchange_strong(expected,
                            participant_->on_discovery_(info));
                }
                participant_->cv_discovery_.notify_one();
            }
            else if (participant_->on_participant_qos_update_ != nullptr &&
                    status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT)
            {
                participant_->participant_qos_updated_.compare_exchange_strong(expected,
                        participant_->on_participant_qos_update_(info));
                participant_->cv_discovery_.notify_one();
            }
            else if (status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT ||
                    status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
            {
                --participant_->matched_;
                participant_->cv_discovery_.notify_one();
            }
        }

    private:

        using eprosima::fastdds::dds::DomainParticipantListener::on_participant_discovery;

        ParticipantListener& operator =(
                const ParticipantListener&) = delete;
        PubSubParticipant* participant_;

    };

public:

    PubSubParticipant(
            unsigned int num_publishers,
            unsigned int num_subscribers,
            unsigned int num_expected_publishers,
            unsigned int num_expected_subscribers)
        : participant_(nullptr)
        , num_publishers_(num_publishers)
        , num_subscribers_(num_subscribers)
        , num_expected_subscribers_(num_expected_subscribers)
        , num_expected_publishers_(num_expected_publishers)
        , publishers_(num_publishers)
        , subscribers_(num_subscribers)
        , participant_listener_(this)
        , pub_listener_(this)
        , sub_listener_(this)
        , matched_(0)
        , on_discovery_(nullptr)
        , on_participant_qos_update_(nullptr)
        , discovery_result_(false)
        , participant_qos_updated_(false)
        , pub_matched_(0)
        , sub_matched_(0)
        , pub_times_liveliness_lost_(0)
        , sub_times_liveliness_lost_(0)
        , sub_times_liveliness_recovered_(0)
    {
        if (enable_datasharing)
        {
            datareader_qos_.data_sharing().automatic();
            datawriter_qos_.data_sharing().automatic();
        }
        else
        {
            datareader_qos_.data_sharing().off();
            datawriter_qos_.data_sharing().off();
        }

        if (use_pull_mode)
        {
            datawriter_qos_.properties().properties().emplace_back("fastdds.push_mode", "false");
        }

#if defined(PREALLOCATED_MEMORY_MODE_TEST)
        datawriter_qos_.historyMemoryPolicy = rtps::PREALLOCATED_MEMORY_MODE;
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
        datawriter_qos_.historyMemoryPolicy = rtps::DYNAMIC_RESERVE_MEMORY_MODE;
#else
        datawriter_qos_.endpoint().history_memory_policy =
                eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
#endif // if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        datawriter_qos_.reliable_writer_qos().times.heartbeat_period.seconds = 0;
        datawriter_qos_.reliable_writer_qos().times.heartbeat_period.nanosec = 100000000;
        datawriter_qos_.reliable_writer_qos().times.nack_response_delay.seconds = 0;
        datawriter_qos_.reliable_writer_qos().times.nack_response_delay.nanosec = 100000000;

        // Increase default max_blocking_time to 1 second, as our CI infrastructure shows some
        // big CPU overhead sometimes
        datawriter_qos_.reliability().max_blocking_time.seconds = 1;
        datawriter_qos_.reliability().max_blocking_time.nanosec = 0;

        // By default, heartbeat period delay is 100 milliseconds.
        datareader_qos_.reliable_reader_qos().times.heartbeat_response_delay = 0.1;
    }

    ~PubSubParticipant()
    {
        if (participant_ != nullptr)
        {
            for (auto p : publishers_)
            {
                if (std::get<1>(p))
                {
                    if (std::get<2>(p))
                    {
                        std::get<1>(p)->delete_datawriter(std::get<2>(p));
                    }
                    participant_->delete_publisher(std::get<1>(p));
                    participant_->delete_topic(std::get<0>(p));
                }
            }
            publishers_.clear();
            for (auto s : subscribers_)
            {
                if (std::get<1>(s))
                {
                    if (std::get<2>(s))
                    {
                        std::get<1>(s)->delete_datareader(std::get<2>(s));
                    }
                    participant_->delete_subscriber(std::get<1>(s));
                    participant_->delete_topic(std::get<0>(s));
                }
            }
            subscribers_.clear();

            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

    bool init_participant()
    {
        matched_ = 0;

        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            (uint32_t)GET_PID() % 230,
            participant_qos_,
            &participant_listener_,
            eprosima::fastdds::dds::StatusMask::none());

        if (participant_ != nullptr)
        {
            participant_qos_ = participant_->get_qos();
            type_.reset(new type_support());
            participant_->register_type(type_);
            return true;
        }

        return false;
    }

    bool init_publisher(
            unsigned int index)
    {
        if (participant_ == nullptr)
        {
            return false;
        }
        if (index >= num_publishers_)
        {
            return false;
        }

        if (publisher_topicname_.empty())
        {
            EPROSIMA_LOG_ERROR(PUBSUBPARTICIPANT, "Publisher topic name not set");
            return false;
        }

        eprosima::fastdds::dds::Topic* topic =
                dynamic_cast<eprosima::fastdds::dds::Topic*>(participant_->lookup_topicdescription(
                    publisher_topicname_));

        if (topic == nullptr)
        {
            topic = participant_->create_topic(publisher_topicname_,
                            type_->get_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        }
        if (topic)
        {
            auto publisher = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
            if (publisher)
            {
                auto datawriter = publisher->create_datawriter(topic, datawriter_qos_, &pub_listener_);
                if (datawriter && datawriter->is_enabled())
                {
                    publishers_[index] = {topic, publisher, datawriter};
                    return true;
                }
            }
        }
        return false;
    }

    bool init_subscriber(
            unsigned int index)
    {
        if (participant_ == nullptr)
        {
            return false;
        }
        if (index >= num_subscribers_)
        {
            return false;
        }

        if (subscriber_topicname_.empty())
        {
            EPROSIMA_LOG_ERROR(PUBSUBPARTICIPANT, "Subscriber topic name not set");
            return false;
        }

        eprosima::fastdds::dds::Topic* topic =
                dynamic_cast<eprosima::fastdds::dds::Topic*>(participant_->lookup_topicdescription(
                    subscriber_topicname_));

        if (topic == nullptr)
        {
            topic = participant_->create_topic(subscriber_topicname_,
                            type_->get_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        }
        if (topic)
        {
            auto subscriber = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
            if (subscriber)
            {
                auto datareader = subscriber->create_datareader(topic, datareader_qos_, &sub_listener_);
                if (datareader && datareader->is_enabled())
                {
                    subscribers_[index] = {topic, subscriber, datareader};
                    return true;
                }
            }
        }
        return false;
    }

    eprosima::fastdds::dds::DataWriter& get_native_writer(
            unsigned int index)
    {
        return *(std::get<2>(publishers_[index]));
    }

    eprosima::fastdds::dds::DataReader& get_native_reader(
            unsigned int index)
    {
        return *(std::get<2>(subscribers_[index]));
    }

    bool send_sample(
            type& msg,
            unsigned int index = 0)
    {
        return (eprosima::fastdds::dds::RETCODE_OK == std::get<2>(publishers_[index])->write((void*)&msg));
    }

    void assert_liveliness_participant()
    {
        participant_->assert_liveliness();
    }

    void assert_liveliness(
            unsigned int index = 0)
    {
        std::get<2>(publishers_[index])->assert_liveliness();
    }

    bool wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero(),
            uint8_t matched = 0,
            bool exact = false)
    {
        // No need to wait in this case
        if (exact && matched == matched_)
        {
            return true;
        }

        std::unique_lock<std::mutex> lock(mutex_discovery_);
        bool ret_value = true;
        std::cout << "Participant is waiting discovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            cv_discovery_.wait(lock, [&]()
                    {
                        if (exact)
                        {
                            return matched_ == matched;
                        }
                        return matched_ >= matched;
                    });
        }
        else
        {
            if (!cv_discovery_.wait_for(lock, timeout, [&]()
                    {
                        if (exact)
                        {
                            return matched_ == matched;
                        }
                        return matched_ >= matched;
                    }))
            {
                ret_value = false;
            }
        }

        if (ret_value)
        {
            std::cout << "Participant discovery finished successfully..." << std::endl;
        }
        else
        {
            std::cout << "Participant discovery finished unsuccessfully..." << std::endl;
        }

        return ret_value;
    }

    void pub_wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(pub_mutex_);

        std::cout << "Publisher is waiting discovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            pub_cv_.wait(lock, [&]()
                    {
                        return pub_matched_ == num_expected_publishers_;
                    });
        }
        else
        {
            pub_cv_.wait_for(lock, timeout, [&]()
                    {
                        return pub_matched_ == num_expected_publishers_;
                    });
        }

        std::cout << "Publisher discovery finished " << std::endl;
    }

    void pub_wait_discovery(
            unsigned int expected_match,
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(pub_mutex_);

        std::cout << "Publisher is waiting discovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            pub_cv_.wait(lock, [&]()
                    {
                        return pub_matched_ == expected_match;
                    });
        }
        else
        {
            pub_cv_.wait_for(lock, timeout, [&]()
                    {
                        return pub_matched_ == expected_match;
                    });
        }

        std::cout << "Publisher discovery finished " << std::endl;
    }

    void sub_wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(sub_mutex_);

        std::cout << "Subscriber is waiting discovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            sub_cv_.wait(lock, [&]()
                    {
                        return sub_matched_ == num_expected_subscribers_;
                    });
        }
        else
        {
            sub_cv_.wait_for(lock, timeout, [&]()
                    {
                        return sub_matched_ == num_expected_subscribers_;
                    });
        }

        std::cout << "Subscriber discovery finished " << std::endl;
    }

    void sub_wait_discovery(
            unsigned int expected_match,
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(sub_mutex_);

        std::cout << "Subscriber is waiting discovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            sub_cv_.wait(lock, [&]()
                    {
                        return sub_matched_ == expected_match;
                    });
        }
        else
        {
            sub_cv_.wait_for(lock, timeout, [&]()
                    {
                        return sub_matched_ == expected_match;
                    });
        }

        std::cout << "Subscriber discovery finished " << std::endl;
    }

    void pub_wait_liveliness_lost(
            unsigned int times = 1)
    {
        std::unique_lock<std::mutex> lock(pub_liveliness_mutex_);
        pub_liveliness_cv_.wait(lock, [&]()
                {
                    return pub_times_liveliness_lost_ >= times;
                });
    }

    void sub_wait_liveliness_recovered(
            unsigned int num_recovered)
    {
        std::unique_lock<std::mutex> lock(sub_liveliness_mutex_);
        sub_liveliness_cv_.wait(lock, [&]()
                {
                    return sub_times_liveliness_recovered_ >= num_recovered;
                });
    }

    void sub_wait_liveliness_lost(
            unsigned int num_lost)
    {
        std::unique_lock<std::mutex> lock(sub_liveliness_mutex_);
        sub_liveliness_cv_.wait(lock, [&]()
                {
                    return sub_times_liveliness_lost_ >= num_lost;
                });
    }

    template<class _Rep,
            class _Period
            >
    size_t sub_wait_liveliness_lost_for(
            unsigned int expected_num_lost,
            const std::chrono::duration<_Rep, _Period>& max_wait)
    {
        std::unique_lock<std::mutex> lock(sub_liveliness_mutex_);
        sub_liveliness_cv_.wait_for(lock, max_wait, [this, &expected_num_lost]() -> bool
                {
                    return sub_times_liveliness_lost_ >= expected_num_lost;
                });

        return sub_times_liveliness_lost_;
    }

    void sub_wait_data_received(
            unsigned int num_received)
    {
        std::unique_lock<std::mutex> lock(sub_data_mutex_);
        sub_data_cv_.wait(lock, [&]()
                {
                    return sub_times_data_received_ >= num_received;
                });
    }

    PubSubParticipant& property_policy(
            const eprosima::fastdds::rtps::PropertyPolicy property_policy)
    {
        participant_qos_.properties() = property_policy;
        return *this;
    }

    PubSubParticipant& disable_builtin_transport()
    {
        participant_qos_.transport().use_builtin_transports = false;
        return *this;
    }

    PubSubParticipant& add_user_transport_to_pparams(
            std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> userTransportDescriptor)
    {
        participant_qos_.transport().user_transports.push_back(userTransportDescriptor);
        return *this;
    }

    PubSubParticipant& setup_transports(
            eprosima::fastdds::rtps::BuiltinTransports transports)
    {
        participant_qos_.setup_transports(transports);
        return *this;
    }

    PubSubParticipant& user_data(
            const std::vector<eprosima::fastdds::rtps::octet>& user_data)
    {
        participant_qos_.user_data().data_vec(user_data);
        return *this;
    }

    bool update_user_data(
            const std::vector<eprosima::fastdds::rtps::octet>& user_data)
    {
        // Update QoS before updating user data as statistics properties might have changed internally
        participant_qos_ = participant_->get_qos();
        participant_qos_.user_data().data_vec(user_data);
        return eprosima::fastdds::dds::RETCODE_OK == participant_->set_qos(participant_qos_);
    }

    PubSubParticipant& wire_protocol(
            const eprosima::fastdds::dds::WireProtocolConfigQos& wire_protocol)
    {
        participant_qos_.wire_protocol() = wire_protocol;
        return *this;
    }

    bool update_wire_protocol(
            const eprosima::fastdds::dds::WireProtocolConfigQos& wire_protocol)
    {
        eprosima::fastdds::dds::DomainParticipantQos participant_qos = participant_qos_;
        participant_qos.wire_protocol() = wire_protocol;
        if (eprosima::fastdds::dds::RETCODE_OK == participant_->set_qos(participant_qos))
        {
            participant_qos_ = participant_qos;
            return true;
        }
        return false;
    }

    PubSubParticipant& flow_controller(
            const std::shared_ptr<eprosima::fastdds::rtps::FlowControllerDescriptor>& flow_controller)
    {
        participant_qos_.flow_controllers().clear();
        participant_qos_.flow_controllers().push_back(flow_controller);
        return *this;
    }

    PubSubParticipant& initial_peers(
            const eprosima::fastdds::rtps::LocatorList& initial_peers)
    {
        participant_qos_.wire_protocol().builtin.initialPeersList = initial_peers;
        return *this;
    }

    PubSubParticipant& pub_property_policy(
            const eprosima::fastdds::rtps::PropertyPolicy property_policy)
    {
        datawriter_qos_.properties() = property_policy;
        return *this;
    }

    PubSubParticipant& sub_property_policy(
            const eprosima::fastdds::rtps::PropertyPolicy property_policy)
    {
        datareader_qos_.properties() = property_policy;
        return *this;
    }

    PubSubParticipant& pub_topic_name(
            std::string topicName)
    {
        // Generate topic name
        std::ostringstream t;
        t << topicName << "_" << asio::ip::host_name() << "_" << GET_PID();
        publisher_topicname_ = t.str();
        return *this;
    }

    PubSubParticipant& sub_topic_name(
            std::string topicName)
    {
        // Generate topic name
        std::ostringstream t;
        t << topicName << "_" << asio::ip::host_name() << "_" << GET_PID();
        subscriber_topicname_ = t.str();
        return *this;
    }

    PubSubParticipant& reliability(
            const eprosima::fastdds::dds::ReliabilityQosPolicyKind kind)
    {
        datawriter_qos_.reliability().kind = kind;
        datareader_qos_.reliability().kind = kind;
        return *this;
    }

    PubSubParticipant& pub_liveliness_kind(
            const eprosima::fastdds::dds::LivelinessQosPolicyKind kind)
    {
        datawriter_qos_.liveliness().kind = kind;
        return *this;
    }

    PubSubParticipant& pub_liveliness_lease_duration(
            const Duration_t lease_duration)
    {
        datawriter_qos_.liveliness().lease_duration = lease_duration;
        return *this;
    }

    PubSubParticipant& pub_liveliness_announcement_period(
            const Duration_t announcement_period)
    {
        datawriter_qos_.liveliness().announcement_period = announcement_period;
        return *this;
    }

    PubSubParticipant& sub_liveliness_kind(
            const eprosima::fastdds::dds::LivelinessQosPolicyKind& kind)
    {
        datareader_qos_.liveliness().kind = kind;
        return *this;
    }

    PubSubParticipant& sub_liveliness_lease_duration(
            const Duration_t lease_duration)
    {
        datareader_qos_.liveliness().lease_duration = lease_duration;
        return *this;
    }

    PubSubParticipant& pub_deadline_period(
            const Duration_t& deadline_period)
    {
        datawriter_qos_.deadline().period = deadline_period;
        return *this;
    }

    PubSubParticipant& sub_deadline_period(
            const Duration_t& deadline_period)
    {
        datareader_qos_.deadline().period = deadline_period;
        return *this;
    }

    bool sub_update_deadline_period(
            const Duration_t& deadline_period,
            unsigned int index)
    {
        if (index >= num_subscribers_)
        {
            return false;
        }
        if (std::get<2>(subscribers_[index]) == nullptr)
        {
            return false;
        }

        eprosima::fastdds::dds::DataReaderQos qos;
        qos = std::get<2>(subscribers_[index])->get_qos();
        qos.deadline().period = deadline_period;

        return eprosima::fastdds::dds::RETCODE_OK == std::get<2>(subscribers_[index])->set_qos(qos);
    }

    void pub_liveliness_lost()
    {
        std::unique_lock<std::mutex> lock(pub_liveliness_mutex_);
        pub_times_liveliness_lost_++;
        pub_liveliness_cv_.notify_one();
    }

    void sub_liveliness_lost()
    {
        std::unique_lock<std::mutex> lock(sub_liveliness_mutex_);
        sub_times_liveliness_lost_++;
        sub_liveliness_cv_.notify_one();
    }

    void sub_liveliness_recovered()
    {
        std::unique_lock<std::mutex> lock(sub_liveliness_mutex_);
        sub_times_liveliness_recovered_++;
        sub_liveliness_cv_.notify_one();
    }

    void data_received()
    {
        std::unique_lock<std::mutex> lock(sub_data_mutex_);
        sub_times_data_received_++;
        sub_data_cv_.notify_one();
    }

    unsigned int pub_times_liveliness_lost()
    {
        std::unique_lock<std::mutex> lock(pub_liveliness_mutex_);
        return pub_times_liveliness_lost_;
    }

    unsigned int sub_times_liveliness_lost()
    {
        std::unique_lock<std::mutex> lock(sub_liveliness_mutex_);
        return sub_times_liveliness_lost_;
    }

    unsigned int sub_times_liveliness_recovered()
    {
        std::unique_lock<std::mutex> lock(sub_liveliness_mutex_);
        return sub_times_liveliness_recovered_;
    }

    void wait_discovery_result()
    {
        std::unique_lock<std::mutex> lock(mutex_discovery_);

        std::cout << "Participant is waiting discovery result..." << std::endl;

        cv_discovery_.wait(lock, [&]() ->  bool
                {
                    return discovery_result_;
                });

        std::cout << "Participant gets discovery result..." << std::endl;
    }

    void wait_qos_update()
    {
        std::unique_lock<std::mutex> lock(mutex_discovery_);

        std::cout << "Participant is waiting QoS update..." << std::endl;

        cv_discovery_.wait(lock, [&]() -> bool
                {
                    return participant_qos_updated_;
                });

        std::cout << "Participant gets QoS update..." << std::endl;
    }

    void set_on_discovery_function(
            std::function<bool(const eprosima::fastdds::dds::ParticipantBuiltinTopicData&)> f)
    {
        on_discovery_ = f;
    }

    void set_on_participant_qos_update_function(
            std::function<bool(const eprosima::fastdds::dds::ParticipantBuiltinTopicData&)> f)
    {
        on_participant_qos_update_ = f;
    }

    PubSubParticipant& fill_server_qos(
            eprosima::fastdds::dds::WireProtocolConfigQos& qos,
            eprosima::fastdds::rtps::Locator_t& locator_server,
            uint16_t port,
            uint32_t kind)
    {
        qos.builtin.discovery_config.discoveryProtocol = eprosima::fastdds::rtps::DiscoveryProtocol::SERVER;
        // Generate server's listening locator
        eprosima::fastdds::rtps::IPLocator::setIPv4(locator_server, 127, 0, 0, 1);
        eprosima::fastdds::rtps::IPLocator::setPhysicalPort(locator_server, port);
        locator_server.kind = kind;
        // Leave logical port as 0 to use TCP DS default logical port
        qos.builtin.metatrafficUnicastLocatorList.push_back(locator_server);

        return wire_protocol(qos);
    }

private:

    PubSubParticipant& operator =(
            const PubSubParticipant&) = delete;

    void pub_matched()
    {
        std::unique_lock<std::mutex> lock(pub_mutex_);
        ++pub_matched_;
        pub_cv_.notify_one();
    }

    void pub_unmatched()
    {
        std::unique_lock<std::mutex> lock(pub_mutex_);
        --pub_matched_;
        pub_cv_.notify_one();
    }

    void sub_matched()
    {
        std::unique_lock<std::mutex> lock(sub_mutex_);
        ++sub_matched_;
        sub_cv_.notify_one();
    }

    void sub_unmatched()
    {
        std::unique_lock<std::mutex> lock(sub_mutex_);
        --sub_matched_;
        sub_cv_.notify_one();
    }

    //! The participant
    eprosima::fastdds::dds::DomainParticipant* participant_;
    //! Participant attributes
    eprosima::fastdds::dds::DomainParticipantQos participant_qos_;
    //! Number of publishers in this participant
    unsigned int num_publishers_;
    //! Number of subscribers in this participant
    unsigned int num_subscribers_;
    //! Number of expected subscribers to match
    unsigned int num_expected_subscribers_;
    //! Number of expected subscribers to match
    unsigned int num_expected_publishers_;
    //! A vector of publishers
    std::vector<std::tuple<
                eprosima::fastdds::dds::Topic*,
                eprosima::fastdds::dds::Publisher*,
                eprosima::fastdds::dds::DataWriter*>> publishers_;
    //! A vector of subscribers
    std::vector<std::tuple<
                eprosima::fastdds::dds::Topic*,
                eprosima::fastdds::dds::Subscriber*,
                eprosima::fastdds::dds::DataReader*>> subscribers_;
    //! Publisher attributes
    eprosima::fastdds::dds::DataWriterQos datawriter_qos_;
    //! Subscriber attributes
    eprosima::fastdds::dds::DataReaderQos datareader_qos_;
    //! A listener for participants
    ParticipantListener participant_listener_;
    //! A listener for publishers
    PubListener pub_listener_;
    //! A listener for subscribers
    SubListener sub_listener_;
    std::string publisher_topicname_;
    std::string subscriber_topicname_;

    //! Discovery
    std::mutex mutex_discovery_;
    std::condition_variable cv_discovery_;
    std::atomic<unsigned int> matched_;
    std::function<bool(const eprosima::fastdds::dds::ParticipantBuiltinTopicData& info)> on_discovery_;
    std::function<bool(const eprosima::fastdds::dds::ParticipantBuiltinTopicData& info)> on_participant_qos_update_;
    std::atomic_bool discovery_result_;
    std::atomic_bool participant_qos_updated_;

    std::mutex pub_mutex_;
    std::mutex sub_mutex_;
    std::condition_variable pub_cv_;
    std::condition_variable sub_cv_;
    std::atomic<unsigned int> pub_matched_;
    std::atomic<unsigned int> sub_matched_;

    //! Number of times liveliness was lost on the publishing side
    unsigned int pub_times_liveliness_lost_;
    //! The number of times liveliness was lost on the subscribing side
    unsigned int sub_times_liveliness_lost_;
    //! The number of times liveliness was recovered on the subscribing side
    unsigned int sub_times_liveliness_recovered_;
    //! A mutex protecting liveliness data
    std::mutex sub_liveliness_mutex_;
    //! A condition variable for liveliness data
    std::condition_variable sub_liveliness_cv_;
    //! A mutex protecting liveliness of publisher
    std::mutex pub_liveliness_mutex_;
    //! A condition variable for liveliness of publisher
    std::condition_variable pub_liveliness_cv_;

    //! A mutex protecting received data
    std::mutex sub_data_mutex_;
    //! A condition variable for received data
    std::condition_variable sub_data_cv_;
    //! Number of times a subscriber received data
    size_t sub_times_data_received_ = 0;

    eprosima::fastdds::dds::TypeSupport type_;
};

#endif // _TEST_BLACKBOX_PUBSUBPARTICIPANT_HPP_
