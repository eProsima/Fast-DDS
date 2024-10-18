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
 * @file RTPSWithRegistrationReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_
#define _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_

#include <condition_variable>
#include <list>

#include <asio.hpp>

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>

#include <fastdds/utils/IPFinder.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <gtest/gtest.h>

using eprosima::fastdds::rtps::IPLocator;

template<class TypeSupport>
class RTPSWithRegistrationReader
{
public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    using OnWriterDiscoveryFunctor = std::function<void (
                        eprosima::fastdds::rtps::WriterDiscoveryStatus,
                        const eprosima::fastdds::rtps::GUID_t&,
                        const eprosima::fastdds::rtps::PublicationBuiltinTopicData*
                        )>;

private:

    class Listener : public eprosima::fastdds::rtps::ReaderListener
    {
    public:

        Listener(
                RTPSWithRegistrationReader& reader)
            : reader_(reader)
        {
        }

        ~Listener()
        {
        }

        void on_new_cache_change_added(
                eprosima::fastdds::rtps::RTPSReader* reader,
                const eprosima::fastdds::rtps::CacheChange_t* const change) override
        {
            ASSERT_NE(reader, nullptr);
            ASSERT_NE(change, nullptr);

            reader_.receive_one(reader, change);
        }

        void on_reader_matched(
                eprosima::fastdds::rtps::RTPSReader* /*reader*/,
                const eprosima::fastdds::rtps::MatchingInfo& info) override
        {
            if (info.status == eprosima::fastdds::rtps::MATCHED_MATCHING)
            {
                reader_.matched();
            }
            else if (info.status == eprosima::fastdds::rtps::REMOVED_MATCHING)
            {
                reader_.unmatched();
            }
        }

        void on_writer_discovery(
                eprosima::fastdds::rtps::RTPSReader* reader,
                eprosima::fastdds::rtps::WriterDiscoveryStatus reason,
                const eprosima::fastdds::rtps::GUID_t& writer_guid,
                const eprosima::fastdds::rtps::PublicationBuiltinTopicData* writer_info) override
        {
            reader_.on_writer_discovery(reader, reason, writer_guid, writer_info);
        }

    private:

        Listener& operator =(
                const Listener&) = delete;

        RTPSWithRegistrationReader& reader_;
    }
    listener_;

public:

    RTPSWithRegistrationReader(
            const std::string& topic_name)
        : RTPSWithRegistrationReader(topic_name, nullptr)
    {
    }

    RTPSWithRegistrationReader(
            const std::string& topic_name,
            eprosima::fastdds::rtps::RTPSParticipant* participant)
        : listener_(*this)
        , participant_(participant)
        , destroy_participant_(nullptr == participant)
        , reader_(nullptr)
        , history_(nullptr)
        , initialized_(false)
        , receiving_(false)
        , matched_(0)
    {
        sub_builtin_data_.type_name = type_.get_name();
        // Generate topic name
        std::ostringstream t;
        t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
        sub_builtin_data_.topic_name = t.str();

        // By default, heartbeat period delay is 100 milliseconds.
        reader_attr_.times.heartbeat_response_delay.seconds = 0;
        reader_attr_.times.heartbeat_response_delay.nanosec = 100000000;

        participant_attr_.builtin.discovery_config.discoveryProtocol =
                eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE;
        participant_attr_.builtin.use_WriterLivelinessProtocol = true;

        if (type_.is_compute_key_provided)
        {
            reader_attr_.endpoint.topicKind = eprosima::fastdds::rtps::WITH_KEY;
        }
        else
        {
            reader_attr_.endpoint.topicKind = eprosima::fastdds::rtps::NO_KEY;
        }
    }

    virtual ~RTPSWithRegistrationReader()
    {
        destroy();
    }

    void init()
    {
        matched_ = 0;

        if (nullptr == participant_)
        {
            participant_ = eprosima::fastdds::rtps::RTPSDomain::createParticipant(
                static_cast<uint32_t>(GET_PID()) % 230, participant_attr_);
            ASSERT_NE(participant_, nullptr);
        }

        //Create readerhistory
        hattr_.payloadMaxSize = type_.max_serialized_type_size;
        history_ = new eprosima::fastdds::rtps::ReaderHistory(hattr_);
        ASSERT_NE(history_, nullptr);

        // Create reader
        if (has_payload_pool_)
        {
            if (custom_entity_id_ != eprosima::fastdds::rtps::c_EntityId_Unknown)
            {
                reader_ = eprosima::fastdds::rtps::RTPSDomain::createRTPSReader(participant_, custom_entity_id_,
                                reader_attr_, payload_pool_,
                                history_, &listener_);
            }
            else
            {
                reader_ = eprosima::fastdds::rtps::RTPSDomain::createRTPSReader(participant_, reader_attr_,
                                payload_pool_,
                                history_, &listener_);
            }
        }
        else
        {
            reader_ = eprosima::fastdds::rtps::RTPSDomain::createRTPSReader(participant_, reader_attr_,
                            history_, &listener_);
        }

        if (reader_ == nullptr)
        {
            return;
        }

        eprosima::fastdds::rtps::TopicDescription topic_desc;
        topic_desc.topic_name = sub_builtin_data_.topic_name;
        topic_desc.type_name = sub_builtin_data_.type_name;

        initialized_ = participant_->register_reader(reader_, topic_desc, reader_qos_, content_filter_property_);
    }

    void update()
    {
        if (reader_ == nullptr)
        {
            return;
        }

        initialized_ = participant_->update_reader(reader_, reader_qos_, content_filter_property_);
    }

    bool isInitialized() const
    {
        return initialized_;
    }

    void destroy()
    {
        if (destroy_participant_ && participant_ != nullptr)
        {
            eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_);
        }
        participant_ = nullptr;

        if (history_ != nullptr)
        {
            delete(history_);
            history_ = nullptr;
        }

        reader_ = nullptr;
        receiving_ = false;
        initialized_ = false;
        matched_ = 0;
    }

    void expected_data(
            const std::list<type>& msgs,
            bool reset_seq = false)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        total_msgs_ = msgs;

        if (reset_seq)
        {
            last_seq_ = eprosima::fastdds::rtps::SequenceNumber_t();
        }
    }

    void expected_data(
            std::list<type>&& msgs)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        total_msgs_ = std::move(msgs);
    }

    const std::list<type>& not_received_data() const
    {
        return total_msgs_;
    }

    void startReception(
            size_t number_samples_expected = 0)
    {
        mutex_.lock();
        current_received_count_ = 0;
        if (number_samples_expected > 0)
        {
            number_samples_expected_ = number_samples_expected;
        }
        else
        {
            number_samples_expected_ = total_msgs_.size();
        }
        receiving_ = true;
        mutex_.unlock();

        std::unique_lock<eprosima::fastdds::RecursiveTimedMutex> lock(*history_->getMutex());
        while (history_->changesBegin() != history_->changesEnd())
        {
            eprosima::fastdds::rtps::CacheChange_t* change = *history_->changesBegin();
            std::cout << "Late processing change " << change->sequenceNumber << std::endl;
            receive_one(reader_, change, false);
        }
    }

    void stopReception()
    {
        mutex_.lock();
        receiving_ = false;
        mutex_.unlock();
    }

    void block(
            std::function<bool()> checker,
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (std::chrono::seconds::zero() == timeout)
        {
            cv_.wait(lock, checker);
        }
        else
        {
            cv_.wait_for(lock, timeout, checker);
        }
    }

    void block_for_all(
            std::chrono::seconds timeout = std::chrono::seconds::zero())
    {
        block([this]() -> bool
                {
                    return number_samples_expected_ == current_received_count_;
                }, timeout);
    }

    size_t block_for_at_least(
            size_t at_least)
    {
        block([this, at_least]() -> bool
                {
                    return current_received_count_ >= at_least;
                });
        return current_received_count_;
    }

    void block_until_seq_number_greater_or_equal(
            const eprosima::fastdds::rtps::SequenceNumber_t& min_seq)
    {
        block([this, min_seq]() -> bool
                {
                    return last_seq_ >= min_seq;
                });
    }

    eprosima::fastdds::rtps::SequenceNumber_t get_last_received_sequence_number() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_seq_;
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
            cvDiscovery_.wait(lock, [&]() -> bool
                    {
                        return matched_ >= matches;
                    });
        }
        else
        {
            cvDiscovery_.wait_for(lock, timeout, [&]()
                    {
                        return matched_ >= matches;
                    });
        }
    }

    void wait_undiscovery()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        if (matched_ != 0)
        {
            cvDiscovery_.wait(lock, [this]() -> bool
                    {
                        return matched_ == 0;
                    });
        }

        EXPECT_EQ(matched_, 0u);
    }

    void check_seq_number_greater_or_equal(
            const eprosima::fastdds::rtps::SequenceNumber_t& min_seq)
    {
        ASSERT_GE(last_seq_, min_seq);
    }

    void matched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        ++matched_;
        cvDiscovery_.notify_one();
    }

    void unmatched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        --matched_;
        cvDiscovery_.notify_one();
    }

    unsigned int getReceivedCount() const
    {
        return static_cast<unsigned int>(current_received_count_);
    }

    /*** Function to change QoS ***/
    RTPSWithRegistrationReader& payload_pool(
            const std::shared_ptr<eprosima::fastdds::rtps::IPayloadPool>& pool)
    {
        payload_pool_ = pool;
        has_payload_pool_ = true;
        return *this;
    }

    RTPSWithRegistrationReader& memoryMode(
            const eprosima::fastdds::rtps::MemoryManagementPolicy_t memoryPolicy)
    {
        hattr_.memoryPolicy = memoryPolicy;
        return *this;
    }

    RTPSWithRegistrationReader& set_entity_id(
            const eprosima::fastdds::rtps::EntityId_t& entity_id)
    {
        custom_entity_id_ = entity_id;
        return *this;
    }

    RTPSWithRegistrationReader& history_depth(
            const int32_t depth)
    {
        hattr_.maximumReservedCaches = depth;
        hattr_.initialReservedCaches = std::min(hattr_.initialReservedCaches, depth);
        return *this;
    }

    RTPSWithRegistrationReader& reliability(
            const eprosima::fastdds::rtps::ReliabilityKind_t kind)
    {
        reader_attr_.endpoint.reliabilityKind = kind;

        if (kind == eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE)
        {
            reader_qos_.m_reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        }
        else
        {
            reader_qos_.m_reliability.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        }
        sub_builtin_data_.reliability = reader_qos_.m_reliability;

        return *this;
    }

    RTPSWithRegistrationReader& durability(
            const eprosima::fastdds::rtps::DurabilityKind_t kind)
    {
        reader_attr_.endpoint.durabilityKind = kind;
        reader_qos_.m_durability.durabilityKind(kind);
        sub_builtin_data_.durability = reader_qos_.m_durability;

        return *this;
    }

    RTPSWithRegistrationReader& add_to_multicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        eprosima::fastdds::rtps::Locator_t loc;
        IPLocator::setIPv4(loc, ip);
        loc.port = static_cast<uint16_t>(port);
        reader_attr_.endpoint.multicastLocatorList.push_back(loc);

        return *this;
    }

    RTPSWithRegistrationReader& add_property(
            const std::string& prop,
            const std::string& value)
    {
        reader_attr_.endpoint.properties.properties().emplace_back(prop, value);

        return *this;
    }

    RTPSWithRegistrationReader& persistence_guid_att(
            const eprosima::fastdds::rtps::GuidPrefix_t& guidPrefix,
            const eprosima::fastdds::rtps::EntityId_t& entityId)
    {
        reader_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        reader_attr_.endpoint.persistence_guid.entityId = entityId;
        return *this;
    }

    uint32_t get_matched() const
    {
        return matched_;
    }

#if HAVE_SQLITE3
    RTPSWithRegistrationReader& make_transient(
            const std::string& filename,
            const eprosima::fastdds::rtps::GuidPrefix_t& guidPrefix)
    {
        reader_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        reader_attr_.endpoint.persistence_guid.entityId = 0x55555555;

        std::cout << "Initializing transient READER " << reader_attr_.endpoint.persistence_guid << " with file " <<
            filename << std::endl;

        return durability(eprosima::fastdds::rtps::DurabilityKind_t::TRANSIENT)
                       .add_property("dds.persistence.plugin", "builtin.SQLITE3")
                       .add_property("dds.persistence.sqlite3.filename", filename);
    }

    RTPSWithRegistrationReader& make_persistent(
            const std::string& filename,
            const eprosima::fastdds::rtps::GuidPrefix_t& guidPrefix)
    {
        reader_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        reader_attr_.endpoint.persistence_guid.entityId = 0x55555555;

        std::cout << "Initializing persistent READER " << reader_attr_.endpoint.persistence_guid << " with file " <<
            filename << std::endl;

        return durability(eprosima::fastdds::rtps::DurabilityKind_t::PERSISTENT)
                       .add_property("dds.persistence.plugin", "builtin.SQLITE3")
                       .add_property("dds.persistence.sqlite3.filename", filename);
    }

#endif // if HAVE_SQLITE3

    RTPSWithRegistrationReader& user_data(
            const std::vector<eprosima::fastdds::rtps::octet>& user_data)
    {
        reader_qos_.m_userData = user_data;
        sub_builtin_data_.user_data = reader_qos_.m_userData;
        return *this;
    }

    RTPSWithRegistrationReader& set_on_writer_discovery(
            const OnWriterDiscoveryFunctor& functor)
    {
        on_writer_discovery_functor = functor;
        return *this;
    }

    RTPSWithRegistrationReader& partitions(
            std::vector<std::string>& partitions)
    {
        reader_qos_.m_partition.setNames(partitions);
        sub_builtin_data_.partition = reader_qos_.m_partition;
        return *this;
    }

    RTPSWithRegistrationReader& content_filter_property(
            const eprosima::fastdds::rtps::ContentFilterProperty& content_filter_property)
    {
        content_filter_property_ = &content_filter_property;
        return *this;
    }

    const eprosima::fastdds::rtps::GUID_t& guid() const
    {
        return reader_->getGuid();
    }

    eprosima::fastdds::rtps::RTPSReader& get_native_reader() const
    {
        return *reader_;
    }

    const eprosima::fastdds::dds::ReaderQos& get_reader_qos() const
    {
        return reader_qos_;
    }

    eprosima::fastdds::rtps::TopicDescription get_topic_description() const
    {
        eprosima::fastdds::rtps::TopicDescription topic_desc;
        topic_desc.topic_name = sub_builtin_data_.topic_name;
        topic_desc.type_name = sub_builtin_data_.type_name;
        return topic_desc;
    }

    const eprosima::fastdds::rtps::RTPSParticipant* get_rtps_participant() const
    {
        return participant_;
    }

private:

    void receive_one(
            eprosima::fastdds::rtps::RTPSReader* reader,
            const eprosima::fastdds::rtps::CacheChange_t* change,
            bool check_seq = true)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        // Check order of changes.
        if (check_seq)
        {
            EXPECT_LT(last_seq_, change->sequenceNumber);
            if (last_seq_ < change->sequenceNumber)
            {
                last_seq_ = change->sequenceNumber;
            }
        }

        if (receiving_)
        {
            type data;
            eprosima::fastcdr::FastBuffer buffer((char*)change->serializedPayload.data,
                    change->serializedPayload.length);
            eprosima::fastcdr::Cdr cdr(buffer
                    );

            cdr.read_encapsulation();
            cdr >> data;

            auto it = std::find(total_msgs_.begin(), total_msgs_.end(), data);
            EXPECT_NE(it, total_msgs_.end());
            if (it != total_msgs_.end())
            {
                total_msgs_.erase(it);
                ++current_received_count_;
                if (check_seq)
                {
                    default_receive_print<type>(data);
                }
                cv_.notify_one();
            }

            eprosima::fastdds::rtps::ReaderHistory* history = reader->get_history();
            EXPECT_EQ(history, history_);

            history->remove_change((eprosima::fastdds::rtps::CacheChange_t*)change);
        }
        else
        {
            std::cerr << "Received unexpected " << change->sequenceNumber << std::endl;
        }
    }

    void on_writer_discovery(
            eprosima::fastdds::rtps::RTPSReader* reader,
            eprosima::fastdds::rtps::WriterDiscoveryStatus reason,
            const eprosima::fastdds::rtps::GUID_t& writer_guid,
            const eprosima::fastdds::rtps::PublicationBuiltinTopicData* writer_info)
    {
        ASSERT_EQ(reader_, reader);

        if (on_writer_discovery_functor)
        {
            on_writer_discovery_functor(reason, writer_guid, writer_info);
        }
    }

    RTPSWithRegistrationReader& operator =(
            const RTPSWithRegistrationReader&) = delete;

    eprosima::fastdds::rtps::RTPSParticipant* participant_;
    eprosima::fastdds::rtps::RTPSParticipantAttributes participant_attr_;
    bool destroy_participant_{false};
    eprosima::fastdds::rtps::RTPSReader* reader_;
    eprosima::fastdds::rtps::ReaderAttributes reader_attr_;
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData sub_builtin_data_;
    eprosima::fastdds::dds::ReaderQos reader_qos_;
    eprosima::fastdds::rtps::ReaderHistory* history_;
    eprosima::fastdds::rtps::HistoryAttributes hattr_;
    std::atomic<bool> initialized_;
    std::list<type> total_msgs_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;
    std::atomic<bool> receiving_;
    std::atomic<uint32_t> matched_;
    eprosima::fastdds::rtps::EntityId_t custom_entity_id_ = eprosima::fastdds::rtps::c_EntityId_Unknown;
    eprosima::fastdds::rtps::SequenceNumber_t last_seq_;
    std::atomic<size_t> current_received_count_;
    std::atomic<size_t> number_samples_expected_;
    type_support type_;
    std::shared_ptr<eprosima::fastdds::rtps::IPayloadPool> payload_pool_;
    bool has_payload_pool_ = false;
    OnWriterDiscoveryFunctor on_writer_discovery_functor;
    const eprosima::fastdds::rtps::ContentFilterProperty* content_filter_property_ = nullptr;
};

#endif // _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_
