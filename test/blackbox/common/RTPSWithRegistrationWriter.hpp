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
 * @file RTPSWithRegistrationWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_
#define _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_

#include <condition_variable>
#include <list>
#include <string>

#include <asio.hpp>
#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <gtest/gtest.h>

#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/transport/TransportDescriptorInterface.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

template<class TypeSupport>
class RTPSWithRegistrationWriter
{
public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    using OnReaderDiscoveryFunctor = std::function <void (
                        eprosima::fastdds::rtps::ReaderDiscoveryStatus,
                        const eprosima::fastdds::rtps::GUID_t&,
                        const eprosima::fastdds::rtps::SubscriptionBuiltinTopicData*
                        )>;

private:

    class Listener : public eprosima::fastdds::rtps::WriterListener
    {
    public:

        Listener(
                RTPSWithRegistrationWriter& writer)
            : writer_(writer)
        {
        }

        ~Listener()
        {
        }

        void on_writer_matched(
                eprosima::fastdds::rtps::RTPSWriter* /*writer*/,
                const eprosima::fastdds::rtps::MatchingInfo& info) override
        {
            if (info.status == eprosima::fastdds::rtps::MATCHED_MATCHING)
            {
                writer_.matched();
            }
            else if (info.status == eprosima::fastdds::rtps::REMOVED_MATCHING)
            {
                writer_.unmatched();
            }
        }

        void on_reader_discovery(
                eprosima::fastdds::rtps::RTPSWriter* writer,
                eprosima::fastdds::rtps::ReaderDiscoveryStatus reason,
                const eprosima::fastdds::rtps::GUID_t& reader_guid,
                const eprosima::fastdds::rtps::SubscriptionBuiltinTopicData* reader_info) override
        {
            writer_.on_reader_discovery(writer, reason, reader_guid, reader_info);
        }

    private:

        Listener& operator =(
                const Listener&) = delete;

        RTPSWithRegistrationWriter& writer_;

    }
    listener_;

public:

    RTPSWithRegistrationWriter(
            const std::string& topic_name)
        : RTPSWithRegistrationWriter(topic_name, nullptr)
    {
    }

    RTPSWithRegistrationWriter(
            const std::string& topic_name,
            eprosima::fastdds::rtps::RTPSParticipant* participant)
        : listener_(*this)
        , participant_(participant)
        , destroy_participant_(nullptr == participant)
        , writer_(nullptr)
        , history_(nullptr)
        , initialized_(false)
        , matched_(0)
    {
        pub_builtin_data_.type_name = type_.get_name();
        // Generate topic name
        std::ostringstream t;
        t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
        pub_builtin_data_.topic_name = t.str();

        // The tests are assuming a default durability of TRANSIENT_LOCAL, which is not the default mandated by the DDS standard.
        pub_builtin_data_.durability.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        writer_attr_.times.heartbeat_period.seconds = 0;
        writer_attr_.times.heartbeat_period.nanosec = 100000000;
        writer_attr_.times.nack_response_delay.seconds = 0;
        writer_attr_.times.nack_response_delay.nanosec = 100000000;

        participant_attr_.builtin.discovery_config.discoveryProtocol =
                eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE;
        participant_attr_.builtin.use_WriterLivelinessProtocol = true;

        if (type_.is_compute_key_provided)
        {
            writer_attr_.endpoint.topicKind = eprosima::fastdds::rtps::WITH_KEY;
        }
        else
        {
            writer_attr_.endpoint.topicKind = eprosima::fastdds::rtps::NO_KEY;
        }
    }

    virtual ~RTPSWithRegistrationWriter()
    {
        destroy();
    }

    void init()
    {
        matched_ = 0;

        //Create participant
        if (nullptr == participant_)
        {
            participant_ = eprosima::fastdds::rtps::RTPSDomain::createParticipant(
                static_cast<uint32_t>(GET_PID()) % 230, participant_attr_);
        }
        ASSERT_NE(participant_, nullptr);

        //Create writerhistory
        hattr_.payloadMaxSize = type_.max_serialized_type_size;
        if (has_payload_pool_)
        {
            history_ = new eprosima::fastdds::rtps::WriterHistory(hattr_, payload_pool_);
        }
        else
        {
            history_ = new eprosima::fastdds::rtps::WriterHistory(hattr_);
        }

        //Create writer
        if (custom_entity_id_ != eprosima::fastdds::rtps::c_EntityId_Unknown)
        {
            writer_ = eprosima::fastdds::rtps::RTPSDomain::createRTPSWriter(
                participant_, custom_entity_id_, writer_attr_, history_, &listener_);
        }
        else
        {
            writer_ = eprosima::fastdds::rtps::RTPSDomain::createRTPSWriter(
                participant_, writer_attr_, history_, &listener_);
        }

        if (writer_ == nullptr)
        {
            return;
        }

        eprosima::fastdds::rtps::TopicDescription topic_desc;
        topic_desc.type_name = type_.get_name();
        topic_desc.topic_name = pub_builtin_data_.topic_name;
        ASSERT_EQ(participant_->register_writer(writer_, topic_desc, writer_qos_), true);

        initialized_ = true;
    }

    void update()
    {
        if (writer_ == nullptr)
        {
            return;
        }

        ASSERT_TRUE(participant_->update_writer(writer_, writer_qos_));
    }

    void destroy()
    {
        if (destroy_participant_ && participant_ != nullptr)
        {
            eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_);
        }
        if (history_ != nullptr)
        {
            delete(history_);
        }

        participant_ = nullptr;
        history_ = nullptr;
        writer_ = nullptr;
        initialized_ = false;
        matched_ = 0;
    }

    bool isInitialized() const
    {
        return initialized_;
    }

    void send(
            std::list<type>& msgs)
    {
        auto it = msgs.begin();

        while (it != msgs.end())
        {
            eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);
            size_t current_alignment{ 0 };
            uint32_t cdr_size = static_cast<uint32_t>(
                calculator.calculate_serialized_size(*it, current_alignment));
            eprosima::fastdds::rtps::CacheChange_t* ch = history_->create_change(
                cdr_size, eprosima::fastdds::rtps::ALIVE);

            eprosima::fastcdr::FastBuffer buffer((char*)ch->serializedPayload.data, ch->serializedPayload.max_size);
            eprosima::fastcdr::Cdr cdr(buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                    eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);

            cdr.serialize_encapsulation();
            cdr << *it;

            ch->serializedPayload.length = static_cast<uint32_t>(cdr.get_serialized_data_length());
            if (ch->serializedPayload.length > 65000u)
            {
                ch->setFragmentSize(65000u);
            }

            history_->add_change(ch);
            it = msgs.erase(it);
        }
    }

    eprosima::fastdds::rtps::CacheChange_t* send_sample(
            type& msg)
    {
        eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);
        size_t current_alignment{ 0 };
        uint32_t cdr_size = static_cast<uint32_t>(calculator.calculate_serialized_size(msg, current_alignment));
        eprosima::fastdds::rtps::CacheChange_t* ch = history_->create_change(
            cdr_size, eprosima::fastdds::rtps::ALIVE);

        eprosima::fastcdr::FastBuffer buffer((char*)ch->serializedPayload.data, ch->serializedPayload.max_size);
        eprosima::fastcdr::Cdr cdr(buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);

        cdr.serialize_encapsulation();
        cdr << msg;

        ch->serializedPayload.length = static_cast<uint32_t>(cdr.get_serialized_data_length());
        if (ch->serializedPayload.length > 65000u)
        {
            ch->setFragmentSize(65000u);
        }

        history_->add_change(ch);
        return ch;
    }

    bool remove_change (
            const eprosima::fastdds::rtps::SequenceNumber_t& sequence_number)
    {
        return history_->remove_change(sequence_number);
    }

    void matched()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++matched_;
        cv_.notify_one();
    }

    void unmatched()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        --matched_;
        cv_.notify_one();
    }

    void wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        bool post_assertion = (matched_ == 0 && timeout == std::chrono::seconds::zero()) ? true : false;
        wait_discovery(1, timeout);
        if (post_assertion)
        {
            ASSERT_NE(matched_, 0u);
        }
    }

    void wait_discovery(
            size_t matches,
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (timeout == std::chrono::seconds::zero())
        {
            cv_.wait(lock, [&]() -> bool
                    {
                        return matched_ >= matches;
                    });
        }
        else
        {
            cv_.wait_for(lock, timeout, [&]()
                    {
                        return matched_ >= matches;
                    });
        }
    }

    void wait_undiscovery()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (matched_ != 0)
        {
            cv_.wait(lock, [this]() -> bool
                    {
                        return matched_ == 0;
                    });
        }

        EXPECT_EQ(matched_, 0u);
    }

    template<class _Rep,
            class _Period
            >
    bool waitForAllAcked(
            const std::chrono::duration<_Rep, _Period>& max_wait)
    {
        eprosima::fastdds::dds::Duration_t timeout;
        if (max_wait == std::chrono::seconds::zero())
        {
            timeout = eprosima::fastdds::dds::c_TimeInfinite;
        }
        else
        {
            auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(max_wait);
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
            nsecs -= secs;
            timeout.seconds = static_cast<int32_t>(secs.count());
            timeout.nanosec = static_cast<uint32_t>(nsecs.count());
        }
        return writer_->wait_for_all_acked(timeout);
    }

    /*** Function to change QoS ***/
    RTPSWithRegistrationWriter& payload_pool(
            const std::shared_ptr<eprosima::fastdds::rtps::IPayloadPool>& pool)
    {
        payload_pool_ = pool;
        has_payload_pool_ = true;
        return *this;
    }

    RTPSWithRegistrationWriter& memoryMode(
            const eprosima::fastdds::rtps::MemoryManagementPolicy_t memoryPolicy)
    {
        hattr_.memoryPolicy = memoryPolicy;
        return *this;
    }

    RTPSWithRegistrationWriter& reliability(
            const eprosima::fastdds::rtps::ReliabilityKind_t kind)
    {
        writer_attr_.endpoint.reliabilityKind = kind;

        if (kind == eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT)
        {
            writer_qos_.m_reliability.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        }
        else
        {
            writer_qos_.m_reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        }
        pub_builtin_data_.reliability = writer_qos_.m_reliability;

        return *this;
    }

    RTPSWithRegistrationWriter& durability(
            const eprosima::fastdds::rtps::DurabilityKind_t kind)
    {
        writer_attr_.endpoint.durabilityKind = kind;
        writer_qos_.m_durability.durabilityKind(kind);
        pub_builtin_data_.durability = writer_qos_.m_durability;

        return *this;
    }

    RTPSWithRegistrationWriter& asynchronously(
            const eprosima::fastdds::rtps::RTPSWriterPublishMode mode)
    {
        writer_attr_.mode = mode;

        return *this;
    }

    RTPSWithRegistrationWriter& heartbeat_period_seconds(
            int32_t sec)
    {
        writer_attr_.times.heartbeat_period.seconds = sec;
        return *this;
    }

    RTPSWithRegistrationWriter& heartbeat_period_nanosec(
            uint32_t nanosec)
    {
        writer_attr_.times.heartbeat_period.nanosec = nanosec;
        return *this;
    }

    RTPSWithRegistrationWriter& add_property(
            const std::string& prop,
            const std::string& value)
    {
        writer_attr_.endpoint.properties.properties().emplace_back(prop, value);
        return *this;
    }

    RTPSWithRegistrationWriter& persistence_guid_att(
            const eprosima::fastdds::rtps::GuidPrefix_t& guidPrefix,
            const eprosima::fastdds::rtps::EntityId_t& entityId)
    {
        writer_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        writer_attr_.endpoint.persistence_guid.entityId = entityId;
        return *this;
    }

#if HAVE_SQLITE3
    RTPSWithRegistrationWriter& make_transient(
            const std::string& filename,
            const eprosima::fastdds::rtps::GuidPrefix_t& guidPrefix)
    {
        writer_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        writer_attr_.endpoint.persistence_guid.entityId = 0xAAAAAAAA;

        std::cout << "Initializing transient WRITER " << writer_attr_.endpoint.persistence_guid
                  << " with file " << filename << std::endl;

        return durability(eprosima::fastdds::rtps::DurabilityKind_t::TRANSIENT)
                       .add_property("dds.persistence.plugin", "builtin.SQLITE3")
                       .add_property("dds.persistence.sqlite3.filename", filename);
    }

    RTPSWithRegistrationWriter& make_persistent(
            const std::string& filename,
            const eprosima::fastdds::rtps::GuidPrefix_t& guidPrefix)
    {
        writer_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        writer_attr_.endpoint.persistence_guid.entityId = 0xAAAAAAAA;

        std::cout << "Initializing persistent WRITER " << writer_attr_.endpoint.persistence_guid
                  << " with file " << filename << std::endl;

        return durability(eprosima::fastdds::rtps::DurabilityKind_t::PERSISTENT)
                       .add_property("dds.persistence.plugin", "builtin.SQLITE3")
                       .add_property("dds.persistence.sqlite3.filename", filename);
    }

#endif // if HAVE_SQLITE3

    RTPSWithRegistrationWriter& set_entity_id(
            const eprosima::fastdds::rtps::EntityId_t& entity_id)
    {
        custom_entity_id_ = entity_id;
        return *this;
    }

    RTPSWithRegistrationWriter& history_depth(
            const int32_t depth)
    {
        hattr_.maximumReservedCaches = depth;
        hattr_.initialReservedCaches = std::min(hattr_.initialReservedCaches, depth);
        return *this;
    }

    RTPSWithRegistrationWriter& disable_builtin_transport()
    {
        participant_attr_.useBuiltinTransports = false;
        return *this;
    }

    RTPSWithRegistrationWriter& add_user_transport_to_pparams(
            std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> userTransportDescriptor)
    {
        participant_attr_.userTransports.push_back(userTransportDescriptor);
        return *this;
    }

    RTPSWithRegistrationWriter& add_flow_controller_descriptor_to_pparams(
            eprosima::fastdds::rtps::FlowControllerSchedulerPolicy scheduler_policy,
            uint32_t bytes_per_period,
            uint32_t period_in_ms)
    {
        auto flow_controller_descriptor = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
        flow_controller_descriptor->name = "my_flow_controller";
        flow_controller_descriptor->scheduler = scheduler_policy;
        flow_controller_descriptor->max_bytes_per_period = bytes_per_period;
        flow_controller_descriptor->period_ms = period_in_ms;
        participant_attr_.flow_controllers.push_back(flow_controller_descriptor);
        writer_attr_.flow_controller_name = flow_controller_descriptor->name.c_str();
        return *this;
    }

    /**
     * @brief Add filter to RTPSWriter.
     *
     * IMPORTANT: RTPSWithRegistrationWriter must have been initialized previously.
     *
     * @param filter Filter interface implementation
     */
    void reader_data_filter(
            eprosima::fastdds::rtps::IReaderDataFilter* filter)
    {
        ASSERT_TRUE(initialized_);
        writer_->reader_data_filter(filter);
    }

    RTPSWithRegistrationWriter& user_data(
            const std::vector<eprosima::fastdds::rtps::octet>& user_data)
    {
        writer_qos_.m_userData = user_data;
        pub_builtin_data_.user_data = writer_qos_.m_userData;
        return *this;
    }

    RTPSWithRegistrationWriter& set_on_reader_discovery(
            const OnReaderDiscoveryFunctor& functor)
    {
        on_reader_discovery_functor = functor;
        return *this;
    }

    RTPSWithRegistrationWriter& partitions(
            std::vector<std::string>& partitions)
    {
        writer_qos_.m_partition.setNames(partitions);
        pub_builtin_data_.partition = writer_qos_.m_partition;
        return *this;
    }

    RTPSWithRegistrationWriter& set_separate_sending(
            bool separate_sending)
    {
        writer_attr_.separate_sending = separate_sending;
        return *this;
    }

    uint32_t get_matched() const
    {
        return matched_;
    }

    bool has_been_fully_delivered(
            const eprosima::fastdds::rtps::SequenceNumber_t& seq_num)
    {
        return writer_->has_been_fully_delivered(seq_num);
    }

    void participant_update_attributes()
    {
        participant_->update_attributes(participant_attr_);
    }

    const eprosima::fastdds::rtps::GUID_t& guid() const
    {
        return writer_->getGuid();
    }

    eprosima::fastdds::rtps::RTPSWriter& get_native_writer() const
    {
        return *writer_;
    }

    const eprosima::fastdds::dds::WriterQos& get_writer_qos() const
    {
        return writer_qos_;
    }

    eprosima::fastdds::rtps::TopicDescription get_topic_description() const
    {
        eprosima::fastdds::rtps::TopicDescription topic_desc;
        topic_desc.topic_name = pub_builtin_data_.topic_name;
        topic_desc.type_name = pub_builtin_data_.type_name;
        return topic_desc;
    }

    const eprosima::fastdds::rtps::RTPSParticipant* get_rtps_participant() const
    {
        return participant_;
    }

private:

    void on_reader_discovery(
            eprosima::fastdds::rtps::RTPSWriter* writer,
            eprosima::fastdds::rtps::ReaderDiscoveryStatus reason,
            const eprosima::fastdds::rtps::GUID_t& reader_guid,
            const eprosima::fastdds::rtps::SubscriptionBuiltinTopicData* reader_info)
    {
        ASSERT_EQ(writer_, writer);

        if (on_reader_discovery_functor)
        {
            on_reader_discovery_functor(reason, reader_guid, reader_info);
        }
    }

    RTPSWithRegistrationWriter& operator =(
            const RTPSWithRegistrationWriter&) = delete;

    eprosima::fastdds::rtps::RTPSParticipant* participant_;
    eprosima::fastdds::rtps::RTPSParticipantAttributes participant_attr_;
    bool destroy_participant_{false};
    eprosima::fastdds::rtps::RTPSWriter* writer_;
    eprosima::fastdds::rtps::WriterAttributes writer_attr_;
    eprosima::fastdds::dds::WriterQos writer_qos_;
    eprosima::fastdds::rtps::PublicationBuiltinTopicData pub_builtin_data_;
    eprosima::fastdds::rtps::WriterHistory* history_;
    eprosima::fastdds::rtps::HistoryAttributes hattr_;
    bool initialized_;
    std::mutex mutex_;
    std::condition_variable cv_;
    uint32_t matched_;
    eprosima::fastdds::rtps::EntityId_t custom_entity_id_ = eprosima::fastdds::rtps::c_EntityId_Unknown;
    type_support type_;
    std::shared_ptr<eprosima::fastdds::rtps::IPayloadPool> payload_pool_;
    bool has_payload_pool_ = false;
    OnReaderDiscoveryFunctor on_reader_discovery_functor;
};

#endif // _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_
