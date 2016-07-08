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
 * @file PubSubReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBREADER_HPP_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <string>
#include <list>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

template<class TypeSupport>
class PubSubReader 
{
    public:

        typedef TypeSupport type_support;
        typedef typename type_support::type type;

    private:

    class Listener: public eprosima::fastrtps::SubscriberListener
    {
        public:
            Listener(PubSubReader &reader) : reader_(reader) {};

            ~Listener(){};

            void onNewDataMessage(eprosima::fastrtps::Subscriber *sub)
            {
                ASSERT_NE(sub, nullptr);

                bool ret = false;
                reader_.receive_one(sub, ret);
            }

            void onSubscriptionMatched(eprosima::fastrtps::Subscriber* /*sub*/, MatchingInfo& info)
            {
                if (info.status == MATCHED_MATCHING)
                    reader_.matched();
                else
                    reader_.unmatched();
            }

        private:

            Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

            PubSubReader &reader_;
    } listener_;

    friend class Listener;

    public:

        PubSubReader(const std::string& topic_name) : listener_(*this), participant_(nullptr), subscriber_(nullptr),
        topic_name_(topic_name), initialized_(false), matched_(0), receiving_(false), current_received_count_(0),
        number_samples_expected_(0)
        {
            subscriber_attr_.topic.topicDataType = type_.getName();
            // Generate topic name
            std::ostringstream t;
            t << topic_name_ << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
            subscriber_attr_.topic.topicName = t.str();
        }

        ~PubSubReader()
        {
            if(participant_ != nullptr)
                Domain::removeParticipant(participant_);
        }

        void init()
        {
            eprosima::fastrtps::ParticipantAttributes pattr;
            pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
            participant_ = eprosima::fastrtps::Domain::createParticipant(pattr);
            ASSERT_NE(participant_, nullptr);

            // Register type
            ASSERT_EQ(eprosima::fastrtps::Domain::registerType(participant_, &type_), true);

            //Create subscribe r
            subscriber_ = eprosima::fastrtps::Domain::createSubscriber(participant_, subscriber_attr_, &listener_);
            ASSERT_NE(subscriber_, nullptr);

            initialized_ = true;
        }

        bool isInitialized() const { return initialized_; }

        void destroy()
        {
            if(participant_ != nullptr)
            {
                Domain::removeParticipant(participant_);
                participant_ = nullptr;
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

            bool ret = false;
            do
            {
                receive_one(subscriber_, ret);
            }
            while(ret);
        }

        void stopReception()
        {
            mutex_.lock();
            receiving_ = false;
            mutex_.unlock();
        }

        std::list<type> block(const std::chrono::seconds &max_wait)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(current_received_count_ != number_samples_expected_)
                cv_.wait_for(lock, max_wait);

            return total_msgs_;
        }

        void waitDiscovery()
        {
            std::cout << "Reader waiting for discovery..." << std::endl;
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            if(matched_ == 0)
                cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

            ASSERT_NE(matched_, 0u);
            std::cout << "Reader discovery phase finished" << std::endl;
        }

        void waitRemoval()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            if(matched_ != 0)
                cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

            ASSERT_EQ(matched_, 0u);
        }

        unsigned int getReceivedCount() const
        {
            return current_received_count_;
        }

        /*** Function to change QoS ***/
        PubSubReader& memoryMode(const eprosima::fastrtps::rtps::MemoryManagementPolicy_t memoryMode)
	{
	    subscriber_attr_.historyMemoryPolicy = memoryMode;
	    return *this;
	}
	PubSubReader& reliability(const eprosima::fastrtps::ReliabilityQosPolicyKind kind)
        {
            subscriber_attr_.qos.m_reliability.kind = kind;
            return *this;
        }

        PubSubReader& history_kind(const eprosima::fastrtps::HistoryQosPolicyKind kind)
        {
            subscriber_attr_.topic.historyQos.kind = kind;
            return *this;
        }

        PubSubReader& history_depth(const int32_t depth)
        {
            subscriber_attr_.topic.historyQos.depth = depth;
            return *this;
        }

        PubSubReader& resource_limits_max_samples(const int32_t max)
        {
            subscriber_attr_.topic.resourceLimitsQos.max_samples = max;
            return *this;
        }
	
	PubSubReader& allocated_samples(const int32_t max)
	{
	    subscriber_attr_.topic.resourceLimitsQos.allocated_samples = max;
	    return *this;
	}

	PubSubReader& heartbeatPeriod(const int32_t secs, const int32_t frac)
	{
	    subscriber_attr_.times.heartbeatResponseDelay.seconds = secs;
	    subscriber_attr_.times.heartbeatResponseDelay.fraction = frac;
	    return *this;
	}
    private:

        void receive_one(eprosima::fastrtps::Subscriber* subscriber, bool& returnedValue)
        {
            returnedValue = false;
            std::unique_lock<std::mutex> lock(mutex_);

            if(receiving_)
            {
                type data;
                SampleInfo_t info;

                if(subscriber->takeNextData((void*)&data, &info))
                {
                    returnedValue = true;

                    // Check order of changes.
                    ASSERT_LT(last_seq, info.sample_identity.sequence_number());
                    last_seq = info.sample_identity.sequence_number();

                    if(info.sampleKind == ALIVE)
                    {
                        auto it = std::find(total_msgs_.begin(), total_msgs_.end(), data);
                        ASSERT_NE(it, total_msgs_.end()); 
                        total_msgs_.erase(it);
                        ++current_received_count_;

                        if(current_received_count_ == number_samples_expected_)
                            cv_.notify_one();
                    }
                }
            }
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

        PubSubReader& operator=(const PubSubReader&)NON_COPYABLE_CXX11;

        eprosima::fastrtps::Participant *participant_;
        eprosima::fastrtps::SubscriberAttributes subscriber_attr_;
        eprosima::fastrtps::Subscriber *subscriber_;
        std::string topic_name_;
        bool initialized_;
        std::list<type> total_msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
        bool receiving_;
        type_support type_;
        SequenceNumber_t last_seq;
        size_t current_received_count_;
        size_t number_samples_expected_;
};

#endif // _TEST_BLACKBOX_PUBSUBREADER_HPP_

