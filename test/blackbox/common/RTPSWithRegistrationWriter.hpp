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
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/transport/TransportDescriptorInterface.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/WriterListener.h>
#include <fastrtps/attributes/TopicAttributes.h>

template<class TypeSupport>
class RTPSWithRegistrationWriter
{
public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    using OnReaderDiscoveryFunctor = std::function <void (
                        eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERY_STATUS,
                        const eprosima::fastrtps::rtps::GUID_t&,
                        const eprosima::fastrtps::rtps::ReaderProxyData*
                        )>;

private:

    class Listener : public eprosima::fastrtps::rtps::WriterListener
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

        void onWriterMatched(
                eprosima::fastrtps::rtps::RTPSWriter* /*writer*/,
                eprosima::fastrtps::rtps::MatchingInfo& info) override
        {
            if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
            {
                writer_.matched();
            }
            else if (info.status == eprosima::fastrtps::rtps::REMOVED_MATCHING)
            {
                writer_.unmatched();
            }
        }

        void on_reader_discovery(
                eprosima::fastrtps::rtps::RTPSWriter* writer,
                eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERY_STATUS reason,
                const eprosima::fastrtps::rtps::GUID_t& reader_guid,
                const eprosima::fastrtps::rtps::ReaderProxyData* reader_info) override
        {
            writer_.on_reader_discovery(writer, reason, reader_guid, reader_info);
        }

    private:

        using eprosima::fastrtps::rtps::WriterListener::onWriterMatched;

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
            eprosima::fastrtps::rtps::RTPSParticipant* participant)
        : listener_(*this)
        , participant_(participant)
        , destroy_participant_(nullptr == participant)
        , writer_(nullptr)
        , history_(nullptr)
        , initialized_(false)
        , matched_(0)
    {
        topic_attr_.topicDataType = type_.getName();
        // Generate topic name
        std::ostringstream t;
        t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
        topic_attr_.topicName = t.str();

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        writer_attr_.times.heartbeatPeriod.seconds = 0;
        writer_attr_.times.heartbeatPeriod.nanosec = 100000000;
        writer_attr_.times.nackResponseDelay.seconds = 0;
        writer_attr_.times.nackResponseDelay.nanosec = 100000000;

        participant_attr_.builtin.discovery_config.discoveryProtocol =
                eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
        participant_attr_.builtin.use_WriterLivelinessProtocol = true;
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
            participant_ = eprosima::fastrtps::rtps::RTPSDomain::createParticipant(
                static_cast<uint32_t>(GET_PID()) % 230, participant_attr_);
        }
        ASSERT_NE(participant_, nullptr);

        //Create writerhistory
        hattr_.payloadMaxSize = type_.m_typeSize;
        history_ = new eprosima::fastrtps::rtps::WriterHistory(hattr_);

        //Create writer
        if (has_payload_pool_)
        {
            writer_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSWriter(
                participant_, writer_attr_, payload_pool_, history_, &listener_);
        }
        else
        {
            writer_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSWriter(
                participant_, writer_attr_, history_, &listener_);
        }

        if (writer_ == nullptr)
        {
            return;
        }

        ASSERT_EQ(participant_->registerWriter(writer_, topic_attr_, writer_qos_), true);

        initialized_ = true;
    }

    void update()
    {
        if (writer_ == nullptr)
        {
            return;
        }

        ASSERT_TRUE(participant_->updateWriter(writer_, topic_attr_, writer_qos_));
    }

    void destroy()
    {
        if (destroy_participant_ && participant_ != nullptr)
        {
            eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_);
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
            eprosima::fastrtps::rtps::CacheChange_t* ch = writer_->new_change(*it, eprosima::fastrtps::rtps::ALIVE);

            eprosima::fastcdr::FastBuffer buffer((char*)ch->serializedPayload.data, ch->serializedPayload.max_size);
            eprosima::fastcdr::Cdr cdr(buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                    eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);

            cdr.serialize_encapsulation();
            cdr << *it;

#if FASTCDR_VERSION_MAJOR == 1
            ch->serializedPayload.length = static_cast<uint32_t>(cdr.getSerializedDataLength());
#else
            ch->serializedPayload.length = static_cast<uint32_t>(cdr.get_serialized_data_length());
#endif // FASTCDR_VERSION_MAJOR == 1
            if (ch->serializedPayload.length > 65000u)
            {
                ch->setFragmentSize(65000u);
            }

            history_->add_change(ch);
            it = msgs.erase(it);
        }
    }

    eprosima::fastrtps::rtps::CacheChange_t* send_sample(
            type& msg)
    {
        eprosima::fastrtps::rtps::CacheChange_t* ch = writer_->new_change(msg, eprosima::fastrtps::rtps::ALIVE);

        eprosima::fastcdr::FastBuffer buffer((char*)ch->serializedPayload.data, ch->serializedPayload.max_size);
        eprosima::fastcdr::Cdr cdr(buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);

        cdr.serialize_encapsulation();
        cdr << msg;

#if FASTCDR_VERSION_MAJOR == 1
        ch->serializedPayload.length = static_cast<uint32_t>(cdr.getSerializedDataLength());
#else
        ch->serializedPayload.length = static_cast<uint32_t>(cdr.get_serialized_data_length());
#endif // FASTCDR_VERSION_MAJOR == 1
        if (ch->serializedPayload.length > 65000u)
        {
            ch->setFragmentSize(65000u);
        }

        history_->add_change(ch);
        return ch;
    }

    bool remove_change (
            const eprosima::fastrtps::rtps::SequenceNumber_t& sequence_number)
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
        eprosima::fastrtps::Duration_t timeout;
        if (max_wait == std::chrono::seconds::zero())
        {
            timeout = eprosima::fastrtps::c_TimeInfinite;
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
            const std::shared_ptr<eprosima::fastrtps::rtps::IPayloadPool>& pool)
    {
        payload_pool_ = pool;
        has_payload_pool_ = true;
        return *this;
    }

    RTPSWithRegistrationWriter& memoryMode(
            const eprosima::fastrtps::rtps::MemoryManagementPolicy_t memoryPolicy)
    {
        hattr_.memoryPolicy = memoryPolicy;
        return *this;
    }

    RTPSWithRegistrationWriter& reliability(
            const eprosima::fastrtps::rtps::ReliabilityKind_t kind)
    {
        writer_attr_.endpoint.reliabilityKind = kind;

        if (kind == eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT)
        {
            writer_qos_.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        }
        else
        {
            writer_qos_.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        }

        return *this;
    }

    RTPSWithRegistrationWriter& durability(
            const eprosima::fastrtps::rtps::DurabilityKind_t kind)
    {
        writer_attr_.endpoint.durabilityKind = kind;
        writer_qos_.m_durability.durabilityKind(kind);

        return *this;
    }

    RTPSWithRegistrationWriter& asynchronously(
            const eprosima::fastrtps::rtps::RTPSWriterPublishMode mode)
    {
        writer_attr_.mode = mode;

        return *this;
    }

    RTPSWithRegistrationWriter& add_throughput_controller_descriptor_to_pparams(
            uint32_t bytesPerPeriod,
            uint32_t periodInMs)
    {
        eprosima::fastrtps::rtps::ThroughputControllerDescriptor descriptor {bytesPerPeriod, periodInMs};
        writer_attr_.throughputController = descriptor;

        return *this;
    }

    RTPSWithRegistrationWriter& heartbeat_period_seconds(
            int32_t sec)
    {
        writer_attr_.times.heartbeatPeriod.seconds = sec;
        return *this;
    }

    RTPSWithRegistrationWriter& heartbeat_period_nanosec(
            uint32_t nanosec)
    {
        writer_attr_.times.heartbeatPeriod.nanosec = nanosec;
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
            const eprosima::fastrtps::rtps::GuidPrefix_t& guidPrefix,
            const eprosima::fastrtps::rtps::EntityId_t& entityId)
    {
        writer_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        writer_attr_.endpoint.persistence_guid.entityId = entityId;
        return *this;
    }

#if HAVE_SQLITE3
    RTPSWithRegistrationWriter& make_persistent(
            const std::string& filename,
            const eprosima::fastrtps::rtps::GuidPrefix_t& guidPrefix)
    {
        writer_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        writer_attr_.endpoint.persistence_guid.entityId = 0xAAAAAAAA;

        std::cout << "Initializing persistent WRITER " << writer_attr_.endpoint.persistence_guid
                  << " with file " << filename << std::endl;

        return durability(eprosima::fastrtps::rtps::DurabilityKind_t::PERSISTENT)
                       .add_property("dds.persistence.plugin", "builtin.SQLITE3")
                       .add_property("dds.persistence.sqlite3.filename", filename);
    }

#endif // if HAVE_SQLITE3

    RTPSWithRegistrationWriter& history_depth(
            const int32_t depth)
    {
        topic_attr_.historyQos.depth = depth;
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
        const char* flow_controller_name = "my_flow_controller";
        auto flow_controller_descriptor = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
        flow_controller_descriptor->name = flow_controller_name;
        flow_controller_descriptor->scheduler = scheduler_policy;
        flow_controller_descriptor->max_bytes_per_period = bytes_per_period;
        flow_controller_descriptor->period_ms = period_in_ms;
        participant_attr_.flow_controllers.push_back(flow_controller_descriptor);
        writer_attr_.flow_controller_name = flow_controller_name;
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
            const std::vector<eprosima::fastrtps::rtps::octet>& user_data)
    {
        writer_qos_.m_userData = user_data;
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
        return *this;
    }

    void set_separate_sending(
            bool separate_sending)
    {
        writer_->set_separate_sending(separate_sending);
    }

    uint32_t get_matched() const
    {
        return matched_;
    }

    bool has_been_fully_delivered(
            const eprosima::fastrtps::rtps::SequenceNumber_t& seq_num)
    {
        return writer_->has_been_fully_delivered(seq_num);
    }

    void participant_update_attributes()
    {
        participant_->update_attributes(participant_attr_);
    }

    const eprosima::fastrtps::rtps::GUID_t& guid() const
    {
        return writer_->getGuid();
    }

private:

    void on_reader_discovery(
            eprosima::fastrtps::rtps::RTPSWriter* writer,
            eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERY_STATUS reason,
            const eprosima::fastrtps::rtps::GUID_t& reader_guid,
            const eprosima::fastrtps::rtps::ReaderProxyData* reader_info)
    {
        ASSERT_EQ(writer_, writer);

        if (on_reader_discovery_functor)
        {
            on_reader_discovery_functor(reason, reader_guid, reader_info);
        }
    }

    RTPSWithRegistrationWriter& operator =(
            const RTPSWithRegistrationWriter&) = delete;

    eprosima::fastrtps::rtps::RTPSParticipant* participant_;
    eprosima::fastrtps::rtps::RTPSParticipantAttributes participant_attr_;
    bool destroy_participant_{false};
    eprosima::fastrtps::rtps::RTPSWriter* writer_;
    eprosima::fastrtps::rtps::WriterAttributes writer_attr_;
    eprosima::fastrtps::WriterQos writer_qos_;
    eprosima::fastrtps::TopicAttributes topic_attr_;
    eprosima::fastrtps::rtps::WriterHistory* history_;
    eprosima::fastrtps::rtps::HistoryAttributes hattr_;
    bool initialized_;
    std::mutex mutex_;
    std::condition_variable cv_;
    uint32_t matched_;
    type_support type_;
    std::shared_ptr<eprosima::fastrtps::rtps::IPayloadPool> payload_pool_;
    bool has_payload_pool_ = false;
    OnReaderDiscoveryFunctor on_reader_discovery_functor;
};

#endif // _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_
