// Copyright 2016, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PubSubWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBWRITER_HPP_

#include <condition_variable>
#include <list>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>
#include <gtest/gtest.h>
#if _MSC_VER
#include <Windows.h>
#endif // _MSC_VER
#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerSchedulerPolicy.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

using eprosima::fastdds::dds::DomainParticipantFactory;
using eprosima::fastdds::rtps::UDPTransportDescriptor;
using eprosima::fastdds::rtps::UDPv4TransportDescriptor;
using eprosima::fastdds::rtps::UDPv6TransportDescriptor;
using eprosima::fastdds::rtps::IPLocator;
using eprosima::fastdds::rtps::BuiltinTransports;
using eprosima::fastdds::rtps::BuiltinTransportsOptions;

template<class TypeSupport>
class PubSubWriter
{
    class ParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:

        ParticipantListener(
                PubSubWriter& writer)
            : writer_(writer)
        {
        }

        ~ParticipantListener()
        {
        }

        void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant*,
                eprosima::fastdds::rtps::ParticipantDiscoveryStatus status,
                const eprosima::fastdds::dds::ParticipantBuiltinTopicData& info,
                bool& should_be_ignored) override
        {
            static_cast<void>(should_be_ignored);
            if (writer_.onDiscovery_ != nullptr)
            {
                writer_.discovery_result_ = writer_.onDiscovery_(info, status);
            }

            if (status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
            {
                writer_.participant_matched();
            }
            else if (status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT ||
                    status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
            {
                writer_.participant_unmatched();
            }
        }

#if HAVE_SECURITY
        void onParticipantAuthentication(
                eprosima::fastdds::dds::DomainParticipant*,
                eprosima::fastdds::rtps::ParticipantAuthenticationInfo&& info) override
        {
            if (info.status == eprosima::fastdds::rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT)
            {
                writer_.authorized();
            }
            else if (info.status == eprosima::fastdds::rtps::ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT)
            {
                writer_.unauthorized();
            }
        }

#endif // if HAVE_SECURITY

        void on_data_reader_discovery(
                eprosima::fastdds::dds::DomainParticipant*,
                eprosima::fastdds::rtps::ReaderDiscoveryStatus reason,
                const eprosima::fastdds::dds::SubscriptionBuiltinTopicData& info,
                bool& /*should_be_ignored*/) override
        {
            if (reason == eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER)
            {
                writer_.add_reader_info(info);

            }
            else if (reason == eprosima::fastdds::rtps::ReaderDiscoveryStatus::CHANGED_QOS_READER)
            {
                writer_.change_reader_info(info);
            }
            else if (reason == eprosima::fastdds::rtps::ReaderDiscoveryStatus::REMOVED_READER)
            {
                writer_.remove_reader_info(info);
            }
        }

        void on_data_writer_discovery(
                eprosima::fastdds::dds::DomainParticipant*,
                eprosima::fastdds::rtps::WriterDiscoveryStatus reason,
                const eprosima::fastdds::dds::PublicationBuiltinTopicData& info,
                bool& /*should_be_ignored*/) override
        {
            using eprosima::fastdds::rtps::WriterDiscoveryStatus;

            if (reason == WriterDiscoveryStatus::DISCOVERED_WRITER)
            {
                writer_.add_writer_info(info);
            }
            else if (reason == WriterDiscoveryStatus::CHANGED_QOS_WRITER)
            {
                writer_.change_writer_info(info);
            }
            else if (reason == WriterDiscoveryStatus::REMOVED_WRITER)
            {
                writer_.remove_writer_info(info);
            }
        }

    private:

        using eprosima::fastdds::dds::DomainParticipantListener::on_participant_discovery;

        ParticipantListener& operator =(
                const ParticipantListener&) = delete;

        PubSubWriter& writer_;

    }
    participant_listener_;

    class Listener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        Listener(
                PubSubWriter& writer)
            : writer_(writer)
            , times_deadline_missed_(0)
            , times_liveliness_lost_(0)
            , times_unack_sample_removed_(0)
        {
        }

        ~Listener()
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* /*datawriter*/,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
        {
            if (0 < info.current_count_change)
            {
                std::cout << "Publisher matched subscriber " << info.last_subscription_handle << std::endl;
                writer_.matched();
            }
            else
            {
                std::cout << "Publisher unmatched subscriber " << info.last_subscription_handle << std::endl;
                writer_.unmatched();
            }
        }

        void on_offered_deadline_missed(
                eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::OfferedDeadlineMissedStatus& status) override
        {
            static_cast<void>(datawriter);
            times_deadline_missed_ = status.total_count;
        }

        void on_offered_incompatible_qos(
                eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::OfferedIncompatibleQosStatus& status) override
        {
            static_cast<void>(datawriter);
            writer_.incompatible_qos(status);
        }

        void on_liveliness_lost(
                eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::LivelinessLostStatus& status) override
        {
            static_cast<void>(datawriter);
            times_liveliness_lost_ = status.total_count;
            writer_.liveliness_lost();
        }

        void on_unacknowledged_sample_removed(
                eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::InstanceHandle_t& handle) override
        {
            EXPECT_EQ(writer_.datawriter_, datawriter);
            times_unack_sample_removed_++;
            instances_removed_unack_.push_back(handle);
        }

        unsigned int missed_deadlines() const
        {
            return times_deadline_missed_;
        }

        unsigned int times_liveliness_lost() const
        {
            return times_liveliness_lost_;
        }

        unsigned int times_unack_sample_removed() const
        {
            return times_unack_sample_removed_;
        }

        std::vector<eprosima::fastdds::dds::InstanceHandle_t>& instances_removed_unack()
        {
            return instances_removed_unack_;
        }

    private:

        Listener& operator =(
                const Listener&) = delete;

        PubSubWriter& writer_;

        //! The number of times deadline was missed
        unsigned int times_deadline_missed_;
        //! The number of times liveliness was lost
        unsigned int times_liveliness_lost_;
        //! The number of times a sample has been removed unacknowledged
        unsigned int times_unack_sample_removed_;
        //! Instance handle collection of those instances that have removed samples unacknowledged
        std::vector<eprosima::fastdds::dds::InstanceHandle_t> instances_removed_unack_;

    }
    listener_;

public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    PubSubWriter(
            const std::string& topic_name)
        : participant_listener_(*this)
        , listener_(*this)
        , participant_(nullptr)
        , topic_(nullptr)
        , publisher_(nullptr)
        , datawriter_(nullptr)
        , status_mask_(eprosima::fastdds::dds::StatusMask::all())
        , initialized_(false)
        , use_domain_id_from_profile_(false)
        , matched_(0)
        , participant_matched_(0)
        , discovery_result_(false)
        , onDiscovery_(nullptr)
        , times_liveliness_lost_(0)
        , times_incompatible_qos_(0)
        , last_incompatible_qos_(eprosima::fastdds::dds::INVALID_QOS_POLICY_ID)
        , use_preferred_domain_id_(false)
        , preferred_domain_id_(0)
#if HAVE_SECURITY
        , authorized_(0)
        , unauthorized_(0)
#endif // if HAVE_SECURITY
    {
        // Load default QoS to permit testing with external XML profile files.
        DomainParticipantFactory::get_instance()->load_profiles();
        participant_qos_ = DomainParticipantFactory::get_instance()->get_default_participant_qos();

        // Generate topic name
        std::ostringstream t;
        t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
        topic_name_ = t.str();

        if (enable_datasharing)
        {
            datawriter_qos_.data_sharing().automatic();
            datawriter_qos_.resource_limits().extra_samples = 5;
        }
        else
        {
            datawriter_qos_.data_sharing().off();
        }

        if (use_pull_mode)
        {
            datawriter_qos_.properties().properties().emplace_back("fastdds.push_mode", "false");
        }

        // By default, memory mode is PREALLOCATED_WITH_REALLOC_MEMORY_MODE
        datawriter_qos_.endpoint().history_memory_policy =
                eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        datawriter_qos_.reliable_writer_qos().times.heartbeat_period.seconds = 0;
        datawriter_qos_.reliable_writer_qos().times.heartbeat_period.nanosec = 100000000;
        datawriter_qos_.reliable_writer_qos().times.nack_response_delay.seconds = 0;
        datawriter_qos_.reliable_writer_qos().times.nack_response_delay.nanosec = 100000000;

        // Increase default max_blocking_time to 1 second, as our CI infrastructure shows some
        // big CPU overhead sometimes
        datawriter_qos_.reliability().max_blocking_time.seconds = 1;
        datawriter_qos_.reliability().max_blocking_time.nanosec = 0;
    }

    virtual ~PubSubWriter()
    {
        destroy();
    }

    eprosima::fastdds::dds::DataWriter& get_native_writer() const
    {
        return *datawriter_;
    }

    void init()
    {
        ASSERT_FALSE(initialized_);
        matched_ = 0;

        if (!xml_file_.empty())
        {
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_file_);
            if (!participant_profile_.empty())
            {
                if (use_domain_id_from_profile_)
                {
                    participant_ = DomainParticipantFactory::get_instance()->create_participant_with_profile(
                        participant_profile_,
                        &participant_listener_,
                        eprosima::fastdds::dds::StatusMask::none());
                }
                else
                {
                    participant_ = DomainParticipantFactory::get_instance()->create_participant_with_profile(
                        (uint32_t)GET_PID() % 230,
                        participant_profile_,
                        &participant_listener_,
                        eprosima::fastdds::dds::StatusMask::none());
                }

                ASSERT_NE(participant_, nullptr);
                ASSERT_TRUE(participant_->is_enabled());
            }
        }
        if (participant_ == nullptr)
        {
            participant_ = DomainParticipantFactory::get_instance()->create_participant(
                (use_preferred_domain_id_ ? preferred_domain_id_ : (uint32_t)GET_PID() % 230),
                participant_qos_,
                &participant_listener_,
                eprosima::fastdds::dds::StatusMask::none());
        }

        if (participant_ != nullptr)
        {
            participant_guid_ = participant_->guid();

            type_.reset(new type_support());

            // Register type
            ASSERT_EQ(participant_->register_type(type_), eprosima::fastdds::dds::RETCODE_OK);

            // Create topic
            topic_ = participant_->create_topic(topic_name_, type_->get_name(),
                            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            ASSERT_NE(topic_, nullptr);
            ASSERT_TRUE(topic_->is_enabled());

            // Create publisher
            createPublisher();
        }

        return;
    }

    virtual void createPublisher()
    {
        if (participant_ != nullptr)
        {
            // Create publisher
            publisher_ = participant_->create_publisher(publisher_qos_);
            ASSERT_NE(publisher_, nullptr);
            ASSERT_TRUE(publisher_->is_enabled());

            if (!xml_file_.empty())
            {
                if (!datawriter_profile_.empty())
                {
                    datawriter_ = publisher_->create_datawriter_with_profile(topic_, datawriter_profile_, &listener_,
                                    status_mask_);
                    ASSERT_NE(datawriter_, nullptr);
                    ASSERT_TRUE(datawriter_->is_enabled());
                }
            }
            if (datawriter_ == nullptr)
            {
                datawriter_ = publisher_->create_datawriter(topic_, datawriter_qos_, &listener_, status_mask_);
            }

            if (datawriter_ != nullptr)
            {
                datawriter_guid_ = datawriter_->guid();
                std::cout << "Created datawriter " << datawriter_guid_ << " for topic " <<
                    topic_name_ << std::endl;

                initialized_ = datawriter_->is_enabled();
            }
        }
        return;
    }

    void removePublisher()
    {
        initialized_ = false;
        if (datawriter_ != nullptr)
        {
            publisher_->delete_datawriter(datawriter_);
        }
        datawriter_ = nullptr;
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }
        publisher_ = nullptr;
        matched_ = 0;
        return;
    }

    bool isInitialized() const
    {
        return initialized_;
    }

    eprosima::fastdds::dds::DomainParticipant* getParticipant()
    {
        return participant_;
    }

    virtual void destroy()
    {
        if (participant_)
        {
            if (datawriter_)
            {
                publisher_->delete_datawriter(datawriter_);
                datawriter_ = nullptr;
            }
            if (publisher_)
            {
                participant_->delete_publisher(publisher_);
                publisher_ = nullptr;
            }
            if (topic_)
            {
                participant_->delete_topic(topic_);
                topic_ = nullptr;
            }
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
            participant_ = nullptr;
        }

        initialized_ = false;
    }

    void send(
            std::list<type>& msgs,
            uint32_t milliseconds = 0)
    {
        auto it = msgs.begin();

        while (it != msgs.end())
        {
            if (eprosima::fastdds::dds::RETCODE_OK == datawriter_->write((void*)&(*it)))
            {
                default_send_print<type>(*it);
                it = msgs.erase(it);
                if (milliseconds > 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
                }
            }
            else
            {
                break;
            }
        }
    }

    eprosima::fastdds::rtps::InstanceHandle_t register_instance(
            type& msg)
    {
        return datawriter_->register_instance((void*)&msg);
    }

    bool unregister_instance(
            type& msg,
            const eprosima::fastdds::rtps::InstanceHandle_t& instance_handle)
    {
        return eprosima::fastdds::dds::RETCODE_OK == datawriter_->unregister_instance((void*)&msg, instance_handle);
    }

    bool dispose(
            type& msg,
            const eprosima::fastdds::rtps::InstanceHandle_t& instance_handle)
    {
        return eprosima::fastdds::dds::RETCODE_OK == datawriter_->dispose((void*)&msg, instance_handle);
    }

    bool send_sample(
            type& msg)
    {
        default_send_print(msg);
        return (eprosima::fastdds::dds::RETCODE_OK == datawriter_->write((void*)&msg));
    }

    eprosima::fastdds::dds::ReturnCode_t send_sample(
            type& msg,
            const eprosima::fastdds::dds::InstanceHandle_t& instance_handle)
    {
        default_send_print(msg);
        return datawriter_->write((void*)&msg, instance_handle);
    }

    void assert_liveliness()
    {
        datawriter_->assert_liveliness();
    }

    void wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting discovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            cv_.wait(lock, [&]()
                    {
                        return matched_ != 0;
                    });
        }
        else
        {
            cv_.wait_for(lock, timeout, [&]()
                    {
                        return matched_ != 0;
                    });
        }

        std::cout << "Writer discovery finished..." << std::endl;
    }

    void wait_discovery(
            unsigned int expected_match,
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting discovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            cv_.wait(lock, [&]()
                    {
                        return matched_ == expected_match;
                    });
        }
        else
        {
            cv_.wait_for(lock, timeout, [&]()
                    {
                        return matched_ == expected_match;
                    });
        }

        std::cout << "Writer discovery finished..." << std::endl;
    }

    bool wait_participant_undiscovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        bool ret_value = true;
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting undiscovery..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            cv_.wait(lock, [&]()
                    {
                        return participant_matched_ == 0;
                    });
        }
        else
        {
            if (!cv_.wait_for(lock, timeout, [&]()
                    {
                        return participant_matched_ == 0;
                    }))
            {
                ret_value = false;
            }
        }

        if (ret_value)
        {
            std::cout << "Writer undiscovery finished successfully..." << std::endl;
        }
        else
        {
            std::cout << "Writer undiscovery finished unsuccessfully..." << std::endl;
        }

        return ret_value;
    }

    void wait_reader_undiscovery(
            unsigned int matched = 0)
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting removal..." << std::endl;

        cv_.wait(lock, [&]()
                {
                    return matched_ <= matched;
                });

        std::cout << "Writer removal finished..." << std::endl;
    }

    bool wait_reader_undiscovery(
            std::chrono::seconds timeout,
            unsigned int matched = 0)
    {
        bool ret_value = true;
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting removal..." << std::endl;

        if (!cv_.wait_for(lock, timeout, [&]()
                {
                    return matched_ <= matched;
                }))
        {
            ret_value = false;
        }

        if (ret_value)
        {
            std::cout << "Writer removal finished successfully..." << std::endl;
        }
        else
        {
            std::cout << "Writer removal finished unsuccessfully..." << std::endl;
        }

        return ret_value;
    }

    void wait_liveliness_lost(
            unsigned int times = 1)
    {
        std::unique_lock<std::mutex> lock(liveliness_mutex_);
        liveliness_cv_.wait(lock, [&]()
                {
                    return times_liveliness_lost_ >= times;
                });
    }

    void liveliness_lost()
    {
        std::unique_lock<std::mutex> lock(liveliness_mutex_);
        times_liveliness_lost_++;
        liveliness_cv_.notify_one();
    }

    void wait_incompatible_qos(
            unsigned int times = 1)
    {
        std::unique_lock<std::mutex> lock(incompatible_qos_mutex_);
        incompatible_qos_cv_.wait(lock, [&]()
                {
                    return times_incompatible_qos_ >= times;
                });
    }

    void incompatible_qos(
            eprosima::fastdds::dds::OfferedIncompatibleQosStatus status)
    {
        std::unique_lock<std::mutex> lock(incompatible_qos_mutex_);
        times_incompatible_qos_ = status.total_count;
        last_incompatible_qos_ = status.last_policy_id;
        incompatible_qos_cv_.notify_one();
    }

#if HAVE_SECURITY
    void waitAuthorized(
            std::chrono::seconds timeout = std::chrono::seconds::zero(),
            unsigned int expected = 1)
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "Writer is waiting authorization..." << std::endl;

        if (timeout == std::chrono::seconds::zero())
        {
            cvAuthentication_.wait(lock, [&]()
                    {
                        return authorized_ >= expected;
                    });
        }
        else
        {
            cvAuthentication_.wait_for(lock, timeout, [&]()
                    {
                        return authorized_ >= expected;
                    });
        }

        std::cout << "Writer authorization finished..." << std::endl;
    }

    void waitUnauthorized()
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "Writer is waiting unauthorization..." << std::endl;

        cvAuthentication_.wait(lock, [&]() -> bool
                {
                    return unauthorized_ > 0;
                });

        std::cout << "Writer unauthorization finished..." << std::endl;
    }

#endif // if HAVE_SECURITY

    template<class _Rep,
            class _Period
            >
    bool waitForAllAcked(
            const std::chrono::duration<_Rep, _Period>& max_wait)
    {
        auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(max_wait);
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
        nsecs -= secs;
        eprosima::fastdds::dds::Duration_t timeout {static_cast<int32_t>(secs.count()),
                                                    static_cast<uint32_t>(nsecs.count())};
        return (eprosima::fastdds::dds::RETCODE_OK ==
               datawriter_->wait_for_acknowledgments(timeout));
    }

    template<class _Rep,
            class _Period
            >
    bool waitForInstanceAcked(
            void* data,
            const eprosima::fastdds::rtps::InstanceHandle_t& instance_handle,
            const std::chrono::duration<_Rep, _Period>& max_wait)
    {
        auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(max_wait);
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
        nsecs -= secs;
        eprosima::fastdds::dds::Duration_t timeout {static_cast<int32_t>(secs.count()),
                                                    static_cast<uint32_t>(nsecs.count())};
        return (eprosima::fastdds::dds::RETCODE_OK ==
               datawriter_->wait_for_acknowledgments(data, instance_handle, timeout));
    }

    void block_until_discover_topic(
            const std::string& topicName,
            int repeatedTimes)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        cvEntitiesInfoList_.wait(lock, [&]()
                {
                    int times = mapTopicCountList_.count(topicName) == 0 ? 0 : mapTopicCountList_[topicName];
                    return times == repeatedTimes;
                });
    }

    void block_until_discover_partition(
            const std::string& partition,
            int repeatedTimes)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        cvEntitiesInfoList_.wait(lock, [&]()
                {
                    int times = mapPartitionCountList_.count(partition) == 0 ? 0 : mapPartitionCountList_[partition];
                    return times == repeatedTimes;
                });
    }

    eprosima::fastdds::dds::DataWriterQos& qos()
    {
        return datawriter_qos_;
    }

    PubSubWriter& deactivate_status_listener(
            eprosima::fastdds::dds::StatusMask mask)
    {
        status_mask_ &= ~mask;
        return *this;
    }

    PubSubWriter& activate_status_listener(
            eprosima::fastdds::dds::StatusMask mask)
    {
        status_mask_ |= mask;
        return *this;
    }

    PubSubWriter& reset_status_listener()
    {
        status_mask_ = eprosima::fastdds::dds::StatusMask::all();
        return *this;
    }

    PubSubWriter& set_domain_id(
            const uint32_t& domain_id)
    {
        use_preferred_domain_id_ = true;
        preferred_domain_id_ = domain_id;
        return *this;
    }

    void set_on_discovery_function(
            std::function<bool(const eprosima::fastdds::rtps::ParticipantBuiltinTopicData&,
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus)> f)
    {
        onDiscovery_ = f;
    }

    /*** Function to change QoS ***/
    PubSubWriter& reliability(
            const eprosima::fastdds::dds::ReliabilityQosPolicyKind kind)
    {
        datawriter_qos_.reliability().kind = kind;
        return *this;
    }

    PubSubWriter& reliability(
            const eprosima::fastdds::dds::ReliabilityQosPolicyKind kind,
            eprosima::fastdds::dds::Duration_t max_blocking_time)
    {
        datawriter_qos_.reliability().kind = kind;
        datawriter_qos_.reliability().max_blocking_time = max_blocking_time;
        return *this;
    }

    PubSubWriter& mem_policy(
            const eprosima::fastdds::rtps::MemoryManagementPolicy mem_policy)
    {
        datawriter_qos_.endpoint().history_memory_policy = mem_policy;
        return *this;
    }

    PubSubWriter& deadline_period(
            const eprosima::fastdds::dds::Duration_t deadline_period)
    {
        datawriter_qos_.deadline().period = deadline_period;
        return *this;
    }

    PubSubWriter& liveliness_kind(
            const eprosima::fastdds::dds::LivelinessQosPolicyKind kind)
    {
        datawriter_qos_.liveliness().kind = kind;
        return *this;
    }

    PubSubWriter& liveliness_lease_duration(
            const eprosima::fastdds::dds::Duration_t lease_duration)
    {
        datawriter_qos_.liveliness().lease_duration = lease_duration;
        return *this;
    }

    PubSubWriter& latency_budget_duration(
            const eprosima::fastdds::dds::Duration_t& latency_duration)
    {
        datawriter_qos_.latency_budget().duration = latency_duration;
        return *this;
    }

    eprosima::fastdds::dds::Duration_t get_latency_budget_duration()
    {
        return datawriter_qos_.latency_budget().duration;
    }

    PubSubWriter& liveliness_announcement_period(
            const eprosima::fastdds::dds::Duration_t announcement_period)
    {
        datawriter_qos_.liveliness().announcement_period = announcement_period;
        return *this;
    }

    PubSubWriter& lifespan_period(
            const eprosima::fastdds::dds::Duration_t lifespan_period)
    {
        datawriter_qos_.lifespan().duration = lifespan_period;
        return *this;
    }

    PubSubWriter& keep_duration(
            const eprosima::fastdds::dds::Duration_t duration)
    {
        datawriter_qos_.reliable_writer_qos().disable_positive_acks.enabled = true;
        datawriter_qos_.reliable_writer_qos().disable_positive_acks.duration = duration;
        return *this;
    }

    PubSubWriter& disable_heartbeat_piggyback(
            bool value)
    {
        datawriter_qos_.reliable_writer_qos().disable_heartbeat_piggyback = value;
        return *this;
    }

    PubSubWriter& max_blocking_time(
            const eprosima::fastdds::dds::Duration_t time)
    {
        datawriter_qos_.reliability().max_blocking_time = time;
        return *this;
    }

    PubSubWriter& add_flow_controller_descriptor_to_pparams(
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy scheduler_policy,
            uint32_t bytesPerPeriod,
            uint32_t periodInMs)
    {
        auto new_flow_controller = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
        new_flow_controller->name = "MyFlowController";
        new_flow_controller->scheduler = scheduler_policy;
        new_flow_controller->max_bytes_per_period = bytesPerPeriod;
        new_flow_controller->period_ms = static_cast<uint64_t>(periodInMs);
        participant_qos_.flow_controllers().push_back(new_flow_controller);
        datawriter_qos_.publish_mode().flow_controller_name = new_flow_controller->name;

        return *this;
    }

    PubSubWriter& add_builtin_flow_controller(
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy scheduler_policy,
            uint32_t bytesPerPeriod,
            uint32_t periodInMs)
    {
        auto new_flow_controller = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
        new_flow_controller->name = "MyBuiltinFlowController";
        new_flow_controller->scheduler = scheduler_policy;
        new_flow_controller->max_bytes_per_period = bytesPerPeriod;
        new_flow_controller->period_ms = static_cast<uint64_t>(periodInMs);
        participant_qos_.flow_controllers().push_back(new_flow_controller);
        participant_qos_.wire_protocol().builtin.flow_controller_name = new_flow_controller->name;

        return *this;
    }

    PubSubWriter& asynchronously(
            const eprosima::fastdds::dds::PublishModeQosPolicyKind kind)
    {
        datawriter_qos_.publish_mode().kind = kind;
        return *this;
    }

    PubSubWriter& history_kind(
            const eprosima::fastdds::dds::HistoryQosPolicyKind kind)
    {
        datawriter_qos_.history().kind = kind;
        return *this;
    }

    PubSubWriter& history_depth(
            const int32_t depth)
    {
        datawriter_qos_.history().depth = depth;
        return *this;
    }

    PubSubWriter& setup_transports(
            BuiltinTransports transports)
    {
        participant_qos_.setup_transports(transports);
        return *this;
    }

    PubSubWriter& setup_transports(
            BuiltinTransports transports,
            const BuiltinTransportsOptions& options)
    {
        participant_qos_.setup_transports(transports, options);
        return *this;
    }

    PubSubWriter& setup_large_data_tcp(
            bool v6 = false,
            const uint16_t& port = 0,
            const BuiltinTransportsOptions& options = BuiltinTransportsOptions())
    {
        participant_qos_.transport().use_builtin_transports = false;
        participant_qos_.transport().max_msg_size_no_frag = options.maxMessageSize;

        /* Transports configuration */
        // UDP transport for PDP over multicast
        // TCP transport for EDP and application data (The listening port must to be unique for
        // each participant in the same host)
        uint16_t tcp_listening_port = port;
        if (v6)
        {
            auto pdp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
            pdp_transport->maxMessageSize = options.maxMessageSize;
            pdp_transport->sendBufferSize = options.sockets_buffer_size;
            pdp_transport->receiveBufferSize = options.sockets_buffer_size;
            participant_qos_.transport().user_transports.push_back(pdp_transport);

            auto data_transport = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
            data_transport->add_listener_port(tcp_listening_port);
            data_transport->calculate_crc = false;
            data_transport->check_crc = false;
            data_transport->apply_security = false;
            data_transport->enable_tcp_nodelay = true;
            data_transport->maxMessageSize = options.maxMessageSize;
            data_transport->sendBufferSize = options.sockets_buffer_size;
            data_transport->receiveBufferSize = options.sockets_buffer_size;
            data_transport->tcp_negotiation_timeout = options.tcp_negotiation_timeout;
            participant_qos_.transport().user_transports.push_back(data_transport);
        }
        else
        {
            auto pdp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
            pdp_transport->maxMessageSize = options.maxMessageSize;
            pdp_transport->sendBufferSize = options.sockets_buffer_size;
            pdp_transport->receiveBufferSize = options.sockets_buffer_size;
            participant_qos_.transport().user_transports.push_back(pdp_transport);

            auto data_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
            data_transport->add_listener_port(tcp_listening_port);
            data_transport->calculate_crc = false;
            data_transport->check_crc = false;
            data_transport->apply_security = false;
            data_transport->enable_tcp_nodelay = true;
            data_transport->maxMessageSize = options.maxMessageSize;
            data_transport->sendBufferSize = options.sockets_buffer_size;
            data_transport->receiveBufferSize = options.sockets_buffer_size;
            data_transport->tcp_negotiation_timeout = options.tcp_negotiation_timeout;
            participant_qos_.transport().user_transports.push_back(data_transport);
        }

        /* Locators */
        eprosima::fastdds::rtps::Locator_t pdp_locator;
        eprosima::fastdds::rtps::Locator_t tcp_locator;
        if (v6)
        {
            // Define locator for PDP over multicast
            pdp_locator.kind = LOCATOR_KIND_UDPv6;
            eprosima::fastdds::rtps::IPLocator::setIPv6(pdp_locator, "ff1e::ffff:efff:1");
            // Define locator for EDP and user data
            tcp_locator.kind = LOCATOR_KIND_TCPv6;
            eprosima::fastdds::rtps::IPLocator::setIPv6(tcp_locator, "::");
            eprosima::fastdds::rtps::IPLocator::setPhysicalPort(tcp_locator, tcp_listening_port);
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(tcp_locator, 0);
        }
        else
        {
            // Define locator for PDP over multicast
            pdp_locator.kind = LOCATOR_KIND_UDPv4;
            eprosima::fastdds::rtps::IPLocator::setIPv4(pdp_locator, "239.255.0.1");
            // Define locator for EDP and user data
            tcp_locator.kind = LOCATOR_KIND_TCPv4;
            eprosima::fastdds::rtps::IPLocator::setIPv4(tcp_locator, "0.0.0.0");
            eprosima::fastdds::rtps::IPLocator::setPhysicalPort(tcp_locator, tcp_listening_port);
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(tcp_locator, 0);
        }

        participant_qos_.wire_protocol().builtin.metatrafficMulticastLocatorList.push_back(pdp_locator);
        participant_qos_.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(tcp_locator);
        participant_qos_.wire_protocol().default_unicast_locator_list.push_back(tcp_locator);

        return *this;
    }

    PubSubWriter& setup_p2p_transports()
    {
        participant_qos_.setup_transports(eprosima::fastdds::rtps::BuiltinTransports::P2P);
        return *this;
    }

    PubSubWriter& disable_builtin_transport()
    {
        participant_qos_.transport().use_builtin_transports = false;
        return *this;
    }

    PubSubWriter& set_wire_protocol_qos(
            const eprosima::fastdds::dds::WireProtocolConfigQos& qos)
    {
        participant_qos_.wire_protocol() = qos;
        return *this;
    }

    PubSubWriter& add_user_transport_to_pparams(
            std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> userTransportDescriptor)
    {
        participant_qos_.transport().user_transports.push_back(userTransportDescriptor);
        return *this;
    }

    PubSubWriter& durability_kind(
            const eprosima::fastdds::dds::DurabilityQosPolicyKind kind)
    {
        datawriter_qos_.durability().kind = kind;
        return *this;
    }

    PubSubWriter& resource_limits_allocated_samples(
            const int32_t initial)
    {
        datawriter_qos_.resource_limits().allocated_samples = initial;
        return *this;
    }

    PubSubWriter& resource_limits_max_samples(
            const int32_t max)
    {
        datawriter_qos_.resource_limits().max_samples = max;
        return *this;
    }

    PubSubWriter& resource_limits_max_instances(
            const int32_t max)
    {
        datawriter_qos_.resource_limits().max_instances = max;
        return *this;
    }

    PubSubWriter& resource_limits_max_samples_per_instance(
            const int32_t max)
    {
        datawriter_qos_.resource_limits().max_samples_per_instance = max;
        return *this;
    }

    PubSubWriter& resource_limits_extra_samples(
            const int32_t extra)
    {
        datawriter_qos_.resource_limits().extra_samples = extra;
        return *this;
    }

    PubSubWriter& matched_readers_allocation(
            size_t initial,
            size_t maximum)
    {
        datawriter_qos_.writer_resource_limits().matched_subscriber_allocation.initial = initial;
        datawriter_qos_.writer_resource_limits().matched_subscriber_allocation.maximum = maximum;
        return *this;
    }

    PubSubWriter& participants_allocation_properties(
            size_t initial,
            size_t maximum)
    {
        participant_qos_.allocation().participants.initial = initial;
        participant_qos_.allocation().participants.maximum = maximum;
        return *this;
    }

    PubSubWriter& expect_no_allocs()
    {
        // TODO(Mcc): Add no allocations check code when feature is completely ready
        return *this;
    }

    PubSubWriter& heartbeat_period_seconds(
            int32_t sec)
    {
        datawriter_qos_.reliable_writer_qos().times.heartbeat_period.seconds = sec;
        return *this;
    }

    PubSubWriter& heartbeat_period_nanosec(
            uint32_t nanosec)
    {
        datawriter_qos_.reliable_writer_qos().times.heartbeat_period.nanosec = nanosec;
        return *this;
    }

    PubSubWriter& unicastLocatorList(
            const eprosima::fastdds::rtps::LocatorList& unicastLocators)
    {
        datawriter_qos_.endpoint().unicast_locator_list = unicastLocators;
        return *this;
    }

    PubSubWriter& add_to_unicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        eprosima::fastdds::rtps::Locator loc;
        if (!IPLocator::setIPv4(loc, ip))
        {
            loc.kind = LOCATOR_KIND_UDPv6;
            if (!IPLocator::setIPv6(loc, ip))
            {
                return *this;
            }
        }

        loc.port = port;
        datawriter_qos_.endpoint().unicast_locator_list.push_back(loc);

        return *this;
    }

    PubSubWriter& multicastLocatorList(
            const eprosima::fastdds::rtps::LocatorList& multicastLocators)
    {
        datawriter_qos_.endpoint().multicast_locator_list = multicastLocators;
        return *this;
    }

    PubSubWriter& add_to_multicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        eprosima::fastdds::rtps::Locator loc;
        if (!IPLocator::setIPv4(loc, ip))
        {
            loc.kind = LOCATOR_KIND_UDPv6;
            if (!IPLocator::setIPv6(loc, ip))
            {
                return *this;
            }
        }

        loc.port = port;
        datawriter_qos_.endpoint().multicast_locator_list.push_back(loc);

        return *this;
    }

    PubSubWriter& metatraffic_unicast_locator_list(
            const eprosima::fastdds::rtps::LocatorList& unicastLocators)
    {
        participant_qos_.wire_protocol().builtin.metatrafficUnicastLocatorList = unicastLocators;
        return *this;
    }

    PubSubWriter& add_to_metatraffic_unicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        eprosima::fastdds::rtps::Locator loc;
        if (!IPLocator::setIPv4(loc, ip))
        {
            loc.kind = LOCATOR_KIND_UDPv6;
            if (!IPLocator::setIPv6(loc, ip))
            {
                return *this;
            }
        }

        loc.port = port;
        participant_qos_.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(loc);

        return *this;
    }

    PubSubWriter& metatraffic_multicast_locator_list(
            const eprosima::fastdds::rtps::LocatorList& unicastLocators)
    {
        participant_qos_.wire_protocol().builtin.metatrafficMulticastLocatorList = unicastLocators;
        return *this;
    }

    PubSubWriter& add_to_metatraffic_multicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        eprosima::fastdds::rtps::Locator loc;
        if (!IPLocator::setIPv4(loc, ip))
        {
            loc.kind = LOCATOR_KIND_UDPv6;
            if (!IPLocator::setIPv6(loc, ip))
            {
                return *this;
            }
        }

        loc.port = port;
        participant_qos_.wire_protocol().builtin.metatrafficMulticastLocatorList.push_back(loc);

        return *this;
    }

    PubSubWriter& set_default_unicast_locators(
            const eprosima::fastdds::rtps::LocatorList& locators)
    {
        participant_qos_.wire_protocol().default_unicast_locator_list = locators;
        return *this;
    }

    PubSubWriter& add_to_default_unicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        eprosima::fastdds::rtps::Locator loc;
        if (!IPLocator::setIPv4(loc, ip))
        {
            loc.kind = LOCATOR_KIND_UDPv6;
            if (!IPLocator::setIPv6(loc, ip))
            {
                return *this;
            }
        }

        loc.port = port;
        participant_qos_.wire_protocol().default_unicast_locator_list.push_back(loc);

        return *this;
    }

    PubSubWriter& set_default_multicast_locators(
            const eprosima::fastdds::rtps::LocatorList& locators)
    {
        participant_qos_.wire_protocol().default_multicast_locator_list = locators;
        return *this;
    }

    PubSubWriter& add_to_default_multicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        eprosima::fastdds::rtps::Locator loc;
        if (!IPLocator::setIPv4(loc, ip))
        {
            loc.kind = LOCATOR_KIND_UDPv6;
            if (!IPLocator::setIPv6(loc, ip))
            {
                return *this;
            }
        }

        loc.port = port;
        participant_qos_.wire_protocol().default_multicast_locator_list.push_back(loc);

        return *this;
    }

    PubSubWriter& initial_peers(
            const eprosima::fastdds::rtps::LocatorList& initial_peers)
    {
        participant_qos_.wire_protocol().builtin.initialPeersList = initial_peers;
        return *this;
    }

    PubSubWriter& static_discovery(
            const char* filename)
    {
        participant_qos_.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = false;
        participant_qos_.wire_protocol().builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol = true;
        participant_qos_.wire_protocol().builtin.discovery_config.static_edp_xml_config(filename);
        return *this;
    }

    PubSubWriter& avoid_builtin_multicast(
            bool value)
    {
        participant_qos_.wire_protocol().builtin.avoid_builtin_multicast = value;
        return *this;
    }

    PubSubWriter& property_policy(
            const eprosima::fastdds::rtps::PropertyPolicy& property_policy)
    {
        participant_qos_.properties() = property_policy;
        return *this;
    }

    PubSubWriter& entity_property_policy(
            const eprosima::fastdds::rtps::PropertyPolicy& property_policy)
    {
        datawriter_qos_.properties() = property_policy;
        return *this;
    }

    PubSubWriter& setPublisherIDs(
            uint8_t UserID,
            uint8_t EntityID)
    {
        datawriter_qos_.endpoint().user_defined_id = UserID;
        datawriter_qos_.endpoint().entity_id = EntityID;
        return *this;
    }

    PubSubWriter& setManualTopicName(
            std::string topicName)
    {
        topic_name_ = topicName;
        return *this;
    }

    PubSubWriter& disable_multicast(
            int32_t participantId)
    {
        participant_qos_.wire_protocol().participant_id = participantId;

        eprosima::fastdds::rtps::LocatorList default_unicast_locators;
        eprosima::fastdds::rtps::Locator default_unicast_locator;
        eprosima::fastdds::rtps::Locator loopback_locator;
        if (!use_udpv4)
        {
            default_unicast_locator.kind = LOCATOR_KIND_UDPv6;
            loopback_locator.kind = LOCATOR_KIND_UDPv6;
        }

        default_unicast_locators.push_back(default_unicast_locator);
        participant_qos_.wire_protocol().builtin.metatrafficUnicastLocatorList = default_unicast_locators;

        if (!IPLocator::setIPv4(loopback_locator, 127, 0, 0, 1))
        {
            IPLocator::setIPv6(loopback_locator, "::1");
        }
        participant_qos_.wire_protocol().builtin.initialPeersList.push_back(loopback_locator);
        return *this;
    }

    PubSubWriter& partition(
            const std::string& partition)
    {
        publisher_qos_.partition().push_back(partition.c_str());
        return *this;
    }

    PubSubWriter& user_data(
            std::vector<eprosima::fastdds::rtps::octet> user_data)
    {
        participant_qos_.user_data() = user_data;
        return *this;
    }

    PubSubWriter& endpoint_userData(
            std::vector<eprosima::fastdds::rtps::octet> user_data)
    {
        datawriter_qos_.user_data() = user_data;
        return *this;
    }

    PubSubWriter& user_data_max_size(
            uint32_t max_user_data)
    {
        participant_qos_.allocation().data_limits.max_user_data = max_user_data;
        return *this;
    }

    PubSubWriter& properties_max_size(
            uint32_t max_properties)
    {
        participant_qos_.allocation().data_limits.max_properties = max_properties;
        return *this;
    }

    PubSubWriter& partitions_max_size(
            uint32_t max_partitions)
    {
        participant_qos_.allocation().data_limits.max_partitions = max_partitions;
        return *this;
    }

    PubSubWriter& max_multicast_locators_number(
            size_t max_multicast_locators)
    {
        participant_qos_.allocation().locators.max_multicast_locators = max_multicast_locators;
        return *this;
    }

    PubSubWriter& lease_duration(
            eprosima::fastdds::dds::Duration_t lease_duration,
            eprosima::fastdds::dds::Duration_t announce_period)
    {
        participant_qos_.wire_protocol().builtin.discovery_config.leaseDuration = lease_duration;
        participant_qos_.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = announce_period;
        return *this;
    }

    PubSubWriter& initial_announcements(
            uint32_t count,
            const eprosima::fastdds::dds::Duration_t& period)
    {
        participant_qos_.wire_protocol().builtin.discovery_config.initial_announcements.count = count;
        participant_qos_.wire_protocol().builtin.discovery_config.initial_announcements.period = period;
        return *this;
    }

    PubSubWriter& ownership_strength(
            uint32_t strength)
    {
        datawriter_qos_.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
        datawriter_qos_.ownership_strength().value = strength;
        return *this;
    }

    PubSubWriter& load_publisher_attr(
            const std::string& /*xml*/)
    {
        /*TODO
           std::unique_ptr<eprosima::fastdds::xmlparser::BaseNode> root;
           if (eprosima::fastdds::xmlparser::XMLParser::loadXML(xml.data(), xml.size(),
                root) == eprosima::fastdds::xmlparser::XMLP_ret::XML_OK)
           {
            for (const auto& profile : root->getChildren())
            {
                if (profile->getType() == eprosima::fastdds::xmlparser::NodeType::PUBLISHER)
                {
                    datawriter_qos_ =
         *(dynamic_cast<eprosima::fastdds::xmlparser::DataNode<eprosima::fastdds::xmlparser::PublisherAttributes>
         *>(
                                profile.get())->get());
                }
            }
           }
         */
        publisher_qos_.partition().push_back("A");
        return *this;
    }

    PubSubWriter& max_initial_peers_range(
            uint32_t maxInitialPeerRange)
    {
        participant_qos_.transport().use_builtin_transports = false;
        std::shared_ptr<UDPTransportDescriptor> descriptor;
        if (use_udpv4)
        {
            descriptor = std::make_shared<UDPv4TransportDescriptor>();
        }
        else
        {
            descriptor = std::make_shared<UDPv6TransportDescriptor>();
        }
        descriptor->maxInitialPeersRange = maxInitialPeerRange;
        participant_qos_.transport().user_transports.push_back(descriptor);
        return *this;
    }

    PubSubWriter& socket_buffer_size(
            uint32_t sockerBufferSize)
    {
        participant_qos_.transport().listen_socket_buffer_size = sockerBufferSize;
        participant_qos_.transport().send_socket_buffer_size = sockerBufferSize;
        return *this;
    }

    PubSubWriter& guid_prefix(
            const eprosima::fastdds::rtps::GuidPrefix_t& prefix)
    {
        participant_qos_.wire_protocol().prefix = prefix;
        return *this;
    }

    PubSubWriter& participant_id(
            int32_t participantId)
    {
        participant_qos_.wire_protocol().participant_id = participantId;
        return *this;
    }

    PubSubWriter& datasharing_off()
    {
        datawriter_qos_.data_sharing().off();
        return *this;
    }

    PubSubWriter& datasharing_auto(
            std::vector<uint16_t> domain_id = std::vector<uint16_t>())
    {
        datawriter_qos_.data_sharing().automatic(domain_id);
        return *this;
    }

    PubSubWriter& datasharing_auto(
            const std::string directory,
            std::vector<uint16_t> domain_id = std::vector<uint16_t>())
    {
        datawriter_qos_.data_sharing().automatic(directory, domain_id);
        return *this;
    }

    PubSubWriter& datasharing_on(
            const std::string directory,
            std::vector<uint16_t> domain_id = std::vector<uint16_t>())
    {
        datawriter_qos_.data_sharing().on(directory, domain_id);
        return *this;
    }

    PubSubWriter& set_events_thread_settings(
            const eprosima::fastdds::rtps::ThreadSettings& settings)
    {
        participant_qos_.timed_events_thread(settings);
        return *this;
    }

    const std::string& topic_name() const
    {
        return topic_name_;
    }

    eprosima::fastdds::rtps::GUID_t participant_guid()
    {
        return participant_guid_;
    }

    eprosima::fastdds::rtps::GUID_t datawriter_guid()
    {
        return datawriter_guid_;
    }

    eprosima::fastdds::rtps::InstanceHandle_t datawriter_ihandle()
    {
        return eprosima::fastdds::rtps::InstanceHandle_t(datawriter_guid());
    }

    bool update_partition(
            const std::string& partition)
    {
        publisher_qos_.partition().clear();
        publisher_qos_.partition().push_back(partition.c_str());
        return (eprosima::fastdds::dds::RETCODE_OK == publisher_->set_qos(publisher_qos_));
    }

    bool set_qos()
    {
        return (eprosima::fastdds::dds::RETCODE_OK == datawriter_->set_qos(datawriter_qos_));
    }

    bool set_qos(
            const eprosima::fastdds::dds::DataWriterQos& att)
    {
        return (eprosima::fastdds::dds::RETCODE_OK == datawriter_->set_qos(att));
    }

    eprosima::fastdds::dds::DataWriterQos get_qos()
    {
        return (datawriter_->get_qos());
    }

    bool remove_all_changes(
            size_t* number_of_changes_removed)
    {
        return (eprosima::fastdds::dds::RETCODE_OK == datawriter_->clear_history(number_of_changes_removed));
    }

    bool is_matched() const
    {
        return matched_ > 0;
    }

    unsigned int get_matched() const
    {
        return matched_;
    }

    unsigned int get_participants_matched() const
    {
        return participant_matched_;
    }

    unsigned int missed_deadlines() const
    {
        return listener_.missed_deadlines();
    }

    unsigned int times_liveliness_lost() const
    {
        return listener_.times_liveliness_lost();
    }

    unsigned int times_unack_sample_removed() const
    {
        return listener_.times_unack_sample_removed();
    }

    std::vector<eprosima::fastdds::dds::InstanceHandle_t>& instances_removed_unack()
    {
        return listener_.instances_removed_unack();
    }

    unsigned int times_incompatible_qos() const
    {
        return times_incompatible_qos_;
    }

    eprosima::fastdds::dds::QosPolicyId_t last_incompatible_qos() const
    {
        return last_incompatible_qos_;
    }

    eprosima::fastdds::dds::OfferedIncompatibleQosStatus get_incompatible_qos_status() const
    {
        eprosima::fastdds::dds::OfferedIncompatibleQosStatus status;
        datawriter_->get_offered_incompatible_qos_status(status);
        return status;
    }

    eprosima::fastdds::dds::PublicationMatchedStatus get_publication_matched_status() const
    {
        eprosima::fastdds::dds::PublicationMatchedStatus status;
        datawriter_->get_publication_matched_status(status);
        return status;
    }

    void set_xml_filename(
            const std::string& name)
    {
        xml_file_ = name;
    }

    void set_participant_profile(
            const std::string& profile)
    {
        participant_profile_ = profile;
    }

    void set_participant_profile(
            const std::string& profile,
            bool use_domain_id_from_profile)
    {
        set_participant_profile(profile);
        use_domain_id_from_profile_ = use_domain_id_from_profile;
    }

    void set_datawriter_profile(
            const std::string& profile)
    {
        datawriter_profile_ = profile;
    }

    void participant_set_qos()
    {
        participant_->set_qos(participant_qos_);
    }

#if HAVE_SQLITE3
    PubSubWriter& make_transient(
            const std::string& filename,
            const std::string& persistence_guid)
    {
        add_persitence_properties(filename, persistence_guid);
        durability_kind(eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS);
        return *this;
    }

    PubSubWriter& make_persistent(
            const std::string& filename,
            const std::string& persistence_guid)
    {
        add_persitence_properties(filename, persistence_guid);
        durability_kind(eprosima::fastdds::dds::PERSISTENT_DURABILITY_QOS);
        return *this;
    }

    void add_persitence_properties(
            const std::string& filename,
            const std::string& persistence_guid)
    {
        participant_qos_.properties().properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
        participant_qos_.properties().properties().emplace_back("dds.persistence.sqlite3.filename", filename);
        datawriter_qos_.properties().properties().emplace_back("dds.persistence.guid", persistence_guid);
    }

#endif // if HAVE_SQLITE3

    PubSubWriter& use_writer_liveliness_protocol(
            bool use_wlp)
    {
        participant_qos_.wire_protocol().builtin.use_WriterLivelinessProtocol = use_wlp;
        return *this;
    }

    PubSubWriter& data_representation(
            const std::vector<eprosima::fastdds::dds::DataRepresentationId_t>& values)
    {
        datawriter_qos_.representation().m_value = values;
        return *this;
    }

    eprosima::fastdds::dds::TypeSupport get_type_support()
    {
        return type_;
    }

protected:

    void participant_matched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        ++participant_matched_;
        cv_.notify_one();
    }

    void participant_unmatched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        --participant_matched_;
        cv_.notify_one();
    }

    void matched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        ++matched_;
        cv_.notify_one();
    }

    void unmatched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        --matched_;
        cv_.notify_one();
    }

#if HAVE_SECURITY
    void authorized()
    {
        mutexAuthentication_.lock();
        ++authorized_;
        mutexAuthentication_.unlock();
        cvAuthentication_.notify_all();
    }

    void unauthorized()
    {
        mutexAuthentication_.lock();
        ++unauthorized_;
        mutexAuthentication_.unlock();
        cvAuthentication_.notify_all();
    }

#endif // if HAVE_SECURITY

    void add_writer_info(
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& writer_data)
    {
        mutexEntitiesInfoList_.lock();
        auto ret = mapWriterInfoList_.insert(std::make_pair(writer_data.guid, writer_data));

        if (!ret.second)
        {
            ret.first->second = writer_data;
        }

        auto ret_topic = mapTopicCountList_.insert(std::make_pair(writer_data.topic_name, 1));

        if (!ret_topic.second)
        {
            ++ret_topic.first->second;
        }

        for (auto partition : writer_data.partition.names())
        {
            auto ret_partition = mapPartitionCountList_.insert(std::make_pair(partition, 1));

            if (!ret_partition.second)
            {
                ++ret_partition.first->second;
            }
        }

        mutexEntitiesInfoList_.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void change_writer_info(
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& writer_data)
    {
        mutexEntitiesInfoList_.lock();
        auto ret = mapWriterInfoList_.insert(std::make_pair(writer_data.guid, writer_data));

        ASSERT_FALSE(ret.second);
        eprosima::fastdds::dds::PublicationBuiltinTopicData old_writer_data = ret.first->second;
        ret.first->second = writer_data;

        ASSERT_GT(mapTopicCountList_.count(writer_data.topic_name.to_string()), 0ul);

        // Remove previous partitions
        for (auto partition : old_writer_data.partition.names())
        {
            auto partition_it = mapPartitionCountList_.find(partition);
            ASSERT_TRUE(partition_it != mapPartitionCountList_.end());
            --(*partition_it).second;
            if ((*partition_it).second == 0)
            {
                mapPartitionCountList_.erase(partition);
            }
        }

        // Add new partitions
        for (auto partition : writer_data.partition.names())
        {
            auto ret_partition = mapPartitionCountList_.insert(std::make_pair(partition, 1));

            if (!ret_partition.second)
            {
                ++ret_partition.first->second;
            }
        }

        mutexEntitiesInfoList_.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void add_reader_info(
            const eprosima::fastdds::dds::SubscriptionBuiltinTopicData& reader_data)
    {
        mutexEntitiesInfoList_.lock();
        auto ret = mapReaderInfoList_.insert(std::make_pair(reader_data.guid, reader_data));

        if (!ret.second)
        {
            ret.first->second = reader_data;
        }

        auto ret_topic = mapTopicCountList_.insert(std::make_pair(reader_data.topic_name.to_string(), 1));

        if (!ret_topic.second)
        {
            ++ret_topic.first->second;
        }

        for (auto partition : reader_data.partition.names())
        {
            auto ret_partition = mapPartitionCountList_.insert(std::make_pair(partition, 1));

            if (!ret_partition.second)
            {
                ++ret_partition.first->second;
            }
        }

        mutexEntitiesInfoList_.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void change_reader_info(
            const eprosima::fastdds::dds::SubscriptionBuiltinTopicData& reader_data)
    {
        mutexEntitiesInfoList_.lock();
        auto ret = mapReaderInfoList_.insert(std::make_pair(reader_data.guid, reader_data));

        ASSERT_FALSE(ret.second);
        eprosima::fastdds::dds::SubscriptionBuiltinTopicData old_reader_data = ret.first->second;
        ret.first->second = reader_data;

        ASSERT_GT(mapTopicCountList_.count(reader_data.topic_name.to_string()), 0ul);

        // Remove previous partitions
        for (auto partition : old_reader_data.partition.names())
        {
            auto partition_it = mapPartitionCountList_.find(partition);
            ASSERT_TRUE(partition_it != mapPartitionCountList_.end());
            --(*partition_it).second;
            if ((*partition_it).second == 0)
            {
                mapPartitionCountList_.erase(partition);
            }
        }

        for (auto partition : reader_data.partition.names())
        {
            auto ret_partition = mapPartitionCountList_.insert(std::make_pair(partition, 1));

            if (!ret_partition.second)
            {
                ++ret_partition.first->second;
            }
        }

        mutexEntitiesInfoList_.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void remove_writer_info(
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& writer_data)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        ASSERT_GT(mapWriterInfoList_.count(writer_data.guid), 0ul);

        mapWriterInfoList_.erase(writer_data.guid);

        ASSERT_GT(mapTopicCountList_.count(writer_data.topic_name.to_string()), 0ul);

        --mapTopicCountList_[writer_data.topic_name.to_string()];

        for (auto partition : writer_data.partition.names())
        {
            auto partition_it = mapPartitionCountList_.find(partition);
            ASSERT_TRUE(partition_it != mapPartitionCountList_.end());
            --(*partition_it).second;
            if ((*partition_it).second == 0)
            {
                mapPartitionCountList_.erase(partition);
            }
        }

        lock.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void remove_reader_info(
            const eprosima::fastdds::dds::SubscriptionBuiltinTopicData& reader_data)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        ASSERT_GT(mapReaderInfoList_.count(reader_data.guid), 0ul);

        mapReaderInfoList_.erase(reader_data.guid);

        ASSERT_GT(mapTopicCountList_.count(reader_data.topic_name.to_string()), 0ul);

        --mapTopicCountList_[reader_data.topic_name.to_string()];

        for (auto partition : reader_data.partition.names())
        {
            auto partition_it = mapPartitionCountList_.find(partition);
            ASSERT_TRUE(partition_it != mapPartitionCountList_.end());
            --(*partition_it).second;
            if ((*partition_it).second == 0)
            {
                mapPartitionCountList_.erase(partition);
            }
        }

        lock.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    PubSubWriter& operator =(
            const PubSubWriter&) = delete;

    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::DomainParticipantQos participant_qos_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::PublisherQos publisher_qos_;
    eprosima::fastdds::dds::DataWriter* datawriter_;
    eprosima::fastdds::dds::DataWriterQos datawriter_qos_;
    eprosima::fastdds::dds::StatusMask status_mask_;
    std::string topic_name_;
    eprosima::fastdds::rtps::GUID_t participant_guid_;
    eprosima::fastdds::rtps::GUID_t datawriter_guid_;
    bool initialized_;
    bool use_domain_id_from_profile_;
    std::mutex mutexDiscovery_;
    std::condition_variable cv_;
    std::atomic<unsigned int> matched_;
    unsigned int participant_matched_;
    eprosima::fastdds::dds::TypeSupport type_;
    std::mutex mutexEntitiesInfoList_;
    std::condition_variable cvEntitiesInfoList_;
    std::map<eprosima::fastdds::rtps::GUID_t, eprosima::fastdds::dds::PublicationBuiltinTopicData> mapWriterInfoList_;
    std::map<eprosima::fastdds::rtps::GUID_t, eprosima::fastdds::dds::SubscriptionBuiltinTopicData> mapReaderInfoList_;
    std::map<std::string,  int> mapTopicCountList_;
    std::map<std::string,  int> mapPartitionCountList_;
    bool discovery_result_;

    std::string xml_file_ = "";
    std::string participant_profile_ = "";
    std::string datawriter_profile_ = "";

    std::function<bool(const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& info,
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus status)> onDiscovery_;

    //! A mutex for liveliness
    std::mutex liveliness_mutex_;
    //! A condition variable for liveliness
    std::condition_variable liveliness_cv_;
    //! The number of times liveliness was lost
    unsigned int times_liveliness_lost_;

    //! A mutex for incompatible qos
    std::mutex incompatible_qos_mutex_;
    //! A condition variable for incompatible qos
    std::condition_variable incompatible_qos_cv_;
    //! Number of times incompatible_qos was received
    unsigned int times_incompatible_qos_;
    //! Latest conflicting PolicyId
    eprosima::fastdds::dds::QosPolicyId_t last_incompatible_qos_;
    //! Preferred domain ID
    bool use_preferred_domain_id_;
    uint32_t preferred_domain_id_;

#if HAVE_SECURITY
    std::mutex mutexAuthentication_;
    std::condition_variable cvAuthentication_;
    unsigned int authorized_;
    unsigned int unauthorized_;
#endif // if HAVE_SECURITY
};


template<class TypeSupport>
class PubSubWriterWithWaitsets : public PubSubWriter<TypeSupport>
{
public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

protected:

    class WaitsetThread
    {
    public:

        WaitsetThread(
                PubSubWriterWithWaitsets& writer)
            : writer_(writer)
        {
        }

        ~WaitsetThread()
        {
            stop();
        }

        void start(
                const eprosima::fastdds::dds::Duration_t& timeout)
        {
            waitset_.attach_condition(writer_.datawriter_->get_statuscondition());
            waitset_.attach_condition(guard_condition_);

            std::unique_lock<std::mutex> lock(mutex_);
            if (nullptr == thread_)
            {
                running_ = true;
                guard_condition_.set_trigger_value(false);
                timeout_ = timeout;
                thread_ = new std::thread(&WaitsetThread::run, this);
            }
        }

        void stop()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            running_ = false;
            if (nullptr != thread_)
            {
                lock.unlock();

                // We need to trigger the wake up
                guard_condition_.set_trigger_value(true);
                thread_->join();
                lock.lock();
                delete thread_;
                thread_ = nullptr;
            }
        }

        void run()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (running_)
            {
                lock.unlock();
                auto wait_result = waitset_.wait(active_conditions_, timeout_);
                if (wait_result == eprosima::fastdds::dds::RETCODE_TIMEOUT)
                {
                    writer_.on_waitset_timeout();
                }
                else
                {
                    if (!guard_condition_.get_trigger_value())
                    {
                        ASSERT_FALSE(active_conditions_.empty());
                        EXPECT_EQ(active_conditions_[0], &writer_.datawriter_->get_statuscondition());
                        process(&writer_.datawriter_->get_statuscondition());
                    }
                }
                lock.lock();
            }
        }

        void process(
                eprosima::fastdds::dds::StatusCondition* condition)
        {
            eprosima::fastdds::dds::StatusMask triggered_statuses = writer_.datawriter_->get_status_changes();
            triggered_statuses &= condition->get_enabled_statuses();

            if (triggered_statuses.is_active(eprosima::fastdds::dds::StatusMask::publication_matched()))
            {
                eprosima::fastdds::dds::PublicationMatchedStatus status;
                writer_.datawriter_->get_publication_matched_status(status);

                if (0 < status.current_count_change)
                {
                    std::cout << "Publisher matched subscriber " << status.last_subscription_handle << std::endl;
                    writer_.matched();
                }
                else if (0 > status.current_count_change)
                {
                    std::cout << "Publisher unmatched subscriber " << status.last_subscription_handle << std::endl;
                    writer_.unmatched();
                }
            }

            if (triggered_statuses.is_active(eprosima::fastdds::dds::StatusMask::offered_deadline_missed()))
            {
                eprosima::fastdds::dds::OfferedDeadlineMissedStatus status;
                writer_.datawriter_->get_offered_deadline_missed_status(status);
                times_deadline_missed_ = status.total_count;
            }

            if (triggered_statuses.is_active(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()))
            {
                eprosima::fastdds::dds::OfferedIncompatibleQosStatus status;
                writer_.datawriter_->get_offered_incompatible_qos_status(status);
                writer_.incompatible_qos(status);
            }

            if (triggered_statuses.is_active(eprosima::fastdds::dds::StatusMask::liveliness_lost()))
            {
                eprosima::fastdds::dds::LivelinessLostStatus status;
                writer_.datawriter_->get_liveliness_lost_status(status);

                times_liveliness_lost_ = status.total_count;
                writer_.liveliness_lost();
            }
        }

        unsigned int missed_deadlines() const
        {
            return times_deadline_missed_;
        }

        unsigned int times_liveliness_lost() const
        {
            return times_liveliness_lost_;
        }

    protected:

        // The reader this waitset thread serves
        PubSubWriterWithWaitsets& writer_;

        // The waitset where the thread will be blocked
        eprosima::fastdds::dds::WaitSet waitset_;

        // The active conditions that triggered the wake up
        eprosima::fastdds::dds::ConditionSeq active_conditions_;

        // The thread that does the job
        std::thread* thread_ = nullptr;

        // Whether the thread is running or not
        bool running_ = false;

        // A Mutex to guard the thread start/stop
        std::mutex mutex_;

        // A user-triggered condition used to signal the thread to stop
        eprosima::fastdds::dds::GuardCondition guard_condition_;

        //! The number of times deadline was missed
        unsigned int times_deadline_missed_ = 0;

        //! The number of times liveliness was lost
        unsigned int times_liveliness_lost_ = 0;

        //! The timeout for the wait operation
        eprosima::fastdds::dds::Duration_t timeout_;
    }
    waitset_thread_;

    friend class WaitsetThread;

public:

    PubSubWriterWithWaitsets(
            const std::string& topic_name)
        : PubSubWriter<TypeSupport>(topic_name)
        , waitset_thread_(*this)
        , timeout_(eprosima::fastdds::dds::c_TimeInfinite)
        , times_waitset_timeout_(0)
    {
    }

    ~PubSubWriterWithWaitsets() override
    {
    }

    void createPublisher() override
    {
        if (participant_ != nullptr)
        {
            // Create publisher
            publisher_ = participant_->create_publisher(publisher_qos_);
            ASSERT_NE(publisher_, nullptr);
            ASSERT_TRUE(publisher_->is_enabled());

            if (!xml_file_.empty())
            {
                if (!datawriter_profile_.empty())
                {
                    datawriter_ = publisher_->create_datawriter_with_profile(topic_, datawriter_profile_, nullptr);
                    ASSERT_NE(datawriter_, nullptr);
                    ASSERT_TRUE(datawriter_->is_enabled());
                }
            }
            if (datawriter_ == nullptr)
            {
                datawriter_ = publisher_->create_datawriter(topic_, datawriter_qos_, nullptr);
            }

            if (datawriter_ != nullptr)
            {
                initialized_ = datawriter_->is_enabled();
                if (initialized_)
                {
                    std::cout << "Created datawriter " << datawriter_->guid() << " for topic " <<
                        topic_name_ << std::endl;

                    // Set the desired status condition mask and start the waitset thread
                    datawriter_->get_statuscondition().set_enabled_statuses(status_mask_);
                    waitset_thread_.start(timeout_);
                }
            }
        }
        return;
    }

    void destroy() override
    {
        if (initialized_)
        {
            waitset_thread_.stop();
        }

        PubSubWriter<TypeSupport>::destroy();
    }

    unsigned int missed_deadlines() const
    {
        return waitset_thread_.missed_deadlines();
    }

    unsigned int times_liveliness_lost() const
    {
        return waitset_thread_.times_liveliness_lost();
    }

    void wait_waitset_timeout(
            unsigned int times = 1)
    {
        std::unique_lock<std::mutex> lock(waitset_timeout_mutex_);

        waitset_timeout_cv_.wait(lock, [&]()
                {
                    return times_waitset_timeout_ >= times;
                });
    }

    unsigned int times_waitset_timeout()
    {
        std::unique_lock<std::mutex> lock(waitset_timeout_mutex_);
        return times_waitset_timeout_;
    }

    PubSubWriterWithWaitsets& waitset_timeout(
            const eprosima::fastdds::dds::Duration_t& timeout)
    {
        timeout_ = timeout;
        return *this;
    }

protected:

    void on_waitset_timeout()
    {
        std::unique_lock<std::mutex> lock(waitset_timeout_mutex_);
        ++times_waitset_timeout_;
        waitset_timeout_cv_.notify_one();
    }

    //! The timeout for the waitset
    eprosima::fastdds::dds::Duration_t timeout_;

    //! A mutex for waitset timeout
    std::mutex waitset_timeout_mutex_;
    //! A condition variable to notify when the waitset has timed out
    std::condition_variable waitset_timeout_cv_;
    //! Number of times the waitset has timed out
    unsigned int times_waitset_timeout_;

    using PubSubWriter<TypeSupport>::xml_file_;
    using PubSubWriter<TypeSupport>::participant_;
    using PubSubWriter<TypeSupport>::topic_name_;
    using PubSubWriter<TypeSupport>::topic_;
    using PubSubWriter<TypeSupport>::publisher_;
    using PubSubWriter<TypeSupport>::publisher_qos_;
    using PubSubWriter<TypeSupport>::datawriter_;
    using PubSubWriter<TypeSupport>::datawriter_qos_;
    using PubSubWriter<TypeSupport>::datawriter_profile_;
    using PubSubWriter<TypeSupport>::initialized_;
    using PubSubWriter<TypeSupport>::status_mask_;
};


#endif // _TEST_BLACKBOX_PUBSUBWRITER_HPP_
