/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBWRITER_HPP_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <string>
#include <list>
#include <condition_variable>
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4005)
#endif  // _MSC_VER
#include <boost/asio.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

template<class TypeSupport>
class PubSubWriter 
{
    class Listener : public eprosima::fastrtps::PublisherListener
    {
        public:

            Listener(PubSubWriter &writer) : writer_(writer){};

            ~Listener(){};

            void onPublicationMatched(eprosima::fastrtps::Publisher* /*pub*/, MatchingInfo &info)
            {
                if (info.status == MATCHED_MATCHING)
                    writer_.matched();
                else
                    writer_.unmatched();
            }

        private:

            Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

            PubSubWriter &writer_;

    } listener_;

    public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    PubSubWriter(const std::string &topic_name) : listener_(*this), participant_(nullptr),
    publisher_(nullptr), topic_name_(topic_name), initialized_(false), matched_(0)
    {
            publisher_attr_.topic.topicDataType = type_.getName();
            // Generate topic name
            std::ostringstream t;
            t << topic_name_ << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
            publisher_attr_.topic.topicName = t.str();
    }
    
    ~PubSubWriter()
    {
        if(participant_ != nullptr)
            eprosima::fastrtps::Domain::removeParticipant(participant_);
    }

    void init()
    {
        //Create participant
        eprosima::fastrtps::ParticipantAttributes pattr;
        pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
        participant_ = eprosima::fastrtps::Domain::createParticipant(pattr);

        if(participant_ != nullptr)
        {
            // Register type
            eprosima::fastrtps::Domain::registerType(participant_, &type_);

            //Create publisher
            publisher_ = eprosima::fastrtps::Domain::createPublisher(participant_, publisher_attr_, &listener_);

            if(publisher_ != nullptr)
            {
                initialized_ = true;
                return;
            }

            eprosima::fastrtps::Domain::removeParticipant(participant_);
        }
    }

    bool isInitialized() const { return initialized_; }

    void destroy()
    {
        if(participant_ != nullptr)
        {
            eprosima::fastrtps::Domain::removeParticipant(participant_);
            participant_ = nullptr;
        }
    }

    void send(std::list<type>& msgs)
    {
        auto it = msgs.begin();
        
        while(it != msgs.end())
        {
            if(publisher_->write((void*)&(*it)))
            {
                it = msgs.erase(it);
            }
            else
                break;
        }
    }

    void waitDiscovery()
    {
        std::cout << "Writer waiting for discovery..." << std::endl;
        std::unique_lock<std::mutex> lock(mutex_);

        if(matched_ == 0)
            cv_.wait_for(lock, std::chrono::seconds(10));

        ASSERT_NE(matched_, 0u);
        std::cout << "Writer discovery phase finished" << std::endl;
    }

    void waitRemoval()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if(matched_ != 0)
            cv_.wait_for(lock, std::chrono::seconds(10));

        ASSERT_EQ(matched_, 0u);
    }

    bool waitForAllAcked(const std::chrono::seconds& max_wait)
    {
        return publisher_->wait_for_all_acked(Time_t((int32_t)max_wait.count(), 0));
    }

    /*** Function to change QoS ***/
    PubSubWriter& reliability(const eprosima::fastrtps::ReliabilityQosPolicyKind kind)
    {
        publisher_attr_.qos.m_reliability.kind = kind;
        return *this;
    }

    PubSubWriter& asynchronously(const eprosima::fastrtps::PublishModeQosPolicyKind kind)
    {
        publisher_attr_.qos.m_publishMode.kind = kind;
        return *this;
    }

    PubSubWriter& history_kind(const eprosima::fastrtps::HistoryQosPolicyKind kind)
    {
        publisher_attr_.topic.historyQos.kind = kind;
        return *this;
    }

    PubSubWriter& history_depth(const int32_t depth)
    {
        publisher_attr_.topic.historyQos.depth = depth;
        return *this;
    }

    PubSubWriter& durability_kind(const eprosima::fastrtps::DurabilityQosPolicyKind kind)
    {
        publisher_attr_.qos.m_durability.kind = kind;
        return *this;
    }

    PubSubWriter& resource_limits_max_samples(const int32_t max)
    {
        publisher_attr_.topic.resourceLimitsQos.max_samples = max;
        return *this;
    }

    PubSubWriter& heartbeat_period_seconds(int32_t sec)
    {
        publisher_attr_.times.heartbeatPeriod.seconds = sec;
        return *this;
    }

    PubSubWriter& heartbeat_period_fraction(uint32_t frac)
    {
        publisher_attr_.times.heartbeatPeriod.fraction = frac;
        return *this;
    }

    private:

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

    PubSubWriter& operator=(const PubSubWriter&)NON_COPYABLE_CXX11;

    eprosima::fastrtps::Participant *participant_;
    eprosima::fastrtps::PublisherAttributes publisher_attr_;
    eprosima::fastrtps::Publisher *publisher_;
    std::string topic_name_;
    bool initialized_;
    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_;
    type_support type_;
};

#endif // _TEST_BLACKBOX_PUBSUBWRITER_HPP_
