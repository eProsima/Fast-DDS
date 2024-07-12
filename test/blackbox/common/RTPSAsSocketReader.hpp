// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSAsSocketReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASSOCKETREADER_HPP_
#define _TEST_BLACKBOX_RTPSASSOCKETREADER_HPP_

#include <condition_variable>
#include <list>
#include <string>

#if defined(_WIN32)
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

#include <asio.hpp>

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>

#include <fastdds/utils/IPLocator.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <gtest/gtest.h>

using eprosima::fastdds::rtps::IPLocator;

template<class TypeSupport>
class RTPSAsSocketReader
{
public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

private:

    class Listener : public eprosima::fastdds::rtps::ReaderListener
    {
    public:

        Listener(
                RTPSAsSocketReader& reader)
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

    private:

        Listener& operator =(
                const Listener&) = delete;

        RTPSAsSocketReader& reader_;
    }
    listener_;

public:

    RTPSAsSocketReader(
            const std::string& magicword)
        : listener_(*this)
        , participant_(nullptr)
        , reader_(nullptr)
        , history_(nullptr)
        , initialized_(false)
        , receiving_(false)
        , current_received_count_(0)
        , number_samples_expected_(0)
        , port_(0)
    {
        std::ostringstream mw;
        mw << magicword << "_" << asio::ip::host_name() << "_" << GET_PID();
        magicword_ = mw.str();

        // By default, memory mode is PREALLOCATED_WITH_REALLOC_MEMORY_MODE
        hattr_.memoryPolicy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

        // By default, heartbeat period delay is 100 milliseconds.
        reader_attr_.times.heartbeat_response_delay.seconds = 0;
        reader_attr_.times.heartbeat_response_delay.nanosec = 100000000;
    }

    virtual ~RTPSAsSocketReader()
    {
        if (participant_ != nullptr)
        {
            eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_);
        }
        if (history_ != nullptr)
        {
            delete(history_);
        }
    }

    // TODO Change api of  set_IP4_address to support const string.
    void init()
    {
        eprosima::fastdds::rtps::RTPSParticipantAttributes pattr;
        pattr.builtin.discovery_config.discoveryProtocol = eprosima::fastdds::rtps::DiscoveryProtocol::NONE;
        pattr.builtin.use_WriterLivelinessProtocol = false;
        pattr.participantID = 1;
        participant_ = eprosima::fastdds::rtps::RTPSDomain::createParticipant((uint32_t)GET_PID() % 230, pattr);
        ASSERT_NE(participant_, nullptr);

        //Create readerhistory
        hattr_.payloadMaxSize = 255 + type_.max_serialized_type_size;
        history_ = new eprosima::fastdds::rtps::ReaderHistory(hattr_);
        ASSERT_NE(history_, nullptr);

        //Create reader
        auto attr = reader_attr_;
        attr.accept_messages_from_unkown_writers =
                eprosima::fastdds::rtps::RELIABLE != reader_attr_.endpoint.reliabilityKind;
        reader_ = eprosima::fastdds::rtps::RTPSDomain::createRTPSReader(participant_, attr, history_,
                        &listener_);
        ASSERT_NE(reader_, nullptr);

        register_writer();

        initialized_ = true;
    }

    bool isInitialized() const
    {
        return initialized_;
    }

    void destroy()
    {
        if (participant_ != nullptr)
        {
            eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_);
            participant_ = nullptr;
        }

        if (history_ != nullptr)
        {
            delete(history_);
            history_ = nullptr;
        }
    }

    void expected_data(
            const std::list<type>& msgs)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        total_msgs_ = msgs;
    }

    void expected_data(
            std::list<type>&& msgs)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        total_msgs_ = std::move(msgs);
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
            receive_one(reader_, change);
        }
    }

    void stopReception()
    {
        mutex_.lock();
        receiving_ = false;
        mutex_.unlock();
    }

    void block(
            std::function<bool()> checker)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, checker);
    }

    void block_for_all()
    {
        block([this]() -> bool
                {
                    return number_samples_expected_ == current_received_count_;
                });
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

    unsigned int getReceivedCount() const
    {
        return current_received_count_;
    }

    /*** Function to change QoS ***/
    RTPSAsSocketReader& reliability(
            const eprosima::fastdds::rtps::ReliabilityKind_t kind)
    {
        reader_attr_.endpoint.reliabilityKind = kind;

        if (kind == eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE)
        {
            reader_attr_.endpoint.setEntityID(1);
        }
        return *this;
    }

    RTPSAsSocketReader& add_to_multicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        ip_ = ip;
        port_ = port;

        eprosima::fastdds::rtps::Locator_t loc;
        IPLocator::setIPv4(loc, ip);
        loc.port = static_cast<uint16_t>(port);
        reader_attr_.endpoint.multicastLocatorList.push_back(loc);

        return *this;
    }

    void register_writer()
    {
        if (reader_attr_.endpoint.reliabilityKind == eprosima::fastdds::rtps::RELIABLE)
        {
            if (port_ == 0)
            {
                std::cout << "ERROR: locator has to be registered previous to call this" << std::endl;
            }

            // Add remote writer (in this case a reader in the same machine)
            eprosima::fastdds::rtps::GUID_t guid = participant_->getGuid();

            eprosima::fastdds::rtps::Locator_t loc;
            IPLocator::setIPv4(loc, ip_);
            loc.port = static_cast<uint16_t>(port_);

            eprosima::fastdds::rtps::GUID_t wguid;
            wguid.guidPrefix.value[0] = guid.guidPrefix.value[0];
            wguid.guidPrefix.value[1] = guid.guidPrefix.value[1];
            wguid.guidPrefix.value[2] = guid.guidPrefix.value[2];
            wguid.guidPrefix.value[3] = guid.guidPrefix.value[3];
            wguid.guidPrefix.value[4] = guid.guidPrefix.value[4];
            wguid.guidPrefix.value[5] = guid.guidPrefix.value[5];
            wguid.guidPrefix.value[6] = guid.guidPrefix.value[6];
            wguid.guidPrefix.value[7] = guid.guidPrefix.value[7];
            wguid.guidPrefix.value[8] = 2;
            wguid.guidPrefix.value[9] = 0;
            wguid.guidPrefix.value[10] = 0;
            wguid.guidPrefix.value[11] = 0;
            wguid.entityId.value[0] = 0;
            wguid.entityId.value[1] = 0;
            wguid.entityId.value[2] = 2;
            wguid.entityId.value[3] = 3;

            eprosima::fastdds::rtps::PublicationBuiltinTopicData pub_info{};
            pub_info.reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
            pub_info.remote_locators.add_multicast_locator(loc);
            pub_info.guid = wguid;
            pub_info.persistence_guid = wguid;

            reader_->matched_writer_add(pub_info);
        }
    }

    RTPSAsSocketReader& disable_positive_acks(
            bool disable)
    {
        reader_attr_.disable_positive_acks = disable;
        return *this;
    }

private:

    void receive_one(
            eprosima::fastdds::rtps::RTPSReader* reader,
            const eprosima::fastdds::rtps::CacheChange_t* change)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (receiving_)
        {
            type data;
            eprosima::fastcdr::FastBuffer buffer((char*)change->serializedPayload.data,
                    change->serializedPayload.length);
            eprosima::fastcdr::Cdr cdr(buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                    eprosima::fastcdr::CdrVersion::DDS_CDR);
            std::string magicword;
            cdr >> magicword;

            if (magicword.compare(magicword_) == 0)
            {
                // Check order of changes.
                ASSERT_LT(last_seq_, change->sequenceNumber);
                last_seq_ = change->sequenceNumber;

                cdr >> data;

                auto it = std::find(total_msgs_.begin(), total_msgs_.end(), data);
                ASSERT_NE(it, total_msgs_.end());
                total_msgs_.erase(it);
                ++current_received_count_;
                default_receive_print<type>(data);
                cv_.notify_one();
            }

            eprosima::fastdds::rtps::ReaderHistory* history = reader->get_history();
            ASSERT_NE(history, nullptr);

            history->remove_change((eprosima::fastdds::rtps::CacheChange_t*)change);
        }
    }

    RTPSAsSocketReader& operator =(
            const RTPSAsSocketReader&) = delete;

    eprosima::fastdds::rtps::RTPSParticipant* participant_;
    eprosima::fastdds::rtps::ReaderAttributes reader_attr_;
    eprosima::fastdds::rtps::RTPSReader* reader_;
    eprosima::fastdds::rtps::ReaderHistory* history_;
    eprosima::fastdds::rtps::HistoryAttributes hattr_;
    std::atomic<bool> initialized_;
    std::list<type> total_msgs_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::string magicword_;
    std::atomic<bool> receiving_;
    eprosima::fastdds::rtps::SequenceNumber_t last_seq_;
    std::atomic<size_t> current_received_count_;
    std::atomic<size_t> number_samples_expected_;
    std::string ip_;
    uint32_t port_;
    type_support type_;
};

#endif // _TEST_BLACKBOX_RTPSASSOCKETREADER_HPP_
