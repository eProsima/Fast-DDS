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

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/common/SequenceNumber.h>
#include <fastrtps/utils/IPLocator.h>

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <string>
#include <list>
#include <condition_variable>
#include <asio.hpp>
#include <gtest/gtest.h>


#if defined(_WIN32)
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif

using eprosima::fastrtps::rtps::IPLocator;

template<class TypeSupport>
class RTPSAsSocketReader
{
    public:

        typedef TypeSupport type_support;
        typedef typename type_support::type type;

    private:

    class Listener: public eprosima::fastrtps::rtps::ReaderListener
    {
        public:
            Listener(RTPSAsSocketReader &reader) : reader_(reader) {};

            ~Listener(){};

            void onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* reader, const eprosima::fastrtps::rtps::CacheChange_t* const change)
            {
                ASSERT_NE(reader, nullptr);
                ASSERT_NE(change, nullptr);

                reader_.receive_one(reader, change);
            }

        private:

            Listener& operator=(const Listener&) = delete;

            RTPSAsSocketReader &reader_;
    } listener_;

    public:

        RTPSAsSocketReader(const std::string& magicword) : listener_(*this), participant_(nullptr), reader_(nullptr),
        history_(nullptr), initialized_(false), receiving_(false), current_received_count_(0),
        number_samples_expected_(0), port_(0)
        {
            std::ostringstream mw;
            mw << magicword << "_" << asio::ip::host_name() << "_" << GET_PID();
            magicword_ = mw.str();

#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
            hattr_.memoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
            hattr_.memoryPolicy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
#else
            hattr_.memoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
#endif

            // By default, heartbeat period delay is 100 milliseconds.
            reader_attr_.times.heartbeatResponseDelay.seconds = 0;
            reader_attr_.times.heartbeatResponseDelay.fraction = 4294967 * 100;
        }

        virtual ~RTPSAsSocketReader()
        {
            if(participant_ != nullptr)
                eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_);
            if(history_ != nullptr)
                delete(history_);
        }

        // TODO Change api of  set_IP4_address to support const string.
        void init()
        {
            eprosima::fastrtps::rtps::RTPSParticipantAttributes pattr;
            pattr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = false;
            pattr.builtin.use_WriterLivelinessProtocol = false;
            pattr.builtin.domainId = (uint32_t)GET_PID() % 230;
            pattr.participantID = 1;
            participant_ = eprosima::fastrtps::rtps::RTPSDomain::createParticipant(pattr);
            ASSERT_NE(participant_, nullptr);

            //Create readerhistory
            hattr_.payloadMaxSize = 255 + type_.m_typeSize;
            history_ = new eprosima::fastrtps::rtps::ReaderHistory(hattr_);
            ASSERT_NE(history_, nullptr);

            //Create reader
            reader_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSReader(participant_, reader_attr_, history_, &listener_);
            ASSERT_NE(reader_, nullptr);

            register_writer();

            initialized_ = true;
        }

        bool isInitialized() const { return initialized_; }

        void destroy()
        {
            if(participant_ != nullptr)
            {
                eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_);
                participant_ = nullptr;
            }

            if(history_ != nullptr)
            {
                delete(history_);
                history_ = nullptr;
            }
        }

        void expected_data(const std::list<type>& msgs)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            total_msgs_ = msgs;
        }

        void expected_data(std::list<type>&& msgs)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            total_msgs_ = std::move(msgs);
        }

        void startReception(size_t number_samples_expected = 0)
        {
            mutex_.lock();
            current_received_count_ = 0;
            if(number_samples_expected > 0)
                number_samples_expected_ = number_samples_expected;
            else
                number_samples_expected_ = total_msgs_.size();
            receiving_ = true;
            mutex_.unlock();

            std::unique_lock<std::recursive_timed_mutex> lock(*history_->getMutex());
            while(history_->changesBegin() != history_->changesEnd())
            {
                eprosima::fastrtps::rtps::CacheChange_t* change = *history_->changesBegin();
                receive_one(reader_, change);
            }
        }

        void stopReception()
        {
            mutex_.lock();
            receiving_ = false;
            mutex_.unlock();
        }

        void block(std::function<bool()> checker)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, checker);
        }

        void block_for_all()
        {
            block([this]() -> bool {
                    return number_samples_expected_ == current_received_count_;
                    });
        }

        size_t block_for_at_least(size_t at_least)
        {
            block([this, at_least]() -> bool {
                    return current_received_count_ >= at_least;
                    });
            return current_received_count_;
        }

        unsigned int getReceivedCount() const
        {
            return current_received_count_;
        }

        /*** Function to change QoS ***/
        RTPSAsSocketReader& reliability(const eprosima::fastrtps::rtps::ReliabilityKind_t kind)
        {
            reader_attr_.endpoint.reliabilityKind = kind;

            if(kind == eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE)
                reader_attr_.endpoint.setEntityID(1);
            return *this;
        }

        RTPSAsSocketReader& add_to_multicast_locator_list(const std::string& ip, uint32_t port)
        {
            ip_ = ip;
            port_ = port;

            eprosima::fastrtps::rtps::Locator_t loc;
            IPLocator::setIPv4(loc, ip);
            loc.port = static_cast<uint16_t>(port);
            reader_attr_.endpoint.multicastLocatorList.push_back(loc);

            return *this;
        }

        void register_writer()
        {
            if(reader_attr_.endpoint.reliabilityKind == eprosima::fastrtps::rtps::RELIABLE)
            {
                if(port_ == 0)
                    std::cout << "ERROR: locator has to be registered previous to call this" << std::endl;

                //Add remote writer (in this case a reader in the same machine)
                eprosima::fastrtps::rtps::GUID_t guid = participant_->getGuid();

                eprosima::fastrtps::rtps::RemoteWriterAttributes wattr;
                eprosima::fastrtps::rtps::Locator_t loc;
                IPLocator::setIPv4(loc, ip_);
                loc.port = static_cast<uint16_t>(port_);
                wattr.endpoint.multicastLocatorList.push_back(loc);
                wattr.endpoint.reliabilityKind = eprosima::fastrtps::rtps::RELIABLE;
                wattr.guid.guidPrefix.value[0] = guid.guidPrefix.value[0];
                wattr.guid.guidPrefix.value[1] = guid.guidPrefix.value[1];
                wattr.guid.guidPrefix.value[2] = guid.guidPrefix.value[2];
                wattr.guid.guidPrefix.value[3] = guid.guidPrefix.value[3];
                wattr.guid.guidPrefix.value[4] = guid.guidPrefix.value[4];
                wattr.guid.guidPrefix.value[5] = guid.guidPrefix.value[5];
                wattr.guid.guidPrefix.value[6] = guid.guidPrefix.value[6];
                wattr.guid.guidPrefix.value[7] = guid.guidPrefix.value[7];
                wattr.guid.guidPrefix.value[8] = 2;
                wattr.guid.guidPrefix.value[9] = 0;
                wattr.guid.guidPrefix.value[10] = 0;
                wattr.guid.guidPrefix.value[11] = 0;
                wattr.guid.entityId.value[0] = 0;
                wattr.guid.entityId.value[1] = 0;
                wattr.guid.entityId.value[2] = 2;
                wattr.guid.entityId.value[3] = 3;
                reader_->matched_writer_add(wattr);
            }
        }

    private:

        void receive_one(eprosima::fastrtps::rtps::RTPSReader* reader, const eprosima::fastrtps::rtps::CacheChange_t* change)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if(receiving_)
            {
                type data;
                eprosima::fastcdr::FastBuffer buffer((char*)change->serializedPayload.data, change->serializedPayload.length);
                eprosima::fastcdr::Cdr cdr(buffer);
                std::string magicword;
                cdr >> magicword;

                if(magicword.compare(magicword_) == 0)
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

                eprosima::fastrtps::rtps::ReaderHistory *history = reader->getHistory();
                ASSERT_NE(history, nullptr);

                history->remove_change((eprosima::fastrtps::rtps::CacheChange_t*)change);
            }
        }

        RTPSAsSocketReader& operator=(const RTPSAsSocketReader&) = delete;

        eprosima::fastrtps::rtps::RTPSParticipant *participant_;
        eprosima::fastrtps::rtps::ReaderAttributes reader_attr_;
        eprosima::fastrtps::rtps::RTPSReader *reader_;
        eprosima::fastrtps::rtps::ReaderHistory *history_;
        eprosima::fastrtps::rtps::HistoryAttributes hattr_;
        bool initialized_;
        std::list<type> total_msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::string magicword_;
        bool receiving_;
        eprosima::fastrtps::rtps::SequenceNumber_t last_seq_;
        size_t current_received_count_;
        size_t number_samples_expected_;
        std::string ip_;
        uint32_t port_;
        type_support type_;
};

#endif // _TEST_BLACKBOX_RTPSASSOCKETREADER_HPP_
