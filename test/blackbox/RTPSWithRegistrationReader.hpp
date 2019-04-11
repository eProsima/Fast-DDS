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
 * @file RTPSWithRegistrationReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_
#define _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/qos/ReaderQos.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <list>
#include <condition_variable>
#include <asio.hpp>
#include <gtest/gtest.h>

using eprosima::fastrtps::rtps::IPLocator;

template<class TypeSupport>
class RTPSWithRegistrationReader
{
    public:

        typedef TypeSupport type_support;
        typedef typename type_support::type type;

    private:

        class Listener: public eprosima::fastrtps::rtps::ReaderListener
        {
            public:
                Listener(RTPSWithRegistrationReader &reader) : reader_(reader) {};

                ~Listener(){};

                void onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* reader, const eprosima::fastrtps::rtps::CacheChange_t* const change)
                {
                    ASSERT_NE(reader, nullptr);
                    ASSERT_NE(change, nullptr);

                    reader_.receive_one(reader, change);
                }

                void onReaderMatched(eprosima::fastrtps::rtps::RTPSReader* /*reader*/, eprosima::fastrtps::rtps::MatchingInfo& info)
                {
                    if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                        reader_.matched();
                }

            private:

                Listener& operator=(const Listener&) = delete;

                RTPSWithRegistrationReader &reader_;
        } listener_;

    public:

        RTPSWithRegistrationReader(const std::string& topic_name) : listener_(*this),
        participant_(nullptr), reader_(nullptr), history_(nullptr), initialized_(false), receiving_(false), matched_(0)
        {
            topic_attr_.topicDataType = type_.getName();
            // Generate topic name
            std::ostringstream t;
            t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
            topic_attr_.topicName = t.str();

            // By default, heartbeat period delay is 100 milliseconds.
            reader_attr_.times.heartbeatResponseDelay.seconds = 0;
            reader_attr_.times.heartbeatResponseDelay.fraction = 4294967 * 100;
        }

        virtual ~RTPSWithRegistrationReader()
        {
            if(participant_ != nullptr)
                eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_);
            if(history_ != nullptr)
                delete(history_);
        }

        void init()
        {
            eprosima::fastrtps::rtps::RTPSParticipantAttributes pattr;
            pattr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
            pattr.builtin.use_WriterLivelinessProtocol = true;
            pattr.builtin.domainId = (uint32_t)GET_PID() % 230;
            participant_ = eprosima::fastrtps::rtps::RTPSDomain::createParticipant(pattr);
            ASSERT_NE(participant_, nullptr);

            //Create readerhistory
            hattr_.payloadMaxSize = type_.m_typeSize;
            history_ = new eprosima::fastrtps::rtps::ReaderHistory(hattr_);
            ASSERT_NE(history_, nullptr);

            //Create reader
            reader_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSReader(participant_, reader_attr_, history_, &listener_);
            ASSERT_NE(reader_, nullptr);

            ASSERT_EQ(participant_->registerReader(reader_, topic_attr_, reader_qos_), true);

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

            reader_ = nullptr;
            receiving_ = false;
            initialized_ = false;
            matched_ = 0;
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

        const std::list<type>& not_received_data() const
        {
            return total_msgs_;
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

        void block_until_seq_number_greater_or_equal(const eprosima::fastrtps::rtps::SequenceNumber_t& min_seq)
        {
            block([this, min_seq]() -> bool {
                return last_seq_ >= min_seq;
            });
        }


        void wait_discovery()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            if (matched_ == 0)
            {
                cvDiscovery_.wait(lock, [this]() -> bool {
                    return matched_ != 0;
                });
            }
        }

        void check_seq_number_greater_or_equal(const eprosima::fastrtps::rtps::SequenceNumber_t& min_seq)
        {
            ASSERT_GE(last_seq_, min_seq);
        }

        void matched()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);
            ++matched_;
            cvDiscovery_.notify_one();
        }

        unsigned int getReceivedCount() const
        {
            return current_received_count_;
        }

        /*** Function to change QoS ***/
        RTPSWithRegistrationReader& memoryMode(const eprosima::fastrtps::rtps::MemoryManagementPolicy_t memoryPolicy)
        {
            hattr_.memoryPolicy=memoryPolicy;
            return *this;
        }

        RTPSWithRegistrationReader& reliability(const eprosima::fastrtps::rtps::ReliabilityKind_t kind)
        {
            reader_attr_.endpoint.reliabilityKind = kind;

            if(kind == eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE)
                reader_qos_.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
            else
                reader_qos_.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;

            return *this;
        }

        RTPSWithRegistrationReader& durability(const eprosima::fastrtps::rtps::DurabilityKind_t kind)
        {
            reader_attr_.endpoint.durabilityKind = kind;
            reader_qos_.m_durability.durabilityKind(kind);

            return *this;
        }

        RTPSWithRegistrationReader& add_to_multicast_locator_list(const std::string& ip, uint32_t port)
        {
            eprosima::fastrtps::rtps::Locator_t loc;
            IPLocator::setIPv4(loc, ip);
            loc.port = static_cast<uint16_t>(port);
            reader_attr_.endpoint.multicastLocatorList.push_back(loc);

            return *this;
        }

        RTPSWithRegistrationReader& add_property(const std::string& prop, const std::string& value)
        {
            reader_attr_.endpoint.properties.properties().emplace_back(prop, value);

            return *this;
        }

        RTPSWithRegistrationReader& make_persistent(const std::string& filename, const eprosima::fastrtps::rtps::GuidPrefix_t& guidPrefix)
        {
            reader_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
            reader_attr_.endpoint.persistence_guid.entityId = 0x55555555;

            std::cout << "Initializing persistent READER " << reader_attr_.endpoint.persistence_guid << " with file " << filename << std::endl;

            return durability(eprosima::fastrtps::rtps::DurabilityKind_t::PERSISTENT)
                .add_property("dds.persistence.plugin", "builtin.SQLITE3")
                .add_property("dds.persistence.sqlite3.filename", filename);
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

                eprosima::fastrtps::rtps::ReaderHistory *history = reader->getHistory();
                ASSERT_NE(history, nullptr);

                history->remove_change((eprosima::fastrtps::rtps::CacheChange_t*)change);
            }
        }

        RTPSWithRegistrationReader& operator=(const RTPSWithRegistrationReader&) = delete;

        eprosima::fastrtps::rtps::RTPSParticipant *participant_;
        eprosima::fastrtps::rtps::RTPSReader* reader_;
        eprosima::fastrtps::rtps::ReaderAttributes reader_attr_;
        eprosima::fastrtps::TopicAttributes topic_attr_;
        eprosima::fastrtps::ReaderQos reader_qos_;
        eprosima::fastrtps::rtps::ReaderHistory* history_;
        eprosima::fastrtps::rtps::HistoryAttributes hattr_;
        bool initialized_;
        std::list<type> total_msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        bool receiving_;
        unsigned int matched_;
        eprosima::fastrtps::rtps::SequenceNumber_t last_seq_;
        size_t current_received_count_;
        size_t number_samples_expected_;
        type_support type_;
};

#endif // _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_
