/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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

template<class TypeSupport, eprosima::fastrtps::ReliabilityQosPolicyKind Mode>
class PubSubReader 
{
    class Listener: public eprosima::fastrtps::SubscriberListener
    {
        public:
            Listener(PubSubReader &reader) : reader_(reader) {};

            ~Listener(){};

            void onNewDataMessage(eprosima::fastrtps::Subscriber *sub)
            {
                ASSERT_NE(sub, nullptr);

                reader_.newNumber(reader_.receiver_(sub));
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

        typedef TypeSupport type_support;
        typedef typename type_support::type type;


        PubSubReader(std::function<uint16_t (eprosima::fastrtps::Subscriber*)> receiver) : listener_(*this), lastvalue_(std::numeric_limits<uint16_t>::max()),
        receiver_(receiver), participant_(nullptr), subscriber_(nullptr), initialized_(false), matched_(0)
        {
        }

        ~PubSubReader()
        {
            if(participant_ != nullptr)
                Domain::removeParticipant(participant_);
        }

        void init(uint16_t nmsgs)
        {
            eprosima::fastrtps::ParticipantAttributes pattr;
            pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
            participant_ = eprosima::fastrtps::Domain::createParticipant(pattr);
            ASSERT_NE(participant_, nullptr);

            // Register type
            ASSERT_EQ(eprosima::fastrtps::Domain::registerType(participant_, &type_), true);

            //Create subscribe r
            eprosima::fastrtps::SubscriberAttributes sattr;
            sattr.topic.topicKind = NO_KEY;
            sattr.topic.topicDataType = type_.getName();
            configSubscriber(sattr);
            subscriber_ = eprosima::fastrtps::Domain::createSubscriber(participant_, sattr, &listener_);
            ASSERT_NE(subscriber_, nullptr);

            // Initialize list of msgs
            for(uint16_t count = 1; count <= nmsgs; ++count)
            {
                msgs_.push_back(count);
            }

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

        void newNumber(uint16_t number)
        {
            ASSERT_NE(number, 0);
            std::unique_lock<std::mutex> lock(mutex_);
            std::list<uint16_t>::iterator it = std::find(msgs_.begin(), msgs_.end(), number);
            ASSERT_NE(it, msgs_.end());
            if(lastvalue_ == *it)
                cv_.notify_one();
            msgs_.erase(it);
        }

        std::list<uint16_t> getNonReceivedMessages()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return msgs_;
        }

        uint16_t lastvalue_;

        void block(uint16_t lastvalue, const std::chrono::seconds &seconds)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            lastvalue_ = lastvalue;
            if(!msgs_.empty() && lastvalue_ == *msgs_.rbegin())
                cv_.wait_for(lock, seconds);
        }

        void waitDiscovery()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            if(matched_ == 0)
                cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

            ASSERT_NE(matched_, 0u);
        }

        void waitRemoval()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            if(matched_ != 0)
                cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

            ASSERT_EQ(matched_, 0u);
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

        void configSubscriber(SubscriberAttributes &sattr)
        {
            std::string reliability_str;

            if(mode_ == eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            {
                sattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
                reliability_str = "AsReliable";
            }
            else
            {
                sattr.qos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
                reliability_str = "AsNonReliable";
            }

            std::ostringstream t;

            t << "PubSub" << reliability_str << type_.getName() << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();

            sattr.topic.topicName = t.str();
        }

    private:

        PubSubReader& operator=(const PubSubReader&)NON_COPYABLE_CXX11;

        const eprosima::fastrtps::ReliabilityQosPolicyKind mode_ = Mode;

        std::function<uint16_t (eprosima::fastrtps::Subscriber*)> receiver_;

        eprosima::fastrtps::Participant *participant_;
        eprosima::fastrtps::Subscriber *subscriber_;
        bool initialized_;
        std::list<uint16_t> msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
        type_support type_;
};

#endif // _TEST_BLACKBOX_PUBSUBREADER_HPP_

